import struct
import unittest
from types import SimpleNamespace

from scripts.kf.data_match import (
    Allocation,
    Section,
    _place_proven_commons,
    _proven_issue,
    linked_common_proof,
)
from scripts.kf.linked_symbols import read_sym
from scripts.kf.mips_elf import SHF_MIPS_GPREL


def _record(address: int, kind: int, payload: bytes = b"") -> bytes:
    return struct.pack("<IB", address, kind) + payload


def _name(text: str) -> bytes:
    return bytes([len(text)]) + text.encode("latin-1")


def _claim(symbol: str, va: int, size: int):
    return SimpleNamespace(symbol=symbol, va=va, size=size,
                           section_name=".bss", scope="global")


def _unit(*claims):
    return SimpleNamespace(data=list(claims))


def _recon(*requests):
    return SimpleNamespace(common_requests=[
        SimpleNamespace(name=name, size=size) for name, size in requests])


class ReadSymTests(unittest.TestCase):
    def test_reads_symbols_across_debug_records(self) -> None:
        data = b"MND\x01\x00\x00\x00\x00"
        data += _record(0x80011000, 1, _name("main"))
        data += _record(0, 0x88, struct.pack("<I", 7) + _name("MAIN.C"))
        data += _record(0x80011000, 0x8C, bytes(20) + _name("C:\\MAIN.C") + _name("main"))
        data += _record(0, 0x96, bytes(8) + struct.pack("<H", 1) + bytes(4)
                        + _name("TAG") + _name("x"))
        data += _record(0x800A1EF4, 2, _name("disp_db"))
        self.assertEqual(read_sym(data), {"main": 0x80011000, "disp_db": 0x800A1EF4})

    def test_rejects_unknown_records(self) -> None:
        with self.assertRaisesRegex(ValueError, "unknown SYM record type 0x6e"):
            read_sym(b"MND\x01\x00\x00\x00\x00" + _record(0, 0x6E))


