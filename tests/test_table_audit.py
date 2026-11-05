"""Integration checks that deliberately corrupt real build inputs/outputs."""
from __future__ import annotations

from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
import table_audit as audit
import coff
import compare_exe
import project


@unittest.skipUnless((ROOT / 'build/th09.exe').exists() and (ROOT / 'build/obj_exe/exe_tables.obj').exists(),
                     'requires a locally built EXE and private original')
class TableAuditTests(unittest.TestCase):
    def check_image(self, executable: Path, obj: Path | None = None) -> tuple[int, int]:
        return audit.check_tables(executable, Path(project.EXE_MAP), Path(project.ORIGINAL),
                                  obj or ROOT / 'build/obj_exe/exe_tables.obj')

    def corrupt(self, address: int, value: int) -> bytes:
        image = compare_exe.PE(project.EXE)
        offset = image.offset(address)
        assert offset is not None
        data = bytearray(image.data)
        struct.pack_into('<I', data, offset, value)
        return bytes(data)

    def test_current_tables(self) -> None:
        count, pointers = self.check_image(Path(project.EXE))
        self.assertEqual(count, 80)
        self.assertGreater(pointers, 600)

    def test_last_cpu_row_is_checked(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'bad.exe'
            path.write_bytes(self.corrupt(0x4a0330 + 70 * 28 + 24, 123))
            with self.assertRaisesRegex(ValueError, r'g_PlayerCpuLevels\[70\]'):
                self.check_image(path)

    def test_matching_contents_at_the_wrong_address_are_rejected(self) -> None:
        with patch.object(audit, 'linked_address', return_value=0x4e6000):
            with self.assertRaisesRegex(ValueError, 'wrong address'):
                self.check_image(Path(project.EXE))

    def test_valid_but_wrong_callback_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'bad.exe'
            path.write_bytes(self.corrupt(0x4a00c0, 0x431110))
            with self.assertRaisesRegex(ValueError, 'linked pointer differs'):
                self.check_image(path)

    def test_truncated_compiled_table_is_rejected(self) -> None:
        cpu = next(row for row in audit.contracts() if row.name == 'g_PlayerCpuLevels')
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'short.obj'
            coff.write_object(str(path), [('.data', 0xc0300040, bytes(5 * 28), [])],
                              {'?g_PlayerCpuLevels@@3PAUPlayerCpuLevel@@A': (1, 0)}, [], {})
            with patch.object(audit, 'contracts', return_value=[cpu]):
                with self.assertRaisesRegex(ValueError, 'truncated table, need 71 rows'):
                    self.check_image(Path(project.EXE), path)


if __name__ == '__main__':
    unittest.main()
