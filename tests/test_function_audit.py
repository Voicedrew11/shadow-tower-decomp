from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path

from scripts.kf.audit_functions import (
    FunctionExtent,
    ImageWords,
    audit_image,
    fallthrough_boundary,
    hoisted_start,
    matching_return,
    frame_at,
    switch_extension,
)
from scripts.kf.propose_function_admission import proposed_data, proposed_functions
from scripts.kf.retail import DATA_FIELDS, FUNCTION_FIELDS, ImageLayout, write_tsv


BASE = 0x80010000
JR_RA = 0x03E00008
NOP = 0


def words(*values: int) -> ImageWords:
    payload = struct.pack(f"<{len(values)}I", *values)
    image = ImageWords.__new__(ImageWords)
    image.layout = ImageLayout("T.EXE", "-", 0x800 + len(payload), BASE, BASE, len(payload))
    image.payload = payload
    return image


def jal(target: int) -> int:
    return 0x0C000000 | ((target >> 2) & 0x03FFFFFF)


def jump(target: int) -> int:
    return 0x08000000 | ((target >> 2) & 0x03FFFFFF)


class FunctionAuditTests(unittest.TestCase):
    def test_psyq4_epilogue_restores_the_stack_before_the_return(self) -> None:
        code = words(
            0x27BDFFE8,  # addiu sp,sp,-24
            0xAFBF0010,  # sw ra,16(sp)
            0x8FBF0010,  # lw ra,16(sp)
            0x27BD0018,  # addiu sp,sp,24
            JR_RA, NOP,
        )
        frame = frame_at(code, BASE)
        self.assertEqual(matching_return(code, BASE, frame), BASE + 0x10)

    def test_loads_hoisted_above_the_frame_belong_to_the_function(self) -> None:
        code = words(
            JR_RA, NOP,
            0x3C028019,  # lui v0,0x8019
            0x8C450F1C,  # lw a1,3868(v0)
            0x27BDFFD8,  # addiu sp,sp,-40
            0xAFBF0020,  # sw ra,32(sp)
        )
        self.assertEqual(hoisted_start(code, BASE + 0x10), BASE + 8)
        branch = words(JR_RA, NOP, 0x10400002, NOP, 0x27BDFFD8, 0xAFBF0020)
        self.assertIsNone(hoisted_start(branch, BASE + 0x10))

    def test_call_target_after_a_stub_splits_its_owner(self) -> None:
        code = words(
            0x240A00B0, 0x01400008, 0x24090032, NOP,  # B0 stub: li t2; jr t2; li t1; pad
            0x240A00B0, 0x01400008, 0x24090033, NOP,  # second stub
            0x27BDFFE8, 0xAFBF0010, jal(BASE + 0x10), NOP,  # caller
            0x8FBF0010, 0x27BD0018, JR_RA, NOP,
        )
        self.assertEqual(fallthrough_boundary(code, BASE + 0x10), BASE + 0x0C)
        extents = [FunctionExtent(BASE, BASE + 0x20), FunctionExtent(BASE + 0x20, BASE + 0x40)]
        rows = audit_image("T.EXE", code, extents, {})
        split = [row for row in rows if row["confidence"] == "call-split"]
        self.assertEqual(len(split), 1)
        self.assertEqual(split[0]["va"], f"0x{BASE + 0x10:08x}")
        self.assertEqual(split[0]["boundary"], f"split-owner-end=0x{BASE + 0x0C:08x}")

    def switch(self, second_case: int) -> ImageWords:
        """A table at BASE, a two-case switch at BASE + 8, a leaf after it."""
        f = BASE + 8
        return words(
            f + 0x2C, second_case,  # the jump table
            0x2C820002,  # sltiu v0,a0,2
            0x1040000D,  # beqz v0,default
            NOP,
            0x3C028001,  # lui v0,0x8001
            0x24420000,  # addiu v0,v0,0
            0x00041880,  # sll v1,a0,2
            0x00621821,  # addu v1,v1,v0
            0x8C620000,  # lw v0,0(v1)
            NOP,
            0x00400008, NOP,  # jr v0: a call carve stops here
            jump(f + 0x3C), 0x24020001,  # case 0: j default; li v0,1
            jump(f + 0x3C), 0x24020002,  # case 1: j default; li v0,2
            JR_RA, NOP,  # default
            JR_RA, NOP,  # the next admitted function
        )

    def test_carve_cut_at_its_switch_dispatch_owns_the_cases(self) -> None:
        f = BASE + 8
        code = self.switch(f + 0x34)
        extents = [FunctionExtent(f, f + 0x2C), FunctionExtent(f + 0x44, f + 0x4C)]
        self.assertEqual(switch_extension(code, extents[0], f + 0x44), f + 0x44)
        rows = [row for row in audit_image("T.EXE", code, extents, {})
                if row["confidence"] == "switch-extend"]
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["va"], f"0x{f + 0x2C:08x}")
        self.assertEqual(rows[0]["owner_va"], f"0x{f:08x}")
        self.assertEqual(rows[0]["return_va"], f"0x{f + 0x3C:08x}")

    def test_switch_case_outside_the_gap_does_not_extend(self) -> None:
        f = BASE + 8
        code = self.switch(f + 0x44)  # case 1 is the next function
        self.assertIsNone(switch_extension(code, FunctionExtent(f, f + 0x2C), f + 0x44))
        ends_at_return = words(JR_RA, NOP, JR_RA, NOP)
        self.assertIsNone(switch_extension(ends_at_return, FunctionExtent(BASE, BASE + 8),
                                           BASE + 0x10))

    def test_switch_extend_grows_the_owner_and_cuts_the_gap(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            config = Path(tmp)
            write_tsv(config / "functions.tsv", FUNCTION_FIELDS, [{
                "image": "VS.EXE", "va": "0x80020000", "file_offset": "0xf800",
                "size": "0x2c", "body_size": "0x2c", "fragments": 1, "kind": "function",
                "confidence": "mips-call-carve", "name": "", "labels": "",
                "provenance": "scripts/kf/audit_functions.py", "note": "carve",
            }], [])
            write_tsv(config / "data.tsv", DATA_FIELDS, [{
                "image": "VS.EXE", "va": "0x8002002c", "file_offset": "0xf82c",
                "size": "0x18", "kind": "unclassified", "confidence": "coverage-gap",
                "name": "", "datatype": "", "provenance": "payload-complement", "note": "",
            }], [])
            candidate = {
                "image": "VS.EXE", "va": "0x8002002c", "owner_va": "0x80020000",
                "return_va": "0x8002003c", "confidence": "switch-extend", "note": "cases",
            }
            functions = proposed_functions(config, [candidate])
            self.assertEqual(len(functions), 1)
            self.assertEqual((functions[0]["size"], functions[0]["body_size"]),
                             ("0x44", "0x44"))
            self.assertEqual(proposed_data(config, [candidate]), [])

    def test_call_target_after_fallthrough_code_is_an_interior_entry(self) -> None:
        code = words(0x20A70060, 0x24060000, 0x8CE80048, NOP, JR_RA, NOP)
        self.assertIsNone(fallthrough_boundary(code, BASE + 8))


if __name__ == "__main__":
    unittest.main()
