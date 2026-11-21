"""Relink source/config target objects at their claimed retail section addresses.

This checks the delinker's bytes and placements with the independent GNU MIPS
linker, not the delinker's inverse encoding helpers. It is not reconstruction
progress or proof that all reachable bytes have been modeled. Packed DATA/BSS
claims must admit one base per ELF section; symbols are never scattered or
overridden to make an unplaceable section appear linkable.
"""

from __future__ import annotations

import argparse
import io
import json
import re
import shutil
import subprocess
import tempfile
from collections import Counter, defaultdict
from dataclasses import asdict, dataclass, field
from pathlib import Path

from elftools.common.exceptions import ELFError
from elftools.elf.elffile import ELFFile

from scripts.kf.allocation_notes import read_requests
from scripts.kf.delink import load_catalog, sanitize_symbol
from scripts.kf.manifest import Unit, load as load_manifest
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import (
    IMAGE_GP, IMAGE_KEYS, IMAGE_LAYOUTS, IMAGE_NAMES, OVERLAY_SOURCES, read_tsv,
)
from scripts.kf.sema.image import RetailImage


SECTIONS = {".text": "SHT_PROGBITS", ".rodata": "SHT_PROGBITS", ".rdata": "SHT_PROGBITS",
            ".data": "SHT_PROGBITS", ".sdata": "SHT_PROGBITS", ".bss": "SHT_NOBITS",
            ".sbss": "SHT_NOBITS"}
LINKER = "mipsel-linux-gnu-ld"
# Sections a claimed COMMON request may be placed in (see named_common_bss_claim).
COMMON_SECTIONS = (".bss", ".sbss")
SYMBOL_NAME = re.compile(r"[A-Za-z_$][A-Za-z0-9_$]*\Z")
ADDRESS_NAME = re.compile(r"DAT_([0-9a-fA-F]{8})\Z")


@dataclass
class SectionPlacement:
    name: str
    address: int
    size: int
    claimed_bytes: int
    storage: str


@dataclass
class UnitResult:
    image: str
    unit: str
    object: str
    sections: list[SectionPlacement] = field(default_factory=list)
    issues: list[dict] = field(default_factory=list)
    initialized_bytes_compared: int = 0
    relocation_rows: int = 0
    linker_ran: bool = False
    linker_diagnostics: str = ""
    merged_commons: list[dict] = field(default_factory=list)

    def issue(self, kind: str, **details: object) -> None:
        self.issues.append({"kind": kind, **details})


def address_book(image: str, config_dir: Path = RETAIL_CONFIG) -> dict[str, set[int]]:
    """Input identities only; never infer S from delinker output.

    An overlay also sees its host's identities outside its own load window.
    """
    book: dict[str, set[int]] = defaultdict(set)
    catalog = load_catalog(config_dir)
    for item in (*catalog.functions[image], *catalog.data[image]):
        book[item.symbol].add(item.va)
    # Include curated BSS, overlay RAM and hardware identities outside load data.
    for row in read_tsv(config_dir / "data_identities.tsv")[1]:
        if row["image"] == image:
            va = int(row["va"], 0)
            book[sanitize_symbol(row["name"], f"DAT_{va:08x}")].add(va)
    for name, va in link_labels(image, config_dir).items():
        book[name].add(va)
    source = OVERLAY_SOURCES.get(image)
    if source is not None:
        # An overlay's external references outside its own window resolve to
        # its host's identities (retail.referent_image); its own names win.
        layout = IMAGE_LAYOUTS[image]
        for name, addresses in address_book(source.host, config_dir).items():
            if name not in book and not any(layout.contains(va) for va in addresses):
                book[name] = set(addresses)
    return dict(book)


def link_labels(image: str, config_dir: Path = RETAIL_CONFIG) -> dict[str, int]:
    """Zero-size link-layout labels (BSS_END): boundaries, not storage."""
    path = config_dir / "link_labels.tsv"
    if not path.is_file():
        return {}
    labels: dict[str, int] = {}
    for row in read_tsv(path)[1]:
        if row["image"] != image:
            continue
        if row["name"] in labels or not SYMBOL_NAME.fullmatch(row["name"]):
            raise ValueError(f"{path}: invalid or duplicate label {row['name']!r}")
        labels[row["name"]] = int(row["va"], 0)
    return labels


