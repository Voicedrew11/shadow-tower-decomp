"""Carve conservative MIPS ELF target objects from the three retail programs.

The PS-X EXE format contains linked bytes, not an object-file relocation table.
This tool therefore consumes the hand-owned WIP inventories in ``config/retail``
and records every relocation decision it makes.  The default ``safe`` policy is
deliberately narrower than the candidate census.
"""

from __future__ import annotations

import argparse
import bisect
import fcntl
import hashlib
import re
import struct
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from dataclasses import replace
from typing import Callable, Iterable, Sequence

from scripts.kf.mips_elf import (
    BSS_SECTION_SYMBOL,
    DATA_SECTION_SYMBOL,
    JUMPTABLE_SECTION_SYMBOL,
    RODATA_SECTION_SYMBOL,
    STB_GLOBAL,
    STB_LOCAL,
    STT_FUNC,
    STT_OBJECT,
    DefinedSymbol,
    MipsRelocation,
    SECTION_SYMBOL,
    write_mips_elf,
)
from scripts.kf.relocations import (
    ambiguous_sites,
    decode_hi_lo_target,
    decode_mips26_target,
    encode_hi_lo_addend,
    encode_mips26_addend,
    named_data_referent,
    validate_relocation,
)
from scripts.kf.retail import image_key as retail_image_key
from scripts.kf.retail import (
    IMAGE_LAYOUTS,
    format_hex,
    format_size,
    parse_int,
    read_tsv,
    referent_image,
    verify_retail_image,
    write_tsv,
)


# Working name for a module's anonymous small-data literal range (the first;
# later ranges add `_<index>`). It never reaches an object's symbol table:
# module packing rewrites every reference to the .sdata section symbol plus
# the range's packed offset, as ASPSX does for the compiler's local `$LC`
# labels.
SDATA_LITERAL_SYMBOL = "$sdata_literals"
SDATA_SECTION_SYMBOL = ".sdata"
SBSS_SECTION_SYMBOL = ".sbss"


def literal_symbol(index: int) -> str:
    return SDATA_LITERAL_SYMBOL if index == 0 else f"{SDATA_LITERAL_SYMBOL}_{index}"


def _ranges(value) -> tuple[tuple[int, int], ...]:
    """One (va, size) range or a sequence of them, as a tuple of ranges."""
    if not value:
        return ()
    if isinstance(value[0], int):
        return (tuple(value),)
    return tuple(tuple(item) for item in value)


@dataclass(frozen=True)
class Owned:
    """The anonymous ranges a module claims, and the symbol each resolves to.

    ``.rodata`` holds the unit's constants (RODATA), ``.rdata`` its switch
    tables (JUMPTABLES); each SDATA literal range has a working symbol that
    module packing turns into a ``.sdata`` offset.
    """

    rodata: tuple[int, int] | None = None
    sdata: tuple[tuple[int, int], ...] = ()
    jumptables: tuple[int, int] | None = None

    def resolve(self, address: int) -> tuple[str, int] | None:
        """(symbol, range start) of the range holding ``address``."""
        for symbol, extent in ((RODATA_SECTION_SYMBOL, self.rodata),
                               (JUMPTABLE_SECTION_SYMBOL, self.jumptables)):
            if extent is not None and extent[0] <= address < extent[0] + extent[1]:
                return symbol, extent[0]
        for index, (start, size) in enumerate(self.sdata):
            if start <= address < start + size:
                return literal_symbol(index), start
        return None

    def __bool__(self) -> bool:
        return bool(self.rodata or self.sdata or self.jumptables)


__all__ = (
    "decode_hi_lo_target",
    "decode_mips26_target",
    "encode_hi_lo_addend",
    "encode_mips26_addend",
)


OBJECT_FIELDS = (
    "image",
    "va",
    "size",
    "body_size",
    "name",
    "unit",
    "scope",
    "provider",
    "library",
    "object",
    "relocations",
    "data_size",
    "bss_size",
    "rodata_size",
    "confidence",
    "provenance",
)
USED_RELOCATION_FIELDS = (
    "image",
    "function_va",
    "site_va",
    "paired_site_va",
    "action",
    "relocation",
    "symbol",
    "addend",
    "source_kind",
    "source_channel",
    "confidence",
)
WITHHELD_RELOCATION_FIELDS = (
    "image",
    "function_va",
    "site_va",
    "paired_site_va",
    "kind",
    "channel",
    "target_va",
    "confidence",
    "status",
    "reason",
)
WITHHELD_FUNCTION_FIELDS = (
    "image",
    "va",
    "size",
    "body_size",
    "fragments",
    "name",
    "reason",
)

@dataclass(frozen=True)
class Function:
    image: str
    va: int
    size: int
    body_size: int
    fragments: int
    symbol: str
    confidence: str
    provenance: str
    provider: str = ""
    library: str = ""

    @property
    def end(self) -> int:
        return self.va + self.size

    def contains(self, address: int, size: int = 1) -> bool:
        return self.va <= address and address + size <= self.end

    @property
    def scope(self) -> str:
        return "vendored" if self.provider else "decomp"


@dataclass(frozen=True)
class DataObject:
    image: str
    va: int
    size: int
    symbol: str

    @property
    def end(self) -> int:
        return self.va + self.size


@dataclass(frozen=True)
class Datum:
    """A global a unit claims with DATA(); ``storage`` is load or bss."""

    va: int
    size: int
    symbol: str
    storage: str
    scope: str = ""
    section: str = ""
    reservation_size: int = 0

    @property
    def section_name(self) -> str:
        return self.section or (".data" if self.storage == "load" else ".bss")

    @property
    def end(self) -> int:
        return self.va + self.size

    @property
    def binding(self) -> int:
        return STB_LOCAL if self.scope == "static" else STB_GLOBAL

    @property
    def alignment(self) -> int:
        if self.reservation_size:
            # Native ASPSX fixed requests concatenate without internal alignment.
            return 1
        # Working packing constraint bounded by the retail address, not proof
        # of the original section alignment or allocation class. The pinned
        # GCC 2.5.7/maspsx path gives both local and exported tentative BSS
        # eight-byte allocation strides. Retail's menu-bank gap independently
        # confirms this for globals (game_menu_banks.md). Unknown linkage and
        # initialized data retain the narrower working constraint.
        candidates = (
            (8, 4, 2)
            if self.storage == "bss" and self.scope in {"static", "global"}
            else (4, 2)
        )
        for candidate in candidates:
            if self.va % candidate == 0:
                return candidate
        return 1


class _DataOwner:
    """Structural stand-in for a claimed datum while validating rows sited in it.

    ``contains`` always answers False so a target inside the datum resolves
    through the catalog to the datum's own symbol instead of a code section.
    """

    def __init__(self, image: str, datum: Datum) -> None:
        self.image = image
        self.va = datum.va
        self.size = datum.size

    def contains(self, address: int, size: int = 1) -> bool:
        return False


@dataclass(frozen=True)
class Catalog:
    functions: dict[str, tuple[Function, ...]]
    function_starts: dict[str, dict[int, Function]]
    data: dict[str, tuple[DataObject, ...]]


def sanitize_symbol(value: str, fallback: str) -> str:
    symbol = re.sub(r"[^0-9A-Za-z_$]", "_", value.strip())
    if not symbol:
        symbol = fallback
    if symbol[0].isdigit():
        symbol = f"_{symbol}"
    return symbol


