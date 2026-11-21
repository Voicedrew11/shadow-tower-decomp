"""Validate the hand-owned retail executable censuses."""

from __future__ import annotations

import argparse
import csv
import hashlib
import io
import os
import re
import struct
import tomllib
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


PSX_EXE_HEADER_SIZE = 0x800
ARCHIVE_SECTOR_SIZE = 0x800


@dataclass(frozen=True)
class ImageLayout:
    """One linked image: a PS-X EXE (0x800-byte header) or a headerless overlay.

    `entry` is the PS-X EXE header pc0; overlays have none.
    """

    name: str
    sha256: str
    file_size: int
    entry: int | None
    load_address: int
    load_size: int
    header_size: int = PSX_EXE_HEADER_SIZE

    @property
    def load_end(self) -> int:
        return self.load_address + self.load_size

    def contains(self, address: int, size: int = 1) -> bool:
        return size >= 0 and self.load_address <= address and address + size <= self.load_end

    def file_offset(self, address: int) -> int:
        return self.header_size + address - self.load_address


@dataclass(frozen=True)
class RetailDisc:
    serial: str
    title: str
    sha256: str


@dataclass(frozen=True)
class RetailArchive:
    """A FromSoftware `.T` archive: u16 entry count, then u16 sector offsets."""

    name: str
    disc_path: str
    sha256: str
    file_size: int


@dataclass(frozen=True)
class OverlaySource:
    """Where a headerless overlay image is read from, and the program it extends."""

    archive: str
    index: int
    host: str


def image_key(image: str) -> str:
    """The lowercase `--image` stem of an image name (`ST.EXE` -> `st`)."""
    return image.rsplit(".", 1)[0].lower()


def _load_images(path: Path) -> tuple[RetailDisc, dict[str, ImageLayout], dict[str, str]]:
    """Read the per-game disc and image table from `config/images.toml`.

    The layouts cover root PS-X EXEs and archive overlays; the disc-file table
    lists only the root executables (the files a disc rebuild replaces).
    """
    with path.open("rb") as stream:
        table = tomllib.load(stream)
    disc = RetailDisc(**table["disc"])
    layouts: dict[str, ImageLayout] = {}
    disc_files: dict[str, str] = {}
    for row in table["image"]:
        name = row["name"]
        if name in layouts or not name.endswith(".EXE"):
            raise ValueError(f"{path}: invalid or duplicate image {name!r}")
        layouts[name] = ImageLayout(
            name, row["sha256"], row["file_size"], row["entry"],
            row["load_address"], row["load_size"],
        )
        disc_files[name] = row["disc_file"]
    for row in table.get("overlay", ()):
        name = row["name"]
        if name in layouts or row["host"] not in disc_files:
            raise ValueError(f"{path}: invalid or duplicate overlay {name!r}")
        layouts[name] = ImageLayout(
            name, row["sha256"], row["load_size"], None,
            row["load_address"], row["load_size"], 0,
        )
    if not layouts:
        raise ValueError(f"{path}: no images")
    if len({image_key(name) for name in layouts}) != len(layouts):
        raise ValueError(f"{path}: image keys are not unique")
    return disc, layouts, disc_files


def _load_overlays(path: Path) -> tuple[dict[str, RetailArchive], dict[str, OverlaySource]]:
    """Read the archives and the overlay images carried inside them."""
    with path.open("rb") as stream:
        table = tomllib.load(stream)
    archives = {row["name"]: RetailArchive(**row) for row in table.get("archive", ())}
    sources: dict[str, OverlaySource] = {}
    for row in table.get("overlay", ()):
        if row["archive"] not in archives:
            raise ValueError(f"{path}: overlay {row['name']!r} names an unknown archive")
        sources[row["name"]] = OverlaySource(row["archive"], row["index"], row["host"])
    return archives, sources


IMAGES_CONFIG = Path(__file__).resolve().parents[2] / "config" / "images.toml"
RETAIL_DISC, IMAGE_LAYOUTS, IMAGE_DISC_FILES = _load_images(IMAGES_CONFIG)
RETAIL_ARCHIVES, OVERLAY_SOURCES = _load_overlays(IMAGES_CONFIG)


