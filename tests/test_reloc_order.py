"""Controls for ELF REL order of interleaved R_MIPS_HI16/R_MIPS_LO16 pairs."""

from __future__ import annotations

import io
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

from scripts.kf.lnk import elf_view
from scripts.kf.mips_elf import MipsRelocation, rel_order, write_mips_elf
from scripts.kf.roundtrip import LINKER

HI16, LO16 = 5, 6
# ST.EXE 0x8002268c..0x800226f4 interleaves two world_state pairs: the outer
# lui pairs with the last addiu; the inner lui/addiu sit between them. Here
# the outer target is world+0x10 and the inner one world+0x834c (carried).
OUTER_HI, INNER_HI, INNER_LO, OUTER_LO = 0x0, 0x4, 0x8, 0xC
INTERLEAVED = struct.pack('<5I', 0x3C040000, 0x3C050001, 0x24A5834C, 0x24840010, 0)


def rel_rows(blob: bytes) -> list[tuple[int, int]]:
    elf = ELFFile(io.BytesIO(blob))
    section = next(s for s in elf.iter_sections()
                   if isinstance(s, RelocationSection) and s.name == '.rel.text')
    return [(r['r_offset'], r['r_info_type']) for r in section.iter_relocations()]


def interleaved(symbol: str = 'world') -> list[MipsRelocation]:
    return [
        MipsRelocation(OUTER_HI, 'R_MIPS_HI16', symbol, OUTER_LO),
        MipsRelocation(INNER_HI, 'R_MIPS_HI16', symbol, INNER_LO),
        MipsRelocation(INNER_LO, 'R_MIPS_LO16', symbol),
        MipsRelocation(OUTER_LO, 'R_MIPS_LO16', symbol),
    ]


class RelocationOrderTests(unittest.TestCase):
    def test_interleaved_hi16_moves_just_before_its_own_lo16(self) -> None:
        blob = write_mips_elf(INTERLEAVED, 'control', len(INTERLEAVED), interleaved())
        self.assertEqual(rel_rows(blob), [
            (INNER_HI, HI16), (INNER_LO, LO16), (OUTER_HI, HI16), (OUTER_LO, LO16),
        ])

    def test_interleaved_pairs_with_one_low_half_keep_site_order(self) -> None:
        # The linker computes the same carry from either LO16: no move.
        text = struct.pack('<4I', 0x3C040000, 0x3C050000, 0x24A50010, 0x24840010)
        order = rel_order(interleaved(), text)
        self.assertEqual([r.offset for r in order], [OUTER_HI, INNER_HI, INNER_LO, OUTER_LO])

    def test_pairs_against_different_symbols_keep_site_order(self) -> None:
        relocations = interleaved()
        relocations[1:3] = [MipsRelocation(INNER_HI, 'R_MIPS_HI16', 'other', INNER_LO),
                            MipsRelocation(INNER_LO, 'R_MIPS_LO16', 'other')]
        self.assertEqual([r.offset for r in rel_order(relocations, INTERLEAVED)],
                         [OUTER_HI, INNER_HI, INNER_LO, OUTER_LO])

    def test_converging_hi16s_and_intervening_calls_keep_site_order(self) -> None:
        # Moving these HI16s past the R_MIPS_26 would change the addend
        # objdiff reports for one side only (the regression of the first
        # order-by-pair attempt).
        text = bytes(0x24)
        relocations = [
            MipsRelocation(0x00, 'R_MIPS_HI16', 'viewer', 0x20),
            MipsRelocation(0x10, 'R_MIPS_HI16', 'viewer', 0x20),
            MipsRelocation(0x1C, 'R_MIPS_26', 'func'),
            MipsRelocation(0x20, 'R_MIPS_LO16', 'viewer'),
        ]
        self.assertEqual([r.offset for r in rel_order(relocations, text)],
                         [0x00, 0x10, 0x1C, 0x20])

    def test_missing_own_lo16_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, 'own LO16'):
            rel_order([MipsRelocation(0, 'R_MIPS_HI16', 'a', 8)], bytes(12))

    @unittest.skipUnless(shutil.which(LINKER), 'pinned GNU MIPS ld required')
    def test_repeated_equivalent_lows_do_not_depend_on_pair_provenance(self) -> None:
        # A store and a later address calculation use the same outer base.
        # A native expression can identify either LO16 as the high's pair;
        # decoded register flow can select the other. Both require the same
        # carry, so the inspection objects and their linked words must agree.
        text = struct.pack('<5I', 0x3C040000, 0x3C050001, 0x24A5834C,
                           0xAC820010, 0x24860010)
        expected = struct.pack('<5I', 0x3C048011, 0x3C058011, 0x24A50344,
                               0xAC828008, 0x24868008)
        objects = []
        for pair in (0xC, 0x10):
            relocations = interleaved()
            relocations[0] = MipsRelocation(0, 'R_MIPS_HI16', 'world', pair)
            relocations.append(MipsRelocation(0x10, 'R_MIPS_LO16', 'world'))
            blob = write_mips_elf(text, 'control', len(text), relocations)
            objects.append(blob)
            with self.subTest(pair=pair), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                (root / 'in.o').write_bytes(blob)
                subprocess.run([LINKER, '-EL', '-G', '0', '-Ttext', '0x80010000',
                                '-e', '0x80010000', '--defsym=world=0x80107ff8',
                                '-o', str(root / 'out.elf'), str(root / 'in.o')],
                               check=True, capture_output=True)
                linked = ELFFile(io.BytesIO((root / 'out.elf').read_bytes()))
                self.assertEqual(linked.get_section_by_name('.text').data()[:20], expected)
        self.assertEqual(objects[0], objects[1])

    def test_moved_shifts_the_pair_and_equality_ignores_it(self) -> None:
        high = MipsRelocation(0x04, 'R_MIPS_HI16', 'a', 0x10)
        self.assertEqual(high.moved(0x100).pair, 0x110)
        self.assertEqual(high.moved(0x100).offset, 0x104)
        self.assertEqual(high, MipsRelocation(0x04, 'R_MIPS_HI16', 'a'))

    @unittest.skipUnless(shutil.which(LINKER), 'pinned GNU MIPS ld required')
    def test_gnu_ld_completes_each_hi16_with_its_own_lo16(self) -> None:
        # world = 0x80107ff8: outer 0x80108008 and inner 0x80110344 both need
        # %hi 0x8011; pairing the outer lui with the inner addiu gives 0x8010.
        expected = struct.pack('<4I', 0x3C048011, 0x3C058011, 0x24A50344, 0x24848008)
        for relocations, faithful in ((interleaved(), True),
                                      ([MipsRelocation(r.offset, r.kind, r.symbol)
                                        for r in interleaved()], False)):
            with self.subTest(faithful=faithful), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                (root / 'in.o').write_bytes(write_mips_elf(
                    INTERLEAVED, 'control', len(INTERLEAVED), relocations))
                subprocess.run([LINKER, '-EL', '-G', '0', '-Ttext', '0x80010000',
                                '-e', '0x80010000', '--defsym=world=0x80107ff8',
                                '-o', str(root / 'out.elf'), str(root / 'in.o')],
                               check=True, capture_output=True)
                linked = ELFFile(io.BytesIO((root / 'out.elf').read_bytes()))
                words = linked.get_section_by_name('.text').data()[:16]
                self.assertEqual(words == expected, faithful)


