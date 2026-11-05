"""Print the order one source file's object emits its functions in.

    python3 scripts/emit_order.py src/stage.cpp [--ruler N] [--all]

Compiles the file as the exe does (its flags, the precompiled global.h)
into build/emit/ and lists the object's code COMDATs in section order,
the order LINK places them in, each with its size and the original's
address from config/symbols.csv. A `<` marks a function the original
has before the one with an address listed above it: that is where the
object's order and the original's part (or a copy LINK folds into
another object's). The file can be any copy of a source (a probe
outside src/ is compiled with src/'s headers).

--ruler N prepends N tiny inline functions, Ruler0 to Ruler<N-1>, to
the precompiled header and calls them all from a function appended to
the file. The compiler works through the header's inlines alongside
the file (docs/natural-link.md), so the rulers that follow each of the
file's functions show how far through the header it is by then: the
first ruler after a function is how many header inlines its code
balanced, counted from the header's start. Runs of rulers print as one
line. --all lists the other COMDATs (data, unknown names) too.
"""
from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import build
import coff

OUT = 'build/emit'


def precompile(flags: list[str], ruler: int) -> list[str]:
    """Compiles the precompiled header, with `ruler` rulers before it,
    into OUT unless it is up to date; returns the flags that use it."""
    header = build.PCH
    if ruler:
        header = f'ruler{ruler}.h'
        text = ''.join(f'inline int Ruler{i}(int x) {{ return x + {i}; }}\n' for i in range(ruler))
        text += f'#include "{build.PCH}"\n'
        path = f'{OUT}/{header}'
        if not os.path.exists(path) or open(path).read() != text:
            open(path, 'w').write(text)
    stub, obj, pch = (f'{OUT}/pch{ruler}{ext}' for ext in ('.cpp', '.obj', '.pch'))
    open(stub, 'w').write(f'#include "{header}"\n')
    newest = max(os.path.getmtime(p) for p in Path('src').rglob('*.h'))
    if not os.path.exists(pch) or os.path.getmtime(pch) < newest:
        code, out = build.run_tool('cl', '/c', *flags, '/Yc' + header, '/Fp' + build.win(pch),
                                   '/I' + build.win(OUT), '/I' + build.win(build.SJIS),
                                   '/Fo' + build.win(obj), build.win(stub))
        if code:
            sys.exit(out)
    return ['/FI' + header, '/Yu' + header, '/Fp' + build.win(pch), '/I' + build.win(OUT)]


def compile_source(src: str, ruler: int) -> str:
    os.makedirs(OUT, exist_ok=True)
    flags = build.file_flags(src, build.EXE_CFLAGS) + build.EXE_DEFINES
    flags += precompile(build.EXE_CFLAGS + build.EXE_DEFINES, ruler)
    text = open(src, encoding='utf-8').read()
    if ruler:
        text += '\nint (*g_Rulers[])(int) = {\n' + ''.join(f'    Ruler{i},\n' for i in range(ruler)) + '};\n'
    copy = f'{OUT}/{Path(src).stem}.cpp'
    open(copy, 'wb').write(build.to_sjis(text))
    obj = copy[:-4] + '.obj'
    code, out = build.run_tool('cl', '/c', *flags, '/I' + build.win(build.SJIS),
                               '/I' + build.win(os.path.dirname(os.path.abspath(src))),
                               '/Fo' + build.win(obj), build.win(copy))
    if code:
        sys.exit(out)
    return obj


def comdats(path: str) -> list[tuple[str, int, bool]]:
    """(symbol, size, is code) for each section with a symbol at 0, in
    section order."""
    obj = coff.Object(path)
    first = {}
    for s in obj.symbols:
        if s and s.section > 0 and s.value == 0 and s.storage in (coff.CLASS_EXTERNAL, coff.CLASS_STATIC) \
                and s.name[:1] not in '.$':
            first.setdefault(s.section, s.name)
    return [(first[i + 1], len(sec.data), bool(sec.flags & coff.CODE))
            for i, sec in enumerate(obj.sections) if i + 1 in first]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('source')
    parser.add_argument('--ruler', type=int, default=0)
    parser.add_argument('--all', action='store_true')
    args = parser.parse_args()
    src = os.path.relpath(os.path.abspath(args.source), ROOT)
    os.chdir(ROOT)
    build.compile_all(build.EXE, exe_flags=True)  # build/src's Shift-JIS headers
    addresses = build.read_symbols()
    names = {}
    for line in open('config/symbols.csv', encoding='utf-8').readlines()[1:]:
        parts = line.rstrip('\n').split(',', 3)
        for s in parts[3].split():
            names[s] = parts[2]
    last = 0
    run = []

    def flush():
        if run:
            print(f'      ruler {run[0]}' + (f'-{run[-1]}' if len(run) > 1 else ''))
            run.clear()
    for symbol, size, code in comdats(compile_source(src, args.ruler)):
        ruler = re.match(r'\?Ruler(\d+)@@', symbol)
        if ruler:
            run.append(int(ruler.group(1)))
            continue
        address = addresses.get(symbol)
        if not code and not args.all:
            continue
        flush()
        mark = ' '
        if address is not None:
            mark = '<' if address < last else ' '
            last = address
        where = f'{address:#x}' if address is not None else '-'
        print(f'{mark} {size:5x} {where:>9} {names.get(symbol, symbol)}')
    flush()


if __name__ == '__main__':
    main()
