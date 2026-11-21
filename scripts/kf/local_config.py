"""Resolve and initialize machine-local retail executable configuration."""

from __future__ import annotations

import hashlib
import json
import os
import struct
import tomllib
from pathlib import Path

from scripts.kf.paths import LOCAL_CONFIG
from scripts.kf.retail import (
    IMAGE_DISC_FILES,
    IMAGE_LAYOUTS,
    OVERLAY_SOURCES,
    RETAIL_ARCHIVES,
    RETAIL_DISC,
    overlay_bytes,
    sha256_path,
    verify_retail_image,
)


def validate_retail_dir(path: Path) -> Path:
    path = path.expanduser().resolve()
    for image in IMAGE_LAYOUTS:
        executable = path / image
        if not executable.is_file():
            raise ValueError(f"{executable}: missing retail image")
        verify_retail_image(executable, image)
    return path


def materialize_overlays(path: Path) -> int:
    """Write each missing overlay image from its archive in `path`.

    Overlays are slices of a hash-checked archive, so this only derives files
    that `validate_retail_dir` then checks by their own SHA-256.
    """
    path = path.expanduser().resolve()
    archives: dict[str, bytes] = {}
    written = 0
    for image, source in OVERLAY_SOURCES.items():
        target = path / image
        if target.is_file():
            continue
        if source.archive not in archives:
            archive = path / source.archive
            if not archive.is_file():
                raise ValueError(f"{archive}: missing retail archive for {image}")
            archives[source.archive] = archive.read_bytes()
        target.write_bytes(overlay_bytes(archives[source.archive], image))
        written += 1
    return written


def _disc_binary(disc: Path) -> Path:
    """Resolve a single-track `.cue` to its `.bin`, or accept the `.bin` itself."""
    disc = disc.expanduser().resolve()
    if disc.suffix.casefold() != ".cue":
        return disc
    from scripts.kf.emulator import _cue_bin

    return _cue_bin(disc)


def _iso_file(raw: bytes, disc_path: str) -> tuple[int, int]:
    """Find `DIR/.../NAME` in the disc's ISO9660 tree; return (lba, size)."""
    from scripts.psxbuild.disc import PAYLOAD_OFFSET, PAYLOAD_SIZE, SECTOR_SIZE

    def payload(lba: int, size: int) -> bytes:
        if (lba + (size + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE) * SECTOR_SIZE > len(raw):
            raise ValueError("ISO9660 extent exceeds disc")
        return b"".join(
            raw[n * SECTOR_SIZE + PAYLOAD_OFFSET:n * SECTOR_SIZE + PAYLOAD_OFFSET + PAYLOAD_SIZE]
            for n in range(lba, lba + (size + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE)
        )[:size]

    pvd = payload(16, PAYLOAD_SIZE)
    if pvd[:7] != b"\x01CD001\x01":
        raise ValueError("expected an ISO9660 primary volume descriptor")
    extent = struct.unpack_from("<I", pvd, 158)[0], struct.unpack_from("<I", pvd, 166)[0]
    parts = disc_path.upper().split("/")
    for depth, part in enumerate(parts):
        directory = payload(*extent)
        found = None
        position = 0
        while position < len(directory):
            size = directory[position]
            if not size:
                position = (position // PAYLOAD_SIZE + 1) * PAYLOAD_SIZE
                continue
            record = directory[position:position + size]
            name = record[33:33 + record[32]].decode("ascii", "replace").split(";")[0].upper()
            is_directory = bool(record[25] & 2)
            if name == part and is_directory == (depth < len(parts) - 1):
                found = struct.unpack_from("<I", record, 2)[0], struct.unpack_from("<I", record, 10)[0]
                break
            position += size
        if found is None:
            raise ValueError(f"{disc_path}: missing from the disc")
        extent = found
    return extent


def extract_disc(disc: Path, output: Path) -> Path:
    """Copy each configured image off a verified MODE2/2352 disc.

    Root programs are written under their image names (the boot file becomes
    `PSX.EXE`); archives are copied whole and their overlay images sliced out.
    That is the layout `kf init --retail-dir` validates.
    """
    from scripts.psxbuild.disc import PAYLOAD_OFFSET, PAYLOAD_SIZE, SECTOR_SIZE, iso_files

    binary = _disc_binary(disc)
    if not binary.is_file():
        raise ValueError(f"{binary}: disc image is missing")
    if sha256_path(binary) != RETAIL_DISC.sha256:
        raise ValueError(f"{binary}: disc SHA-256 does not match {RETAIL_DISC.serial}")
    raw = binary.read_bytes()
    files = iso_files(raw)

    def read_extent(lba: int, size: int) -> bytes:
        sectors = (size + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE
        return b"".join(
            raw[(lba + n) * SECTOR_SIZE + PAYLOAD_OFFSET:(lba + n) * SECTOR_SIZE + PAYLOAD_OFFSET + PAYLOAD_SIZE]
            for n in range(sectors)
        )[:size]

    output = output.expanduser().resolve()
    output.mkdir(parents=True, exist_ok=True)
    for image, disc_file in IMAGE_DISC_FILES.items():
        if disc_file not in files:
            raise ValueError(f"{disc_file}: missing from the disc root")
        (output / image).write_bytes(read_extent(*files[disc_file][:2]))
    for archive in RETAIL_ARCHIVES.values():
        data = read_extent(*_iso_file(raw, archive.disc_path))
        if len(data) != archive.file_size or hashlib.sha256(data).hexdigest() != archive.sha256:
            raise ValueError(f"{archive.disc_path}: archive identity mismatch")
        (output / archive.name).write_bytes(data)
        for image, source in OVERLAY_SOURCES.items():
            if source.archive == archive.name:
                (output / image).write_bytes(overlay_bytes(data, image))
    return validate_retail_dir(output)


def configured_retail_dir(cli_path: Path | None = None, *, validate: bool = True) -> Path:
    candidate: Path | None = cli_path
    if candidate is None and os.environ.get("KF_RETAIL_DIR"):
        candidate = Path(os.environ["KF_RETAIL_DIR"])
    if candidate is None and LOCAL_CONFIG.is_file():
        with LOCAL_CONFIG.open("rb") as stream:
            data = tomllib.load(stream)
        value = data.get("retail", {}).get("dir")
        if value:
            candidate = Path(str(value))
    if candidate is None:
        raise ValueError(
            "retail directory is not configured; run `kf init --retail-dir PATH` "
            "or set KF_RETAIL_DIR"
        )
    return validate_retail_dir(candidate) if validate else candidate.expanduser().resolve()


def initialize(path: Path) -> Path:
    materialize_overlays(path)
    retail = validate_retail_dir(path)
    content = "[retail]\n" + f"dir = {json.dumps(str(retail))}\n"
    if LOCAL_CONFIG.is_file() and LOCAL_CONFIG.read_text(encoding="utf-8") == content:
        return retail
    LOCAL_CONFIG.parent.mkdir(parents=True, exist_ok=True)
    temporary = LOCAL_CONFIG.with_name(f".{LOCAL_CONFIG.name}.tmp")
    temporary.write_text(content, encoding="utf-8")
    temporary.replace(LOCAL_CONFIG)
    return retail
