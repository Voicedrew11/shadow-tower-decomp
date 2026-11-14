"""Source launcher controls using a synthetic Mode 2 ISO9660 disc."""

import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

from scripts.kf.retail import IMAGE_DISC_FILES
from scripts.psxbuild import disc


# Each configured program occupies two sectors from LBA 18, then two asset sectors.
IMAGES = tuple(IMAGE_DISC_FILES)
DISC_FILES = tuple(IMAGE_DISC_FILES.values())
PROGRAM_LBAS = {name: 18 + 2 * index for index, name in enumerate(DISC_FILES)}
ASSET_LBA = 18 + 2 * len(DISC_FILES)
TOTAL_SECTORS = ASSET_LBA + 2


def directory_record(name, lba, size, *, directory=False):
    record = bytearray(33 + len(name) + (len(name) % 2 == 0))
    record[0] = len(record)
    struct.pack_into('<I', record, 2, lba)
    struct.pack_into('>I', record, 6, lba)
    struct.pack_into('<I', record, 10, size)
    struct.pack_into('>I', record, 14, size)
    record[25] = 2 if directory else 0
    record[32] = len(name)
    record[33:33 + len(name)] = name
    return record


def synthetic_disc():
    raw = bytearray(TOTAL_SECTORS * disc.SECTOR_SIZE)
    for lba in range(TOTAL_SECTORS):
        raw[lba * disc.SECTOR_SIZE + 15] = 2
    pvd = bytearray(2048)
    pvd[:7] = b'\x01CD001\x01'
    struct.pack_into('<H', pvd, 128, 2048)
    pvd[156:190] = directory_record(b'\0', 17, 2048, directory=True)
    start = 16 * disc.SECTOR_SIZE + 24
    raw[start:start + 2048] = pvd
    records = b''.join(directory_record(name.encode() + b';1', lba, 4096)
                       for name, lba in PROGRAM_LBAS.items())
    start = 17 * disc.SECTOR_SIZE + 24
    raw[start:start + len(records)] = records
    return bytes(raw)


