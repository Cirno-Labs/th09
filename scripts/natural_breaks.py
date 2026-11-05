"""List the natural link's game functions in the original's order.

Each line gives the original address, the natural link's, the shift
between them, the name and the object LINK took it from. An `X` marks a
broken pair: the function does not follow its predecessor in the
original with the original's spacing (what `just natural` counts as
adjacent pairs). Run `just natural` first; this reads its map.

    python3 scripts/natural_breaks.py [OBJECT] [--summary]

OBJECT limits the list to objects whose name contains it; --summary
counts the broken pairs of each object instead.
"""
from __future__ import annotations

import collections
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import compare_exe
import project

MAP = 'build/natural/th09.map'


def rows() -> list[tuple[int, int, str, str, bool]]:
    """(original address, natural address, name, object, whether it
    follows its predecessor) of every game function the link placed."""
    linked = compare_exe.read_map(MAP)
    out = []
    previous = None
    for address, size, name, symbols in sorted(compare_exe.read_symbols()):
        if not size or not 0x401000 <= address < project.GAME_END:
            continue
        symbol = next((s for s in symbols if s in linked), None)
        if symbol is None:
            continue
        at, obj = linked[symbol]
        follows = previous is not None and at - previous[1] == address - previous[0]
        out.append((address, at, name, obj, follows))
        previous = (address, at)
    return out


def main() -> None:
    os.chdir(ROOT)
    args = [a for a in sys.argv[1:] if a != '--summary']
    only = args[0] if args else None
    listed = [r for r in rows() if not only or only in r[3]]
    if '--summary' in sys.argv:
        broken = collections.Counter(obj for *_, obj, follows in listed if not follows)
        for obj, count in broken.most_common():
            print(f'{count:4} {obj}')
        print(f'{sum(broken.values()):4} broken pairs')
        return
    for address, at, name, obj, follows in listed:
        print(f"{'  ' if follows else 'X '}{address:#x} {at:#x} {at - address:+#x} {name} [{obj}]")


if __name__ == '__main__':
    main()