def _load_gp(path: Path) -> dict[str, int]:
    """Each image's startup $gp; an overlay runs with its host's."""
    with path.open("rb") as stream:
        table = tomllib.load(stream)
    result = {row["name"]: row["gp"] for row in table["image"] if "gp" in row}
    for row in table.get("overlay", ()):
        if row["host"] in result:
            result[row["name"]] = result[row["host"]]
    return result


IMAGE_GP = _load_gp(IMAGES_CONFIG)


def referent_image(image: str, va: int) -> str:
    """The image whose inventories own an address that `image`'s code references.

    An overlay extends its host program at run time: its calls and data
    references outside its own load window reach the host's functions, data
    and BSS (an FDAT overlay calls into ST.EXE). Those referents keep the
    host's identities. Every other address, and every address seen from a
    root program, belongs to the referencing image itself.
    """
    source = OVERLAY_SOURCES.get(image)
    if source is None or IMAGE_LAYOUTS[image].contains(va):
        return image
    return source.host
IMAGE_ORDER = {image: index for index, image in enumerate(IMAGE_LAYOUTS)}
IMAGE_KEYS = tuple(image_key(image) for image in IMAGE_LAYOUTS)
IMAGE_NAMES = {image_key(image): image for image in IMAGE_LAYOUTS}


FUNCTION_FIELDS = (
    "image",
    "va",
    "file_offset",
    "size",
    "body_size",
    "fragments",
    "kind",
    "confidence",
    "name",
    "labels",
    "provenance",
    "note",
)
DATA_FIELDS = (
    "image",
    "va",
    "file_offset",
    "size",
    "kind",
    "confidence",
    "name",
    "datatype",
    "provenance",
    "note",
)
RELOC_FIELDS = (
    "image",
    "site_va",
    "site_file_offset",
    "paired_site_va",
    "kind",
    "channel",
    "target_va",
    "target_region",
    "target_name",
    "opcode",
    "register",
    "confidence",
    "status",
    "provenance",
)
VENDORED_FUNCTION_FIELDS = (
    "image",
    "va",
    "size",
    "name",
    "aliases",
    "provider",
    "library",
    "module",
    "member_offset",
    "source_version",
    "evidence",
    "confidence",
    "provenance",
    "note",
)

# Confidence names carry the SDK release they were proven against: `sdk42`
# for the supplied archives, `psyq420` for a ghidra_psx_ldr signature set.
# `relink-map-sdk42` marks a member the byte-identical native relink's PSYLINK
# map places: too short or too static for a function ID (END.EXE StSetMask,
# STR.EXE's and TITLE.EXE's relink-placed LIBSND/LIBSPU/LIBPAD members).
VENDORED_CONFIDENCE = re.compile(
    r"fid-sdk\d+(?:-ambiguous)?"
    r"|exact-sdk\d+(?:-short|-ambiguous|-complete)?"
    r"|psyq\d{3,4}-signature(?:-ambiguous)?"
    r"|relink-map-sdk\d+"
    r"|sdk-lineage-supported"
)


def parse_int(value: str) -> int:
    return int(value, 0)


def format_hex(value: int) -> str:
    return f"0x{value:08x}"


def format_size(value: int) -> str:
    return f"0x{value:x}"