@dataclass(frozen=True)
class Definition:
    """An initialized global datum in an image's load payload, and its owner."""

    symbol: str
    va: int
    size: int
    owner: str  # the claiming unit, or "config" for a curated identity only
    storage: str = "load"  # "bss": COMMON storage an earlier object allocated


def initialized_definitions(image: str, units: tuple[Unit, ...] = (),
                            config_dir: Path = RETAIL_CONFIG) -> dict[str, Definition]:
    """Global initialized definitions a COMMON request of `image` can merge into.

    A curated `load`-storage identity, or a unit's `load` DATA() claim, whose
    extent lies inside the PS-X EXE payload: initialized storage that the
    linker cannot have allocated for a COMMON request (that goes to BSS).
    """
    from scripts.kf.inventory import load_data_identities
    from scripts.kf.retail import IMAGE_LAYOUTS

    layout = IMAGE_LAYOUTS.get(image)
    if layout is None:
        return {}
    found: dict[str, Definition] = {}
    for identity in load_data_identities(config_dir).values():
        if identity.image == image and identity.storage == "load" and identity.scope == "global":
            symbol = sanitize_symbol(identity.name, f"DAT_{identity.va:08x}")
            found[symbol] = Definition(symbol, identity.va, identity.size, "config")
    for unit in units:
        if unit.image != image:
            continue
        for datum in unit.data:
            if datum.storage == "load" and datum.scope == "global":
                found[datum.symbol] = Definition(datum.symbol, datum.va, datum.size, unit.unit)
    return {symbol: item for symbol, item in found.items()
            if item.size > 0 and layout.contains(item.va, item.size)}


def psylink_common_alignment(size: int) -> int:
    """PSYLINK 2.73D's alignment of a COMMON request of `size` bytes.

    The largest power of two not above the size, capped at 8 (sizes 1, 2,
    3, 4, 6, 8 align to 1, 2, 2, 4, 4, 8; docs/executable-linking.md).
    """
    alignment = 1
    while alignment < 8 and alignment * 2 <= size:
        alignment *= 2
    return alignment


def allocated_definitions(image: str, units: tuple[Unit, ...] = (),
                          config_dir: Path = RETAIL_CONFIG) -> dict[str, Definition]:
    """Global BSS storage a later COMMON request of `image` can merge into.

    PSYLINK allocates a COMMON name once, in the batch of the first object
    that requests it (that object's fixed .sbss/.bss, then its new COMMON
    names in symbol-hash bucket order, contiguous apart from size alignment);
    a later object's request of the same name contributes no storage. A
    curated global `bss` identity, or a unit's global .bss/.sbss DATA()
    claim, is such storage. `usable_definitions` keeps a unit-owned one only
    for units linked after its owner.
    """
    from scripts.kf.inventory import load_data_identities

    if image not in IMAGE_LAYOUTS:
        return {}
    found: dict[str, Definition] = {}
    for identity in load_data_identities(config_dir).values():
        if identity.image == image and identity.storage == "bss" and identity.scope == "global":
            symbol = sanitize_symbol(identity.name, f"DAT_{identity.va:08x}")
            found[symbol] = Definition(symbol, identity.va, identity.size, "config", "bss")
    for unit in units:
        if unit.image != image:
            continue
        for datum in unit.data:
            if (datum.storage == "bss" and datum.scope == "global"
                    and datum.section_name in COMMON_SECTIONS):
                found[datum.symbol] = Definition(datum.symbol, datum.va, datum.size,
                                                 unit.unit, "bss")
    return {symbol: item for symbol, item in found.items() if item.size > 0}


