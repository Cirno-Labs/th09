"""Symbol-name ambiguity must not place different CRT functions together."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import Mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import exe_layout
import coff
import compare_exe
import project


class LibrarySymbolTests(unittest.TestCase):
    def test_exact_crt_name_wins_over_underscore_variant(self) -> None:
        self.assertEqual(exe_layout.library_symbol('_tolower', {'_tolower', '__tolower'}), '_tolower')

    def test_plain_c_name_gets_underscore_when_needed(self) -> None:
        self.assertEqual(exe_layout.library_symbol('memcpy', {'_memcpy'}), '_memcpy')

    def test_existing_leading_underscore_can_be_decorated(self) -> None:
        self.assertEqual(exe_layout.library_symbol('_ftol2', {'__ftol2'}), '__ftol2')

    def test_missing_symbol_has_no_binding(self) -> None:
        self.assertIsNone(exe_layout.library_symbol('_tolower', {'_toupper'}))


class CodeEntryTests(unittest.TestCase):
    def section(self, data: bytes, relocs: list[tuple[int, int, int]] | None = None) -> coff.Section:
        return coff.Section('.text', data, relocs or [], coff.TEXT)

    def test_identical_empty_callbacks_can_share_the_attested_entry(self) -> None:
        self.assertTrue(exe_layout.same_code_entry(self.section(b'\xc3'), 0x453c49,
                                                  self.section(b'\xc3'), 0x453c49))

    def test_equal_bodies_at_different_addresses_do_not_prove_identity(self) -> None:
        self.assertFalse(exe_layout.same_code_entry(self.section(b'\xc3'), 0x453c49,
                                                   self.section(b'\xc3'), 0x43d2b0))

    def test_relocations_must_not_be_ignored_when_sharing_entries(self) -> None:
        self.assertFalse(exe_layout.same_code_entry(self.section(b'\xe8\0\0\0\0', [(1, 0, coff.REL32)]), 0x401000,
                                                   self.section(b'\xe8\0\0\0\0', [(1, 1, coff.REL32)]), 0x401000))

    def test_different_bodies_cannot_share_an_entry(self) -> None:
        self.assertFalse(exe_layout.same_code_entry(self.section(b'\xc3'), 0x453c49,
                                                   self.section(b'\xc2\x04\0'), 0x453c49))

    def test_call_target_operand_is_ignored_but_instructions_are_not(self) -> None:
        body = self.section(b'\xe8\0\0\0\0\xc3', [(1, 0, coff.REL32)])
        self.assertTrue(exe_layout.same_code_pattern(body, b'\xe8\x12\x34\x56\x78\xc3'))
        self.assertFalse(exe_layout.same_code_pattern(body, b'\xe8\x12\x34\x56\x78\x90'))

    def test_a_shorter_callee_is_not_the_same_implementation(self) -> None:
        self.assertFalse(exe_layout.same_code_pattern(self.section(b'\x33\xc0\xc3'), b'\xc3'))


class IdenticalSectionTests(unittest.TestCase):
    def test_folding_repeats_through_destructors_vtables_and_constructors(self) -> None:
        obj = Mock(spec=coff.Object)
        constructor = b'\xb8\0\0\0\0\xc3'
        obj.sections = [
            coff.Section('.text', constructor, [(1, 0, coff.DIR32)], coff.TEXT),
            coff.Section('.rdata', bytes(4), [(0, 1, coff.DIR32)], coff.RDATA),
            coff.Section('.text', b'\xc3', [], coff.TEXT),
            coff.Section('.text', constructor, [(1, 2, coff.DIR32)], coff.TEXT),
            coff.Section('.rdata', bytes(4), [(0, 3, coff.DIR32)], coff.RDATA),
            coff.Section('.text', b'\xc3', [], coff.TEXT),
        ]
        obj.symbols = [coff.Symbol('table_a', 0, 2, 3), coff.Symbol('destructor_a', 0, 3, 3),
                       coff.Symbol('table_b', 0, 5, 3), coff.Symbol('destructor_b', 0, 6, 3)]
        positions = {(0, 0): (1, 0), (0, 1): (3, 0), (0, 2): (1, 16),
                     (0, 3): (2, 0), (0, 4): (3, 16), (0, 5): (2, 16)}
        aliases = exe_layout.fold_identical_sections(
            [exe_layout.Unit('test.obj', obj)], {}, {}, positions, positions.keys(),
            {(0, 3), (0, 4), (0, 5)})
        self.assertEqual(aliases, {(0, 3): (0, 0), (0, 4): (0, 1), (0, 5): (0, 2)})
        for copy, original in aliases.items():
            self.assertEqual(positions[copy], positions[original])

    def eh_pair_object(self, copies: int) -> Mock:
        # Each funclet loads its function information, which points back.
        obj = Mock(spec=coff.Object)
        obj.sections = []
        obj.symbols = []
        for pair in range(copies):
            funclet, info = len(obj.symbols), len(obj.symbols) + 1
            obj.sections.append(coff.Section('.text$x', b'\xb8\0\0\0\0\xc3',
                                             [(1, info, coff.DIR32)], coff.TEXT))
            obj.sections.append(coff.Section('.xdata$x', bytes(8),
                                             [(4, funclet, coff.DIR32)], coff.RDATA))
            obj.symbols += [coff.Symbol(f'funclet_{pair}', 0, 2 * pair + 1, 3),
                            coff.Symbol(f'info_{pair}', 0, 2 * pair + 2, 3)]
        return obj

    def test_mutually_referring_copies_fold_together(self) -> None:
        obj = self.eh_pair_object(2)
        positions = {(0, 0): (1, 0x100), (0, 1): (3, 0x40), (0, 2): (2, 0), (0, 3): (2, 8)}
        aliases = exe_layout.fold_identical_sections(
            [exe_layout.Unit('test.obj', obj)], {}, {}, positions, positions.keys(),
            {(0, 2), (0, 3)})
        self.assertEqual(aliases, {(0, 2): (0, 0), (0, 3): (0, 1)})

    def test_mutually_referring_copies_keep_the_first_original(self) -> None:
        obj = self.eh_pair_object(3)
        positions = {(0, 0): (1, 0x200), (0, 1): (3, 0x80), (0, 2): (1, 0x100),
                     (0, 3): (3, 0x40), (0, 4): (2, 0), (0, 5): (2, 8)}
        aliases = exe_layout.fold_identical_sections(
            [exe_layout.Unit('test.obj', obj)], {}, {}, positions, positions.keys(),
            {(0, 4), (0, 5)})
        self.assertEqual(aliases, {(0, 4): (0, 2), (0, 5): (0, 3)})

    def test_distinct_anchored_entries_are_preserved(self) -> None:
        obj = Mock(spec=coff.Object)
        obj.sections = [coff.Section('.text', b'\xc3', [], coff.TEXT)] * 2
        positions = {(0, 0): (1, 0), (0, 1): (1, 16)}
        aliases = exe_layout.fold_identical_sections(
            [exe_layout.Unit('test.obj', obj)], {}, {}, positions, positions.keys(), set())
        self.assertFalse(aliases)
        self.assertEqual(positions[(0, 1)], (1, 16))

    def test_equal_operands_do_not_hide_distinct_storage(self) -> None:
        obj = Mock(spec=coff.Object)
        obj.sections = [coff.Section('.text', b'\xb8\0\0\0\0\xc3',
                                     [(1, index, coff.DIR32)], coff.TEXT) for index in range(2)]
        obj.symbols = [coff.Symbol('storage_a', 0, 3, 3), coff.Symbol('storage_b', 0, 4, 3)]
        positions = {(0, 0): (1, 0), (0, 1): (2, 0), (0, 2): (4, 0), (0, 3): (4, 4)}
        aliases = exe_layout.fold_identical_sections(
            [exe_layout.Unit('test.obj', obj)], {}, {}, positions, {(0, 0), (0, 1)}, {(0, 1)})
        self.assertFalse(aliases)


class StorageGroupTests(unittest.TestCase):
    def setUp(self) -> None:
        obj = Mock(spec=coff.Object)
        obj.sections = [coff.Section('.rdata', b'first---', [], coff.RDATA),
                        coff.Section('.rdata', b'second--', [], coff.RDATA)]
        self.units = [exe_layout.Unit('library.obj', obj)]
        self.groups = [[((0, 0), 0), ((0, 1), 8)]]
        self.reference = Mock(spec=compare_exe.PE)
        self.reference.sections = {'.rdata': (0x490100, 16, 0, 16)}
        self.reference.section_of.return_value = '.rdata'
        self.reference.read.side_effect = lambda address, size: (
            b'first---second--'[address - 0x490100:address - 0x490100 + size])

    def test_anchor_recovers_an_unreferenced_sibling(self) -> None:
        locations = exe_layout.storage_group_locations(
            self.units, self.groups, {(0, 0): {0x100: 1}}, self.reference, 0x490000)
        self.assertEqual(locations, {(0, 0): 0x100, (0, 1): 0x108})

    def test_a_unique_whole_group_can_be_located_without_callers(self) -> None:
        self.assertEqual(exe_layout.storage_group_locations(
            self.units, self.groups, {}, self.reference, 0x490000),
            {(0, 0): 0x100, (0, 1): 0x108})

    def test_a_different_sibling_rejects_the_group(self) -> None:
        self.units[0].obj.sections[1].data = b'changed!'
        self.assertFalse(exe_layout.storage_group_locations(
            self.units, self.groups, {(0, 0): {0x100: 1}}, self.reference, 0x490000))

    def test_multiple_matching_copies_do_not_establish_an_address(self) -> None:
        self.reference.read.side_effect = lambda address, size: (
            b'first---second--'[(address - 0x490100) % 0x100: (address - 0x490100) % 0x100 + size])
        self.assertFalse(exe_layout.storage_group_locations(
            self.units, self.groups, {(0, 0): {0x100: 1, 0x200: 1}}, self.reference, 0x490000))

    def test_storage_split_reports_original_relative_offsets(self) -> None:
        obj = Mock(spec=coff.Object)
        obj.sections = [coff.Section('.rdata', b'first---second--', [], coff.RDATA)]
        obj.symbols = [coff.Symbol('_first', 0, 1, coff.CLASS_EXTERNAL),
                       coff.Symbol('_second', 8, 1, coff.CLASS_EXTERNAL)]
        self.assertEqual(exe_layout.split_storage(obj), {0: [(0, 1), (8, 2)]})
        self.assertEqual(obj.sections[1].data, b'second--')
        self.assertEqual(obj.symbols[1].section, 2)
        self.assertEqual(obj.symbols[1].value, 0)

    def test_crt_entries_split_without_changing_callback_identity(self) -> None:
        obj = Mock(spec=coff.Object)
        obj.sections = [coff.Section('.CRT$XCU', bytes(8),
                                     [(0, 2, coff.DIR32), (4, 3, coff.DIR32)], coff.RDATA)]
        obj.symbols = [coff.Symbol('_slot_a', 0, 1, coff.CLASS_STATIC),
                       coff.Symbol('_slot_b', 4, 1, coff.CLASS_STATIC),
                       coff.Symbol('_init_a', 0, 0, coff.CLASS_EXTERNAL),
                       coff.Symbol('_init_b', 0, 0, coff.CLASS_EXTERNAL)]
        self.assertEqual(exe_layout.split_storage(obj), {0: [(0, 1), (4, 2)]})
        self.assertEqual(obj.sections[0].relocs, [(0, 2, coff.DIR32)])
        self.assertEqual(obj.sections[1].relocs, [(0, 3, coff.DIR32)])


class ImportThunkTests(unittest.TestCase):
    def setUp(self) -> None:
        self.symbol = coff.Symbol('_Api@4', 0, 0, coff.CLASS_EXTERNAL)
        self.obj = Mock(spec=coff.Object)
        self.obj.externals.return_value = [self.symbol]
        self.units = [exe_layout.Unit('input.obj', self.obj)]
        self.known = {'__imp__Api@4': 0x48e000}
        self.stub = b'\xff\x25' + struct.pack('<I', self.known['__imp__Api@4'])
        self.reference = Mock(spec=compare_exe.PE)
        self.reference.sections = {'.text': (0x401000, 8, 0, 8)}
        self.reference.read.return_value = b'\xcc' + self.stub + b'\xcc'

    def test_emits_relocation_and_redirects_references(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            unit = exe_layout.import_thunks(self.units, self.known, self.reference, Path(folder))
            assert unit is not None
            section = unit.obj.sections[0]
            self.assertEqual(section.data, b'\xff\x25\0\0\0\0')
            offset, index, kind = section.relocs[0]
            self.assertEqual((offset, kind), (2, coff.DIR32))
            self.assertEqual(unit.obj.symbols[index].name, '__imp__Api@4')
            self.assertEqual(self.symbol.name, '__layout_import_stub__Api@4')
            self.assertEqual(self.known[self.symbol.name], 0x401001)

    def test_ambiguous_fingerprint_does_not_redirect(self) -> None:
        self.reference.read.return_value = self.stub * 2
        with tempfile.TemporaryDirectory() as folder:
            self.assertIsNone(exe_layout.import_thunks(self.units, self.known, self.reference, Path(folder)))
        self.assertEqual(self.symbol.name, '_Api@4')

    def test_known_address_disagreement_is_rejected(self) -> None:
        self.known['_Api@4'] = 0x401002
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaisesRegex(ValueError, 'import stub address disagrees'):
                exe_layout.import_thunks(self.units, self.known, self.reference, Path(folder))

    def test_existing_definition_is_preserved(self) -> None:
        self.symbol.section = 1
        with tempfile.TemporaryDirectory() as folder:
            self.assertIsNone(exe_layout.import_thunks(self.units, self.known, self.reference, Path(folder)))


class StoragePaddingTests(unittest.TestCase):
    def object_with_padding(self, padding: bytes) -> Mock:
        obj = Mock(spec=coff.Object)
        symbol = coff.Symbol('_storage', 0, 1, coff.CLASS_EXTERNAL)
        obj.externals.return_value = [symbol]
        obj.symbols = [symbol]
        obj.sections = [coff.Section('.bss', bytes(4) + padding, [], 0xc0300080)]
        return obj

    def test_only_known_object_padding_is_trimmed(self) -> None:
        obj = self.object_with_padding(bytes(4))
        exe_layout.trim_storage_padding(obj, {})
        self.assertEqual(len(obj.sections[0].data), 8)
        exe_layout.trim_storage_padding(obj, {'_storage': 4})
        self.assertEqual(len(obj.sections[0].data), 4)

    def test_nonzero_storage_is_not_discarded(self) -> None:
        obj = self.object_with_padding(b'\x01\0\0\0')
        with self.assertRaisesRegex(ValueError, 'non-padding storage'):
            exe_layout.trim_storage_padding(obj, {'_storage': 4})

    def test_initialized_array_padding_does_not_overlap_the_next_table(self) -> None:
        obj = self.object_with_padding(bytes(4))
        obj.sections[0].name = '.data'
        exe_layout.trim_storage_padding(obj, {'_storage': 4})
        self.assertEqual(len(obj.sections[0].data), 4)

    def test_symbol_in_padding_is_not_discarded(self) -> None:
        obj = self.object_with_padding(bytes(4))
        obj.symbols.append(coff.Symbol('_other', 4, 1, coff.CLASS_EXTERNAL))
        with self.assertRaisesRegex(ValueError, 'non-padding storage'):
            exe_layout.trim_storage_padding(obj, {'_storage': 4})

    def test_relocation_in_padding_is_not_discarded(self) -> None:
        obj = self.object_with_padding(bytes(4))
        obj.sections[0].relocs = [(4, 0, coff.DIR32)]
        with self.assertRaisesRegex(ValueError, 'non-padding storage'):
            exe_layout.trim_storage_padding(obj, {'_storage': 4})


@unittest.skipUnless(Path(project.EXE).exists() and Path(project.ORIGINAL).exists(),
                     'requires a locally built EXE and private original')
class LibraryPlacementTests(unittest.TestCase):
    def test_initialized_data_sections_match_including_unused_library_storage(self) -> None:
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        for name in ['.rdata', '.data', '.data1']:
            address, size, _, raw_size = original.sections[name]
            with self.subTest(section=name):
                self.assertEqual(rebuilt.read(address, min(size, raw_size)),
                                 original.read(address, min(size, raw_size)))

    def test_recovered_constructors_and_startup_wrappers_match(self) -> None:
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        for address, size in [(0x421c40, 24), (0x421c60, 18), (0x42b010, 182),
                              (0x42d290, 18), (0x431500, 62),
                              (0x48d7b0, 10), (0x48d7ba, 22), (0x48d7d0, 22),
                              (0x48d7f0, 10), (0x48d800, 23), (0x48d820, 10),
                              (0x48d830, 10), (0x48d880, 23), (0x48d8c0, 22),
                              (0x48d8e0, 10), (0x48d8f0, 10), (0x48d920, 36)]:
            with self.subTest(address=hex(address)):
                self.assertEqual(rebuilt.read(address, size), original.read(address, size))

    def test_game_global_initializer_order_matches(self) -> None:
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        self.assertEqual(rebuilt.read(0x4a0000, 0x70), original.read(0x4a0000, 0x70))

    def test_ordinary_library_tables_keep_unreferenced_members(self) -> None:
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        for address, size in [(0x4999d0, 1284), (0x493de8, 576),
                              (0x4949c0, 232), (0x4a5240, 400)]:
            with self.subTest(address=hex(address)):
                self.assertEqual(rebuilt.read(address, size), original.read(address, size))

    def test_crt_common_storage_and_section_extent_match(self) -> None:
        symbols = compare_exe.read_map(project.EXE_MAP)
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        self.assertEqual(symbols['___ctype1_refcount'][0], 0x4e2e34)
        self.assertEqual(rebuilt.read(0x4810a2, 208), original.read(0x4810a2, 208))
        self.assertEqual(rebuilt.sections['.data'][1], original.sections['.data'][1])
        self.assertEqual(rebuilt.sections['.rdata'][0:2], original.sections['.rdata'][0:2])

    def test_sse2_rotation_implementations_and_callees_match(self) -> None:
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        for address, size in [(0x4668f0, 128), (0x466970, 144)]:
            with self.subTest(address=hex(address)):
                self.assertEqual(rebuilt.read(address, size), original.read(address, size))

    def test_folded_codec_constructors_keep_original_entry_identity(self) -> None:
        symbols = compare_exe.read_map(project.EXE_MAP)
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        for codec in ['DXT1', 'DXT2', 'DXT3', 'DXT4', 'DXT5']:
            with self.subTest(codec=codec):
                symbol = f'??0CD3DXCodec_{codec}@@QAE@PAUD3DX_BLT@@@Z'
                self.assertEqual(symbols[symbol][0], 0x45dc1e)
        self.assertEqual(rebuilt.read(0x45dc1e, 24), original.read(0x45dc1e, 24))

    def test_empty_sdk_callbacks_keep_their_shared_sdk_entries(self) -> None:
        symbols = compare_exe.read_map(project.EXE_MAP)
        for symbol, address in [('?QuantizeColorKey@CD3DXCodecYUV@@MAEXXZ', 0x453c49),
                                ('?jpeg_mem_init@D3DX@@YAJPAUjpeg_common_struct@1@@Z', 0x4889b8)]:
            with self.subTest(symbol=symbol):
                self.assertEqual(symbols[symbol][0], address)

    def test_sound_storage_does_not_displace_d3dx_globals(self) -> None:
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        # D3DXDebugMute uses the pointer immediately after SoundPlayer.
        self.assertEqual(rebuilt.read(0x4533f9, 144), original.read(0x4533f9, 144))

    def test_import_stubs_are_at_original_addresses(self) -> None:
        symbols = compare_exe.read_map(project.EXE_MAP)
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        for symbol, address in [('_DirectInput8Create@20', 0x44e5a0),
                                ('_DirectSoundCreate8@12', 0x44e5a6),
                                ('_Direct3DCreate8@4', 0x44e5ac),
                                ('_WINNLSEnableIME@8', 0x47b04e),
                                ('_RtlUnwind@16', 0x48d204)]:
            with self.subTest(symbol=symbol):
                self.assertEqual(symbols[symbol][0], address)
                self.assertEqual(rebuilt.read(address, 6), original.read(address, 6))
        self.assertEqual(rebuilt.sections['.text'][1], original.sections['.text'][1])

    def test_tolower_is_placed_at_its_original_address(self) -> None:
        address, size = 0x486150, 34
        original = compare_exe.PE(project.ORIGINAL)
        rebuilt = compare_exe.PE(project.EXE)
        original_offset = original.offset(address)
        rebuilt_offset = rebuilt.offset(address)
        self.assertIsNotNone(original_offset)
        self.assertIsNotNone(rebuilt_offset)
        assert original_offset is not None and rebuilt_offset is not None
        self.assertEqual(rebuilt.data[rebuilt_offset:rebuilt_offset + size],
                         original.data[original_offset:original_offset + size])


if __name__ == '__main__':
    unittest.main()
