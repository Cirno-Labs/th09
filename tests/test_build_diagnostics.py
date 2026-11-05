"""Build failures must retain diagnostics emitted before MSVC starts."""
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import build


class BuildDiagnosticsTests(unittest.TestCase):
    def test_wine_startup_error_is_reported(self) -> None:
        result = subprocess.CompletedProcess(
            args=['wine'], returncode=1, stdout='',
            stderr='wineserver: bind: Operation not permitted\n',
        )
        with patch.object(build.subprocess, 'run', return_value=result):
            code, output = build.run_tool('link')
        self.assertEqual(code, 1)
        self.assertIn('wineserver: bind: Operation not permitted', output)

    def test_compiler_and_stderr_diagnostics_are_both_reported(self) -> None:
        result = subprocess.CompletedProcess(
            args=['wine'], returncode=2,
            stdout='player.cpp\nplayer.cpp(1): error C2143: syntax error\n',
            stderr='wine: additional diagnostic\n',
        )
        with patch.object(build.subprocess, 'run', return_value=result):
            code, output = build.run_tool('cl', '/c', 'player.cpp')
        self.assertEqual(code, 2)
        self.assertEqual(output.splitlines(), [
            'player.cpp(1): error C2143: syntax error',
            'wine: additional diagnostic',
        ])


if __name__ == '__main__':
    unittest.main()