def usable_definitions(definitions: dict[str, Definition], unit: Unit,
                       units: tuple[Unit, ...]) -> dict[str, Definition]:
    """The definitions a COMMON request of `unit` may merge into.

    Initialized data and curated BSS identities always qualify; a unit's BSS
    claim only when that unit links (manifest order) before `unit`, since an
    earlier request of the name would itself be the allocation.
    """
    order = {item.unit: index for index, item in enumerate(units) if item.image == unit.image}
    position = order.get(unit.unit, len(order))
    return {symbol: item for symbol, item in definitions.items()
            if item.storage != "bss" or item.owner == "config"
            or order.get(item.owner, len(order)) < position}


def merged_common(symbol, unit: Unit,
                  definitions: dict[str, Definition] | None) -> Definition | None:
    """The definition elsewhere that a COMMON request merges into.

    A tentative definition (`u_long x;`) is emitted as COMMON. PSYLINK, like
    GNU ld, allocates COMMON storage only when no object defines the symbol
    and no earlier object requested it; otherwise the request merges into
    that storage and contributes none of its own. A unit may therefore keep
    a tentative definition of a global another unit initializes, or that an
    earlier object allocated (ASPSX needs its size to emit the -G8
    $gp-relative access an `extern` would not get). Admitted only when this
    unit claims no datum of that name and the definition is a global datum
    of the image (initialized_definitions, allocated_definitions), at least
    as large as the request and aligned for it; anything else stays an
    unsupported COMMON allocation.
    """
    if definitions is None or any(datum.symbol == symbol.name for datum in unit.data):
        return None
    definition = definitions.get(symbol.name)
    alignment = symbol['st_value']
    if definition is not None and definition.storage == "bss":
        # The object's COMMON value is its section alignment, not PSYLINK's.
        alignment = psylink_common_alignment(symbol['st_size'])
    if (definition is None or definition.owner == unit.unit
            or symbol['st_info']['bind'] != 'STB_GLOBAL'
            or not 0 < symbol['st_size'] <= definition.size
            or alignment <= 0 or alignment & (alignment - 1)
            or definition.va % alignment):
        return None
    return definition


def named_common_bss_claim(elf: ELFFile, unit: Unit):
    """Recognize one fully claimed COMMON request that ld can place as BSS.

    COMMON carries no input section. Only a single, exact, globally named
    reservation is admissible here; the claimed address supplies its placement
    and must satisfy the request's alignment. Other COMMON remains unresolved.
    The claim's section is `.bss`, or `.sbss` for a small request a -G8 unit
    leaves to the linker; it must be the only claim there, and the object's
    section of that name must be empty.
    """
    symtab = elf.get_section_by_name(".symtab")
    if symtab is None:
        return None
    common = [symbol for symbol in symtab.iter_symbols()
              if symbol['st_shndx'] == 'SHN_COMMON']
    if len(common) != 1:
        return None
    symbol = common[0]
    named = [datum for datum in unit.data
             if datum.symbol == symbol.name and datum.section_name in COMMON_SECTIONS]
    if len(named) != 1:
        return None
    claim = named[0]
    claims = [datum for datum in unit.data if datum.section_name == claim.section_name]
    section = elf.get_section_by_name(claim.section_name)
    if len(claims) != 1 or (section is not None and section['sh_size']):
        return None
    alignment = symbol['st_value']
    if (not symbol.name or symbol.name != claim.symbol
            or symbol['st_info']['bind'] != 'STB_GLOBAL'
            or claim.scope != 'global'
            or symbol['st_size'] != claim.size or claim.size <= 0
            or claim.reservation_size
            or alignment <= 0 or alignment & (alignment - 1)
            or claim.va % alignment):
        return None
    return claim