class SourceDiscControls(unittest.TestCase):
    def test_retail_mode_uses_environment_disc_without_executables(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            original = synthetic_disc()
            path = root / 'original.bin'
            path.write_bytes(original)
            with mock.patch.dict('os.environ', {'KF_RETAIL_DISC': str(path),
                                              'XDG_CACHE_HOME': str(root / 'cache')}), \
                    mock.patch.object(disc, 'RETAIL_DISC_SHA256', hashlib.sha256(original).hexdigest()):
                self.assertEqual(disc.main(['--retail', '--prepare-only']), 0)
                self.assertEqual(disc.main(['--executables', str(root / 'missing-build'),
                                            '--retail', '--prepare-only']), 0)
            cues = list((root / 'cache').rglob('game.cue'))
            self.assertEqual(len(cues), 1)
            self.assertEqual(disc._cue_bin(cues[0]), path)
            self.assertEqual(path.read_bytes(), original)
            self.assertFalse(list((root / 'cache').rglob('*.bin')))

    def test_extents_are_read_from_iso_directory(self):
        self.assertEqual(disc.iso_files(synthetic_disc()),
                         {name: (lba, 4096) for name, lba in PROGRAM_LBAS.items()})

    def test_all_programs_replaced_original_unchanged_and_cache_reusable(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            original = synthetic_disc()
            input_path = root / 'original disc.bin'
            input_path.write_bytes(original)
            programs = root / 'programs'
            programs.mkdir()
            for index, name in enumerate(IMAGES):
                (programs / name).write_bytes(b'PS-X EXE' + bytes([index + 1]) * (2048 - 8))
            with mock.patch.object(disc, 'RETAIL_DISC_SHA256', hashlib.sha256(original).hexdigest()):
                cue = disc.prepare(input_path, programs, root / 'cache')
                output = cue.with_suffix('.bin')
                stamp = output.stat().st_mtime_ns
                self.assertEqual(disc.prepare(input_path, programs, root / 'cache'), cue)
                self.assertEqual(output.stat().st_mtime_ns, stamp)
            self.assertEqual(input_path.read_bytes(), original)
            rebuilt = output.read_bytes()
            files = disc.iso_files(original)
            for image, disc_file in IMAGE_DISC_FILES.items():
                lba, _ = files[disc_file]
                start = lba * disc.SECTOR_SIZE
                sector = bytearray(rebuilt[start:start + disc.SECTOR_SIZE])
                self.assertEqual(sector[24:2072], (programs / image).read_bytes())
                control = bytearray(sector)
                disc.regenerate_sector(control)
                self.assertEqual(control, sector)
                next_start = start + disc.SECTOR_SIZE
                self.assertEqual(rebuilt[next_start + 24:next_start + 2072], bytes(2048))
            self.assertEqual(rebuilt[:18 * disc.SECTOR_SIZE], original[:18 * disc.SECTOR_SIZE])

    def test_rejects_wrong_disc_and_missing_program(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            original = synthetic_disc()
            path = root / 'input.bin'
            path.write_bytes(original)
            with self.assertRaisesRegex(ValueError, 'SHA-256'):
                disc.prepare(path, root, root / 'cache')
            with mock.patch.object(disc, 'RETAIL_DISC_SHA256', hashlib.sha256(original).hexdigest()):
                with self.assertRaisesRegex(ValueError, 'missing'):
                    disc.prepare(path, root, root / 'cache')
            self.assertFalse((root / 'cache').exists())

    def test_larger_executable_moves_to_new_sectors_without_moving_assets(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            original = synthetic_disc()
            path = root / 'input.bin'
            path.write_bytes(original)
            for name in IMAGES:
                (root / name).write_bytes(b'PS-X EXE' + bytes(2048 - 8))
            large = b'PS-X EXE' + b'X' * 5000
            grown_image, grown = IMAGES[1], DISC_FILES[1]
            (root / grown_image).write_bytes(large)
            with mock.patch.object(disc, 'RETAIL_DISC_SHA256', hashlib.sha256(original).hexdigest()):
                cue = disc.prepare(path, root, root / 'cache')
            rebuilt = cue.with_suffix('.bin').read_bytes()
            files = disc.iso_files(rebuilt, with_records=True)
            lba, size, record = files[grown]
            self.assertEqual((lba, size), (TOTAL_SECTORS, len(large)))
            self.assertEqual(struct.unpack_from('>I', rebuilt, record + 6)[0], lba)
            self.assertEqual(struct.unpack_from('>I', rebuilt, record + 14)[0], size)
            for name, program_lba in PROGRAM_LBAS.items():
                if name != grown:
                    self.assertEqual(files[name][:2], (program_lba, 4096))
            self.assertEqual(rebuilt[ASSET_LBA * disc.SECTOR_SIZE:TOTAL_SECTORS * disc.SECTOR_SIZE],
                             original[ASSET_LBA * disc.SECTOR_SIZE:TOTAL_SECTORS * disc.SECTOR_SIZE])
            payload = b''.join(rebuilt[n * disc.SECTOR_SIZE + 24:n * disc.SECTOR_SIZE + 2072]
                               for n in range(TOTAL_SECTORS, TOTAL_SECTORS + 3))
            self.assertEqual(payload[:size], large)
            self.assertEqual(payload[size:], bytes(len(payload) - size))
            for number in (16, 17, TOTAL_SECTORS, TOTAL_SECTORS + 1, TOTAL_SECTORS + 2):
                sector = bytearray(rebuilt[number * disc.SECTOR_SIZE:(number + 1) * disc.SECTOR_SIZE])
                control = bytearray(sector)
                disc.regenerate_sector(control)
                self.assertEqual(control, sector)
            self.assertEqual(struct.unpack_from('<I', rebuilt, 16 * disc.SECTOR_SIZE + 24 + 80)[0], TOTAL_SECTORS + 3)
            self.assertEqual(path.read_bytes(), original)

    def test_cue_relative_path_with_spaces_and_ambiguous_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'my disc.bin').write_bytes(b'disc')
            cue = root / 'disc.cue'
            cue.write_text('FILE "my disc.bin" BINARY\n  TRACK 01 MODE2/2352\n')
            self.assertEqual(disc.disc_path(cue), root / 'my disc.bin')
            self.assertEqual(disc.disc_path(root), root / 'my disc.bin')
            (root / 'another.cue').write_text(cue.read_text())
            with self.assertRaises(ValueError):
                disc.disc_path(root)


if __name__ == '__main__':
    unittest.main()
