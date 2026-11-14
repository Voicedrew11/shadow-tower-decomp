"""Resolve and initialize machine-local retail executable configuration."""

from __future__ import annotations

import json
import os
import tomllib
from pathlib import Path

from scripts.kf.paths import LOCAL_CONFIG
from scripts.kf.retail import IMAGE_DISC_FILES, IMAGE_LAYOUTS, RETAIL_DISC, parse_psx_exe, sha256_path


def validate_retail_dir(path: Path) -> Path:
    path = path.expanduser().resolve()
    for image, expected in IMAGE_LAYOUTS.items():
        executable = path / image
        if not executable.is_file():
            raise ValueError(f"{executable}: missing retail executable")
        if parse_psx_exe(executable) != expected:
            raise ValueError(f"{executable}: retail identity/layout mismatch")
    return path


def _disc_binary(disc: Path) -> Path:
    """Resolve a single-track `.cue` to its `.bin`, or accept the `.bin` itself."""
    disc = disc.expanduser().resolve()
    if disc.suffix.casefold() != ".cue":
        return disc
    from scripts.kf.emulator import _cue_bin

    return _cue_bin(disc)


def extract_disc(disc: Path, output: Path) -> Path:
    """Copy each configured image's root file off a verified MODE2/2352 disc.

    Files are written under their image names (the boot file becomes
    `PSX.EXE`), which is the layout `kf init --retail-dir` validates.
    """
    from scripts.psxbuild.disc import PAYLOAD_OFFSET, PAYLOAD_SIZE, SECTOR_SIZE, iso_files

    binary = _disc_binary(disc)
    if not binary.is_file():
        raise ValueError(f"{binary}: disc image is missing")
    if sha256_path(binary) != RETAIL_DISC.sha256:
        raise ValueError(f"{binary}: disc SHA-256 does not match {RETAIL_DISC.serial}")
    raw = binary.read_bytes()
    files = iso_files(raw)
    output = output.expanduser().resolve()
    output.mkdir(parents=True, exist_ok=True)
    for image, disc_file in IMAGE_DISC_FILES.items():
        if disc_file not in files:
            raise ValueError(f"{disc_file}: missing from the disc root")
        lba, size = files[disc_file][:2]
        sectors = (size + PAYLOAD_SIZE - 1) // PAYLOAD_SIZE
        data = b"".join(
            raw[(lba + n) * SECTOR_SIZE + PAYLOAD_OFFSET:(lba + n) * SECTOR_SIZE + PAYLOAD_OFFSET + PAYLOAD_SIZE]
            for n in range(sectors)
        )[:size]
        (output / image).write_bytes(data)
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
    retail = validate_retail_dir(path)
    content = "[retail]\n" + f"dir = {json.dumps(str(retail))}\n"
    if LOCAL_CONFIG.is_file() and LOCAL_CONFIG.read_text(encoding="utf-8") == content:
        return retail
    LOCAL_CONFIG.parent.mkdir(parents=True, exist_ok=True)
    temporary = LOCAL_CONFIG.with_name(f".{LOCAL_CONFIG.name}.tmp")
    temporary.write_text(content, encoding="utf-8")
    temporary.replace(LOCAL_CONFIG)
    return retail