class LinkedCommonProofTests(unittest.TestCase):
    def test_every_request_at_its_claim_is_proven(self) -> None:
        unit = _unit(_claim("a", 0x100, 4), _claim("b", 0x200, 8))
        proof = linked_common_proof(unit, _recon(("a", 8), ("b", 8)),
                                    {"a": 0x100, "b": 0x200})
        self.assertEqual(proof, {"a", "b"})

    def test_one_misplaced_request_proves_nothing(self) -> None:
        unit = _unit(_claim("a", 0x100, 4), _claim("b", 0x200, 8))
        proof = linked_common_proof(unit, _recon(("a", 8), ("b", 8)),
                                    {"a": 0x100, "b": 0x208})
        self.assertEqual(proof, frozenset())

    def test_unclaimed_or_short_request_or_stale_link_proves_nothing(self) -> None:
        unit = _unit(_claim("a", 0x100, 8))
        self.assertFalse(linked_common_proof(unit, _recon(("c", 8)), {"c": 0x100}))
        self.assertFalse(linked_common_proof(unit, _recon(("a", 4)), {"a": 0x100}))
        self.assertFalse(linked_common_proof(unit, _recon(("a", 8)), None))

    def test_small_common_claims_in_sbss_are_proven_the_same_way(self) -> None:
        small = SimpleNamespace(symbol="w", va=0x80063304, size=4,
                                section_name=".sbss", scope="global")
        unit = _unit(small)
        self.assertEqual(linked_common_proof(unit, _recon(("w", 4)), {"w": 0x80063304}),
                         {"w"})
        self.assertFalse(linked_common_proof(unit, _recon(("w", 4)), {"w": 0x80063308}))
        self.assertFalse(linked_common_proof(unit, _recon(("w", 4)), None))
        self.assertTrue(_proven_issue("reconstruction", {
            "kind": "owned-symbol-layout", "symbol": "w", "actual_section": "SHN_COMMON",
            "expected_section": ".sbss", "actual_size": 4, "expected_size": 4},
            frozenset({"w"})))

    def test_proven_small_common_is_placed_beside_fixed_sbss(self) -> None:
        def claim(symbol, va, size, section):
            return SimpleNamespace(symbol=symbol, va=va, size=size,
                                   section_name=section, scope="global")

        unit = _unit(claim("shake", 0x800881D0, 4, ".sbss"),
                     claim("hole", 0x800881D4, 4, ".sbss"),
                     claim("w", 0x800881D8, 4, ".sbss"),
                     claim("big", 0x80090000, 8, ".bss"),
                     claim("tail", 0x80090008, 8, ".bss"))
        recon = SimpleNamespace(
            common_requests=[SimpleNamespace(name=name, size=size) for name, size in
                             (("hole", 4), ("w", 4), ("tail", 8))],
            sections={".sbss": Section(".sbss", 8, 4, b"", 3 | SHF_MIPS_GPREL),
                      ".bss": Section(".bss", 8, 8, b"", 3)},
            allocations={".sbss": [Allocation("shake", 0, 4, 0, 0)],
                         ".bss": [Allocation("big", 0, 8, 1, 0)]})
        _place_proven_commons(unit, recon, frozenset({"hole", "w", "tail"}))
        self.assertEqual(recon.sections[".sbss"].size, 12)
        self.assertEqual(recon.sections[".sbss"].flags, 3 | SHF_MIPS_GPREL)
        self.assertEqual(recon.allocations[".sbss"], [
            Allocation("shake", 0, 4, 0, 0), Allocation("hole", 4, 4, 1, 0),
            Allocation("w", 8, 4, 1, 0)])
        self.assertEqual(recon.sections[".bss"].size, 16)
        self.assertEqual(recon.allocations[".bss"], [
            Allocation("big", 0, 8, 1, 0), Allocation("tail", 8, 8, 1, 0)])

    def test_unproven_or_unclaimed_fixed_sbss_is_left_alone(self) -> None:
        unit = _unit(SimpleNamespace(symbol="w", va=0x800881D8, size=4,
                                     section_name=".sbss", scope="global"))
        section = Section(".sbss", 8, 4, b"", 3 | SHF_MIPS_GPREL)
        recon = SimpleNamespace(
            common_requests=[SimpleNamespace(name="w", size=4)],
            sections={".sbss": section},
            allocations={".sbss": [Allocation("other", 0, 4, 0, 0)]})
        _place_proven_commons(unit, recon, frozenset({"w"}))
        self.assertIs(recon.sections[".sbss"], section)
        _place_proven_commons(unit, recon, frozenset())
        self.assertEqual(recon.allocations[".sbss"], [Allocation("other", 0, 4, 0, 0)])

    def test_only_common_restatements_are_waived(self) -> None:
        proven = frozenset({"a"})
        self.assertTrue(_proven_issue("reconstruction", {
            "kind": "unresolved-common-placement", "symbol": "a"}, proven))
        self.assertTrue(_proven_issue("reconstruction", {
            "kind": "owned-symbol-layout", "symbol": "a", "actual_section": "SHN_COMMON",
            "expected_section": ".bss", "actual_size": 8, "expected_size": 4}, proven))
        self.assertTrue(_proven_issue("target", {
            "kind": "conflicting-section-bases", "section": ".bss",
            "claims": [{"symbol": "a"}]}, proven))
        self.assertFalse(_proven_issue("target", {
            "kind": "conflicting-section-bases", "section": ".bss",
            "claims": [{"symbol": "a"}, {"symbol": "b"}]}, proven))
        self.assertFalse(_proven_issue("reconstruction", {
            "kind": "owned-symbol-layout", "symbol": "a", "actual_section": ".data",
            "expected_section": ".bss", "actual_size": 8, "expected_size": 4}, proven))
        self.assertFalse(_proven_issue("reconstruction", {
            "kind": "unresolved-common-placement", "symbol": "b"}, proven))


if __name__ == "__main__":
    unittest.main()
