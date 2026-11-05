"""Link the exe's own objects in natural order, as the original was.

The standalone exe (`just exe`) places every contribution itself. This
links the compiled objects directly instead, with the original's
libraries and no /ORDER: static functions keep register conventions that
public names would lose, and /ORDER cannot name them, yet the original
has them among their objects' other functions. Natural order is object
order, then each object's definition order, header inlines included.

Run `just exe` first; this reuses its objects. The report counts the game
functions at their original address, gives the end of the placed
prefix from 0x401000 (rounds push it forward), and lists the first
misplaced functions.
"""
from __future__ import annotations

import glob
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import build
import compare_exe
import project

OUT = 'build/natural'


def link() -> Path:
    objs = sorted(p for p in glob.glob('build/obj_exe/*.obj') if not p.endswith('.rsrc.obj'))
    ordered = build.link_order(objs, build.read_symbols(), build.EXE_CONFIG['link_order'])
    os.makedirs(OUT, exist_ok=True)
    exe = f'{OUT}/{project.NAME}.exe'
    flags = [*build.EXE_CONFIG['ldflags'], '/OPT:REF', '/OPT:ICF']
    code, out = build.run_tool('link', *flags, '/MAP:' + build.win(exe[:-4] + '.map'),
                               '/OUT:' + build.win(exe), *map(build.win, ordered),
                               build.win('build/obj_exe/th09.res'), *build.EXE_CONFIG['libs'])
    if code:
        sys.exit(out)
    return Path(exe)


def report(exe: Path, shown: int) -> None:
    linked = compare_exe.read_map(str(exe.with_suffix('.map')))
    ours, orig = compare_exe.PE(str(exe)), compare_exe.PE(project.ORIGINAL)
    total = placed = same = 0
    prefix = None
    moved = []
    for address, size, name, symbols in sorted(compare_exe.read_symbols()):
        if not size or not 0x401000 <= address < project.GAME_END:
            continue
        symbol = next((s for s in symbols if s in linked), None)
        if symbol is None:
            continue
        total += 1
        at, obj = linked[symbol]
        if at != address:
            prefix = address if prefix is None else prefix
            moved.append(f'  {address:#x} at {at:#x} ({at - address:+#x}) {name} [{obj}]')
            continue
        placed += 1
        same += ours.read(at, size) == orig.read(address, size)
    print(f'game functions: {placed} of {total} at their original address, {same} identical raw')
    print(f'placed prefix: 0x401000-{prefix or project.GAME_END:#x}')
    print('\n'.join(moved[:shown]))
    data = open(exe, 'rb').read()
    original = open(project.ORIGINAL, 'rb').read()
    differ = sum(a != b for a, b in zip(data, original)) + abs(len(data) - len(original))
    print(f'whole file: {differ} bytes differ')


def main() -> None:
    os.chdir(ROOT)
    shown = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    report(link(), shown)


if __name__ == '__main__':
    main()
