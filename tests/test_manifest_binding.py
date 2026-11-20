"""Name binding of shared sources against another image's identities."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.kf import retail
from scripts.kf.manifest import (
    _check_together_read_only, _rebind_claims_by_name, _rebind_data_claims_by_name,
)
from scripts.kf.model import Claim, DataClaim, DataIdentity, RodataClaim


def _datum(image: str, va: int, name: str) -> DataIdentity:
    fields = DataIdentity.__dataclass_fields__
    values = {
        "image": image,
        "va": va,
        "size": 4,
        "name": name,
        "datatype": "u32",
        "storage": "bss",
        "scope": "global",
    }
    return DataIdentity(**{key: values.get(key, "") for key in fields})


class NameBindingTests(unittest.TestCase):
    def test_functions_rebind_to_the_image_address(self) -> None:
        identities = {
            ("GAME.EXE", 0x8001AAB0): "memory_malloc_checked",
            ("OPEN.EXE", 0x80015DD4): "memory_malloc_checked",
        }
        claims = (Claim(0x8001AAB0, 0x38, "memory_malloc_checked", 12),)
        rebound = _rebind_claims_by_name(None, "OPEN.EXE", claims, identities)
        self.assertEqual(rebound[0].va, 0x80015DD4)
        self.assertEqual(
            (rebound[0].size, rebound[0].name, rebound[0].line),
            (0x38, "memory_malloc_checked", 12),
        )

    def test_unknown_or_ambiguous_names_are_rejected(self) -> None:
        identities = {("OPEN.EXE", 0x80015DD4): "memory_malloc_checked"}
        with self.assertRaises(ValueError):
            _rebind_claims_by_name(
                None, "OPEN.EXE", (Claim(0x1000, 4, "missing", 1),), identities
            )
        identities[("OPEN.EXE", 0x80015E0C)] = "memory_malloc_checked"
        with self.assertRaises(ValueError):
            _rebind_claims_by_name(
                None, "OPEN.EXE", (Claim(0x1000, 4, "memory_malloc_checked", 1),), identities
            )

    def test_data_rebinds_by_name(self) -> None:
        identities = {
            ("GAME.EXE", 0x800A01FC): _datum("GAME.EXE", 0x800A01FC, "memory_allocation_stack"),
            ("OPEN.EXE", 0x80075854): _datum("OPEN.EXE", 0x80075854, "memory_allocation_stack"),
        }
        claims = (DataClaim(0x800A01FC, 0x44, "memory_allocation_stack", 3, ".bss"),)
        rebound = _rebind_data_claims_by_name(None, "OPEN.EXE", claims, identities)
        self.assertEqual(rebound[0].va, 0x80075854)


class OverlayReadOnlyTests(unittest.TestCase):
    def test_overlay_units_claim_switch_tables_inside_rodata(self) -> None:
        tables = (RodataClaim(0x801EB730, 0x7C, 9),)
        with self.assertRaisesRegex(ValueError, "RODATA"):
            _check_together_read_only("map.c", "FDAT_002.BIN", tables)
        _check_together_read_only("map.c", "FDAT_002.BIN", ())
        _check_together_read_only("menu.c", "ST.EXE", tables)

    def test_a_program_that_keeps_read_only_together_claims_tables_inside_rodata(self) -> None:
        tables = (RodataClaim(0x800110A0, 0x1C, 9),)
        with mock.patch.dict(retail.IMAGE_READ_ONLY, {"VS.EXE": "together"}):
            self.assertEqual(retail.read_only_mode("VS.EXE"), "together")
            with self.assertRaisesRegex(ValueError, "RODATA"):
                _check_together_read_only("snd.c", "VS.EXE", tables)
        with mock.patch.dict(retail.IMAGE_READ_ONLY, {"VS.EXE": "separate"}):
            _check_together_read_only("snd.c", "VS.EXE", tables)

    def test_images_toml_selects_the_read_only_mode(self) -> None:
        base = ('[[image]]\nname = "A.EXE"\n{extra}\n'
                '[[archive]]\nname = "X.T"\n'
                '[[overlay]]\nname = "X_001.BIN"\narchive = "X.T"\nhost = "A.EXE"\n{overlay}\n')
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "images.toml"
            path.write_text(base.format(extra="", overlay=""))
            self.assertEqual(retail._load_read_only(path),
                             {"A.EXE": "separate", "X_001.BIN": "together"})
            path.write_text(base.format(extra='read_only = "together"', overlay=""))
            self.assertEqual(retail._load_read_only(path)["A.EXE"], "together")
            path.write_text(base.format(extra='read_only = "mixed"', overlay=""))
            with self.assertRaisesRegex(ValueError, "unknown read_only"):
                retail._load_read_only(path)
            path.write_text(base.format(extra="", overlay='read_only = "separate"'))
            with self.assertRaisesRegex(ValueError, "always keeps"):
                retail._load_read_only(path)

    def test_the_configured_images_use_their_retail_read_only_layout(self) -> None:
        self.assertEqual(retail.read_only_mode("ST.EXE"), "separate")
        self.assertEqual(retail.read_only_mode("FDAT_002.BIN"), "together")


class OverlaySourceBindingTests(unittest.TestCase):
    MANIFEST = """
[profiles.p]
language = "c"
compiler = "gcc272cdk-native"
optimization = "O2"
small_data = 0
aspsx_version = "2.79"
cc1_flags = []
"""

    # FDAT_322.BIN's shared handler fdat_801ed6e4, claimed by its name, with
    # the overlay's handler-table range.
    SOURCE = ('#include <kf/lib/address.h>\n'
              'RODATA(0x801eb560, 0x88)\n'
              'ADDRESS(0x801ed6e4, 0xb0)\n'
              'void fdat_801ed6e4(void)\n{\n}\n')

    def load(self, *units: tuple[str, str]):
        import tempfile
        from pathlib import Path

        from scripts.kf.manifest import load
        from scripts.kf.paths import BUILD, REPO

        root = BUILD / "test-manifest-binding"
        root.mkdir(parents=True, exist_ok=True)
        source = root / "overlay.c"
        source.write_text(self.SOURCE)
        relative = source.relative_to(REPO).as_posix()
        text = self.MANIFEST + "".join(
            f'\n[[unit]]\nunit = "{unit}"\nimage = "{image}"\nsource = "{relative}"\n'
            f'profile = "p"\nbind = "name"\n' for unit, image in units)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "units.toml"
            path.write_text(text)
            return load(path)

    def test_an_overlay_source_of_its_own_binds_by_name_with_its_rodata(self) -> None:
        manifest = self.load(("fdat_322.main", "FDAT_322.BIN"))
        unit = manifest.units[0]
        self.assertEqual(unit.rodata, (0x801EB560, 0x88))
        self.assertEqual([f.va for f in unit.functions], [0x801ED6E4])

    def test_a_shared_source_still_cannot_bind_its_ranges_by_name(self) -> None:
        with self.assertRaisesRegex(ValueError, "image-specific"):
            self.load(("fdat_322.main", "FDAT_322.BIN"), ("fdat_402.main", "FDAT_402.BIN"))


if __name__ == "__main__":
    unittest.main()
