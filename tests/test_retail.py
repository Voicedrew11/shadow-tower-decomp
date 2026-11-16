from __future__ import annotations

import hashlib
import struct
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from scripts.kf import retail
from scripts.kf.retail import (
    IMAGE_LAYOUTS,
    IMAGE_NAMES,
    OVERLAY_SOURCES,
    ImageLayout,
    _merged_coverage,
    archive_entry,
    image_key,
    parse_psx_exe,
    verify_retail_image,
)
from scripts.kf.propose_function_admission import subtract_interval
from scripts.kf.seed_retail import escape_trailing_spaces
from scripts.kf.seed_vendored_functions import (
    SignaturePattern,
    find_masked_bytes,
    find_signature,
    load_sdk_symbols,
    parse_object_symbols,
    parse_signature,
)


class RetailTests(unittest.TestCase):
    def test_subtract_interval(self) -> None:
        self.assertEqual(
            subtract_interval(0x100, 0x200, [(0x80, 0x110), (0x140, 0x160)]),
            [(0x110, 0x140), (0x160, 0x200)],
        )

    def test_escape_trailing_spaces_preserves_byte_count(self) -> None:
        self.assertEqual(escape_trailing_spaces("KF   "), r"KF\x20\x20\x20")
        self.assertEqual(escape_trailing_spaces("KF"), "KF")

    def test_merged_coverage(self) -> None:
        self.assertEqual(
            _merged_coverage([(8, 12), (0, 4), (3, 9), (20, 24)]),
            [(0, 12), (20, 24)],
        )

    def test_parse_psx_exe(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "TEST.EXE"
            data = bytearray(0x820)
            data[:8] = b"PS-X EXE"
            struct.pack_into("<4I", data, 0x10, 0x80010000, 0, 0x80010000, 0x20)
            path.write_bytes(data)
            parsed = parse_psx_exe(path)
        self.assertEqual(
            parsed,
            ImageLayout(
                "TEST.EXE",
                hashlib.sha256(data).hexdigest(),
                0x820,
                0x80010000,
                0x80010000,
                0x20,
            ),
        )

    def test_image_keys_strip_any_extension(self) -> None:
        self.assertEqual(image_key("ST.EXE"), "st")
        self.assertEqual(image_key("FDAT_012.BIN"), "fdat_012")
        self.assertEqual(IMAGE_NAMES["fdat_012"], "FDAT_012.BIN")

    def test_fdat_overlays_share_st_address_space_after_its_bss(self) -> None:
        self.assertEqual(len(OVERLAY_SOURCES), 36)
        for image, source in OVERLAY_SOURCES.items():
            layout = IMAGE_LAYOUTS[image]
            with self.subTest(image=image):
                self.assertEqual(image, f"FDAT_{source.index:03d}.BIN")
                self.assertEqual((source.archive, source.host), ("FDAT.T", "ST.EXE"))
                self.assertEqual(source.index % 10, 2)
                self.assertIsNone(layout.entry)
                self.assertEqual(layout.header_size, 0)
                self.assertEqual(layout.file_size, layout.load_size)
                self.assertEqual(layout.load_size % retail.ARCHIVE_SECTOR_SIZE, 0)
                self.assertEqual(layout.load_address, 0x801EB560)
                self.assertEqual(layout.file_offset(0x801EB564), 4)

    def test_archive_entry_slices_sector_offsets(self) -> None:
        archive = bytearray(0x1800)
        struct.pack_into("<4H", archive, 0, 2, 1, 2, 3)
        archive[0x800:0x1000] = b"A" * 0x800
        archive[0x1000:0x1800] = b"B" * 0x800
        self.assertEqual(archive_entry(bytes(archive), 1), b"B" * 0x800)
        with self.assertRaises(ValueError):
            archive_entry(bytes(archive), 2)

    def test_verify_overlay_checks_size_and_hash(self) -> None:
        payload = struct.pack("<I", 0x03E00008) * 0x200
        layout = ImageLayout(
            "OVL_001.BIN", hashlib.sha256(payload).hexdigest(), len(payload), None,
            0x801EB560, len(payload), 0,
        )
        source = retail.OverlaySource("OVL.T", 1, "ST.EXE")
        with (
            tempfile.TemporaryDirectory() as directory,
            mock.patch.dict(IMAGE_LAYOUTS, {"OVL_001.BIN": layout}),
            mock.patch.dict(OVERLAY_SOURCES, {"OVL_001.BIN": source}),
        ):
            path = Path(directory) / "OVL_001.BIN"
            path.write_bytes(payload)
            self.assertEqual(verify_retail_image(path, "OVL_001.BIN"), payload)
            self.assertEqual(
                retail.load_payload(path, "OVL_001.BIN"), (0x801EB560, payload)
            )
            path.write_bytes(payload[:-4] + bytes(4))
            with self.assertRaises(ValueError):
                verify_retail_image(path, "OVL_001.BIN")

    def test_parse_interleaved_object_symbols(self) -> None:
        parsed = parse_object_symbols(
            "\n".join((
                "16 : Section symbol number f000 '.text' in group 0 alignment 8",
                "16 : Section symbol number f001 '.data' in group 0 alignment 8",
                "6 : Switch to section f000",
                "2 : Code 12 bytes",
                "0000: 00 01 02 03 04 05 06 07 08 09 0a 0b",
                "6 : Switch to section f001",
                "2 : Code 4 bytes",
                "0000: aa bb cc dd",
                "6 : Switch to section f000",
                "2 : Code 8 bytes",
                "0000: 10 11 12 13 14 15 16 17",
                "10 : Patch type 74 at offset 0",
                "12 : XDEF symbol number 1 'first' at offset 0 in section f000",
                "12 : XDEF symbol number 2 'second' at offset c in section f000",
            ))
        )
        self.assertEqual(parsed.text_size, 20)
        self.assertEqual(parsed.relocation_count, 1)
        self.assertEqual(parsed.compared_bits, 134)
        self.assertEqual(parsed.xdefs, ((0, "first"), (12, "second")))

    def test_wildcard_signature_matching(self) -> None:
        data, mask = parse_signature("11 22 ?? 44 55 66 77 88 99 AA BB CC DD EE FF 00")
        pattern = SignaturePattern("LIB.LIB", "MEMBER.OBJ", data, mask, ())
        payload = b"pad!" + bytes.fromhex(
            "11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00"
        )
        self.assertEqual(find_signature(pattern, payload), [4])

    def test_masked_object_matching(self) -> None:
        data = bytes.fromhex("01 02 03 04 05 06 07 08 09 0a 0b 0c")
        mask = b"\xff" * 8 + b"\x00" * 4
        payload = b"xxxx" + data[:8] + b"WXYZ"
        self.assertEqual(find_masked_bytes(data, mask, payload), [4])

    def test_sdk_loader_accepts_standalone_object(self) -> None:
        listing = "\n".join((
            "16 : Section symbol number 1 '.text' in group 0 alignment 8",
            "6 : Switch to section 1",
            "2 : Code 8 bytes",
            "0000: 08 00 e0 03 00 00 00 00",
            "12 : XDEF symbol number 2 '__main' at offset 0 in section 1",
        ))
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "NONE2.OBJ"
            path.write_bytes(b"LNK\x02")
            with mock.patch(
                "scripts.kf.seed_vendored_functions.run_psyk",
                return_value=listing,
            ) as run:
                symbols = load_sdk_symbols(
                    Path("psyk"), Path(directory), (("NONE2.OBJ", "NONE2"),)
                )
        self.assertEqual(symbols[("NONE2.OBJ", "NONE2")].text_size, 8)
        run.assert_called_once_with(Path("psyk"), "list", "--code", str(path))


if __name__ == "__main__":
    unittest.main()
