# EXE parity checks

Build the standalone EXE with `FEAT=orig_checksum python3 tools/build.py exe`,
so that it reports the 1.50a size and checksum the archive and replays expect. Its final build step
audits the 78 game tables recorded in `src/exe/data_tables.h`: the compiler
checks array/element sizes, and the image audit checks their complete contents
and 626 non-null pointer entries against original destinations or string data.
Run the audit separately with `python3 scripts/table_audit.py`.

Run the deterministic gameplay comparison with:

```sh
python3 scripts/parity.py run --frames 600
```

This builds a small Windows debugger and starts three independent game
processes: original A, original B, then the rebuilt EXE. Both original runs must
agree before the rebuilt comparison proceeds. Each process gets a fresh game
directory, the same attested archives, and the same initial config (or the game
defaults when the original directory has no config). No hook DLL is loaded.

The game enters its first built-in demo replay without keyboard input. At
`App::OnUpdate` entry the debugger stops all game threads and records each
consecutive replay frame starting at frame zero. It compares RNG seed and call
count, replay inputs, stage/difficulty, damage settings, and both players'
positions, states, health, charge, scores, lives, bullet counts and enemy counts.
Floats are compared by bits. The absolute application frame counter is retained
for diagnosis and excluded from equality because startup/loading times vary.

The debugger temporarily replaces one instruction byte in process memory with
a breakpoint, restores it, and single-steps the original instruction. It does
not modify either executable on disk or replace game functions. Debugging
changes execution timing; the repeated-original control checks determinism of
the recorded state under those conditions.

Artifacts stay in `build/parity/run-*/`: per-process traces, executable/probe
hashes, observed symbol addresses, logs, and a comparison result. The command
prints the first differing replay frame and fields, exits 1 for a state
difference, and exits 2 for an invalid/incomplete capture or failed control.
Compare existing traces with:

```sh
python3 scripts/parity.py compare ORIGINAL.jsonl REBUILT.jsonl --frames 600
python3 -m unittest discover -s tests
```

This is a bounded check of the first archive demo and the specified observables.
It does not establish rendering/audio parity, arbitrary replay compatibility,
network parity, every entity's internal state, or behavior after the captured
interval. A round reset or missing frame fails capture rather than silently
realigning the traces. Runtime captures are explicit checks; pre-commit keeps
the existing function-matching gate and does not launch gameplay.