def native_object(patches: list[tuple[int, int, int]]) -> bytes:
    """A synthetic Psy-Q LNK object: .text INTERLEAVED, patches against `world`."""
    def name(value: str) -> bytes:
        return bytes([len(value)]) + value.encode('ascii')

    blob = bytearray(b'LNK\x02' + bytes([46, 7]))
    blob += bytes([16]) + struct.pack('<HHB', 1, 0, 8) + name('.text')
    blob += bytes([14]) + struct.pack('<H', 2) + name('world')
    blob += bytes([12]) + struct.pack('<HHI', 3, 1, 0) + name('control')
    blob += bytes([6]) + struct.pack('<H', 1)
    code = struct.pack('<4I', 0x3C040000, 0x3C050000, 0x24A50000, 0x24840000)
    blob += bytes([2]) + struct.pack('<H', len(code)) + code
    for kind, offset, addend in patches:
        # add(symbol, constant), serialized right operand first.
        blob += bytes([10, kind]) + struct.pack('<H', offset)
        blob += bytes([44, 0]) + struct.pack('<I', addend) + bytes([2]) + struct.pack('<H', 2)
    return bytes(blob + b'\0')


class NativeViewOrderTests(unittest.TestCase):
    def test_native_view_pairs_each_hi16_by_its_expression(self) -> None:
        view = elf_view(native_object([
            (82, OUTER_HI, 0x10), (82, INNER_HI, 0x834C),
            (84, INNER_LO, 0x834C), (84, OUTER_LO, 0x10),
        ]))
        self.assertEqual(ELFFile(io.BytesIO(view)).get_section_by_name('.text').data(),
                         INTERLEAVED[:16])
        self.assertEqual(rel_rows(view), [
            (INNER_HI, HI16), (INNER_LO, LO16), (OUTER_HI, HI16), (OUTER_LO, LO16),
        ])

    def test_native_view_without_a_matching_expression_keeps_site_order(self) -> None:
        # A lui whose %hi expression no LO16 repeats has no recorded pair.
        view = elf_view(native_object([
            (82, OUTER_HI, 0x0), (82, INNER_HI, 0x834C),
            (84, INNER_LO, 0x834C), (84, OUTER_LO, 0x10),
        ]))
        self.assertEqual(rel_rows(view), [
            (OUTER_HI, HI16), (INNER_HI, HI16), (INNER_LO, LO16), (OUTER_LO, LO16),
        ])


if __name__ == '__main__':
    unittest.main()