def image_key(image: str) -> str:
    return retail_image_key(image)


def _put_word(data: bytearray, offset: int, word: int) -> None:
    struct.pack_into("<I", data, offset, word & 0xFFFFFFFF)


def _vendored_functions(
    config_dir: Path,
) -> dict[tuple[str, int], dict[str, str]]:
    functions: dict[tuple[str, int], dict[str, str]] = {}
    path = config_dir / "functions_vendored.tsv"
    if path.is_file():
        _, rows = read_tsv(path)
        for row in rows:
            functions[(row["image"], parse_int(row["va"]))] = row
    return functions


def _game_call_targets(
    config_dir: Path,
    function_rows: list[dict[str, str]],
    vendored: dict[tuple[str, int], dict[str, str]],
) -> dict[str, set[int]]:
    """Direct-call targets of admitted ``jal`` candidates sited in game code.

    The linked image resolves a duplicated library name to exactly one body.
    Calls from non-vendored functions are the retail evidence for which body
    that was; rejected rows and calls from inside vendored code do not count.
    """
    targets: dict[str, set[int]] = {image: set() for image in IMAGE_LAYOUTS}
    path = config_dir / "relocs.tsv"
    if not path.is_file():
        return targets
    extents: dict[str, list[tuple[int, int]]] = {image: [] for image in IMAGE_LAYOUTS}
    for row in function_rows:
        extents[row["image"]].append((parse_int(row["va"]), parse_int(row["size"])))
    for image in extents:
        extents[image].sort()
    _, rows = read_tsv(path)
    for row in rows:
        if row.get("kind") != "mips26" or row.get("opcode") != "jal":
            continue
        if row.get("status") == "rejected":
            continue
        image = row["image"]
        site = parse_int(row["site_va"])
        starts = extents.get(image, [])
        index = bisect.bisect_right(starts, (site, 0xFFFFFFFF)) - 1
        if index < 0:
            continue
        start, size = starts[index]
        if not start <= site < start + size or (image, start) in vendored:
            continue
        targets[image].add(parse_int(row["target_va"]))
    return targets


def _identity_names(config_dir: Path, filename: str) -> dict[tuple[str, int], str]:
    """Curated source-level names keyed by exact (image, va)."""
    path = config_dir / filename
    if not path.is_file():
        return {}
    _, rows = read_tsv(path)
    return {
        (row["image"], parse_int(row["va"])): row["name"]
        for row in rows
        if row.get("name")
    }


def load_catalog(config_dir: Path) -> Catalog:
    vendored = _vendored_functions(config_dir)
    _, function_rows = read_tsv(config_dir / "functions.tsv")
    # Reconstructed source spells labelled functions and data by their curated
    # identities, so target objects carry the same symbols. Address-derived
    # identities remain the explicit unresolved spelling.
    function_identities = _identity_names(config_dir, "function_identities.tsv")
    data_identities = _identity_names(config_dir, "data_identities.tsv")

    preferred: dict[tuple[str, int], str] = {}
    for row in function_rows:
        key = row["image"], parse_int(row["va"])
        provider_row = vendored.get(key, {})
        if key in vendored:
            name = row["name"] or provider_row.get("name", "")
        else:
            name = function_identities.get(key) or row["name"]
        preferred[key] = sanitize_symbol(name, f"func_{key[1]:08x}")
    duplicate_names = {
        image: Counter(
            name for (candidate_image, _), name in preferred.items()
            if candidate_image == image
        )
        for image in IMAGE_LAYOUTS
    }
    # Several bodies can share one library name (LIBAPI and LIBGPU both carry a
    # memset). Only the single instance that admitted game code calls keeps the
    # plain name; every other copy stays address-qualified so reconstructed
    # source can name the linked symbol without inventing one.
    game_calls = _game_call_targets(config_dir, function_rows, vendored)
    plain_duplicate: dict[tuple[str, str], int] = {}
    for (candidate_image, candidate_va), name in preferred.items():
        if duplicate_names[candidate_image][name] > 1 and candidate_va in game_calls[candidate_image]:
            plain_duplicate.setdefault((candidate_image, name), []).append(candidate_va)
    plain_duplicate = {
        key: called[0] for key, called in plain_duplicate.items() if len(called) == 1
    }

    functions: dict[str, list[Function]] = {image: [] for image in IMAGE_LAYOUTS}
    starts: dict[str, dict[int, Function]] = {image: {} for image in IMAGE_LAYOUTS}
    for row in function_rows:
        image = row["image"]
        va = parse_int(row["va"])
        provider_row = vendored.get((image, va), {})
        symbol = preferred[(image, va)]
        if duplicate_names[image][symbol] > 1 and plain_duplicate.get((image, symbol)) != va:
            symbol = f"{symbol}_{va:08x}"
        function = Function(
            image=image,
            va=va,
            size=parse_int(row["size"]),
            body_size=parse_int(row["body_size"]),
            fragments=int(row["fragments"]),
            symbol=symbol,
            confidence=row["confidence"],
            provenance=row["provenance"],
            provider=provider_row.get("provider", ""),
            library=provider_row.get("library", ""),
        )
        functions[image].append(function)
        starts[image][va] = function

    _, data_rows = read_tsv(config_dir / "data.tsv")
    data: dict[str, list[DataObject]] = {image: [] for image in IMAGE_LAYOUTS}
    data_starts: set[tuple[str, int]] = set()
    for row in data_rows:
        va = parse_int(row["va"])
        data_starts.add((row["image"], va))
        data[row["image"]].append(DataObject(
            image=row["image"],
            va=va,
            size=parse_int(row["size"]),
            symbol=sanitize_symbol(
                data_identities.get((row["image"], va)) or row["name"],
                f"DAT_{va:08x}",
            ),
        ))
    identity_path = config_dir / "data_identities.tsv"
    identity_rows = read_tsv(identity_path)[1] if identity_path.is_file() else []
    for row in identity_rows:
        image = row["image"]
        va = parse_int(row["va"])
        if row["storage"] != "bss" or (image, va) in data_starts:
            continue
        data[image].append(DataObject(
            image=image,
            va=va,
            size=parse_int(row["size"]),
            symbol=sanitize_symbol(row["name"], f"DAT_{va:08x}"),
        ))

    return Catalog(
        functions={image: tuple(sorted(rows, key=lambda item: item.va))
                   for image, rows in functions.items()},
        function_starts=starts,
        data={image: tuple(sorted(rows, key=lambda item: item.va))
              for image, rows in data.items()},
    )


def _containing_function(catalog: Catalog, image: str, address: int) -> Function | None:
    functions = catalog.functions[image]
    index = bisect.bisect_right([item.va for item in functions], address) - 1
    if index >= 0 and functions[index].contains(address):
        return functions[index]
    return None


def _containing_data(catalog: Catalog, image: str, address: int) -> DataObject | None:
    objects = catalog.data[image]
    index = bisect.bisect_right([item.va for item in objects], address) - 1
    if index >= 0 and objects[index].va <= address < objects[index].end:
        return objects[index]
    return None