def plan(elf: ELFFile, unit: Unit, result: UnitResult,
         definitions: dict[str, Definition] | None = None) -> dict[str, int]:
    """Derive one section base from every claim and actual ELF symbol offset.

    `definitions` (initialized_definitions) lets a COMMON request that merges
    into another unit's initialized datum resolve as a reference.
    """
    if (elf.elfclass != 32 or not elf.little_endian
            or elf['e_machine'] != 'EM_MIPS' or elf['e_type'] != 'ET_REL'):
        result.issue("unsupported-object-format")
        return {}
    symtab = elf.get_section_by_name(".symtab")
    if symtab is None:
        result.issue("missing-symbol-table")
        return {}
    try:
        exported_requests = read_requests(elf)
    except ValueError as error:
        result.issue('invalid-allocation-provenance', detail=str(error))
        exported_requests = []
    for request in exported_requests:
        result.issue('unplaced-exported-allocation', symbol=request['name'],
                     reservation_size=request['reservation_size'])
    common_claim = named_common_bss_claim(elf, unit)
    named_bss = {datum.symbol: datum for datum in unit.data
                 if datum.section_name in COMMON_SECTIONS and datum.scope == 'global'}
    symbols: dict[str, list] = defaultdict(list)
    for symbol in symtab.iter_symbols():
        # COMMON has no input section: SHF_ALLOC enumeration cannot see it,
        # and the discard script below would silently drop an unreferenced
        # reservation. A shared allocation/placement contract is still needed;
        # equal COMMON symbols on both sides are not proof of retail storage.
        if (symbol['st_shndx'] == 'SHN_COMMON'
                and (common_claim is None or symbol.name != common_claim.symbol)):
            claim = named_bss.get(symbol.name)
            merged = merged_common(symbol, unit, definitions)
            if merged is not None:
                result.merged_commons.append({
                    "symbol": symbol.name, "requested_size": symbol['st_size'],
                    "alignment": symbol['st_value'], "definition_va": merged.va,
                    "definition_size": merged.size, "definition_owner": merged.owner,
                    "definition_storage": merged.storage})
            elif claim is None:
                result.issue("unsupported-common-allocation", symbol=symbol.name,
                             size=symbol['st_size'], alignment=symbol['st_value'])
            else:
                result.issue("unresolved-common-placement", symbol=symbol.name,
                             requested_size=symbol['st_size'],
                             alignment=symbol['st_value'], claimed_va=claim.va,
                             claimed_size=claim.size)
        if symbol.name:
            symbols[symbol.name].append(symbol)
    # name, address, extent, expected section. Function sizes exclude linker
    # padding, but .text owns the entire contiguous run, including delay slots.
    claims = [(f.symbol, f.va, f.body_size, ".text") for f in unit.functions]
    claims += [(d.symbol, d.va, d.size, d.section_name)
               for d in unit.data]
    reservations = {d.symbol: d.reservation_size for d in unit.data if d.reservation_size}
    bases: dict[str, set[int]] = defaultdict(set)
    sizes: Counter = Counter()
    witnesses: dict[str, list[dict]] = defaultdict(list)
    for name, va, size, section_name in claims:
        matches = symbols.get(name, [])
        if len(matches) != 1:
            result.issue("missing-or-ambiguous-owned-symbol", symbol=name, count=len(matches))
            continue
        symbol = matches[0]
        index = symbol['st_shndx']
        is_claimed_common = (common_claim is not None and name == common_claim.symbol
                             and index == 'SHN_COMMON')
        actual_section = (common_claim.section_name if is_claimed_common else
                          elf.get_section(index).name if isinstance(index, int) else index)
        if actual_section != section_name or symbol['st_size'] != size:
            result.issue("owned-symbol-layout", symbol=name, expected_section=section_name,
                         actual_section=actual_section, expected_size=size,
                         actual_size=symbol['st_size'])
            continue
        offset = 0 if is_claimed_common else symbol['st_value']
        section = None if is_claimed_common else elf.get_section(index)
        if (not is_claimed_common
                and offset + max(size, reservations.get(name, 0)) > section['sh_size']):
            result.issue("owned-symbol-out-of-bounds", symbol=name)
            continue
        bases[section_name].add(va - offset)
        sizes[section_name] += size
        witness = {"symbol": name, "va": va, "offset": offset, "size": size,
                   "implied_base": va - offset}
        if name in reservations:
            witness["reservation_size"] = reservations[name]
        witnesses[section_name].append(witness)
    text_extent = unit.functions[-1].end - unit.functions[0].va if unit.functions else 0
    if unit.functions:
        sizes[".text"] = text_extent
    for name, extent in ((".rodata", unit.rodata), (".rdata", unit.jumptables)):
        if extent:
            bases[name].add(extent[0])
            sizes[name] = extent[1]
    if unit.sdata and not witnesses[".sdata"]:
        # A unit whose .sdata holds only its SDATA() literals: the target
        # module packs those ranges alone, so the first starts the section.
        bases[".sdata"].add(unit.sdata[0][0])
        sizes[".sdata"] = unit.sdata[-1][0] + unit.sdata[-1][1] - unit.sdata[0][0]
    placed = {}
    seen = set()
    for section in elf.iter_sections():
        name, size = section.name, section['sh_size']
        if not section['sh_flags'] & 2:  # SHF_ALLOC
            continue
        if name in seen:
            result.issue("duplicate-allocated-section", section=name)
            continue
        seen.add(name)
        # ELF ABI metadata, not PS-X runtime storage. Target objects currently
        # emit neither; recognizing them does not admit arbitrary extra data.
        if name in {".reginfo", ".MIPS.abiflags"}:
            continue
        if name not in SECTIONS or section['sh_type'] != SECTIONS[name]:
            result.issue("unsupported-allocated-section", section=name, size=size)
            continue
        if not size:
            if common_claim is not None and name == common_claim.section_name:
                continue
            if sizes[name]:
                result.issue("owned-section-extent", section=name, size=0, claimed=sizes[name])
            continue
        candidates = bases.get(name, set())
        if len(candidates) != 1:
            result.issue("conflicting-section-bases" if candidates else "unclaimed-section",
                         section=name, size=size, claims=witnesses[name])
            continue
        owned = sorted((s['offset'], s['offset'] + s.get('reservation_size', s['size']), s['symbol'])
                       for s in witnesses[name])
        for left, right in zip(owned, owned[1:]):
            if left[1] > right[0]:
                result.issue("overlapping-owned-symbols", section=name,
                             symbols=[left[2], right[2]])
        base = next(iter(candidates))
        if base < 0 or base + size > 0x100000000 or base % max(1, section['sh_addralign']):
            result.issue("invalid-section-placement", section=name, address=base, size=size,
                         alignment=section['sh_addralign'])
            continue
        if sizes[name] > size or (name == ".text" and size != text_extent):
            result.issue("owned-section-extent", section=name, size=size, claimed=sizes[name])
            continue
        placed[name] = base
        result.sections.append(SectionPlacement(
            name, base, size, sizes[name], "bss" if name in {".bss", ".sbss"} else "load",
        ))
    if common_claim is not None:
        name, size, base = common_claim.section_name, common_claim.size, common_claim.va
        if (name in seen and elf.get_section_by_name(name)['sh_size']
                or sizes[name] != size or bases[name] != {base}):
            result.issue('owned-section-extent', section=name, size=size,
                         claimed=sizes[name])
        else:
            placed[name] = base
            result.sections.append(SectionPlacement(name, base, size, size, 'bss'))
            seen.add(name)
    for name in bases:
        if name not in seen:
            result.issue("missing-owned-section", section=name)
    return placed


