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
- With a precompiled header, its inline functions are not compiled
  where the header defines them. The compiler works through them in
  the header's order alongside the file: after each function the file
  defines, it compiles the header's next inline functions, as many as
  balance that function's size (its code, about one per 20 bytes of
  machine code, not its comments or declarations). Every inline
  function of the header counts, used or not, and windows.h, math.h
  and d3dx8math.inl hold hundreds before the game's: math.h's cosf is
  about 80 small functions in, d3dx8math.inl's D3DXVec3Length about
  480. A file's functions that call a header inline not compiled yet
  wait for the end of the file. A small file reaches its end first, so
  the header's inline functions follow its own there, as the rule
  above had it.
- An implicit constructor or the `vector constructor iterator` is
  generated at its first use, which can be a header inline the compiler
  works through: Sprite's constructor generates SpriteBase's and the
  iterator.

Both compilers, 13.10.3077 (C:\msvc710) and 13.10.6030, order the same
way, deterministically; /Zi, /Gm and the header's own optimization
change nothing.

With the precompiled global.h (the original's ./system/global.h), the
stage object compiles HermiteInterpolate, then cosf, sinf and sqrtf,
then ModulateChannel, as at 0x401000, only because RunScript and the
other functions that wait are defined in between: the leaf functions
alone are far too small to move through math.h and d3dx8math.inl.

So a small accessor between two of an object's functions in the
original is a header inline (with `/Ob0` it stays a call, and the first
object in link order that uses it keeps it), and an object's functions
follow its definition order once each function's callees and the
header's progress are accounted for.

## Identical code folding

`/OPT:ICF` does not always keep the first copy in link order. Of the
two identical static ModulateChannel functions, the stage's and the
sprite engine's, the natural link keeps the sprite engine's, and keeps
the stage's when the sprite engine's object links first or last
instead. Making the sprite engine's accessors inline was enough to flip
it. The map lists every folded name under the kept copy's address but
not always under its object.

Making a function inline-visible can change its callers' code (a caller
knows what an inline function does not modify, as with
`SpriteEngine::GetAnm`), so every move needs `just compare`.
