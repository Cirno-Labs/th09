"""Place compiled contributions in the 1.50a layout before the final link.

All emitted bytes and relocations come from the compiler and SDK/CRT
objects. The reference supplies addresses and fingerprints, never code.
Unplaced contributions remain in a separate section, so an imperfect
match cannot silently drop a function or a constant.
"""
from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
import struct
import sys
import csv
import json
from collections.abc import Sequence

sys.path.insert(0, str(Path(__file__).resolve().parent))
from coff_archive import members

import coff
import project
import compare_exe


@dataclass
class Unit:
    path: str
    obj: coff.Object


def align(value: int, boundary: int) -> int:
    return (value + boundary - 1) & -boundary


def exe_objects(objects: Sequence[str], features: Sequence[str]) -> list[str]:
    """Build a relocation-preserving object from this build's contributions."""
    reference = compare_exe.PE(project.ORIGINAL)
    text_base, text_size, _, _ = reference.sections['.text']
    text_end = text_base + text_size
    known = {symbol: address for address, _, _, symbols in compare_exe.read_symbols() for symbol in symbols}
    paths = list(objects)
    output_dir = Path(paths[0]).parent.with_name(Path(paths[0]).parent.name + '_layout')
    output_dir.mkdir(parents=True, exist_ok=True)
    units = [Unit(path, coff.Object(path)) for path in paths]
    needed = json.loads(Path('config/exe-libraries.json').read_text())
    extracted = output_dir / 'libraries'
    extracted.mkdir(exist_ok=True)
    seen_libraries: set[str] = set()
    for folder in project.config['build']['lib']:
        local = Path(project.ROOT) / '.wine/drive_c' / folder[3:].replace('\\', '/')
        if not local.exists():
            local = Path(folder)
        if not local.exists():
            continue
        for library in sorted(local.glob('*'), key=lambda item: item.name.lower()):
            stem = library.stem.lower()
            if stem not in needed or stem in seen_libraries or library.suffix.lower() != '.lib':
                continue
            seen_libraries.add(stem)
            for member, raw in members(library):
                if member.lower() not in needed[stem] or raw[:2] != b'\x4c\x01':
                    continue
                output = extracted / (stem + '_' + member)
                output.write_bytes(raw)
                obj = coff.Object(str(output))
                if any(section.name.startswith('.idata') for section in obj.sections):
                    continue
                units.append(Unit(str(output), obj))
    for unit in units:
        raw = Path(unit.path).read_bytes()
        pointer, count = struct.unpack_from('<II', raw, 8)
        strings = pointer + count * 18
        for section in unit.obj.sections:
            if section.name.startswith('/') and section.name[1:].isdigit():
                start = strings + int(section.name[1:])
                end = raw.index(b'\0', start)
                section.name = raw[start:end].decode('latin-1')
    missing = needed.keys() - seen_libraries
    if missing:
        raise FileNotFoundError('missing layout libraries: ' + ', '.join(sorted(missing)))
    print('source/library objects', len(paths), len(units) - len(paths))
    owners: dict[str, tuple[int, int]] = {}
    for u, unit in enumerate(units):
        for i, sym in enumerate(unit.obj.symbols):
            if sym and sym.storage == 2 and (sym.section > 0 or sym.section == 0 and sym.value):
                previous = owners.get(sym.name)
                old = units[previous[0]].obj.symbols[previous[1]] if previous else None
                # An initialized definition overrides a C tentative (common)
                # definition, regardless of the archive's member order.
                if old is None or old.section == 0 and (sym.section > 0 or sym.value > old.value):
                    owners[sym.name] = (u, i)

    with Path('config/symbols.csv').open() as stream:
        for row in csv.DictReader(stream):
            address = int(row['address'], 16)
            if not text_base <= address < text_end:
                continue
            label = row['name']
            for candidate in [label, '_' + label, '_' + label.lstrip('_'), '__' + label.lstrip('_')]:
                if candidate in owners:
                    known.setdefault(candidate, address)

    weak: dict[str, str] = {}
    for unit in units:
        raw = Path(unit.path).read_bytes()
        pointer = struct.unpack_from('<I', raw, 8)[0]
        for i, sym in enumerate(unit.obj.symbols):
            if sym and sym.storage == 105:
                index = struct.unpack_from('<I', raw, pointer + (i + 1) * 18)[0]
                weak[sym.name] = unit.obj.symbols[index].name

    def name(u: int, i: int) -> str:
        sym = units[u].obj.symbols[i]
        if sym.name in weak:
            return weak[sym.name]
        if sym.name.startswith(('__real@', '??_C@')):
            owner_u, owner_i = owners.get(sym.name, (u, i))
            return f'__layout_const_{owner_u}_{owner_i}'
        if sym.storage == 2 or sym.section == 0:
            return sym.name
        return f'__layout_{u}_{i}_{sym.name}'

    chosen: set[tuple[int, int]] = set()
    for u, unit in enumerate(units):
        section_defs: dict[int, list[tuple[int, coff.Symbol]]] = defaultdict(list)
        for i, sym in enumerate(unit.obj.symbols):
            if sym and sym.storage == 2 and sym.section > 0:
                section_defs[sym.section - 1].append((i, sym))
        for s, section in enumerate(unit.obj.sections):
            if section.name.startswith('.debug'):
                continue
            definitions = section_defs[s]
            if section.flags & 0x1000 and definitions and not any(owners[sym.name] == (u, i) for i, sym in definitions):
                continue
            chosen.add((u, s))

    placed: dict[tuple[int, int], int] = {}
    candidates: dict[int, list[tuple[int, int, int]]] = defaultdict(list)
    for u, s in chosen:
        section = units[u].obj.sections[s]
        if not section.flags & coff.CODE:
            continue
        for i, sym in enumerate(units[u].obj.symbols):
            if sym and sym.section == s + 1 and sym.name in known:
                if sym.storage == 2 and owners.get(sym.name) != (u, i):
                    continue
                candidates[known[sym.name] - sym.value].append((u, s, i))
    addresses = sorted(a for a in candidates if text_base <= a < text_end)
    slots = {a: b - a for a, b in zip(addresses, addresses[1:] + [text_end])}
    for address in addresses:
        # Match the DLL's selection of duplicate inline definitions.
        u, s, _ = min(candidates[address])
        if (u, s) not in placed:
            placed[u, s] = address - text_base

    image = coff.Image(project.ORIGINAL)
    original = image.read(text_base, text_size)
    inferred = 0
    for u, section_index in sorted(chosen):
        section = units[u].obj.sections[section_index]
        if not section.flags & coff.CODE or (u, section_index) in placed or len(section.data) < 8:
            continue
        mask = bytearray(len(section.data))
        for offset, _, _ in section.relocs:
            mask[offset:offset + 4] = b'\1' * 4
        spans: list[tuple[int, int]] = []
        start = 0
        while start < len(mask):
            if mask[start]:
                start += 1
                continue
            end = start + 1
            while end < len(mask) and not mask[end]:
                end += 1
            spans.append((start, end))
            start = end
        if not spans:
            continue
        first, last = max(spans, key=lambda span: span[1] - span[0])
        if last - first < 6:
            continue
        needle = section.data[first:last]
        hits: list[int] = []
        cursor = 0
        while True:
            found = original.find(needle, cursor)
            if found < 0:
                break
            offset = found - first
            if offset >= 0 and offset + len(mask) <= len(original) and all(m or a == b for a, b, m in zip(section.data, original[offset:offset + len(mask)], mask)):
                hits.append(offset)
            cursor = found + 1
        if len(hits) == 1:
            start = hits[0]
            end = start + len(section.data)
            if any(start < offset + len(units[key[0]].obj.sections[key[1]].data) and offset < end
                   for key, offset in placed.items()):
                continue
            placed[u, section_index] = hits[0]
            inferred += 1
    print('content-placed sections', inferred)
    all_starts = sorted(set(placed.values()))
    slots = {start + text_base: end - start for start, end in zip(all_starts, all_starts[1:] + [text_size])}

    # Anything reachable from data or the reconstructed functions must survive.
    roots = {key for key in chosen if not units[key[0]].obj.sections[key[1]].flags & coff.CODE and (key[0] < len(paths) and units[key[0]].obj.sections[key[1]].flags & 0x80000000 or units[key[0]].obj.sections[key[1]].name in ('.drectve', '.sxdata') or units[key[0]].obj.sections[key[1]].name.startswith('.CRT'))}
    for u, s in list(roots):
        section = units[u].obj.sections[s]
        if section.name == '.sxdata':
            for (index,) in struct.iter_unpack('<I', section.data):
                sym = units[u].obj.symbols[index]
                if sym.section > 0:
                    roots.add((u, sym.section - 1))
    live = roots | set(placed)
    queue = list(live)
    while queue:
        u, s = queue.pop()
        for _, index, _ in units[u].obj.sections[s].relocs:
            sym = units[u].obj.symbols[index]
            target_u = u
            if sym.storage == 2 or sym.section == 0:
                owner = owners.get(weak.get(sym.name, sym.name))
                if owner is None:
                    continue
                target_u, target_index = owner
                sym = units[target_u].obj.symbols[target_index]
            if sym.section > 0:
                key = (target_u, sym.section - 1)
                if key in chosen and key not in live:
                    live.add(key)
                    queue.append(key)

    text = bytearray(b'\xcc' * text_size)
    extra = bytearray()
    positions: dict[tuple[int, int], tuple[int, int]] = {}
    trampolines: list[tuple[int, int, int]] = []
    for u, s in sorted(live):
        section = units[u].obj.sections[s]
        if not section.flags & coff.CODE:
            continue
        offset = placed.get((u, s))
        if offset is not None and len(section.data) <= slots[offset + text_base]:
            text[offset:offset + len(section.data)] = section.data
            positions[u, s] = (1, offset)
        else:
            boundary = 1 << max(0, ((section.flags >> 20) & 15) - 1)
            start = align(len(extra), boundary)
            extra.extend(b'\xcc' * (start - len(extra)))
            extra.extend(section.data)
            positions[u, s] = (2, start)
            if offset is not None:
                text[offset:offset + 5] = b'\xe9\0\0\0\0'
                trampolines.append((offset, u, s))
    sections: list[tuple[str, int, bytes, list[tuple[int, str, int]]]] = [
        ('.text', coff.TEXT, text, []), ('.extra', 0xe0300040, extra, [])]
    for u, s in sorted(live):
        section = units[u].obj.sections[s]
        if section.flags & coff.CODE:
            continue
        positions[u, s] = (len(sections) + 1, 0)
        sections.append((section.name, section.flags & ~0x1000, section.data, []))


    definitions: dict[str, tuple[int, int]] = {}
    absolutes: dict[str, int] = {}
    for u, unit in enumerate(units):
        for i, sym in enumerate(unit.obj.symbols):
            if sym is None:
                continue
            key = (u, sym.section - 1)
            if sym.section > 0 and key in positions:
                if sym.storage == 2 and owners.get(sym.name) != (u, i):
                    continue
                sec, offset = positions[key]
                definitions[name(u, i)] = (sec, offset + sym.value)
            elif sym.section == -1:
                absolutes[name(u, i)] = sym.value

    common = bytearray()
    for symbol, (u, i) in owners.items():
        sym = units[u].obj.symbols[i]
        if sym.section == 0 and sym.value:
            start = align(len(common), min(16, 1 << (sym.value.bit_length() - 1)))
            common.extend(bytes(start + sym.value - len(common)))
            definitions[name(u, i)] = (len(sections) + 1, start)
    sections.append(('.bss', 0xc0500080, common, []))

    # Oversized functions keep a stable entry address; their compiled bodies
    # and local jump tables remain contiguous in .extra.
    for offset, u, s in trampolines:
        body = f'__layout_body_{u}_{s}'
        definitions[body] = positions[u, s]
        sections[0][3].append((offset + 1, body, coff.REL32))
        for i, sym in enumerate(units[u].obj.symbols):
            if sym and sym.section == s + 1 and sym.name in known:
                definitions[name(u, i)] = (1, offset)

    referenced: set[str] = set()
    for u, s in sorted(live):
        sec, offset = positions[u, s]
        for off, i, typ in units[u].obj.sections[s].relocs:
            symbol = name(u, i)
            referenced.add(symbol)
            sections[sec - 1][3].append((offset + off, symbol, typ))
    referenced.update(r[1] for r in sections[0][3])
    undefined = sorted(referenced - definitions.keys() - absolutes.keys())
    for section in sections:
        relocs = section[3]
        unique: dict[tuple[int, int], tuple[str, object]] = {}
        for offset, symbol, kind in relocs:
            target = definitions.get(symbol, absolutes.get(symbol, symbol))
            previous = unique.get((offset, kind))
            if previous is not None and previous[1] != target:
                raise ValueError(f'conflicting relocations in {section[0]} at {offset:#x}: {previous} vs {(symbol, target)}')
            unique[offset, kind] = (symbol, target)
        relocs[:] = [(offset, value[0], kind) for (offset, kind), value in sorted(unique.items())]
    indices = {symbol: i for i, symbol in enumerate([*definitions, *undefined, *absolutes])}
    for (u, s), (sec, offset) in positions.items():
        section = units[u].obj.sections[s]
        if section.name == '.sxdata':
            data = b''.join(struct.pack('<I', indices[name(u, index)]) for (index,) in struct.iter_unpack('<I', section.data))
            old = sections[sec - 1]
            sections[sec - 1] = (old[0], old[1], data, old[3])
    output = str(output_dir / 'th09.obj')
    coff.write_object(output, sections, definitions, undefined, absolutes)
    raw = bytearray(Path(output).read_bytes())
    symbol_offset = struct.unpack_from('<I', raw, 8)[0]
    for u, s in positions:
        section = units[u].obj.sections[s]
        if section.name == '.sxdata':
            for (old_index,) in struct.iter_unpack('<I', section.data):
                index = indices[name(u, old_index)]
                struct.pack_into('<H', raw, symbol_offset + index * 18 + 14, 0x20)
    Path(output).write_bytes(raw)
    print(f'{len(placed)} functions placed; {len(trampolines)} oversized; {len(extra)} extra code bytes; {len(undefined)} imports')
    return [output]

