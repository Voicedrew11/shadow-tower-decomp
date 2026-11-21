"""Reviewed run-time code pointers and the game call sites that use them.

Some game `jalr`/`jr` sites call through a pointer whose value is installed
at run time (a struct member set from an argument, a handler table an
overlay brings), so no value chain inside the calling function proves the
target. Two manually curated tables state the evidence instead:

* `config/retail/indirect_pointers.tsv`: every value source the review found
  for one pointer (a typed member path such as `AreaHooks.unk40`). A source
  is either `table-word`, a code pointer stored in a data word (a handler
  table slot), or `address`, an address-taking `lui`/low pair in the audited
  image's code that hands a function's address on (an argument, a store).
  `source_image` may be `FDAT_*.BIN` for "the same word in every overlay of
  this program". The `evidence` column records why the list is complete
  (every store to the member, every caller of the installer).
* `config/retail/indirect_sites.tsv`: which pointer a game indirect site
  calls through, with the offset of the `lw` that loads it.

`resolve` checks each source against the bytes and the reviewed relocation
census: a `table-word` needs a reviewed `mips32_candidate` row at that word
whose target is the word's value, an `address` a validated reviewed
`mips_hi16_lo16` reference at that `lui`; either target must be a function
start in its image. The reachability audit then checks each site (the owner,
the instruction, and that every definition of the jump register reaching it
is `lw r, load_offset(base)`) and follows the pointer's targets. Completeness
of a source list is the reviewed claim, not a proof; a wrong or missing
source is a curation error like any other census row.
"""

from __future__ import annotations

from dataclasses import dataclass
from fnmatch import fnmatchcase
from functools import lru_cache
from pathlib import Path

from scripts.kf.paths import RETAIL_CONFIG
from scripts.kf.retail import OVERLAY_SOURCES, parse_int, read_tsv

POINTERS = "indirect_pointers.tsv"
SITES = "indirect_sites.tsv"
SOURCES = ("table-word", "address")


@dataclass(frozen=True)
class PointerTarget:
    pointer: str
    image: str  # the image whose function the pointer may hold
    va: int
    source: str
    source_image: str
    source_va: int


@dataclass(frozen=True)
class IndirectSite:
    image: str
    site: int
    owner: int
    pointer: str
    load_offset: int
    evidence: str


def load_sites(image: str, config_dir: Path = RETAIL_CONFIG) -> tuple[IndirectSite, ...]:
    path = config_dir / SITES
    if not path.is_file():
        return ()
    _, rows = read_tsv(path)
    return tuple(
        IndirectSite(row["image"], parse_int(row["site_va"]), parse_int(row["function_va"]),
                     row["pointer"], parse_int(row["load_offset"]), row["evidence"])
        for row in rows if row["image"] == image
    )


@lru_cache(maxsize=2)
def _function_starts(config_dir: Path) -> dict[str, frozenset[int]]:
    starts: dict[str, set[int]] = {}
    for name in ("functions.tsv", "functions_vendored.tsv"):
        _, rows = read_tsv(config_dir / name)
        for row in rows:
            starts.setdefault(row["image"], set()).add(parse_int(row["va"]))
    return {image: frozenset(vas) for image, vas in starts.items()}


def source_images(image: str, pattern: str) -> tuple[str, ...]:
    """The images a `source_image` names: itself, or the matching overlays."""
    if "*" not in pattern:
        return (pattern,)
    return tuple(sorted(name for name, source in OVERLAY_SOURCES.items()
                        if source.host == image and fnmatchcase(name, pattern)))


def resolve(
    image: str,
    references,
    *,
    config_dir: Path = RETAIL_CONFIG,
    read_word=None,
) -> tuple[dict[str, tuple[PointerTarget, ...]], list[str]]:
    """Each pointer's checked targets for IMAGE, and every row problem.

    REFERENCES is IMAGE's semantic evidence (`address` sources); READ_WORD
    reads `(image, va)` (defaults to the retail images).
    """
    path = config_dir / POINTERS
    if not path.is_file():
        return {}, []
    if read_word is None:
        from scripts.kf.sema.image import RetailImage

        loaded: dict[str, RetailImage] = {}

        def read_word(name: str, va: int) -> int | None:
            if name not in loaded:
                loaded[name] = RetailImage.load(name)
            return loaded[name].u32(va)

    _, rows = read_tsv(path)
    _, relocs = read_tsv(config_dir / "relocs.tsv")
    words = {
        (row["image"], parse_int(row["site_va"])): parse_int(row["target_va"])
        for row in relocs
        if row["kind"] == "mips32_candidate" and row["status"] == "reviewed"
    }
    addresses = {
        reference.site: reference.target for reference in references
        if reference.kind == "address" and reference.tier in {"validated", "proven"}
        and any(origin.status == "reviewed" for origin in reference.origins)
    }
    starts = _function_starts(config_dir)
    targets: dict[str, list[PointerTarget]] = {}
    problems: list[str] = []
    for row in rows:
        if row["image"] != image:
            continue
        pointer, source, va = row["pointer"], row["source"], parse_int(row["source_va"])
        names = source_images(image, row["source_image"])
        if source not in SOURCES or not names or not row["evidence"].strip():
            problems.append(f"{pointer}: bad source row {source} {row['source_image']}")
            continue
        for name in names:
            label = f"{pointer}: {source} {name}:0x{va:08x}"
            if source == "table-word":
                value = read_word(name, va)
                if value is None or words.get((name, va)) != value:
                    problems.append(f"{label} has no reviewed pointer row matching its bytes")
                    continue
            else:
                if name != image:
                    problems.append(f"{label}: an address source must be in {image}")
                    continue
                value = addresses.get(va)
                if value is None:
                    problems.append(f"{label} has no validated reviewed address reference")
                    continue
            if value not in starts.get(name, frozenset()):
                problems.append(f"{label} holds 0x{value:08x}, not a function start")
                continue
            targets.setdefault(pointer, []).append(
                PointerTarget(pointer, name, value, source, name, va))
    return {pointer: tuple(dict.fromkeys(found)) for pointer, found in targets.items()}, problems