def sha256_path(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            digest.update(chunk)
    return digest.hexdigest()


def read_tsv(path: Path) -> tuple[tuple[str, ...], list[dict[str, str]]]:
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    if not lines:
        raise ValueError(f"{path}: missing TSV header")
    reader = csv.DictReader(lines, delimiter="\t")
    fields = tuple(reader.fieldnames or ())
    return fields, list(reader)


def write_tsv(
    path: Path,
    fields: Iterable[str],
    rows: Iterable[dict[str, object]],
    comments: Iterable[str],
) -> None:
    stream = io.StringIO(newline="")
    for comment in comments:
        stream.write(f"# {comment}\n")
    writer = csv.DictWriter(
        stream,
        tuple(fields),
        delimiter="\t",
        lineterminator="\n",
        extrasaction="raise",
    )
    writer.writeheader()
    writer.writerows(rows)
    content = stream.getvalue()
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    # Parallel build edges (one delink per image) may rewrite the same
    # generated file; a per-process temporary keeps the replace atomic.
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    temporary.write_text(content, encoding="utf-8", newline="")
    temporary.replace(path)


def parse_psx_exe(path: Path) -> ImageLayout:
    data = path.read_bytes()
    if len(data) < 0x800 or data[:8] != b"PS-X EXE":
        raise ValueError(f"{path}: not a PS-X EXE")
    entry, _gp, load_address, load_size = struct.unpack_from("<4I", data, 0x10)
    if len(data) < 0x800 + load_size:
        raise ValueError(f"{path}: truncated load image")
    return ImageLayout(
        path.name.upper(), sha256_path(path), len(data), entry, load_address, load_size
    )


def archive_entry(data: bytes, index: int) -> bytes:
    """Return entry `index` of a `.T` archive (sector-granular, padding included)."""
    if len(data) < 2:
        raise ValueError("truncated .T archive")
    count = struct.unpack_from("<H", data, 0)[0]
    if not 0 <= index < count or len(data) < 2 + 2 * (index + 2):
        raise ValueError(f".T archive has no entry {index}")
    start, end = (
        value * ARCHIVE_SECTOR_SIZE for value in struct.unpack_from("<2H", data, 2 + 2 * index)
    )
    if not 0 < start < end <= len(data):
        raise ValueError(f".T archive entry {index} has invalid extent {start:#x}..{end:#x}")
    return data[start:end]


def overlay_bytes(archive: bytes, image: str) -> bytes:
    """Slice one configured overlay image out of its verified archive bytes."""
    source = OVERLAY_SOURCES[image]
    if hashlib.sha256(archive).hexdigest() != RETAIL_ARCHIVES[source.archive].sha256:
        raise ValueError(f"{source.archive}: archive SHA-256 mismatch")
    data = archive_entry(archive, source.index)
    if hashlib.sha256(data).hexdigest() != IMAGE_LAYOUTS[image].sha256:
        raise ValueError(f"{image}: {source.archive} entry {source.index} SHA-256 mismatch")
    return data


def verify_retail_image(path: Path, image: str) -> bytes:
    """Return the file bytes of one retail image after checking its identity."""
    layout = IMAGE_LAYOUTS[image]
    if image in OVERLAY_SOURCES:
        if not path.is_file():
            raise ValueError(f"{path}: missing retail overlay")
        data = path.read_bytes()
        if len(data) != layout.file_size or hashlib.sha256(data).hexdigest() != layout.sha256:
            raise ValueError(f"{path}: retail identity/layout mismatch")
        return data
    if parse_psx_exe(path) != layout:
        raise ValueError(f"{path}: retail identity/layout mismatch")
    return path.read_bytes()


def load_payload(path: Path, image: str) -> tuple[int, bytes]:
    """Return the verified load address and load-image bytes of one image."""
    layout = IMAGE_LAYOUTS[image]
    data = verify_retail_image(path, image)
    return layout.load_address, data[layout.header_size:layout.header_size + layout.load_size]


def _require_fields(path: Path, actual: tuple[str, ...], expected: tuple[str, ...]) -> None:
    if actual != expected:
        raise ValueError(f"{path}: fields {actual!r}, expected {expected!r}")


def _require_sorted(path: Path, rows: list[dict[str, str]], key) -> None:
    keys = [key(row) for row in rows]
    if keys != sorted(keys):
        raise ValueError(f"{path}: rows are not canonically sorted")


def _validate_image(row: dict[str, str], path: Path) -> ImageLayout:
    image = row["image"]
    try:
        return IMAGE_LAYOUTS[image]
    except KeyError as error:
        raise ValueError(f"{path}: unknown image {image!r}") from error


def _merged_coverage(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    merged: list[tuple[int, int]] = []
    for start, end in sorted(intervals):
        if start >= end:
            continue
        if merged and start <= merged[-1][1]:
            merged[-1] = (merged[-1][0], max(merged[-1][1], end))
        else:
            merged.append((start, end))
    return merged


def validate_config(config_dir: Path, exe_dir: Path | None = None) -> dict[str, int]:
    function_path = config_dir / "functions.tsv"
    data_path = config_dir / "data.tsv"
    reloc_path = config_dir / "relocs.tsv"
    function_fields, functions = read_tsv(function_path)
    data_fields, data = read_tsv(data_path)
    reloc_fields, relocs = read_tsv(reloc_path)
    _require_fields(function_path, function_fields, FUNCTION_FIELDS)
    _require_fields(data_path, data_fields, DATA_FIELDS)
    _require_fields(reloc_path, reloc_fields, RELOC_FIELDS)

    _require_sorted(
        function_path,
        functions,
        lambda row: (IMAGE_ORDER[row["image"]], parse_int(row["va"])),
    )
    _require_sorted(
        data_path,
        data,
        lambda row: (IMAGE_ORDER[row["image"]], parse_int(row["va"]), row["kind"]),
    )
    _require_sorted(
        reloc_path,
        relocs,
        lambda row: (
            IMAGE_ORDER[row["image"]],
            parse_int(row["site_va"]),
            parse_int(row["paired_site_va"]) if row["paired_site_va"] else -1,
            row["kind"],
            parse_int(row["target_va"]),
        ),
    )

    starts: set[tuple[str, int]] = set()
    functions_by_start: dict[tuple[str, int], dict[str, str]] = {}
    function_intervals: dict[str, list[tuple[int, int]]] = {
        image: [] for image in IMAGE_LAYOUTS
    }
    coverage: dict[str, list[tuple[int, int]]] = {image: [] for image in IMAGE_LAYOUTS}
    for row in functions:
        layout = _validate_image(row, function_path)
        address = parse_int(row["va"])
        size = parse_int(row["size"])
        body_size = parse_int(row["body_size"])
        fragments = int(row["fragments"])
        if address & 3:
            raise ValueError(f"{function_path}: unaligned function {row['image']} {row['va']}")
        if not layout.contains(address, size) or size <= 0:
            raise ValueError(f"{function_path}: invalid extent {row!r}")
        if body_size <= 0 or body_size > size or fragments <= 0:
            raise ValueError(f"{function_path}: invalid body metrics {row!r}")
        if parse_int(row["file_offset"]) != layout.file_offset(address):
            raise ValueError(f"{function_path}: wrong file offset {row!r}")
        key = (row["image"], address)
        if key in starts:
            raise ValueError(f"{function_path}: duplicate start {key!r}")
        starts.add(key)
        functions_by_start[key] = row
        function_intervals[row["image"]].append((address, address + size))
        coverage[row["image"]].append((address, address + size))

    for image, layout in IMAGE_LAYOUTS.items():
        if layout.entry is not None and (image, layout.entry) not in starts:
            raise ValueError(f"{function_path}: missing {image} entry {layout.entry:#x}")
        previous_end = -1
        for start, end in sorted(function_intervals[image]):
            if start < previous_end:
                raise ValueError(
                    f"{function_path}: overlapping {image} function at {start:#x}"
                )
            previous_end = end

    data_keys: set[tuple[str, int, str]] = set()
    data_intervals: dict[str, list[tuple[int, int]]] = {
        image: [] for image in IMAGE_LAYOUTS
    }
    for row in data:
        layout = _validate_image(row, data_path)
        address = parse_int(row["va"])
        size = parse_int(row["size"])
        if not layout.contains(address, size) or size <= 0:
            raise ValueError(f"{data_path}: invalid extent {row!r}")
        if parse_int(row["file_offset"]) != layout.file_offset(address):
            raise ValueError(f"{data_path}: wrong file offset {row!r}")
        key = (row["image"], address, row["kind"])
        if key in data_keys:
            raise ValueError(f"{data_path}: duplicate row {key!r}")
        data_keys.add(key)
        data_intervals[row["image"]].append((address, address + size))
        coverage[row["image"]].append((address, address + size))

    for image, intervals in data_intervals.items():
        previous_end = -1
        for start, end in sorted(intervals):
            if start < previous_end:
                raise ValueError(f"{data_path}: overlapping {image} data at {start:#x}")
            previous_end = end

    for image, layout in IMAGE_LAYOUTS.items():
        merged = _merged_coverage(coverage[image])
        if merged != [(layout.load_address, layout.load_end)]:
            raise ValueError(
                f"{config_dir}: {image} payload is not fully accounted for: {merged!r}"
            )

    reloc_keys: set[tuple[object, ...]] = set()
    for row in relocs:
        layout = _validate_image(row, reloc_path)
        site = parse_int(row["site_va"])
        target = parse_int(row["target_va"])
        paired = parse_int(row["paired_site_va"]) if row["paired_site_va"] else None
        if site & 3 or not layout.contains(site, 4):
            raise ValueError(f"{reloc_path}: invalid site {row!r}")
        if parse_int(row["site_file_offset"]) != layout.file_offset(site):
            raise ValueError(f"{reloc_path}: wrong site file offset {row!r}")
        if paired is not None and (paired & 3 or not layout.contains(paired, 4)):
            raise ValueError(f"{reloc_path}: invalid paired site {row!r}")
        key = (row["image"], site, paired, row["kind"], target)
        if key in reloc_keys:
            raise ValueError(f"{reloc_path}: duplicate candidate {key!r}")
        reloc_keys.add(key)
        if row["status"] not in {"candidate", "reviewed", "rejected"}:
            raise ValueError(f"{reloc_path}: invalid status {row['status']!r}")
        if (
            row["status"] != "rejected"
            and row["kind"] == "mips26"
            and row["opcode"] == "jal"
            and row["channel"] == "reachable-code"
            and row["target_region"] == "load"
            # An overlay's call into its host names the host's function.
            and (referent_image(row["image"], target), target) not in starts
        ):
            raise ValueError(
                f"{reloc_path}: reachable call target lacks a function row {row!r}"
            )

    if exe_dir is not None:
        for name in IMAGE_LAYOUTS:
            verify_retail_image(exe_dir / name, name)

    counts = {
        "functions": len(functions),
        "data": len(data),
        "relocs": len(relocs),
    }
    vendored_path = config_dir / "functions_vendored.tsv"
    if vendored_path.is_file():
        vendored_fields, vendored = read_tsv(vendored_path)
        _require_fields(vendored_path, vendored_fields, VENDORED_FUNCTION_FIELDS)
        _require_sorted(
            vendored_path,
            vendored,
            lambda row: (IMAGE_ORDER[row["image"]], parse_int(row["va"])),
        )
        vendored_keys: set[tuple[str, int]] = set()
        for row in vendored:
            layout = _validate_image(row, vendored_path)
            address = parse_int(row["va"])
            size = parse_int(row["size"])
            key = (row["image"], address)
            if key in vendored_keys:
                raise ValueError(f"{vendored_path}: duplicate provider row {key!r}")
            vendored_keys.add(key)
            try:
                structural = functions_by_start[key]
            except KeyError as error:
                raise ValueError(
                    f"{vendored_path}: provider row lacks structural function {key!r}"
                ) from error
            if size != parse_int(structural["size"]):
                raise ValueError(
                    f"{vendored_path}: size differs from structural function {row!r}"
                )
            if not layout.contains(address, size):
                raise ValueError(f"{vendored_path}: invalid extent {row!r}")
            if parse_int(row["member_offset"]) < 0:
                raise ValueError(f"{vendored_path}: negative member offset {row!r}")
            for field in (
                "provider", "library", "module", "source_version", "evidence",
                "confidence", "provenance", "note",
            ):
                if not row[field]:
                    raise ValueError(f"{vendored_path}: empty {field} in {row!r}")
            if VENDORED_CONFIDENCE.fullmatch(row["confidence"]) is None:
                raise ValueError(
                    f"{vendored_path}: invalid confidence {row['confidence']!r}"
                )
            names = [row["name"], *row["aliases"].split(";")]
            nonempty_names = [name for name in names if name]
            if len(nonempty_names) != len(set(nonempty_names)):
                raise ValueError(f"{vendored_path}: duplicate names in {row!r}")
        counts["vendored_functions"] = len(vendored)
    if (config_dir / "data_contributions.tsv").is_file():
        from scripts.kf.config_data import load as load_data_contributions

        counts["data_contributions"] = len(load_data_contributions(config_dir))
    return counts


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "config_dir",
        nargs="?",
        type=Path,
        default=Path("config/retail"),
    )
    parser.add_argument("--exe-dir", type=Path)
    args = parser.parse_args()
    counts = validate_config(args.config_dir, args.exe_dir)
    print(
        "retail census valid: "
        + ", ".join(f"{name}={count}" for name, count in counts.items())
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
