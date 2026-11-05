"""An incomplete byte comparison must not be reported as 100 percent."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import render_progress as progress


class CompletionDisplayTests(unittest.TestCase):
    def assert_display(self, same: int, total: int, expected: str) -> None:
        row = progress.SummaryRow('Library code', progress.ProgressBar({
            progress.SymbolStatus.NAMED: same,
            progress.SymbolStatus.TODO: total - same,
        }), [], label='Library code')
        self.assertIn(expected, progress.summary([row], 100))
        self.assertIn(expected, progress.SummarySection([row]).render())

    def test_actual_remaining_library_bytes_are_visible(self) -> None:
        self.assert_display(216666, 216727, '99.97%')

    def test_one_byte_remaining_is_not_rounded_to_complete(self) -> None:
        self.assert_display(999999, 1000000, '99.9999%')

    def test_exact_match_is_reported_as_complete(self) -> None:
        self.assert_display(1000000, 1000000, '100.0%')


if __name__ == '__main__':
    unittest.main()