def _resolve_symbol(
    catalog: Catalog,
    function: Function,
    target: int,
    target_name: str,
    owned: Owned | None = None,
    *,
    data_reference: bool = False,
) -> tuple[str, int]:
    # An overlay's references outside its own window name its host's
    # functions and data (retail.referent_image).
    owner_image = referent_image(function.image, target)
    named = named_data_referent(
        catalog, owner_image, sanitize_symbol(target_name, "")
    ) if target_name else None
    if data_reference and named is not None:
        # A biased data base can coincide with unrelated code or RODATA. Use
        # the reviewed allocation, then express unit-owned literals/tables
        # through their actual read-only section contribution.
        hit = owned.resolve(named.va) if owned else None
        if hit is not None:
            return hit[0], target - hit[1]
        return named.name, target - named.va
    if function.contains(target):
        return SECTION_SYMBOL, target - function.va
    hit = owned.resolve(target) if owned else None
    if hit is not None:
        # The owning module's constants, switch tables or small-data literals
        # compare as offsets into the section the compiler emitted them to;
        # module packing turns a literal's working symbol into a .sdata offset.
        return hit[0], target - hit[1]
    target_function = catalog.function_starts[owner_image].get(target)
    if target_function is not None:
        return target_function.symbol, 0
    if target_name:
        symbol = sanitize_symbol(target_name, f"DAT_{target:08x}")
        address_name = re.fullmatch(r"DAT_([0-9A-Fa-f]{8})", symbol)
        if address_name is not None:
            # An address-derived owner spelling names the datum, not its
            # symbol: a curated identity at that address wins so the target
            # carries the same name reconstructed source uses.
            owner_va = int(address_name.group(1), 16)
            if named is not None:
                return named.name, target - owner_va
            return symbol, target - owner_va
        # A reviewed row may name the owning datum even when the address lies
        # outside its extent: `&table[index - 1]` folds to `table - stride`.
        # The addend is measured from that owner, wherever the target lies.
        if named is not None:
            return symbol, target - named.va
        return symbol, 0
    target_data = _containing_data(catalog, owner_image, target)
    if target_data is not None:
        return target_data.symbol, target - target_data.va
    return f"DAT_{target:08x}", 0


def _withheld(row: dict[str, str], owner: Function | None, reason: str) -> dict[str, object]:
    return {
        "image": row["image"],
        "function_va": format_hex(owner.va) if owner else "",
        "site_va": row["site_va"],
        "paired_site_va": row["paired_site_va"],
        "kind": row["kind"],
        "channel": row["channel"],
        "target_va": row["target_va"],
        "confidence": row["confidence"],
        "status": row["status"],
        "reason": reason,
    }


def _competes_for_site(row: dict[str, str], policy: str) -> bool:
    if row["status"] == "rejected":
        return False
    if policy == "all":
        return True
    if policy == "reviewed":
        return row["status"] == "reviewed"
    return row["target_region"] == "load" or row["status"] == "reviewed"


def _merge_relocations(
    target: list[MipsRelocation], relocations: Iterable[MipsRelocation]
) -> None:
    """Add relocations, emitting a shared HI/LO half once.

    A `lui` that feeds several LO16s is one HI16; it is anchored at its
    earliest LO16 (its `pair`), the one ELF REL order must keep it ahead of.
    """
    for item in relocations:
        try:
            index = target.index(item)
        except ValueError:
            target.append(item)
            continue
        known = target[index]
        if item.pair is not None and (known.pair is None or item.pair < known.pair):
            target[index] = replace(known, pair=item.pair)


def _apply_relocation(
    blob: bytearray,
    function: Function,
    row: dict[str, str],
    catalog: Catalog,
    policy: str,
    rodata: tuple[int, int] | None = None,
    sdata=None,
    original: bytes | None = None,
    *,
    jumptables: tuple[int, int] | None = None,
) -> tuple[list[MipsRelocation], dict[str, object]]:
    # Rows of one shared HI/LO family validate against the linked bytes, not
    # the addend an earlier member of the family already wrote.
    validation = validate_relocation(
        blob if original is None else original, function, row, catalog, policy)
    target = validation.target
    offset = validation.offset
    symbol, addend = _resolve_symbol(
        catalog, function, target, row["target_name"],
        Owned(rodata, _ranges(sdata), jumptables),
        data_reference=(row["status"] == "reviewed"
                        and row["kind"] in {"mips_hi16_lo16", "mips32_candidate",
                                            "mips_gprel16"}),
    )

    if row["kind"] == "mips26":
        local_target = function.contains(target)
        _put_word(blob, offset, encode_mips26_addend(validation.word, addend))
        relocation = MipsRelocation(offset, "R_MIPS_26", symbol)
        action = "local-section" if local_target else "external-symbol"
        kinds = "R_MIPS_26"
        relocations = [relocation]
    elif row["kind"] == "mips_hi16_lo16":
        paired_offset = validation.paired_offset
        assert paired_offset is not None and validation.paired_word is not None
        hi_word, lo_word = encode_hi_lo_addend(
            validation.word,
            validation.paired_word,
            addend,
        )
        _put_word(blob, offset, hi_word)
        _put_word(blob, paired_offset, lo_word)
        relocations = [
            MipsRelocation(offset, "R_MIPS_HI16", symbol, paired_offset),
            MipsRelocation(paired_offset, "R_MIPS_LO16", symbol),
        ]
        action = "paired-symbol"
        kinds = "R_MIPS_HI16+R_MIPS_LO16"
    elif row["kind"] == "mips_gprel16":
        if not -0x8000 <= addend < 0x8000:
            raise ValueError("gprel-addend-out-of-range")
        _put_word(blob, offset, (validation.word & 0xFFFF0000) | (addend & 0xFFFF))
        relocations = [MipsRelocation(offset, "R_MIPS_GPREL16", symbol)]
        action = "gp-relative"
        kinds = "R_MIPS_GPREL16"
    elif row["kind"] == "mips32_candidate" and (
        policy == "all" or row["status"] == "reviewed"
    ):
        _put_word(blob, offset, addend)
        relocations = [MipsRelocation(offset, "R_MIPS_32", symbol)]
        action = "raw-word"
        kinds = "R_MIPS_32"
    else:
        raise ValueError("unsupported-relocation-kind")

    used = {
        "image": function.image,
        "function_va": format_hex(function.va),
        "site_va": row["site_va"],
        "paired_site_va": row["paired_site_va"],
        "action": action,
        "relocation": kinds,
        "symbol": symbol,
        "addend": format_hex(addend) if addend >= 0 else f"-{format_hex(-addend)}",
        "source_kind": row["kind"],
        "source_channel": row["channel"],
        "confidence": row["confidence"],
    }
    return relocations, used


def _comments(description: str, policy: str) -> tuple[str, ...]:
    return (
        "GENERATED - do not curate this build artifact by hand.",
        description,
        f"relocation policy: {policy}",
        "Inputs remain the manually curated WIP TSVs in config/retail.",
    )


def _write_bytes_if_changed(path: Path, content: bytes) -> None:
    if path.is_file() and path.read_bytes() == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.tmp")
    temporary.write_bytes(content)
    temporary.replace(path)


