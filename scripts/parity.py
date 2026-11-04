"""Compare deterministic archive-demo gameplay in original and rebuilt TH09.

    python3 scripts/parity.py run --frames 600
    python3 scripts/parity.py compare ORIGINAL.jsonl REBUILT.jsonl --frames 600

`run` first checks original versus original, then original versus rebuilt.
It starts fresh game directories, supplies no keyboard input, and observes the
first built-in demo replay at App::OnUpdate entry. A Windows debugger captures
every gameplay frame while all game threads are stopped. No game code or data
is replaced on disk; only a temporary process-memory breakpoint is used.
Float values are compared as their exact IEEE-754 bits. Wall-clock startup
frame numbers are diagnostic only. This proves only the recorded observables
for the selected frame interval, not complete gameplay or rendering parity.
Private archives, executables, traces and probe binaries remain under build/.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import build
import compare_exe
import play
import project
from table_audit import TARGET_SHA256

FIELDS = {'frame', 'app_frame', 'rng_seed', 'rng_generated', 'stage', 'difficulty',
          'damage', 'cpu_damage', 'input0', 'input1', 'sides'}
SIDE_FIELDS = {'character', 'state', 'health', 'x_bits', 'y_bits', 'charge_bits',
               'score', 'lives_bits', 'bullets', 'enemies'}


@dataclass(frozen=True)
class Difference:
    frame: int
    field: str
    original: int
    rebuilt: int


def digest(path: Path, algorithm: str = 'sha256') -> str:
    with path.open('rb') as source:
        return hashlib.file_digest(source, algorithm).hexdigest()


def load_trace(path: Path, frames: int) -> list[dict[str, Any]]:
    rows = [json.loads(line) for line in path.read_text().splitlines()]
    if frames <= 0 or len(rows) != frames:
        raise ValueError(f'{path}: incomplete capture: {len(rows)}/{frames} frames')
    for index, row in enumerate(rows):
        if not isinstance(row, dict) or set(row) != FIELDS or row['frame'] != index:
            raise ValueError(f'{path}: invalid schema or frame sequence at row {index}')
        if any(type(row[key]) is not int for key in FIELDS - {'sides'}):
            raise ValueError(f'{path}: noninteger state at frame {index}')
        if not isinstance(row['sides'], list) or len(row['sides']) != 2:
            raise ValueError(f'{path}: missing sides at frame {index}')
        for side in row['sides']:
            if (not isinstance(side, dict) or set(side) != SIDE_FIELDS or
                    any(type(value) is not int for value in side.values())):
                raise ValueError(f'{path}: incomplete side state at frame {index}')
    return rows


def compare(first: Path, second: Path, frames: int) -> list[Difference]:
    original, rebuilt = load_trace(first, frames), load_trace(second, frames)
    for left, right in zip(original, rebuilt):
        differences: list[Difference] = []
        for key in sorted(FIELDS - {'app_frame', 'sides'}):
            if left[key] != right[key]:
                differences.append(Difference(left['frame'], key, left[key], right[key]))
        for index in range(2):
            for key in sorted(SIDE_FIELDS):
                if left['sides'][index][key] != right['sides'][index][key]:
                    differences.append(Difference(left['frame'], f'sides[{index}].{key}',
                                                  left['sides'][index][key], right['sides'][index][key]))
        if differences:
            return differences
    return []


def display(differences: list[Difference], frames: int) -> None:
    if not differences:
        print(f'parity: all recorded state agrees for {frames} consecutive replay frames')
        return
    print(f'parity: first divergence at replay frame {differences[0].frame}')
    for item in differences:
        values = f'{item.original} -> {item.rebuilt}'
        if item.field.endswith('_bits'):
            a, b = (struct.unpack('<f', struct.pack('<I', value))[0]
                    for value in (item.original, item.rebuilt))
            values += f' ({a!r} -> {b!r})'
        print(f'  {item.field}: {values}')


def windows(path: Path) -> str:
    return 'Z:' + str(path.resolve()).replace('/', '\\')


def build_probe(folder: Path) -> Path:
    folder.mkdir(parents=True, exist_ok=True)
    executable = folder / 'parity_probe.exe'
    # Refresh the compiler's Shift-JIS header copies without rebuilding the game.
    for source in (ROOT / 'src').rglob('*.h'):
        target = ROOT / build.SJIS / source.relative_to(ROOT / 'src')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(build.to_sjis(source.read_text()))
    code, output = build.run_tool('cl', '/O2', '/MT', '/GX', '/I' + build.win(build.SJIS),
                                  '/Fo' + windows(folder / 'parity_probe.obj'),
                                  '/Fe' + windows(executable), windows(ROOT / 'scripts/parity_probe.cpp'),
                                  '/link', '/SUBSYSTEM:CONSOLE', '/MACHINE:X86')
    if code:
        raise ValueError(f'cannot build parity observer:\n{output}')
    return executable


def addresses(executable: Path, map_path: Path | None) -> list[int]:
    wanted = ['?OnUpdate@App@@SIHPAU1@@Z', '?g_App@@3UApp@@A',
              '?g_GameManager@@3VGameManager@@A', '?g_Rng@@3VRng@@A',
              '?g_PlayerInputs@@3PAUPlayerInput@@A']
    if map_path:
        symbols = {name: value[0] for name, value in compare_exe.read_map(str(map_path)).items()}
    else:
        if digest(executable) != TARGET_SHA256:
            raise ValueError('original EXE is not the attested Japanese TH09 1.50a')
        symbols = {name: address for address, _, _, names in compare_exe.read_symbols()
                   for name in names}
    try:
        result = [symbols[name] for name in wanted]
    except KeyError as exc:
        raise ValueError(f'missing observer symbol: {exc}') from exc
    image = compare_exe.PE(str(executable))
    if image.read(result[0], 4) != b'\x53\x56\x8b\xf1':
        raise ValueError('observer entry must have the reviewed App::OnUpdate prologue')
    return result


def capture(probe: Path, executable: Path, map_path: Path | None, folder: Path,
            assets: list[Path], config: bytes | None, frames: int, timeout: int) -> Path:
    locations = addresses(executable, map_path)
    folder.mkdir()
    game = folder / 'th09.exe'
    shutil.copyfile(executable, game)
    for source in assets:
        os.link(source, folder / source.name)
    if config is not None:
        (folder / 'th09.cfg').write_bytes(config)
    trace = folder / 'trace.jsonl'
    display_number = play.free_display()
    env = dict(build.ENV, DISPLAY=display_number, LANG='ja_JP.UTF-8', LC_ALL='ja_JP.UTF-8',
               WINEDEBUG='-all')
    xvfb = subprocess.Popen(['Xvfb', display_number, '-screen', '0', '640x480x24', '-nolisten', 'tcp'],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        deadline = time.monotonic() + 5
        socket = Path('/tmp/.X11-unix') / ('X' + display_number[1:])
        while not socket.exists():
            if xvfb.poll() is not None or time.monotonic() > deadline:
                raise ValueError('Xvfb failed to start')
            time.sleep(0.05)
        command = ['wine', str(probe), windows(game), windows(trace), str(frames), str(timeout),
                   *(hex(value) for value in locations)]
        print(f'capturing {folder.name}: {frames} frames of archive demo 0', flush=True)
        with (folder / 'observer.log').open('w') as log:
            subprocess.run(command, cwd=folder, env=env, stdout=log, stderr=subprocess.STDOUT,
                           timeout=timeout + 15, check=True)
        observations = load_trace(trace, frames)
        (folder / 'metadata.json').write_text(json.dumps({
            'scenario': 'archive-demo-0', 'frames': frames, 'exe_sha256': digest(game),
            'addresses': locations, 'config_sha256': hashlib.sha256(config or b'').hexdigest(),
            'probe_source_sha256': digest(ROOT / 'scripts/parity_probe.cpp'),
            'probe_sha256': digest(probe),
            'map_sha256': digest(map_path) if map_path else None,
            'input_frames': sum(bool(row['input0'] or row['input1']) for row in observations),
            'combat_frames': sum(any(side['bullets'] or side['enemies'] for side in row['sides'])
                                 for row in observations),
        }, indent=2) + '\n')
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
        raise ValueError(f'{folder.name}: capture failed; inspect {folder / "observer.log"}') from exc
    finally:
        xvfb.terminate()
        xvfb.wait(timeout=10)
    return trace


def run(frames: int, timeout: int) -> int:
    original, rebuilt = ROOT / project.ORIGINAL, ROOT / project.EXE
    addresses(original, None)
    addresses(rebuilt, ROOT / project.EXE_MAP)
    asset_dir = ROOT / project.config['game']['dir']
    assets = [asset_dir / 'th09.dat', asset_dir / 'thbgm.dat']
    expected_assets = {line.split()[1].split('/')[-1]: line.split()[0]
                       for line in (ROOT / project.config['game']['checksums']).read_text().splitlines()}
    for asset in assets:
        if digest(asset, 'sha1') != expected_assets[asset.name]:
            raise ValueError(f'archive identity differs: {asset}')
    config_path = asset_dir / 'th09.cfg'
    config = config_path.read_bytes() if config_path.exists() else None
    parent = ROOT / 'build/parity'
    parent.mkdir(parents=True, exist_ok=True)
    folder = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    probe = build_probe(folder)
    print(f'artifacts: {folder}', flush=True)
    (folder / 'scenario.json').write_text(json.dumps({
        'scenario': 'archive-demo-0', 'frames': frames, 'target_sha256': TARGET_SHA256,
        'assets': {asset.name: expected_assets[asset.name] for asset in assets},
    }, indent=2) + '\n')
    first = capture(probe, original, None, folder / 'original-a', assets, config, frames, timeout)
    second = capture(probe, original, None, folder / 'original-b', assets, config, frames, timeout)
    control = compare(first, second, frames)
    if control:
        print('original-versus-original control failed; rebuilt parity is unproven')
        display(control, frames)
        return 2
    print('original-versus-original control passed', flush=True)
    candidate = capture(probe, rebuilt, ROOT / project.EXE_MAP, folder / 'rebuilt',
                        assets, config, frames, timeout)
    differences = compare(first, candidate, frames)
    display(differences, frames)
    (folder / 'result.json').write_text(json.dumps({
        'control_passed': True, 'frames': frames, 'parity_passed': not differences,
        'first_divergence': [item.__dict__ for item in differences],
    }, indent=2) + '\n')
    return int(bool(differences))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest='command', required=True)
    scenario = commands.add_parser('run', help='original/original control, then original/rebuilt')
    scenario.add_argument('--frames', type=int, default=600)
    scenario.add_argument('--timeout', type=int, default=180, help='seconds per capture')
    comparison = commands.add_parser('compare', help='compare complete existing frame traces')
    comparison.add_argument('original', type=Path)
    comparison.add_argument('rebuilt', type=Path)
    comparison.add_argument('--frames', type=int, required=True)
    args = parser.parse_args()
    try:
        if args.frames <= 0:
            raise ValueError('frames must be positive')
        if args.command == 'run':
            if args.timeout <= 0:
                raise ValueError('timeout must be positive')
            return run(args.frames, args.timeout)
        differences = compare(args.original, args.rebuilt, args.frames)
        display(differences, args.frames)
        return int(bool(differences))
    except (OSError, ValueError, KeyError) as exc:
        print(f'parity: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
