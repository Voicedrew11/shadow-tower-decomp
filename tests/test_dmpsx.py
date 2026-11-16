"""DMPSX 3.0: INLINE_C.H GTE placeholders become retail COP2 commands."""

import os
from pathlib import Path
import re
import shutil
import struct
import tempfile
import unittest

from scripts.psxbuild.dmpsx import (is_cop2_command, is_placeholder, merge, text_words,
                                    verify, view)

NOP = 0
JR_RA = 0x03E00008
NCLIP = 0x4B400006


def declaration(number: int, name: str) -> bytes:
    return bytes([16]) + struct.pack('<HHB', number, 0, 8) + bytes([len(name)]) + name.encode()


def code(number: int, words: list[int]) -> bytes:
    data = struct.pack(f'<{len(words)}I', *words)
    return bytes([6]) + struct.pack('<H', number) + bytes([2]) + struct.pack('<H', len(data)) + data


def lnk(sections: dict[str, list[int]], exports: tuple[str, ...] = ()) -> bytes:
    """A minimal LNK v2 object: one code record per section, then XDEFs."""
    data = bytearray(b'LNK\x02\x2e\x07')
    for number, name in enumerate(sections, 1):
        data += declaration(number, name)
    for number, words in enumerate(sections.values(), 1):
        data += code(number, words)
    for number, name in enumerate(exports, 1):
        data += bytes([12]) + struct.pack('<HHI', number, 1, 0) + bytes([len(name)]) + name.encode()
    return bytes(data + b'\0')


class VerifyTests(unittest.TestCase):
    def test_word_classes(self):
        self.assertTrue(is_placeholder(0x117F))
        self.assertTrue(is_placeholder(0x13BF | 1 << 25))
        self.assertFalse(is_placeholder(NOP))
        self.assertFalse(is_placeholder(JR_RA))
        self.assertTrue(is_cop2_command(NCLIP))
        self.assertFalse(is_cop2_command(0x48824000))  # mtc2 is a move, not a command

    def test_text_words_follow_code_records(self):
        data = lnk({'.data': [0x117F], '.text': [NOP, 0x117F, JR_RA]})
        self.assertEqual([word for _, word in text_words(data)], [NOP, 0x117F, JR_RA])

    def test_object_without_placeholders_must_stay_identical(self):
        # A placeholder-shaped word in data is not code and is never rewritten.
        before = lnk({'.text': [NOP, JR_RA, NOP], '.data': [0x117F, 0x3F]})
        self.assertEqual(verify(before, before, 'U'), 0)
        changed = lnk({'.text': [NOP, JR_RA, NOP], '.data': [NCLIP, 0x3F]})
        with self.assertRaisesRegex(RuntimeError, 'other than GTE placeholder'):
            verify(before, changed, 'U')

    def test_each_placeholder_must_become_a_command(self):
        before = lnk({'.text': [NOP, NOP, 0x117F, JR_RA, NOP]})
        self.assertEqual(verify(before, lnk({'.text': [NOP, NOP, NCLIP, JR_RA, NOP]}), 'U'), 1)
        # Too few delay slots leaves the word; an undefined macro becomes a nop.
        for result in (0x117F, NOP):
            with self.subTest(result=result), self.assertRaisesRegex(RuntimeError, 'left GTE'):
                verify(before, lnk({'.text': [NOP, NOP, result, JR_RA, NOP]}), 'U')

    def test_view_holds_only_text_and_merge_copies_its_words_back(self):
        text = [NOP, NOP, 0x117F, JR_RA, NOP]
        before = lnk({'.data': [0x117F], '.text': text}, ('f0', 'f1'))
        shown = view(before)
        self.assertEqual(shown, b'LNK\x02' + declaration(2, '.text') + code(2, text) + b'\0')
        rewritten = shown.replace(struct.pack('<I', 0x117F), struct.pack('<I', NCLIP))
        after = merge(before, rewritten, 'U')
        self.assertEqual(after, lnk({'.data': [0x117F], '.text': [NOP, NOP, NCLIP, JR_RA, NOP]},
                                    ('f0', 'f1')))
        self.assertEqual(verify(before, after, 'U'), 1)
        with self.assertRaisesRegex(RuntimeError, 'code records'):
            merge(before, view(lnk({'.text': text[:-1]})), 'U')
        with self.assertRaisesRegex(RuntimeError, 'unreadable'):
            merge(before, rewritten[:-1], 'U')

    def test_truncated_output_is_rejected(self):
        before = lnk({'.text': [NOP, NOP, 0x117F, JR_RA, NOP]})
        with self.assertRaisesRegex(RuntimeError, 'object size'):
            verify(before, before[:-9], 'U')