def externals(elf: ELFFile, book: dict[str, set[int]], result: UnitResult) -> dict[str, int]:
    resolved = {}
    for section in elf.iter_sections():
        if section['sh_type'] not in {"SHT_REL", "SHT_RELA"}:
            continue
        target_section = elf.get_section(section['sh_info'])
        if target_section.name not in SECTIONS:
            continue
        if section['sh_type'] != "SHT_REL":
            result.issue("unsupported-relocation-section", section=section.name)
            continue
        symtab = elf.get_section(section['sh_link'])
        for row in section.iter_relocations():
            result.relocation_rows += 1
            if row['r_info_type'] not in {2, 4, 5, 6, 7}:
                result.issue("unsupported-relocation", section=target_section.name,
                             offset=row['r_offset'], type=row['r_info_type'])
            if row['r_offset'] & 3 or row['r_offset'] + 4 > target_section['sh_size']:
                result.issue("relocation-out-of-bounds", section=target_section.name,
                             offset=row['r_offset'])
            symbol = symtab.get_symbol(row['r_info_sym'])
            if symbol['st_shndx'] != 'SHN_UNDEF':
                continue
            name = symbol.name
            candidates = book.get(name, set())
            address_name = ADDRESS_NAME.fullmatch(name)
            if not candidates and address_name:
                # This is the unresolved identity's literal address, not proof
                # of a datum's classification, extent or reachability.
                candidates = {int(address_name.group(1), 16)}
            if len(candidates) != 1 or not SYMBOL_NAME.fullmatch(name):
                result.issue("unresolved-external", symbol=name, candidates=sorted(candidates))
                continue
            resolved[name] = next(iter(candidates))
    return resolved


