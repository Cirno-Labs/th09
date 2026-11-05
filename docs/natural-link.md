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
  in definition order, even when what they wait for is compiled long
  before (a probe: a function waiting for a header's 20th inline
  function still comes after the file's last one). A function still
  waiting for another waiting function when its turn comes is passed
  over and compiled after the rest.
- A call the compiler drops still makes its caller wait: a call to an
  empty inline function (or one whose result is unused and that does
  nothing) leaves no code once the callee is compiled, so the caller
  waits for it, and the callee's one-byte COMDAT is emitted all the
  same (LINK folds it into another `ret`). That is how a function with
  no inline callee in its code can wait, as Stage::RemoveChain does
  (it calls the empty Stage::OnRemoved). `__forceinline` does not
  inline under `/Ob0`, and inline functions the file defines but
  never uses do not move the header along.
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
- A header outside the precompiled one, which the source includes
  itself, is different: its inline functions are compiled where the
  source includes it, before the file's own functions (the ones the
  file uses are emitted). So an object of the original that starts
  with header inline functions includes that header itself: the GDI
  surface object starts with GdiSurface's accessors, the enemy object
  with EclManager::GetTimelineCount, pl00's with SpawnPlayerEnemy and
  ExAttack::GetSide, the bullet object with
  BulletManager::ClearDrawBuckets. gdisurface.h, enemy.h, bullet.h and
  midi.h are not in global.h for that, and pl.h holds what the
  character objects share. Taking out player.h, gui.h, ascii.h,
  archive.h or stage.h instead loses neighbours.

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

`/OPT:ICF` does not keep the first copy in link order. LINK 7.1's code
(link.exe, decompiled; the `/VERBOSE` "Selected symbol" report prints
the result) works like this:

- Pass 1 adds every section that can fold to a group named by its
  section name (`.text`, `.rdata`, `.text$x`...), in the order it
  reads them: the objects in link order, then the library members as
  they are loaded. A section can fold unless it is writable,
  discardable or empty, or is a `_$E` dynamic initializer; of
  same-named COMDATs only the copy kept is added.
- Each group is sorted with msvcr71's `qsort`, which is not stable, by
  a key (zero in every probe) and then by size. Within each run of
  equal key and size, every section is compared with the ones after
  it and an identical one is folded into it: the copy kept is the
  first of its kind in the sorted group. Then the group is sorted and
  folded again while anything folds (twice more at most), so that
  sections whose relocations now reach folded targets fold too.

So which copy of two identical functions survives depends on every
section of the group and its size, the libraries' included: in a
probe with five identical functions, adding two unused functions to
another object moved the kept copy. The probes (the order of
`/VERBOSE`'s report, for 11 link orders) follow this exactly, but a
model of the whole link does not yet predict its 47 folds: the
library sections' order in the group is not known well enough. Of the
two identical static ModulateChannel functions, the stage's and the
sprite engine's, the natural link keeps the sprite engine's; the
original kept the stage's, which can follow from the objects matching
the original's (the same sections and sizes), not from their order.
The map lists every folded name under the kept copy's address but not
always under its object. The kept copy moves with unrelated changes:
with Tan made an (unused, so never emitted) inline function, the
natural link keeps the stage's ModulateChannel and the placed prefix
reaches 0x401470; with Tan out of line again it keeps the sprite
engine's. Timer::operator== and the small accessors that fold with it
(GdiSurface::GetHeight, Timer::AsFrames) move the same way.

Fmod and Tan stay out of line in unsorted.cpp for another reason: they
stand for math.h's fmodf and tanf (0x405710, 0x42e900), which the
code calls directly, so as inline functions nothing would emit them
and `just compare` would lose them.

Making a function inline-visible can change its callers' code (a caller
knows what an inline function does not modify, as with
`SpriteEngine::GetAnm`), so every move needs `just compare`.

## Measuring an object's order

`scripts/emit_order.py SOURCE` compiles one source (or a probe copy of
one) as the exe does and lists its code COMDATs in the order LINK
places them, with the original's address of each and a `<` where the
order goes back. `--ruler N` adds N tiny inline functions to the
precompiled header, at its start or (`--at timer.h`) just before one of
its headers, and references them from the end of the file, so that
they are all emitted: the rulers that follow each of the file's
functions show how far through the header the compiler is. For the
stage, HermiteInterpolate is followed by about 150 rulers, as much as
it and the three functions after it that wait balance (about 3000
bytes): the code of a waiting function moves the header along when
the next function is compiled.

`scripts/natural_breaks.py [OBJECT] [--summary]` lists the natural
link's functions in the original's order with an `X` at every broken
pair (what `just natural` counts), or with `--summary` the broken pairs
of each object.

## Open in the stage object

- The original's `vector constructor iterator` comes before
  Timer::Initialize, the first of timer.h's functions the stage uses;
  ours is generated with SpriteBase's constructor, after timer.h's
  functions, before AnmLoaded::StartScriptAtOrigin.
- Timer::Timer waits for Timer::Initialize and SpriteBase's
  constructor for it; the original has them after DrawLayer, so the
  compiler reached timer.h before it reached OnDraw. Ours reaches it at
  the end of the file only: even with DrawLayer defined between Reset
  and StepObjects, the header does not get there before OnDraw.

## Open elsewhere

- Most of the breaks left in the large objects (player, GUI, app,
  main menu, sound, MIDI, sprite engine) are functions that wait for
  a header inline function in ours but not in the original: there the
  header's inline functions are compiled well before ours reach them.
  In the ending object the original has SpriteEngine's state clearers,
  then PlayRecord's accessors, then GameManager::IsMode0, as if
  PlayRecord were declared after sprite.h; moving it to gamemanager.h
  gives that order but moves the header along differently for the
  other objects (one neighbour fewer overall). The order and content
  of global.h's queue is the lever, and it needs tuning against every
  object at once.
- The archive object compiles archive.h's File::ReadDword,
  DatArchiveCursor's and DatArchiveEntry's inline functions after
  DuplicateString, its third function: too early for the header's
  progress, too late for a header outside it.
- Bullet::ClearCommands is at the top of the ECL object now that
  bullet.h is out of global.h; the original has it after RunEnemy.
  BulletManager::ClearDrawBuckets, at the top of the bullet object,
  wants bullet.h out, so the two may have been in different headers.
- The spell card object starts with Player::SetUnknown0, as from a
  header outside global.h, but taking player.h out loses neighbours
  elsewhere; Player::StartClearCircle1 to 3, which follow, call
  Timer::operator= and so wait in ours.
- App starts with Tan, tan and D3DXVec3Cross before
  Network::GetOtherUnknown1c, which nothing explains yet.