@dataclass(frozen=True)
class Module:
    """A manifested unit: a contiguous function run or claimed data only."""

    image: str
    unit: str
    stem: str
    vas: tuple[int, ...]
    data: tuple[Datum, ...] = ()
    rodata: tuple[int, int] | None = None
    # SDATA literal ranges, ascending; one (va, size) pair is accepted too.
    sdata: tuple[tuple[int, int], ...] = ()
    jumptables: tuple[int, int] | None = None

    def __post_init__(self) -> None:
        object.__setattr__(self, "sdata", _ranges(self.sdata))

    @property
    def owned(self) -> Owned:
        return Owned(self.rodata, self.sdata, self.jumptables)

    @property
    def object_name(self) -> str:
        start = self.vas[0] if self.vas else self.data[0].va
        return f"{start:08x}_{self.stem}.o"

    @property
    def text_start(self) -> int:
        return self.vas[0] if self.vas else self.data[0].va

    def owns_rodata(self, address: int) -> bool:
        return self.rodata is not None and self.rodata[0] <= address < self.rodata[0] + self.rodata[1]

    def owns_jumptables(self, address: int) -> bool:
        return (self.jumptables is not None
                and self.jumptables[0] <= address < self.jumptables[0] + self.jumptables[1])


def _get_word(blob: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", blob, offset)[0]


def _rebase_section_relocations(
    blob: bytearray, relocations: list[MipsRelocation], offset: int
) -> None:
    """Add ``offset`` to every implicit addend that is relative to ``.text``."""
    pending_hi: MipsRelocation | None = None
    for item in relocations:
        if item.symbol != SECTION_SYMBOL:
            continue
        if item.kind == "R_MIPS_26":
            word = _get_word(blob, item.offset)
            addend = (word & 0x03FFFFFF) << 2
            _put_word(blob, item.offset, encode_mips26_addend(word, addend + offset))
        elif item.kind == "R_MIPS_HI16":
            pending_hi = item
        elif item.kind == "R_MIPS_LO16" and pending_hi is not None:
            hi_word = _get_word(blob, pending_hi.offset)
            lo_word = _get_word(blob, item.offset)
            addend = decode_hi_lo_target(hi_word, lo_word)
            new_hi, new_lo = encode_hi_lo_addend(hi_word, lo_word, addend + offset)
            _put_word(blob, pending_hi.offset, new_hi)
            _put_word(blob, item.offset, new_lo)
            pending_hi = None


def _rebase_hi_lo(
    blob: bytearray,
    relocations: list[MipsRelocation],
    offsets: dict[str, int],
    section_symbol: str,
    what: str,
) -> list[MipsRelocation]:
    """Move HI16/LO16 families against `offsets` symbols to `section_symbol`.

    A HI16 pairs with the LO16s of its symbol that follow it, up to a later
    HI16 of that symbol once a LO16 has been seen: one `lui` may feed several
    signed lows, and consecutive `lui`s on converging paths feed the same low.
    A HI16 with no following LO16 shares the nearest preceding one. Each pair
    is re-encoded from the original words, so a shared half is written once
    with the same value.
    """
    result = list(relocations)
    original = {item.offset: _get_word(blob, item.offset) for item in relocations
                if item.kind in {"R_MIPS_HI16", "R_MIPS_LO16"}}
    paired_lows: set[int] = set()
    for index, high in enumerate(relocations):
        if high.kind != "R_MIPS_HI16" or high.symbol not in offsets:
            continue
        lows = []
        for later in range(index + 1, len(relocations)):
            item = relocations[later]
            if item.symbol != high.symbol:
                continue
            if item.kind == "R_MIPS_HI16" and lows:
                break
            if item.kind == "R_MIPS_LO16":
                lows.append(later)
        if not lows:
            lows = [earlier for earlier in range(index - 1, -1, -1)
                    if relocations[earlier].kind == "R_MIPS_LO16"
                    and relocations[earlier].symbol == high.symbol][:1]
        if not lows:
            raise ValueError(f"unpaired {what} HI16: {high.symbol}")
        hi_word = original[high.offset]
        for low_index in lows:
            low = relocations[low_index]
            lo_word = original[low.offset]
            addend = decode_hi_lo_target(hi_word, lo_word) + offsets[high.symbol]
            new_hi, new_lo = encode_hi_lo_addend(hi_word, lo_word, addend)
            _put_word(blob, high.offset, new_hi)
            _put_word(blob, low.offset, new_lo)
            result[low_index] = replace(low, symbol=section_symbol)
            paired_lows.add(low_index)
        result[index] = replace(high, symbol=section_symbol)
    for index, low in enumerate(relocations):
        if low.kind == "R_MIPS_LO16" and low.symbol in offsets and index not in paired_lows:
            raise ValueError(f"unpaired {what} LO16: {low.symbol}")
    return result


def _module_function_address_relocations(
    blob: bytearray,
    relocations: list[MipsRelocation],
    function_offsets: dict[str, int],
) -> list[MipsRelocation]:
    """Express same-unit function addresses through the joined .text section.

    The assembler emits section-relative relocations for references to a
    function defined in the same translation unit, including direct calls.
    """
    result = list(relocations)
    for index, high in enumerate(result):
        if high.kind == "R_MIPS_26" and high.symbol in function_offsets:
            word = _get_word(blob, high.offset)
            addend = (word & 0x03FFFFFF) << 2
            _put_word(blob, high.offset, encode_mips26_addend(
                word, addend + function_offsets[high.symbol]
            ))
            result[index] = replace(high, symbol=SECTION_SYMBOL)
            continue
    return _rebase_hi_lo(blob, result, function_offsets, SECTION_SYMBOL, "function-address")


def _module_function_data_relocations(
    blob: bytearray,
    relocations: list[MipsRelocation],
    function_offsets: dict[str, int],
) -> list[MipsRelocation]:
    """Rebase same-unit function pointers to the joined .text section."""
    result = list(relocations)
    for index, relocation in enumerate(result):
        if relocation.kind != "R_MIPS_32" or relocation.symbol not in function_offsets:
            continue
        _put_word(blob, relocation.offset,
                  _get_word(blob, relocation.offset) + function_offsets[relocation.symbol])
        result[index] = replace(relocation, symbol=SECTION_SYMBOL)
    return result


def _module_static_bss_relocations(
    blob: bytearray,
    relocations: list[MipsRelocation],
    offsets: dict[str, int],
    section_symbol: str = BSS_SECTION_SYMBOL,
) -> list[MipsRelocation]:
    """Use the BSS section for references to this module's static objects.

    The native assembler resolves a local BSS label through its section symbol;
    under -G a small `.lcomm` lands in .sbss and resolves through that section,
    including its `$gp`-relative accesses. The object's section offset must be
    added to the implicit MIPS addend.
    """
    result = list(relocations)
    for index, high in enumerate(result):
        if high.symbol not in offsets:
            continue
        offset = offsets[high.symbol]
        if high.kind == "R_MIPS_32":
            _put_word(blob, high.offset, _get_word(blob, high.offset) + offset)
            result[index] = replace(high, symbol=section_symbol)
        elif high.kind == "R_MIPS_GPREL16":
            word = _get_word(blob, high.offset)
            addend = (((word & 0xFFFF) ^ 0x8000) - 0x8000) + offset
            if not -0x8000 <= addend < 0x8000:
                raise ValueError(f"static small-data GPREL16 addend out of range: {high.symbol}")
            _put_word(blob, high.offset, (word & 0xFFFF0000) | (addend & 0xFFFF))
            result[index] = replace(high, symbol=section_symbol)
    return _rebase_hi_lo(blob, result, offsets, section_symbol, "static BSS")


def _module_load_relocations(
    blob: bytearray,
    relocations: list[MipsRelocation],
    offsets: dict[str, int],
    section_symbol: str,
) -> list[MipsRelocation]:
    """Use this module's initialized section for same-module data references.

    The pinned assembler resolves locally defined initialized data through
    its section symbol, even when the datum is exported. References to data
    owned by another module retain their named symbols.
    """
    result = list(relocations)
    for index, high in enumerate(result):
        if high.symbol not in offsets:
            continue
        offset = offsets[high.symbol]
        if high.kind == "R_MIPS_32":
            _put_word(blob, high.offset, _get_word(blob, high.offset) + offset)
            result[index] = replace(high, symbol=section_symbol)
        elif high.kind == "R_MIPS_GPREL16":
            # The implicit 16-bit addend becomes the datum's section offset.
            word = _get_word(blob, high.offset)
            low = word & 0xFFFF
            addend = (low - 0x10000 if low & 0x8000 else low) + offset
            if not -0x8000 <= addend < 0x8000:
                raise ValueError(f"GPREL16 addend out of range: {high.symbol}")
            _put_word(blob, high.offset, (word & 0xFFFF0000) | (addend & 0xFFFF))
            result[index] = replace(high, symbol=section_symbol)
    return _rebase_hi_lo(blob, result, offsets, section_symbol, "static load-data")


@dataclass(frozen=True)
class ModuleImage:
    """One module target object plus the sizes recorded in objects.tsv."""

    data: bytes
    relocations: list[MipsRelocation]
    size: int
    body_size: int
    data_size: int
    bss_size: int
    rodata_size: int = 0


def _module_rodata(
    module: Module, blob: bytes, text_size: int,
    reviewed: Sequence[MipsRelocation] = (),
) -> tuple[bytes, list[MipsRelocation]]:
    """Rebase a module's carved read-only range (its RODATA constants or its
    JUMPTABLES): in-module code pointers become .text-relative words and
    pointers into the module's own read-only ranges (a table of the unit's own
    string literals) section-relative words, each with an R_MIPS_32
    relocation. ``reviewed`` holds relocations already applied to ``blob`` from
    reviewed rows sited in the range (foreign pointers, such as an automatic
    aggregate initializer holding a global's field addresses); those words are
    kept as applied. Everything else (string literals, unreviewed foreign
    pointers) keeps its retail bytes."""
    start = module.text_start
    end = start + text_size
    read_only = Owned(module.rodata, (), module.jumptables)
    data = bytearray(blob)
    relocations: list[MipsRelocation] = list(reviewed)
    applied = {relocation.offset for relocation in reviewed}
    for offset in range(0, len(data) - 3, 4):
        if offset in applied:
            continue
        word = _get_word(data, offset)
        hit = read_only.resolve(word)
        if start <= word < end:
            _put_word(data, offset, word - start)
            relocations.append(MipsRelocation(offset, "R_MIPS_32", SECTION_SYMBOL))
        elif hit is not None:
            _put_word(data, offset, word - hit[1])
            relocations.append(MipsRelocation(offset, "R_MIPS_32", hit[0]))
    relocations.sort(key=lambda relocation: relocation.offset)
    return bytes(data), relocations


def _module_data(
    module: Module,
    data_blobs: dict[int, tuple[bytes, list[MipsRelocation]]],
    load_padding: Callable[[int, int], bytes] | None = None,
    *,
    section: str = ".data",
) -> tuple[bytes, list[DefinedSymbol], list[MipsRelocation], int, list[DefinedSymbol]]:
    """Pack one initialized section; the default call also lays out fixed BSS."""
    data = bytearray()
    data_symbols: list[DefinedSymbol] = []
    data_relocations: list[MipsRelocation] = []
    bss_size = 0
    bss_symbols: list[DefinedSymbol] = []
    previous_load_end: int | None = None
    for datum in module.data:
        if datum.storage == "load":
            if datum.section_name != section:
                continue
            blob, relocations = data_blobs[datum.va]
            if len(blob) != datum.size:
                raise ValueError(f"module {module.unit}: truncated data for {datum.symbol}")
            offset = (len(data) + datum.alignment - 1) & -datum.alignment
            padding_size = offset - len(data)
            if padding_size:
                padding_va = datum.va - padding_size
                if previous_load_end != padding_va:
                    raise ValueError(
                        f"module {module.unit}: packing before {datum.symbol} does not "
                        "describe the physical gap after the previous load datum")
                if load_padding is None:
                    raise ValueError(f"module {module.unit}: retail padding reader required")
                padding = load_padding(padding_va, padding_size)
                if len(padding) != padding_size:
                    raise ValueError(f"module {module.unit}: truncated initialized padding")
                data.extend(padding)
            data_relocations.extend(
                item.moved(offset)
                for item in relocations
            )
            data_symbols.append(
                DefinedSymbol(datum.symbol, offset, datum.size, STT_OBJECT, datum.binding)
            )
            data.extend(blob)
            previous_load_end = datum.end
        elif datum.section_name == (".bss" if section == ".data" else section):
            offset = (bss_size + datum.alignment - 1) & -datum.alignment
            bss_symbols.append(
                DefinedSymbol(datum.symbol, offset, datum.size, STT_OBJECT, datum.binding)
            )
            bss_size = offset + (datum.reservation_size or datum.size)
    return bytes(data), data_symbols, data_relocations, bss_size, bss_symbols


def _module_object(
    module: Module,
    functions: dict[int, Function],
    carved: dict[int, tuple[bytes, list[MipsRelocation]]],
    data_blobs: dict[int, tuple[bytes, list[MipsRelocation]]] | None = None,
    rodata_blob: bytes | None = None,
    *,
    load_padding: Callable[[int, int], bytes] | None = None,
    rodata_relocations: Sequence[MipsRelocation] = (),
    jumptable_blob: bytes | None = None,
) -> ModuleImage:
    """Concatenate a module's carved functions into one .text section.

    Claimed data follows in ``.data``/``.sdata``/``.bss`` so the object compares the whole
    translation-unit hypothesis, not only its code. Do not append the probe
    assembler's alignment tails: bytes outside the retail claims can belong to
    another object, and synthesized zeros are not recovered retail storage.
    """
    text = bytearray()
    relocations: list[MipsRelocation] = []
    symbols: list[DefinedSymbol] = []
    body_total = 0
    function_offsets = {
        functions[va].symbol: va - module.vas[0] for va in module.vas
    }
    for va in module.vas:
        function = functions[va]
        if function.va != module.vas[0] + len(text):
            raise ValueError(
                f"module {module.unit}: {function.symbol} at {va:#x} is not contiguous "
                f"with the preceding function"
            )
        blob, function_relocations = carved[va]
        offset = len(text)
        rebased = bytearray(blob)
        # Section-relative addends were written relative to the function's own
        # object; inside the module section they are relative to the run start.
        if offset:
            _rebase_section_relocations(rebased, function_relocations, offset)
        function_relocations = _module_function_address_relocations(
            rebased, function_relocations, function_offsets
        )
        relocations.extend(
            item.moved(offset)
            for item in function_relocations
        )
        symbols.append(DefinedSymbol(function.symbol, offset, function.body_size, STT_FUNC))
        text += rebased
        body_total += function.body_size
    first = symbols[0] if symbols else None
    data, data_symbols, data_relocations, bss_size, bss_symbols = _module_data(
        module, data_blobs or {}, load_padding
    )
    # The SDATA literal ranges pack among the unit's named .sdata claims in
    # address order; only their packed offsets survive into the object.
    sdata_module = module
    literal_names = {literal_symbol(index) for index in range(len(module.sdata))}
    if module.sdata:
        literals = tuple(Datum(va, size, literal_symbol(index), "load", "static", ".sdata")
                         for index, (va, size) in enumerate(module.sdata))
        sdata_module = replace(module, data=tuple(sorted(
            (*module.data, *literals), key=lambda datum: datum.va)))
    sdata, sdata_symbols, sdata_relocations, _, _ = _module_data(
        sdata_module, data_blobs or {}, load_padding, section=".sdata"
    )
    literal_offsets = {symbol.name: symbol.value for symbol in sdata_symbols
                       if symbol.name in literal_names}
    sdata_symbols = [symbol for symbol in sdata_symbols
                     if symbol.name not in literal_names]
    data_blob = bytearray(data)
    data_relocations = _module_function_data_relocations(
        data_blob, data_relocations, function_offsets
    )
    sdata_blob = bytearray(sdata)
    sdata_relocations = _module_function_data_relocations(
        sdata_blob, sdata_relocations, function_offsets
    )
    if literal_offsets:
        # Initialized pointers may address the unit's own small literals.
        data_relocations = _module_load_relocations(
            data_blob, data_relocations, literal_offsets, SDATA_SECTION_SYMBOL)
        sdata_relocations = _module_load_relocations(
            sdata_blob, sdata_relocations, literal_offsets, SDATA_SECTION_SYMBOL)
    data = bytes(data_blob)
    sdata = bytes(sdata_blob)
    _, _, _, sbss_size, sbss_symbols = _module_data(
        module, {}, section=".sbss"
    )
    for section_symbol, section_symbols in (
        (BSS_SECTION_SYMBOL, bss_symbols),
        (SBSS_SECTION_SYMBOL, sbss_symbols),
    ):
        static_bss = {
            symbol.name: symbol.value for symbol in section_symbols
            if symbol.binding == STB_LOCAL
        }
        if static_bss:
            relocations = _module_static_bss_relocations(
                text, relocations, static_bss, section_symbol)
    for section_symbol, section_symbols in (
        (DATA_SECTION_SYMBOL, data_symbols),
        (SDATA_SECTION_SYMBOL, sdata_symbols),
    ):
        module_load = {symbol.name: symbol.value for symbol in section_symbols}
        if section_symbol == SDATA_SECTION_SYMBOL:
            module_load.update(literal_offsets)
        if module_load:
            relocations = _module_load_relocations(
                text, relocations, module_load, section_symbol
            )
    rodata = b""
    if module.rodata is None:
        rodata_relocations = []
    else:
        if rodata_blob is None:
            raise ValueError(f"module {module.unit}: RODATA range was not carved")
        rodata, rodata_relocations = _module_rodata(
            module, rodata_blob, len(text), rodata_relocations)
    rdata, rdata_relocations = b"", []
    if module.jumptables is not None:
        if jumptable_blob is None:
            raise ValueError(f"module {module.unit}: JUMPTABLES range was not carved")
        rdata, rdata_relocations = _module_rodata(module, jumptable_blob, len(text))
    # The section must carry the strongest constraint used to pack its members,
    # not an unrelated four-byte default. Relink validation still requires one
    # consistent base for every member; changing ELF alignment cannot fix gaps.
    return ModuleImage(
        write_mips_elf(
            bytes(text),
            first.name if first else None,
            first.size if first else 0,
            relocations,
            symbols[1:] if first else (),
            data=data,
            data_symbols=data_symbols,
            data_relocations=data_relocations,
            data_alignment=max((d.alignment for d in module.data if d.section_name == ".data"),
                               default=1),
            sdata=sdata,
            sdata_symbols=sdata_symbols,
            sdata_relocations=sdata_relocations,
            sdata_alignment=max((*(d.alignment for d in module.data
                                   if d.section_name == ".sdata"),
                                 *((4,) if module.sdata else ())), default=1),
            bss_size=bss_size,
            bss_symbols=bss_symbols,
            bss_alignment=max((4 if d.reservation_size else d.alignment
                               for d in module.data if d.section_name == ".bss"), default=1),
            sbss_size=sbss_size,
            sbss_symbols=sbss_symbols,
            sbss_alignment=4,
            rodata=rodata,
            rodata_relocations=rodata_relocations,
            rdata=rdata,
            rdata_relocations=rdata_relocations,
        ),
        [*relocations, *data_relocations, *sdata_relocations, *rodata_relocations,
         *rdata_relocations],
        len(text),
        body_total,
        len(data) + len(sdata),
        bss_size + sbss_size,
        len(rodata) + len(rdata),
    )


def delink(
    exe_dir: Path,
    config_dir: Path,
    output_dir: Path,
    images: Iterable[str] = IMAGE_LAYOUTS,
    vas: Iterable[int] = (),
    policy: str = "safe",
    modules: Iterable[Module] = (),
) -> dict[str, dict[str, int]]:
    from scripts.kf.config_data import delink_object, load as load_contributions
    from scripts.kf.sema.image import RetailImage

    selected_images = tuple(dict.fromkeys(images))
    selected_vas = set(vas)
    modules = tuple(modules)
    contributions = load_contributions(config_dir, modules) if not selected_vas else ()
    modules_by_image: dict[str, list[Module]] = defaultdict(list)
    for module in modules:
        modules_by_image[module.image].append(module)
    catalog = load_catalog(config_dir)
    _, relocation_rows = read_tsv(config_dir / "relocs.tsv")
    rows_by_image: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in relocation_rows:
        if row["image"] in selected_images:
            rows_by_image[row["image"]].append(row)

    results: dict[str, dict[str, int]] = {}
    for image in selected_images:
        expected = IMAGE_LAYOUTS[image]
        exe_path = exe_dir / image
        executable = verify_retail_image(exe_path, image)
        image_output = output_dir / image_key(image)
        object_output = image_output / "objects"
        object_output.mkdir(parents=True, exist_ok=True)

        functions = [
            function for function in catalog.functions[image]
            if not selected_vas or function.va in selected_vas
        ]
        selected_function_starts = {function.va for function in functions}
        selected_modules = [
            module for module in modules_by_image[image]
            if not selected_vas or (module.vas and set(module.vas) <= selected_vas)
        ]
        claimed_load_data = sorted(
            (datum for module in selected_modules for datum in module.data
             if datum.storage == "load"),
            key=lambda item: item.va,
        )
        # An initialized pointer table may address a literal owned by the
        # same unit; its reloc must use that unit's read-only section.
        owned_by_datum = {
            datum.va: module.owned
            for module in selected_modules if module.owned
            for datum in module.data if datum.storage == "load"
        }
        rows_by_owner: dict[int, list[dict[str, str]]] = defaultdict(list)
        rows_by_datum: dict[int, list[dict[str, str]]] = defaultdict(list)
        rows_by_rodata: dict[str, list[dict[str, str]]] = defaultdict(list)
        withheld_rows: list[dict[str, object]] = []
        for row in rows_by_image[image]:
            site = parse_int(row["site_va"])
            owner = _containing_function(catalog, image, site)
            if owner is None:
                datum = next(
                    (item for item in claimed_load_data if item.va <= site < item.end), None
                )
                rodata_module = next(
                    (module for module in selected_modules if module.owns_rodata(site)), None
                )
                if datum is not None:
                    rows_by_datum[datum.va].append(row)
                elif rodata_module is not None:
                    rows_by_rodata[rodata_module.unit].append(row)
                elif not selected_vas:
                    withheld_rows.append(_withheld(row, None, "site-outside-function"))
                continue
            if owner.va in selected_function_starts:
                rows_by_owner[owner.va].append(row)

        object_rows: list[dict[str, object]] = []
        used_rows: list[dict[str, object]] = []
        withheld_functions: list[dict[str, object]] = []
        carved: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
        # Members of a module that claims RODATA/JUMPTABLES/SDATA ranges are
        # carved a second time with those ranges resolving to section offsets.
        owned_by_function = {
            va: module.owned
            for module in selected_modules if module.owned
            for va in module.vas
        }
        carved_rodata: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
        for function in functions:
            if function.fragments != 1:
                withheld_functions.append({
                    "image": image,
                    "va": format_hex(function.va),
                    "size": format_size(function.size),
                    "body_size": format_size(function.body_size),
                    "fragments": function.fragments,
                    "name": function.symbol,
                    "reason": "fragment-ranges-not-yet-recorded",
                })
                for row in rows_by_owner[function.va]:
                    withheld_rows.append(_withheld(row, function, "function-is-fragmented"))
                continue

            start = expected.file_offset(function.va)
            linked = executable[start:start + function.size]
            blob = bytearray(linked)
            if len(blob) != function.size:
                raise ValueError(f"{exe_path}: truncated function {function.va:#x}")
            function_relocations: list[MipsRelocation] = []
            ambiguous = ambiguous_sites(
                row for row in rows_by_owner[function.va] if _competes_for_site(row, policy)
            )

            for row in rows_by_owner[function.va]:
                sites = [parse_int(row["site_va"])]
                if row["paired_site_va"]:
                    sites.append(parse_int(row["paired_site_va"]))
                if any(site in ambiguous for site in sites):
                    withheld_rows.append(_withheld(
                        row, function, "ambiguous-overlapping-candidates"
                    ))
                    continue
                try:
                    relocations, used = _apply_relocation(
                        blob, function, row, catalog, policy, original=linked
                    )
                except ValueError as error:
                    withheld_rows.append(_withheld(row, function, str(error)))
                    continue
                _merge_relocations(function_relocations, relocations)
                used_rows.append(used)
            # Shared HI/LO halves are emitted once; the ELF writer decides
            # REL order (mips_elf.rel_order) from the site and each HI16's pair.
            function_relocations.sort(key=lambda item: item.offset)
            if function.va in owned_by_function:
                module_blob = bytearray(executable[start:start + function.size])
                module_relocations: list[MipsRelocation] = []
                for row in rows_by_owner[function.va]:
                    sites = [parse_int(row["site_va"])]
                    if row["paired_site_va"]:
                        sites.append(parse_int(row["paired_site_va"]))
                    if any(site in ambiguous for site in sites):
                        continue
                    try:
                        owned = owned_by_function[function.va]
                        relocations, _used = _apply_relocation(
                            module_blob, function, row, catalog, policy,
                            owned.rodata, owned.sdata, original=linked,
                            jumptables=owned.jumptables,
                        )
                    except ValueError:
                        continue
                    _merge_relocations(module_relocations, relocations)
                module_relocations.sort(key=lambda item: item.offset)
                carved_rodata[function.va] = (bytes(module_blob), module_relocations)

            object_name = f"{function.va:08x}_{function.symbol}.o"
            object_relative = Path("objects") / object_name
            carved[function.va] = (bytes(blob), function_relocations)
            _write_bytes_if_changed(object_output / object_name, write_mips_elf(
                bytes(blob),
                function.symbol,
                function.body_size,
                function_relocations,
            ))
            object_rows.append({
                "image": image,
                "va": format_hex(function.va),
                "size": format_size(function.size),
                "body_size": format_size(function.body_size),
                "name": function.symbol,
                "unit": "",
                "scope": function.scope,
                "provider": function.provider,
                "library": function.library,
                "object": object_relative.as_posix(),
                "relocations": len(function_relocations),
                "data_size": "0x0",
                "bss_size": "0x0",
                "rodata_size": "0x0",
                "confidence": function.confidence,
                "provenance": function.provenance,
            })

        # Claimed load data is carved from the image the same way; every
        # relocation candidate sited inside it goes through the shared
        # validator. Raw pointer words stay withheld under the safe policy
        # unless their exact site, target, and semantic owner were reviewed.
        data_blobs: dict[int, tuple[bytes, list[MipsRelocation]]] = {}
        for datum in claimed_load_data:
            start = expected.file_offset(datum.va)
            blob = bytearray(executable[start:start + datum.size])
            if len(blob) != datum.size:
                raise ValueError(f"{exe_path}: truncated datum {datum.va:#x}")
            owner = _DataOwner(image, datum)
            datum_relocations: list[MipsRelocation] = []
            for row in rows_by_datum[datum.va]:
                owned = owned_by_datum.get(datum.va, Owned())
                try:
                    relocations, used = _apply_relocation(
                        blob, owner, row, catalog, policy,
                        owned.rodata, owned.sdata, jumptables=owned.jumptables,
                    )
                except ValueError as error:
                    withheld_rows.append(_withheld(row, None, f"data:{error}"))
                    continue
                datum_relocations.extend(relocations)
                used_rows.append(used)
            data_blobs[datum.va] = (bytes(blob), datum_relocations)
        # Anonymous small-data literals are carved as bytes; a pointer word
        # inside one would need a reviewed row like any claimed datum.
        for module in selected_modules:
            for va, size in module.sdata:
                start = expected.file_offset(va)
                blob = executable[start:start + size]
                if len(blob) != size:
                    raise ValueError(f"{exe_path}: truncated SDATA range for {module.unit}")
                data_blobs[va] = (blob, [])

        live_objects = {Path(str(row["object"])).name for row in object_rows}
        if not selected_vas:
            for old_object in object_output.glob("*.o"):
                if old_object.name not in live_objects:
                    old_object.unlink()

        # Manifested units are carved again as one contiguous section each, so
        # objdiff compares a translation unit against the retail run it claims.
        module_output = image_output / "modules"
        module_output.mkdir(parents=True, exist_ok=True)
        live_modules: set[str] = set()
        for module in selected_modules:
            if any(va not in carved for va in module.vas):
                missing = [f"{va:#x}" for va in module.vas if va not in carved]
                raise ValueError(f"module {module.unit}: functions not carved: {missing}")
            module_carved = dict(carved)
            module_carved.update({va: carved_rodata[va] for va in module.vas if va in carved_rodata})
            blobs = {}
            for kind, extent in (("RODATA", module.rodata), ("JUMPTABLES", module.jumptables)):
                if extent is not None:
                    blob_start = expected.file_offset(extent[0])
                    blobs[kind] = executable[blob_start:blob_start + extent[1]]
                    if len(blobs[kind]) != extent[1]:
                        raise ValueError(f"{exe_path}: truncated {kind} range for {module.unit}")
            rodata_reviewed: list[MipsRelocation] = []
            if "RODATA" in blobs:
                # Reviewed pointer words inside the constant pool (an automatic
                # aggregate initializer's global addresses) validate and
                # resolve like a claimed datum's.
                patched = bytearray(blobs["RODATA"])
                range_owner = _DataOwner(image, Datum(
                    module.rodata[0], module.rodata[1], "", "load"))
                for row in rows_by_rodata[module.unit]:
                    try:
                        relocations, used = _apply_relocation(
                            patched, range_owner, row, catalog, policy)
                    except ValueError as error:
                        withheld_rows.append(_withheld(row, None, f"rodata:{error}"))
                        continue
                    rodata_reviewed.extend(relocations)
                    used_rows.append(used)
                blobs["RODATA"] = bytes(patched)
            built = _module_object(
                module, catalog.function_starts[image], module_carved, data_blobs,
                blobs.get("RODATA"),
                load_padding=lambda va, size: executable[
                    expected.file_offset(va):expected.file_offset(va) + size],
                rodata_relocations=rodata_reviewed,
                jumptable_blob=blobs.get("JUMPTABLES"),
            )
            _write_bytes_if_changed(module_output / module.object_name, built.data)
            live_modules.add(module.object_name)
            first = catalog.function_starts[image][module.vas[0]] if module.vas else None
            start = module.vas[0] if module.vas else module.data[0].va
            object_rows.append({
                "image": image,
                "va": format_hex(start),
                "size": format_size(built.size),
                "body_size": format_size(built.body_size),
                "name": module.stem,
                "unit": module.unit,
                "scope": "module",
                "provider": "",
                "library": "",
                "object": (Path("modules") / module.object_name).as_posix(),
                "relocations": len(built.relocations),
                "data_size": format_size(built.data_size),
                "bss_size": format_size(built.bss_size),
                "rodata_size": format_size(built.rodata_size),
                "confidence": first.confidence if first else "candidate",
                "provenance": "config/units.toml",
            })
        if not selected_vas:
            for old_object in module_output.glob("*.o"):
                if old_object.name not in live_modules:
                    old_object.unlink()

        data_output = image_output / "data"
        live_data = set()
        for contribution in contributions:
            if contribution.image != image:
                continue
            blob = delink_object(contribution, RetailImage(image, expected, executable, exe_path),
                                 relocation_rows)
            _write_bytes_if_changed(contribution.target_path(output_dir), blob)
            live_data.add(contribution.object_name)
            object_rows.append({
                "image": image, "va": format_hex(contribution.va),
                "size": format_size(contribution.size), "body_size": "0x0",
                "name": contribution.identity, "unit": contribution.unit,
                "scope": "config-data", "provider": contribution.provider,
                "library": contribution.library,
                "object": (Path("data") / contribution.object_name).as_posix(),
                "relocations": 0, "data_size": format_size(contribution.size),
                "bss_size": "0x0", "rodata_size": "0x0", "confidence": "supported",
                "provenance": contribution.evidence,
            })
        if not selected_vas and data_output.is_dir():
            for old_object in data_output.glob("*.o"):
                if old_object.name not in live_data:
                    old_object.unlink()

        focused_rows = list(object_rows)
        report_output = image_output
        if selected_vas:
            focused_vas = sorted(selected_vas)
            focused_address_list = "_".join(f"{va:08x}" for va in focused_vas)
            focused_name = focused_address_list
            if len(focused_name.encode("ascii")) > 240:
                digest = hashlib.sha256(focused_name.encode("ascii")).hexdigest()
                focused_name = f"{focused_vas[0]:08x}_{focused_vas[-1]:08x}_{digest}"
            report_output = image_output / "focused" / focused_name
            report_output.mkdir(parents=True, exist_ok=True)
            write_tsv(
                report_output / "objects.tsv", OBJECT_FIELDS, focused_rows,
                _comments("Targets selected by this focused carve.", policy)
                + (f"selected function VAs: {focused_address_list}",),
            )

        # A focused carve adds or replaces only its selected rows.  Keep the
        # registry of other module targets so concurrent kf try sessions can
        # continue to resolve them.  Serialize read/modify/write with other
        # focused carves of this image.
        object_manifest = image_output / "objects.tsv"
        with (image_output / ".objects.lock").open("w") as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            if selected_vas and object_manifest.is_file():
                _, previous_rows = read_tsv(object_manifest)
                replaced = {(row["scope"], row["va"]) for row in object_rows}
                object_rows = [
                    row for row in previous_rows
                    if (row["scope"], row["va"]) not in replaced
                    and (image_output / row["object"]).is_file()
                ] + object_rows
                object_rows.sort(key=lambda row: (parse_int(row["va"]), row["scope"]))
            write_tsv(
                object_manifest,
                OBJECT_FIELDS,
                object_rows,
                _comments("One target MIPS ELF object per admitted contiguous function.", policy),
            )
        write_tsv(
            report_output / "relocations_used.tsv",
            USED_RELOCATION_FIELDS,
            used_rows,
            _comments("Relocations materialized in target objects.", policy),
        )
        write_tsv(
            report_output / "relocations_withheld.tsv",
            WITHHELD_RELOCATION_FIELDS,
            withheld_rows,
            _comments("Candidates intentionally not materialized, with a reason.", policy),
        )
        write_tsv(
            report_output / "functions_withheld.tsv",
            WITHHELD_FUNCTION_FIELDS,
            withheld_functions,
            _comments("Functions not carved into target objects.", policy),
        )
        results[image] = {
            "objects": len(object_rows),
            "relocations": sum(int(row["relocations"]) for row in object_rows),
            "withheld_relocations": len(withheld_rows),
            "withheld_functions": len(withheld_functions),
        }
    return results


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe-dir", required=True, type=Path)
    parser.add_argument("--config-dir", type=Path, default=Path("config/retail"))
    parser.add_argument("--output-dir", type=Path, default=Path("build/delink"))
    parser.add_argument(
        "--image",
        action="append",
        choices=tuple(IMAGE_LAYOUTS),
        help="repeat to select targets; default: all three",
    )
    parser.add_argument(
        "--va",
        action="append",
        type=parse_int,
        default=[],
        help="repeat to carve selected function starts only",
    )
    parser.add_argument(
        "--policy",
        choices=("safe", "reviewed", "all"),
        default="safe",
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        help="unit manifest whose modules are carved as well (default: config/units.toml)",
    )
    args = parser.parse_args()
    modules: tuple[Module, ...] = ()
    manifest_path = args.manifest or Path("config/units.toml")
    if manifest_path.is_file():
        from scripts.kf.manifest import load as load_manifest

        modules = load_manifest(manifest_path, config_dir=args.config_dir).modules()
    results = delink(
        args.exe_dir,
        args.config_dir,
        args.output_dir,
        args.image or IMAGE_LAYOUTS,
        args.va,
        args.policy,
        modules,
    )
    for image, counts in results.items():
        print(image + ": " + ", ".join(
            f"{name}={count}" for name, count in counts.items()
        ))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