def compare_linked(elf: ELFFile, image: RetailImage, result: UnitResult) -> None:
    expected_sections = {s.name for s in result.sections}
    for section in elf.iter_sections():
        if (section['sh_flags'] & 2 and section['sh_size']
                and section.name not in expected_sections):
            result.issue("unexpected-linked-allocation", section=section.name)
    for placement in result.sections:
        section = elf.get_section_by_name(placement.name)
        if (section is None or section['sh_addr'] != placement.address
                or section['sh_size'] != placement.size):
            result.issue("linker-changed-section-layout", section=placement.name)
            continue
        if placement.storage == "bss":
            if section['sh_type'] != 'SHT_NOBITS':
                result.issue("initialized-bss", section=placement.name)
            continue
        expected = image.read(placement.address, placement.size)
        if expected is None:
            result.issue("section-outside-retail-load", section=placement.name,
                         address=placement.address, size=placement.size)
            continue
        actual = section.data()
        if len(actual) != len(expected):
            result.issue("truncated-linked-section", section=placement.name)
            continue
        result.initialized_bytes_compared += len(expected)
        if actual != expected:
            first = next(i for i, pair in enumerate(zip(expected, actual)) if pair[0] != pair[1])
            result.issue("retail-byte-mismatch", section=placement.name, offset=first,
                         va=placement.address + first, expected=expected[first], actual=actual[first],
                         differing_bytes=sum(a != b for a, b in zip(expected, actual)))


def link_command(linker: str, image: str, entry: int, script: Path, linked: Path,
                 object_path: Path, symbols: dict[str, int]) -> list[str]:
    """GNU ld arguments that place one target object at its retail addresses."""
    command = [linker, "-EL", "-G", "0", "--entry", hex(entry), "-T", str(script),
               "-o", str(linked), str(object_path.resolve())]
    command += [f"--defsym={name}={address:#x}" for name, address in sorted(symbols.items())]
    if image in IMAGE_GP:
        # R_MIPS_GPREL16 resolves against the program's startup $gp.
        command.append(f"--defsym=_gp={IMAGE_GP[image]:#x}")
    return command


def verify_unit(unit: Unit, image: RetailImage, book: dict[str, set[int]],
                object_path: Path, scratch: Path, *, linker: str = LINKER) -> UnitResult:
    result = UnitResult(unit.image, unit.unit, str(object_path))
    try:
        if unit.image != image.image:
            raise ValueError("unit/image namespace mismatch")
        elf = ELFFile(io.BytesIO(object_path.read_bytes()))
        placed = plan(elf, unit, result)
        if result.issues:
            return result
        symbols = externals(elf, book, result)
        if result.issues:
            return result
        # Never scatter a packed section or override an internally defined
        # symbol to its desired VA. GNU ld must honor the entire input layout.
        with tempfile.TemporaryDirectory(prefix="link-", dir=scratch) as temporary:
            root = Path(temporary)
            script = root / "retail.ld"
            common = named_common_bss_claim(elf, unit)
            common_section = common.section_name if common is not None else '.bss'
            script.write_text("SECTIONS {\n" + "\n".join(
                f"  {name} {address:#x} : {{ *({name})"
                + (" *(COMMON)" if name == common_section else "") + " }"
                for name, address in sorted(placed.items(), key=lambda item: item[1])
            ) + "\n  /DISCARD/ : { *(*) }\n}\n")
            linked = root / "linked.elf"
            entry = unit.functions[0].va if unit.functions else min(placed.values())
            # GNU MIPS ld otherwise moves SHN_COMMON requests of up to 8 bytes
            # into .scommon, which this script discards; only claimed BSS is placed.
            command = link_command(linker, unit.image, entry, script, linked,
                                   object_path, symbols)
            process = subprocess.run(command, capture_output=True, text=True, timeout=30)
            result.linker_ran = True
            result.linker_diagnostics = process.stderr.strip()
            if process.returncode:
                result.issue("linker-failed", returncode=process.returncode)
                return result
            compare_linked(ELFFile(io.BytesIO(linked.read_bytes())), image, result)
    except (ELFError, OSError, ValueError, IndexError, subprocess.TimeoutExpired) as error:
        result.issue("artifact-or-link-error", detail=str(error))
    return result


