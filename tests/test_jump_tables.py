"""Switch tables link apart from constants: `.rodata` pools, then `.rdata` tables.

Retail ST.EXE lays out every unit's constant pool before any unit's switch
tables (scripts/psxbuild/sdk.py separate_read_only). A unit claims the two
runs with RODATA() and JUMPTABLES(); the delinker carves them into separate
module sections and the relink places each at its own claim.
"""

from __future__ import annotations

import io
import os
import shutil
import struct
import tempfile
import unittest
from pathlib import Path

from elftools.elf.elffile import ELFFile

from scripts.kf.delink import Catalog, Datum, Function, Module, _apply_relocation, _module_data
from scripts.kf.delink import _module_object
from scripts.kf.manifest import Unit, _range_claim
from scripts.kf.model import RodataClaim, scan_jumptable_claims, scan_rodata_claims
from scripts.kf.roundtrip import UnitResult, plan
from scripts.psxbuild.sdk import separate_read_only
from tests.test_delink import elf_sections

from kf2_images import kf2_images


ASSEMBLY = """\t.rdata
\t.align\t2
$LC0:
\t.ascii\t"name\\000"
\t.text
\t.globl\tg
g:
\tj\t$2
\t.rdata
\t.align\t3
$L8:
\t.word\t$L3
\t.word\t$L4
\t.text
$L3:
"""


class SeparateReadOnlyTests(unittest.TestCase):
    def test_constants_move_to_rodata_and_end_eight_aligned(self):
        result = separate_read_only(ASSEMBLY).split('\n')
        self.assertEqual(result[0], '\t.section\t.rodata')
        table = result.index('$L8:')
        self.assertEqual(result[table - 2:table], ['\t.rdata', '\t.align\t3'])
        self.assertEqual(result[-3:], ['\t.section\t.rodata', '\t.align\t3', ''])

    def test_tables_alone_are_unchanged(self):
        tables = ASSEMBLY[ASSEMBLY.index('\t.text\n\t.globl'):]
        self.assertEqual(separate_read_only(tables), tables)


class JumpTableClaimTests(unittest.TestCase):
    def test_jumptables_claim_is_scanned_apart_from_rodata(self):
        with tempfile.TemporaryDirectory(prefix='kf-model-') as directory:
            source = Path(directory) / 'unit.c'
            source.write_text('RODATA(0x80011000, 0x10)\nJUMPTABLES(0x80011490, 0x18)\n\n'
                              'ADDRESS(0x80010000, 0x10)\nvoid first(void)\n{\n}\n')
            self.assertEqual([(c.va, c.size) for c in scan_jumptable_claims(source)],
                             [(0x80011490, 0x18)])
            self.assertEqual([c.va for c in scan_rodata_claims(source)], [0x80011000])

    @kf2_images
    def test_one_word_aligned_range_that_overlaps_no_other_claim(self):
        source = Path('src/game/unit.c')
        claim = RodataClaim(0x80011490, 0x18, 2)
        validate = _range_claim
        self.assertEqual(validate(source, 'GAME.EXE', 'JUMPTABLES', (claim,), [], ()),
                         (0x80011490, 0x18))
        self.assertIsNone(validate(source, 'GAME.EXE', 'JUMPTABLES', (), [], ()))
        other = Unit('game.other', 'GAME.EXE', 'other.c', 'test', (), (),
                     jumptables=(0x800114a0, 8))
        cases = {
            'two': ((claim, RodataClaim(0x800114b0, 8, 3)), [], ()),
            'unaligned': ((RodataClaim(0x80011492, 4, 2),), [], ()),
            'own rodata': ((claim,), [], (('RODATA', 0x80011480, 0x20),)),
            'other unit': ((claim,), [other], ()),
        }
        for name, (claims, units, own) in cases.items():
            with self.subTest(name), self.assertRaises(ValueError):
                validate(source, 'GAME.EXE', 'JUMPTABLES', claims, units, own)


FIRST = Function('GAME.EXE', 0x80010000, 8, 8, 1, 'first', 'test', 'test')
SECOND = Function('GAME.EXE', 0x80010008, 8, 8, 1, 'second', 'test', 'test')
CARVED = {FIRST.va: (struct.pack('<2I', 0x03E00008, 0), []),
          SECOND.va: (struct.pack('<2I', 0x03E00008, 0), [])}
MODULE = Module('GAME.EXE', 'game.pair', 'pair', (FIRST.va, SECOND.va), (),
                (0x80011000, 0x8), jumptables=(0x80011490, 0x8))
CONSTANTS = b'TIM\0\0\0\0\0'
TABLE = struct.pack('<2I', 0x80010008, 0x80010000)