# INLINE_C.H's type 2 macros and the COP2 command each must become: the GTE
# function codes and field layout of the R3000A coprocessor 2.
SIMPLE = {
    'rtps': 0x4A180001, 'rtpt': 0x4A280030, 'dpcl': 0x4A680029, 'dpcs': 0x4A780010,
    'dpct': 0x4AF8002A, 'intpl': 0x4A980011, 'sqr12': 0x4AA80428, 'sqr0': 0x4AA00428,
    'ncs': 0x4AC8041E, 'nct': 0x4AD80420, 'ncds': 0x4AE80413, 'ncdt': 0x4AF80416,
    'nccs': 0x4B08041B, 'ncct': 0x4B18043F, 'cdp': 0x4B280414, 'cc': 0x4B38041C,
    'nclip': 0x4B400006, 'avsz3': 0x4B58002D, 'avsz4': 0x4B68002E, 'op12': 0x4B78000C,
    'op0': 0x4B70000C, 'gpf12': 0x4B98003D, 'gpf0': 0x4B90003D, 'gpl12': 0x4BA8003E,
    'gpl0': 0x4BA0003E,
}
# MVMVA spellings as (sf, mx, v, cv, lm): mx 0 rotation, 1 light, 2 colour;
# v 3 is IR; cv 0 TR, 1 BK, 3 none.
MVMVA = {
    'rt': (1, 0, 0, 0, 0), 'rtir_sf0': (0, 0, 3, 3, 0),
    'll': (1, 1, 0, 3, 1), 'lc': (1, 2, 3, 1, 1),
}
for _prefix, _mx in (('rt', 0), ('ll', 1), ('lc', 2)):
    for _v, _vector in enumerate(('v0', 'v1', 'v2', 'ir')):
        for _suffix, _cv in (('', 3), ('tr', 0), ('bk', 1)):
            MVMVA[_prefix + _vector + _suffix] = (1, _mx, _v, _cv, 0)


def mvmva(sf, mx, v, cv, lm):
    return 0x4A400012 | sf << 19 | mx << 17 | v << 15 | cv << 13 | lm << 10


@unittest.skipUnless(shutil.which('dosbox-x') and shutil.which('cc1psx-272cdk')
                     and os.environ.get('PSYQ_ASPSX') and os.environ.get('PSYQ_DMPSX')
                     and os.environ.get('PSYQ_INCLUDE'), 'native SDK tools required')
class NativeTests(unittest.TestCase):
    def test_every_inline_c_command_macro_becomes_its_cop2_command(self):
        from scripts.kf.lnk import read
        from scripts.psxbuild.sdk import assemble, compile_c

        header = (Path(os.environ['PSYQ_INCLUDE']) / 'INLINE_C.H').read_text()
        macros = re.findall(r'#define gte_(\w+)\(\)[\s\\]*__asm__ volatile \([\s\\]*'
                            r'((?:"nop;"[\s\\]*)*)"\.word (0x[0-9a-f]+)', header)
        plain = {name: (word, nops.count('nop')) for name, nops, word in macros
                 if not name.endswith('_b')}
        nopless = {name.removesuffix('_b'): (word, nops.count('nop')) for name, nops, word in macros
                   if name.endswith('_b')}
        # 65 commands, each as a plain macro after two nops and as a _b form.
        self.assertEqual(len(plain), 65)
        self.assertEqual({name: (word, 2) for name, (word, _) in nopless.items()}, plain)
        names = sorted(plain)
        expected = {name: SIMPLE.get(name) or mvmva(*MVMVA[name]) for name in names}
        fields = [(1, 0, 3, 3, 0), (0, 2, 1, 0, 1), (1, 3, 2, 2, 0)]
        source = ['#include <INLINE_C.H>']
        for name in names:
            source += [f'void p_{name}(void) {{ gte_{name}(); }}',
                       f'void b_{name}(void) {{ gte_{name}_b(); }}']
        for index, field in enumerate(fields):
            source += [f'void p_mv{index}(void) {{ gte_mvmva({", ".join(map(str, field))}); }}',
                       f'void b_mv{index}(void) {{ gte_mvmva_b({", ".join(map(str, field))}); }}']
            expected[f'mv{index}'] = mvmva(*field)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'probe.c').write_text('\n'.join(source) + '\n')
            compile_c(root / 'probe.c', root, 'PROBE', compiler='gcc272cdk-native',
                      optimization='O2', small_data=0,
                      include_dirs=(Path(os.environ['PSYQ_INCLUDE']),))
            obj = read(assemble(root, 'PROBE', 0))
            self.assertEqual(sorted(path.suffix for path in root.glob('PROBE.*')),
                             ['.I', '.OBJ', '.S', '.TXT'])
        text = next(section.data for section in obj.sections.values() if section.name == '.text')
        for name, word in expected.items():
            for prefix, body in (('p', [NOP, NOP, word]), ('b', [word])):
                with self.subTest(function=f'{prefix}_{name}'):
                    _, start, size = obj.functions[f'{prefix}_{name}']
                    self.assertEqual(list(struct.unpack_from(f'<{size // 4}I', text, start)),
                                     [*body, JR_RA, NOP])

    def test_the_dos_build_gets_a_view_it_can_walk(self):
        # Four exported functions: run on the whole object, DMPSX 3.0 reads each
        # XDEF two bytes long, loses its place and drops the object's last bytes.
        from scripts.psxbuild.sdk import (aspsx_command, assemble, compile_c, dos_run,
                                          tool_succeeded)

        source = ''.join(f'void f{index}(void) {{ }}\n' for index in range(4))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'four.c').write_text(source)
            compile_c(root / 'four.c', root, 'FOUR', compiler='gcc272cdk-native',
                      optimization='O2', small_data=0, include_dirs=())
            shutil.copyfile(os.environ['PSYQ_ASPSX'], root / 'ASPSX.EXE')
            shutil.copyfile(os.environ['PSYQ_DMPSX'], root / 'DMPSX.EXE')
            dos_run(root, [aspsx_command('FOUR', 0), 'DMPSX.EXE FOUR.OBJ -o WHOLE.OBJ'], 'whole')
            tool_succeeded(root, 'FOUR.TXT', 'FOUR.OBJ', b'LNK\x02')
            assembled = (root / 'FOUR.OBJ').read_bytes()
            whole = (root / 'WHOLE.OBJ').read_bytes()
            self.assertEqual(whole, assembled[:len(whole)])
            self.assertLess(len(whole), len(assembled))
            self.assertEqual(assemble(root, 'FOUR', 0), assembled)