def find_overlaps(results: list[UnitResult]) -> None:
    spans = sorted((result.image, s.address, s.address + s.size, i, s.name)
                   for i, result in enumerate(results) for s in result.sections if s.size)
    for pos, (image, start, end, index, name) in enumerate(spans):
        for other_image, other_start, other_end, other_index, other_name in spans[pos + 1:]:
            if other_image != image or other_start >= end:
                break
            details = {"section": name, "other_unit": results[other_index].unit,
                       "other_section": other_name, "address": other_start,
                       "size": min(end, other_end) - other_start}
            results[index].issue("overlapping-section-placements", **details)
            results[other_index].issue("overlapping-section-placements", **{
                **details, "section": other_name, "other_unit": results[index].unit,
                "other_section": name,
            })


def audit(image: RetailImage, units: tuple[Unit, ...], book: dict[str, set[int]],
          delink_dir: Path, scratch: Path) -> dict:
    selected = tuple(unit for unit in units if unit.image == image.image)
    results = [verify_unit(unit, image, book,
                           unit.target_path(delink_dir) if hasattr(unit, "target_path") else
                           delink_dir / unit.image_key / "modules" / unit.object_name, scratch)
               for unit in selected]
    find_overlaps(results)
    counts = Counter(issue['kind'] for result in results for issue in result.issues)
    if not selected:
        counts['empty-selection'] += 1
    return {
        "image": image.image, "scope": "source-and-config-target-objects",
        "linker": shutil.which(LINKER), "units": [asdict(result) for result in results],
        "verified_units": sum(not result.issues for result in results),
        "total_units": len(results), "issues": dict(sorted(counts.items())),
        "initialized_bytes_compared": sum(r.initialized_bytes_compared for r in results),
        "complete_reachable_bytes_proven": False, "linked_image_equality_proven": False,
    }


def run(images=IMAGE_LAYOUTS, *, output: Path | None = None, unit: str | None = None) -> int:
    from scripts.kf.config_data import load as load_contributions

    manifest = load_manifest()
    all_units = (*manifest.units, *load_contributions(modules=manifest.modules()))
    by_name = {u.unit: u for u in all_units}
    images = tuple(images)
    if unit is not None and unit not in by_name:
        raise ValueError(f"unknown unit {unit!r}")
    if unit is not None:
        image = by_name[unit].image
        if image not in images:
            raise ValueError(f"unit {unit!r} is not in the selected image(s)")
        images = (image,)
    if shutil.which(LINKER) is None:
        raise ValueError(f"{LINKER} is required; run in nix develop")
    units = tuple(u for u in all_units if unit is None or u.unit == unit)
    reports = []
    scratch = BUILD / "roundtrip"
    scratch.mkdir(parents=True, exist_ok=True)
    for name in images:
        report = audit(RetailImage.load(name), units, address_book(name), BUILD / "delink", scratch)
        reports.append(report)
        print(f"{name} target relink: {report['verified_units']}/{report['total_units']} units verified")
        for kind, count in report['issues'].items():
            print(f"  {kind}: {count}")
    if output is not None:
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(reports, indent=2) + "\n")
    return int(any(report['issues'] for report in reports))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", choices=IMAGE_KEYS, action="append")
    parser.add_argument("--unit")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    images = tuple(IMAGE_NAMES[name] for name in args.image) if args.image else tuple(IMAGE_LAYOUTS)
    try:
        return run(images, output=args.output, unit=args.unit)
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    raise SystemExit(main())
