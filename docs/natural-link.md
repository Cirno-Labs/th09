# Natural-order link

`just exe` builds the standalone exe through `scripts/exe_layout.py`,
which places every compiled contribution at its original address itself
and links one combined object. That gets everything but the unmatched
functions and the Rich header identical, but the Rich header counts the
link's objects, so it can never match that way.

`just natural` links the exe's own objects directly instead, with the
original's libraries and `/OPT:REF /OPT:ICF` but no `/ORDER`, into
`build/natural/th09.exe`. It reports how many game functions sit at
their original address and how far from 0x401000 every function does
(the placed prefix). The goal is the original's layout from the objects
alone, as th08's exe has, after which the layout hook can go.

## Why not `/ORDER`

th08 reaches its layout with `/ORDER`. th09 cannot: its 79 `static`
functions get custom register conventions under `/O2` (making them
public changes their code), `/ORDER` cannot name them (LNK4037), and the
original has them among their objects' other functions. So the original
was linked in natural order, and its layout follows from the objects,
their order, and the order each object emits its functions.

## How cl 7.1 orders an object's functions

`/O2` implies `/Gy`: every function is its own COMDAT, emitted only if
used, and LINK keeps a COMDAT's first copy in link order. Probes with
the project's flags (`/O2 /Ob0 /Oy- /Gr /GX`) show that the code
generator works bottom-up:

- Functions are compiled in definition order, header inlines included,
  as the compiler reaches them.
- A function whose callees defined in the same file are not compiled
  yet waits. The waiting functions are compiled at the end of the file,
  in definition order.
- With a precompiled header, its inline functions are compiled at the
  end of the file too, before the waiting ones, in the header's order.

For example, without a PCH, `C` calls `static H` defined later, `M` is a
leaf, `N` calls `Q` and `inline R` defined later, `P` calls `C`:

```
defined: C M N H P R Q      emitted: M H R Q C N P
```

So a small accessor between two of an object's functions in the
original is a header inline (with `/Ob0` it stays a call, and the first
object in link order that uses it keeps it), and an object's functions
follow its definition order once each function's callees are accounted
for.

Making a function inline-visible can change its callers' code (a caller
knows what an inline function does not modify, as with
`SpriteEngine::GetAnm`), so every move needs `just compare`.
