"""Controls for $gp-relative (R_MIPS_GPREL16) relocations across the tools."""

from __future__ import annotations

import struct
import unittest
from pathlib import Path

from scripts.kf.delink import (
    Catalog,
    Function,
    MipsRelocation,
    _apply_relocation,
    _module_load_relocations,
)
from scripts.kf.lnk import convert_patch
from scripts.kf.relocations import decode_gprel16_target
from scripts.kf.retail import IMAGE_GP
from scripts.kf.roundtrip import link_command

PSX_GP = 0x80010394


def word(value: int) -> bytearray:
    return bytearray(struct.pack("<I", value))


class GpRelativeTests(unittest.TestCase):
    def test_decode_accepts_loads_stores_addiu_and_cop2_off_gp(self) -> None:
        self.assertEqual(IMAGE_GP["PSX.EXE"], PSX_GP)
        self.assertEqual(decode_gprel16_target("PSX.EXE", 0x8F820004), PSX_GP + 4)  # lw v0,4(gp)
        self.assertEqual(decode_gprel16_target("PSX.EXE", 0xAF82FFFC), PSX_GP - 4)  # sw v0,-4(gp)
        self.assertEqual(decode_gprel16_target("PSX.EXE", 0xCB800008), PSX_GP + 8)  # lwc2 0,8(gp)
        with self.assertRaises(ValueError):
            decode_gprel16_target("PSX.EXE", 0x8C820004)  # lw v0,4(a0)
        with self.assertRaises(ValueError):
            decode_gprel16_target("NOPE.EXE", 0x8F820004)

    def test_delinker_emits_gprel16_and_module_rewrites_to_sdata_offset(self) -> None:
        function = Function("PSX.EXE", 0x80010068, 8, 8, 1, "main", "test", "test")
        catalog = Catalog(
            functions={"PSX.EXE": (function,)},
            function_starts={"PSX.EXE": {function.va: function}},
            data={"PSX.EXE": ()},
        )
        blob = word(0x8F820004) + word(0)
        row = {
            "image": "PSX.EXE", "site_va": "0x80010068", "paired_site_va": "",
            "kind": "mips_gprel16", "channel": "reachable-code",
            "target_va": f"{PSX_GP + 4:#x}", "target_region": "load",
            "target_name": "next_program", "opcode": "lw", "confidence": "manual",
            "status": "reviewed",
        }
        relocations, used = _apply_relocation(blob, function, row, catalog, "safe")
        self.assertEqual(relocations, [MipsRelocation(0, "R_MIPS_GPREL16", "next_program")])
        self.assertEqual(struct.unpack_from("<I", blob)[0], 0x8F820000)
        self.assertEqual(used["action"], "gp-relative")
        rewritten = _module_load_relocations(blob, relocations, {"next_program": 4}, ".sdata")
        self.assertEqual(rewritten, [MipsRelocation(0, "R_MIPS_GPREL16", ".sdata")])
        self.assertEqual(struct.unpack_from("<I", blob)[0], 0x8F820004)

    def test_native_patch_100_is_gprel16(self) -> None:
        self.assertEqual(convert_patch(100, 0x8F820000, 4), ("R_MIPS_GPREL16", 0x8F820004))
        with self.assertRaises(ValueError):
            convert_patch(99, 0, 0)

    def test_target_relink_defines_the_image_gp(self) -> None:
        command = link_command("ld", "PSX.EXE", 0x80010068, Path("r.ld"), Path("o.elf"),
                               Path("t.o"), {"ext": 0x80010150})
        self.assertIn(f"--defsym=_gp={PSX_GP:#x}", command)
        self.assertIn("--defsym=ext=0x80010150", command)
        self.assertFalse(any(arg.startswith("--defsym=_gp") for arg in link_command(
            "ld", "UNKNOWN.EXE", 0, Path("r.ld"), Path("o.elf"), Path("t.o"), {})))


if __name__ == "__main__":
    unittest.main()
