"""Exercise divergence and incomplete-capture failures without game assets."""
from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
import parity


def sample(frame: int) -> dict[str, object]:
    side = {'character': 4, 'state': 0, 'health': 10, 'x_bits': 0,
            'y_bits': 1136656384, 'charge_bits': 1120403456, 'score': 0,
            'lives_bits': 0, 'bullets': 3, 'enemies': 2}
    return {'frame': frame, 'app_frame': frame + 1700, 'rng_seed': 1234,
            'rng_generated': 1028 + frame, 'stage': 3, 'difficulty': 3,
            'damage': 2, 'cpu_damage': 0, 'input0': 1, 'input1': 1,
            'sides': [side, dict(side)]}


class ParityTests(unittest.TestCase):
    def compare_rows(self, first: list[dict[str, object]], second: list[dict[str, object]],
                     frames: int = 3) -> list[parity.Difference]:
        with tempfile.TemporaryDirectory() as folder:
            left, right = Path(folder) / 'original.jsonl', Path(folder) / 'rebuilt.jsonl'
            left.write_text(''.join(json.dumps(row) + '\n' for row in first))
            right.write_text(''.join(json.dumps(row) + '\n' for row in second))
            return parity.compare(left, right, frames)

    def test_transient_divergence_is_not_hidden_by_matching_final_state(self) -> None:
        original = [sample(frame) for frame in range(3)]
        rebuilt = copy.deepcopy(original)
        rebuilt[1]['rng_seed'] = 5678
        self.assertEqual(self.compare_rows(original, rebuilt),
                         [parity.Difference(1, 'rng_seed', 1234, 5678)])

    def test_startup_wall_clock_frames_are_not_gameplay_state(self) -> None:
        original = [sample(frame) for frame in range(3)]
        rebuilt = copy.deepcopy(original)
        for row in rebuilt:
            value = row['app_frame']
            assert isinstance(value, int)
            row['app_frame'] = value + 20
        self.assertEqual(self.compare_rows(original, rebuilt), [])

    def test_identical_but_truncated_captures_fail(self) -> None:
        rows = [sample(0)]
        with self.assertRaisesRegex(ValueError, 'incomplete capture: 1/3'):
            self.compare_rows(rows, rows)

    def test_float_bit_differences_are_preserved(self) -> None:
        original = sample(0)
        rebuilt = copy.deepcopy(original)
        sides = rebuilt['sides']
        assert isinstance(sides, list)
        sides[0]['x_bits'] = 0x80000000  # -0.0 must not collapse to +0.0.
        self.assertEqual(self.compare_rows([original], [rebuilt], frames=1),
                         [parity.Difference(0, 'sides[0].x_bits', 0, 0x80000000)])

    def test_skipped_frame_fails_even_when_both_traces_skip_it(self) -> None:
        rows = [sample(0), sample(2), sample(3)]
        with self.assertRaisesRegex(ValueError, 'frame sequence'):
            self.compare_rows(rows, rows)

    def test_incomplete_side_schema_cannot_pass(self) -> None:
        row = sample(0)
        sides = row['sides']
        assert isinstance(sides, list)
        del sides[0]['health']
        with self.assertRaisesRegex(ValueError, 'incomplete side state'):
            self.compare_rows([row], [row], frames=1)


if __name__ == '__main__':
    unittest.main()