class ModuleJumpTableTests(unittest.TestCase):
    def test_tables_and_constants_carve_into_separate_sections(self):
        built = _module_object(MODULE, {FIRST.va: FIRST, SECOND.va: SECOND}, CARVED, {},
                               CONSTANTS, jumptable_blob=TABLE)
        sections = elf_sections(built.data)
        rdata = built.data[sections['.rdata'][4]:sections['.rdata'][4] + sections['.rdata'][5]]
        rodata = built.data[sections['.rodata'][4]:sections['.rodata'][4] + sections['.rodata'][5]]
        self.assertEqual(struct.unpack('<2I', rdata), (8, 0))
        self.assertEqual(rodata, CONSTANTS)
        self.assertIn('.rel.rdata', sections)
        self.assertNotIn('.rel.rodata', sections)
        self.assertEqual(built.rodata_size, 0x10)

    def test_missing_table_blob_is_an_error(self):
        with self.assertRaises(ValueError):
            _module_object(MODULE, {FIRST.va: FIRST, SECOND.va: SECOND}, CARVED, {}, CONSTANTS)

    def test_code_reference_into_the_tables_resolves_to_rdata(self):
        catalog = Catalog(functions={'GAME.EXE': (FIRST,)},
                          function_starts={'GAME.EXE': {FIRST.va: FIRST}},
                          data={'GAME.EXE': ()})
        blob = bytearray(struct.pack('<2I', 0x3C018001, 0x24211494))
        row = {
            'image': 'GAME.EXE', 'site_va': f'{FIRST.va:#x}', 'paired_site_va': f'{FIRST.va + 4:#x}',
            'kind': 'mips_hi16_lo16', 'channel': 'reachable-code', 'target_va': '0x80011494',
            'target_region': 'load', 'target_name': 'DAT_80011494', 'opcode': 'lui+addiu',
            'confidence': 'paired-pattern', 'status': 'candidate',
        }
        relocations, _used = _apply_relocation(blob, FIRST, row, catalog, 'safe',
                                               (0x80011000, 0x8), jumptables=(0x80011490, 0x8))
        self.assertEqual({item.symbol for item in relocations}, {'.rdata'})
        self.assertEqual(struct.unpack_from('<I', blob, 4)[0] & 0xFFFF, 4)

    def test_relink_places_each_section_at_its_own_claim(self):
        built = _module_object(MODULE, {FIRST.va: FIRST, SECOND.va: SECOND}, CARVED, {},
                               CONSTANTS, jumptable_blob=TABLE)
        unit = Unit('game.pair', 'GAME.EXE', 'pair.c', 'test', (FIRST, SECOND), (),
                    (0x80011000, 0x8), jumptables=(0x80011490, 0x8))
        result = UnitResult('GAME.EXE', 'game.pair', 'pair.o')
        placed = plan(ELFFile(io.BytesIO(built.data)), unit, result)
        self.assertEqual(result.issues, [])
        self.assertEqual(placed, {'.text': 0x80010000, '.rodata': 0x80011000,
                                  '.rdata': 0x80011490})


class SmallBssRunTests(unittest.TestCase):
    """ST.EXE 0x800881b8: two-byte statics at +0/+2/+4, then words at +8/+12/+16."""

    DATA = (Datum(0x800881B8, 2, 'DAT_800881b8', 'bss', 'static', '.sbss'),
            Datum(0x800881BA, 2, 'DAT_800881ba', 'bss', 'static', '.sbss'),
            Datum(0x800881BC, 2, 'DAT_800881bc', 'bss', 'static', '.sbss'),
            Datum(0x800881C0, 4, 'DAT_800881c0', 'bss', 'static', '.sbss'),
            Datum(0x800881C4, 4, 'DAT_800881c4', 'bss', 'static', '.sbss'),
            Datum(0x800881C8, 4, 'DAT_800881c8', 'bss', 'static', '.sbss'))

    def test_statics_carve_at_their_claimed_offsets(self):
        module = Module('ST.EXE', 'st.ui', 'ui', (), self.DATA)
        _, _, _, size, symbols = _module_data(module, {}, section='.sbss')
        self.assertEqual(size, 20)
        self.assertEqual([s.value for s in symbols], [0, 2, 4, 8, 12, 16])

    def test_without_the_leading_static_the_run_has_no_single_base(self):
        module = Module('ST.EXE', 'st.ui', 'ui', (), self.DATA[1:])
        _, _, _, _size, symbols = _module_data(module, {}, section='.sbss')
        bases = {datum.va - symbol.value for datum, symbol in zip(self.DATA[1:], symbols)}
        self.assertGreater(len(bases), 1)


@unittest.skipUnless(shutil.which('dosbox-x') and shutil.which('cc1psx-272cdk')
                     and os.environ.get('PSYQ_ASPSX') and os.environ.get('PSYQ_DMPSX'),
                     'native SDK tools required')
class NativeSectionTests(unittest.TestCase):
    def test_native_object_keeps_constants_and_tables_apart(self):
        from scripts.kf.lnk import elf_view, read
        from scripts.psxbuild.sdk import assemble, compile_c

        source = ('extern void f(const char *);\n'
                  'int g(int x)\n{\n'
                  '    switch (x) {\n'
                  '    case 0: f("constant string"); break;\n'
                  '    case 1: x = 7; break;\n'
                  '    case 2: x = 9; break;\n'
                  '    case 3: x = 11; break;\n'
                  '    case 4: x = 13; break;\n'
                  '    }\n    return x;\n}\n')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'probe.c').write_text(source)
            compile_c(root / 'probe.c', root, 'PROBE', compiler='gcc272cdk-native',
                      optimization='O2', small_data=0, include_dirs=())
            native = assemble(root, 'PROBE', 0)
        sections = {s.name: s for s in read(native).sections.values()}
        self.assertEqual(bytes(sections['.rodata'].data), b'constant string\0')
        self.assertEqual(len(sections['.rdata'].data), 20)
        view = elf_sections(elf_view(native))
        self.assertEqual(view['.rodata'][5], 16)
        self.assertEqual(view['.rdata'][5], 20)


if __name__ == '__main__':
    unittest.main()
