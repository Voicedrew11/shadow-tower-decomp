"""Audit the known code/data reference closure of all admitted game functions.

This is a conservative worklist over the shared semantic evidence, not a proof
that the reference census is exhaustive. Candidate paths stay candidate; a TSV
gap or referenced prefix never becomes a complete object just by being reached.
The report separates source ownership, configuration-only extents, and missing
models. Config-only bytes remain a default-build diagnostic unless a complete
independent provider/object/native-report/relink comparison currently verifies
the exact image-local owner. Inventory presence alone never supplies that proof.
"""

from __future__ import annotations

import argparse
import bisect
import json
from collections import Counter, deque
from dataclasses import asdict, dataclass
from pathlib import Path

from scripts.kf.manifest import Manifest, load as load_manifest
from scripts.kf.paths import BUILD, RETAIL_CONFIG
from scripts.kf.retail import IMAGE_LAYOUTS, IMAGE_NAMES, OVERLAY_SOURCES, read_tsv
from scripts.kf.sema import Context
from scripts.kf.sema.cfg import switch_tables
from scripts.kf.sema.evidence import Reference, TIER_RANK
from scripts.kf.sema.image import RetailImage
from scripts.kf.sema.index import Binding


@dataclass(frozen=True)
class DataExtent:
    image: str
    va: int
    size: int
    name: str
    storage: str
    source: str
    unit: str = ""
    confidence: str = ""
    evidence: str = ""
    note: str = ""
    datatype: str = ""
    classification: str = ""

    @property
    def end(self) -> int:
        return self.va + self.size

    @property
    def claimed(self) -> bool:
        return self.source in {"DATA", "RODATA", "JUMPTABLES", "SDATA"}

    @property
    def key(self) -> str:
        return f"{self.source}:{self.va:08x}:{self.size:x}:{self.name}"


def data_extents(
    manifest: Manifest, config_dir: Path = RETAIL_CONFIG,
) -> tuple[DataExtent, ...]:
    out = []
    for unit in manifest.units:
        out.extend(DataExtent(
            unit.image, datum.va, datum.size, datum.symbol, datum.storage,
            "DATA", unit.unit, "source-claim", unit.source,
        ) for datum in unit.data)
        for kind, va, size in unit.owned_ranges():
            out.append(DataExtent(
                unit.image, va, size, f"{unit.unit}.{kind.lower()}", "load", kind,
                unit.unit, "source-claim", unit.source,
            ))
    for filename in ("data_identities.tsv", "data.tsv"):
        _, rows = read_tsv(config_dir / filename)
        for row in rows:
            out.append(DataExtent(
                row["image"], int(row["va"], 0), int(row["size"], 0),
                row["name"] or row.get("kind", ""),
                row.get("storage", "load"), filename,
                confidence=row["confidence"],
                evidence=row.get("evidence", row.get("provenance", "")),
                note=row.get("note", ""),
                datatype=row.get("datatype", ""),
                classification=row.get("kind", row.get("storage", "")),
            ))
    return tuple(out)


def _function_key(function: Binding) -> str:
    return f"function:{function.va:08x}"


BIOS_VECTORS = {0xA0, 0xB0, 0xC0}
JR_T2 = 0x01400008


def proven_vendored_dispatch(
    img: RetailImage, function: Binding, site: int
) -> tuple[str, int | None] | None:
    """Prove an SDK member's indirect transfer from its own instruction values.

    Two value chains are accepted, both ending inside the same function:

    * a BIOS table stub, `li $t2, 0xA0|0xB0|0xC0` then `jr $t2` (the target is
      the fixed BIOS vector); and
    * an SN startup table loop whose count register is the link-time constant
      zero (`lui r,0; addiu r,r,0` then `beqz r` before the `jalr`), so the
      call cannot execute.

    These specific proofs are recorded on the witness row; every other
    indirect transfer inside a Sony member is exempted as `vendored-indirect`
    by `audit` (a project decision, not a proof). Game-code `jr`/`jalr` stays
    unresolved, as AGENTS.md requires, unless it is a proven switch.
    """
    word = img.u32(site)
    if word is None:
        return None
    if word == JR_T2:
        for back in (4, 8):
            prior = img.u32(site - back) if site - back >= function.va else None
            if prior is not None and prior >> 16 == 0x240A and prior & 0xFFFF in BIOS_VECTORS:
                return "bios-vector-stub", prior & 0xFFFF
        return None
    if word & 0xFC1FFFFF != 0x0000F809:  # jalr $ra, rs
        return None
    zero_registers: set[int] = set()
    for address in range(function.va, site, 4):
        current = img.u32(address)
        following = img.u32(address + 4)
        if current is None or following is None:
            return None
        register = (current >> 16) & 0x1F
        if (
            current & 0xFFE0FFFF == 0x3C000000  # lui r, 0
            and following == (0x24000000 | register << 21 | register << 16)  # addiu r, r, 0
        ):
            zero_registers.add(register)
        if current >> 26 == 0x04 and (current >> 16) & 0x1F == 0:  # beqz rs
            if (current >> 21) & 0x1F in zero_registers:
                return "empty-link-time-table", None
    return None


def proven_switch_dispatch(img: RetailImage, function: Binding) -> dict[int, dict]:
    """Switch `jr` sites of a game function, proved by the shared CFG rule.

    `scripts.kf.sema.cfg.switch_table` is the only switch-table prover: a
    bounded `sltiu`/`beqz` index, a `lui`/`addiu` table base (the `lui` may
    sit in the bound's delay slot), `sll`/`addu`/`lw`/`jr`, no branch into
    the scanned run, the index unchanged, and every table word an instruction
    start inside this function. Vendored members are not proved by it (their
    indirect sites are exempted wholesale as `vendored-indirect`).
    Returns the witness detail per `jr` site.
    """
    if function.vendored or function.fragments != 1:
        return {}
    payload = img.read(function.va, function.body_size)
    if payload is None:
        return {}
    return {
        site: {"site": site, "pattern": "switch-table", "table": proven.table,
               "entries": len(proven.targets)}
        for site, proven in switch_tables(function, payload, img.u32).items()
    }


def access_span(reference: Reference, img: RetailImage) -> tuple[int, int]:
    """Minimum directly observed memory span, or one anchor byte for an address.

    An address-taking expression does not establish an object extent. The
    returned span is only a boundary-crossing control, never a size proposal.
    LWL/LWR/SWL/SWR operate on the containing aligned word.
    """
    assert reference.target is not None
    target = reference.target
    word = img.u32(reference.paired_site) if reference.paired_site is not None else None
    if reference.referent is not None and (word is None or word >> 26 in {8, 9}):
        # Taking an address (including a biased base or one-past pointer) does
        # not read S+A. Follow the named allocation; memory-op lows below still
        # check their real access against its extent.
        return reference.referent.va, 1
    if word is None:
        return target, 1
    opcode = word >> 26
    if opcode in {0x22, 0x26, 0x2A, 0x2E}:
        return target & ~3, 4
    width = {
        0x20: 1, 0x24: 1, 0x28: 1,
        0x21: 2, 0x25: 2, 0x29: 2,
        0x23: 4, 0x2B: 4,
    }.get(opcode, 1)
    return target, width


def audit(
    image: str,
    functions: tuple[Binding, ...],
    extents: tuple[DataExtent, ...],
    references: tuple[Reference, ...],
    img: RetailImage,
    *,
    config_comparisons: tuple = (),
    include_entry: bool = False,
    link_labels: frozenset[int] = frozenset(),
    host_functions: tuple[Binding, ...] = (),
    host_extents: tuple[DataExtent, ...] = (),
) -> dict:
    """Lossless reference worklist with weakest-edge reachability witnesses.

    Roots are selected by admitted non-vendor identity, not manifestation or
    fuzzy score. A reached vendor routine is traversed without adding it to the
    game root/progress denominator. Entire admitted function/object extents are
    scanned conservatively; candidate discovery does not assert execution.
    Optional executable entry roots include SDK startup without changing the
    game-function denominator.

    An overlay's references outside its own load window reach its host
    program (`retail.referent_image`). They are checked against the host's
    owners (`host_functions`, `host_extents`) and recorded as cross-image
    edges; the host's own closure is audited with the host image.
    """
    if img.image != image:
        raise ValueError("reference audit image does not match retail bytes")
    functions = tuple(f for f in functions if f.image == image)
    extents = tuple(d for d in extents if d.image == image)
    references = tuple(sorted(
        (r for r in references if r.image == image),
        key=lambda r: (r.site, r.target if r.target is not None else -1, r.kind),
    ))
    sites = [r.site for r in references]
    roots = tuple(f for f in functions if not f.vendored)
    nodes: dict[str, Binding | DataExtent] = {}
    visits: dict[str, tuple[int, str | None, int | None]] = {}
    outgoing: dict[str, list[int]] = {}
    destinations: dict[int, list[str]] = {}
    issues: dict[tuple, dict] = {}
    dispatch_witnesses: dict[int, dict] = {}
    switch_witnesses: dict[int, dict] = {}
    switches: dict[str, dict[int, dict]] = {}
    queue: deque[str] = deque()

    def issue(code: str, key: str, ref: int | None = None, **detail) -> None:
        issues[code, key, ref] = {"code": code, "owner": key, "reference": ref, **detail}

    def visit(node: Binding | DataExtent, rank: int, parent: str | None, via: int | None):
        key = _function_key(node) if isinstance(node, Binding) else node.key
        if rank > visits.get(key, (0, None, None))[0]:
            nodes[key] = node
            visits[key] = rank, parent, via
            queue.append(key)
        return key

    def owners(va: int) -> list[Binding | DataExtent]:
        code = [f for f in functions if f.va <= va < f.body_end]
        data = [d for d in extents if d.va <= va < d.end]
        # The source owner replaces its finer census/identity rows, including
        # interior string and switch-table entries. Config rows remain attached
        # to the selected range as evidence rather than duplicate allocations.
        claimed = [d for d in data if d.claimed]
        if claimed:
            return [*code, *claimed]
        identities = [d for d in data if d.source == "data_identities.tsv"]
        return [*code, *(identities or data)]

    host = OVERLAY_SOURCES.get(image)
    window = IMAGE_LAYOUTS[image] if host is not None else None
    host_name = host.host if host is not None else ""
    host_functions = tuple(f for f in host_functions if f.image == host_name)
    host_extents = tuple(d for d in host_extents if d.image == host_name)
    cross_image: dict[int, list[str]] = {}

    def host_owners(va: int) -> list[str]:
        code = [_function_key(f) for f in host_functions if f.va <= va < f.body_end]
        data = [d for d in host_extents if d.va <= va < d.end]
        claimed = [d for d in data if d.claimed]
        identities = [d for d in data if d.source == "data_identities.tsv"]
        return [f"{host_name}:{key}"
                for key in (*code, *(d.key for d in (claimed or identities or data)))]

    for root in roots:
        visit(root, TIER_RANK["proven"], None, None)
    entry = img.layout.entry
    include_entry = include_entry and entry is not None
    entry_roots = tuple(f for f in functions if include_entry and f.contains_body(entry))
    for root in entry_roots:
        visit(root, TIER_RANK["proven" if len(entry_roots) == 1 else "candidate"], None, None)
    if include_entry and len(entry_roots) != 1:
        issue("entry-owner-not-unique", image, entry=img.layout.entry, owners=len(entry_roots))
    if not roots:
        issue("no-game-roots", image)
    while queue:
        key = queue.popleft()
        node = nodes[key]
        rank = visits[key][0]
        end = node.body_end if isinstance(node, Binding) else node.end
        if isinstance(node, Binding) and node.fragments != 1:
            issue("fragmented-function", key, fragments=node.fragments)
            continue
        if key not in outgoing:
            lo, hi = bisect.bisect_left(sites, node.va), bisect.bisect_left(sites, end)
            outgoing[key] = list(range(lo, hi))
            if isinstance(node, Binding):
                # The semantic evidence already folds these into the CFG; the
                # audit re-proves them so each reached switch keeps a witness.
                switches[key] = proven_switch_dispatch(img, node)
                for site, witness in switches[key].items():
                    switch_witnesses[site] = {"owner": key, **witness}
        for number in outgoing[key]:
            reference = references[number]
            if reference.site + 4 > end:
                issue("relocation-site-crosses-owner", key, number)
            if reference.paired_site is not None and not (
                node.va <= reference.paired_site and reference.paired_site + 4 <= end
            ):
                issue("relocation-pair-crosses-owner", key, number)
            if reference.tier == "rejected":
                # Explicitly rejected hypotheses are retained but not followed.
                # A byte-invalid live TSV claim is a distinct curation failure.
                if not any(o.status == "rejected" for o in reference.origins):
                    issue("invalid-live-reference", key, number)
                continue
            if reference.target is None:
                if isinstance(node, Binding) and node.vendored:
                    # Project decision (2026-11-17): every indirect transfer
                    # inside a vendored Sony member is exempt from the gate.
                    # It stays auditable as a witness row; a transfer proved
                    # from the member's own instruction values also names
                    # that proof (BIOS vector stub, empty link-time table).
                    proven = proven_vendored_dispatch(img, node, reference.site)
                    dispatch_witnesses[number] = {
                        "owner": key, "site": reference.site,
                        "pattern": "vendored-indirect",
                        "proof": proven[0] if proven else None,
                        "target": proven[1] if proven else None,
                    }
                    continue
                if reference.site in switches.get(key, {}):
                    # A game switch whose every case lands in this function's
                    # body: the transfer adds no edge; its witness row is kept.
                    continue
                issue("unresolved-indirect-control", key, number)
                continue
            path_rank = min(rank, TIER_RANK[reference.tier])
            if window is not None and not window.contains(reference.destination):
                # A cross-image edge into the host: owned there, not traversed.
                hosts = host_owners(reference.destination)
                if not hosts:
                    issue("unmodeled-target", key, number, target=reference.target,
                          image=host_name)
                destinations[number] = hosts
                cross_image[number] = hosts
                continue
            found = owners(reference.destination)
            if not found and reference.target in link_labels:
                # A curated zero-size link-layout label (BSS_END) names a
                # boundary, not storage, so it has no owner to reach.
                destinations[number] = []
                continue
            if not found:
                issue("unmodeled-target", key, number, target=reference.target)
                destinations[number] = []
                continue
            if len(found) > 1:
                issue("ambiguous-target-owner", key, number, target=reference.target)
                path_rank = min(path_rank, TIER_RANK["candidate"])
            destinations[number] = []
            for owner in found:
                target_key = visit(owner, path_rank, key, number)
                destinations[number].append(target_key)
                start, width = access_span(reference, img)
                owner_end = owner.body_end if isinstance(owner, Binding) else owner.end
                if start < owner.va or start + width > owner_end:
                    issue("access-crosses-owner", key, number, target=reference.target,
                          access_start=start, access_size=width)

    tier = {rank: name for name, rank in TIER_RANK.items()}
    range_rows = []
    for key, node in sorted(nodes.items()):
        rank, parent, via = visits[key]
        row = {
            "key": key, "reachability": tier[rank], "via_owner": parent,
            "via_reference": via,
        }
        if isinstance(node, Binding):
            row.update({**node.as_dict(), "kind": "function", "function_kind": node.kind})
            # A reached instruction range cannot silently coexist with a data
            # claim merely because no reference targets the overlap. Preserve
            # the conflicting metadata for review; do not reclassify either
            # owner from a byte-pattern scan or an admitted extent alone.
            if node.fragments == 1:
                overlaps = [
                    {**asdict(d), "data_owner": d.key,
                     "overlap_va": max(node.va, d.va),
                     "overlap_size": min(node.body_end, d.end) - max(node.va, d.va)}
                    for d in extents if d.va < node.body_end and node.va < d.end
                ]
                if overlaps:
                    issue("code-data-owner-overlap", key, overlaps=overlaps)
        else:
            row.update({"kind": "data", **asdict(node)})
            row["comparison"] = "source-owned" if node.claimed else "config-only"
            row["inventory"] = [
                asdict(d) for d in extents if not d.claimed
                and d.va < node.end and node.va < d.end
            ]
            if not node.claimed:
                verified = [c for c in config_comparisons if c.image == image and c.matched
                            and c.va == node.va and c.size == node.size and c.identity == node.name]
                if len(verified) == 1:
                    row["comparison"] = "config-provider-matched"
                    row["comparison_unit"] = verified[0].unit
                else:
                    issue("config-only-data-not-compared", key)
        range_rows.append(row)

    reference_rows = []
    for key in sorted(outgoing):
        rank = visits[key][0]
        for number in outgoing[key]:
            reference = references[number]
            path_rank = min(rank, TIER_RANK[reference.tier])
            if len(destinations.get(number, [])) > 1:
                path_rank = min(path_rank, TIER_RANK["candidate"])
            if path_rank == TIER_RANK["candidate"]:
                issue("candidate-reference-path", key, number)
            reference_rows.append({
                "id": number, **reference.as_dict(), "source_owner": key,
                "path_tier": tier[path_rank], "target_owners": destinations.get(number, []),
            })
    issue_rows = sorted(issues.values(), key=lambda r: (
        r["code"], r["owner"], -1 if r["reference"] is None else r["reference"],
    ))
    return {
        "schema": "kf-data-reachability-v1", "image": image,
        "scope": "known-reference-census", "complete_reachable_bytes_proven": False,
        "limitations": [
            "Computed/GP-relative addresses and references absent from the census need audit.",
            "Config extents may be referenced prefixes, not complete allocations.",
            "Source ownership does not prove matched bytes; the data gate checks objects separately.",
            "Whole admitted extents are traversed conservatively, not dynamic execution paths.",
        ],
        "roots": [_function_key(f) for f in roots],
        "entry_roots": [_function_key(f) for f in entry_roots],
        "summary": {
            "game_roots": len(roots),
            "reached_vendor_functions": sum(
                isinstance(n, Binding) and n.vendored for n in nodes.values()
            ),
            "source_owned_data_ranges": sum(
                isinstance(n, DataExtent) and n.claimed for n in nodes.values()
            ),
            "config_only_data_ranges": sum(
                isinstance(n, DataExtent) and not n.claimed for n in nodes.values()
            ),
            "config_matched_data_ranges": sum(r.get("comparison") == "config-provider-matched"
                                              for r in range_rows),
            "reference_occurrences": len(reference_rows),
            "vendored_indirect_exempt": len(dispatch_witnesses),
            "proven_vendored_dispatch": sum(
                w["proof"] is not None for w in dispatch_witnesses.values()
            ),
            "proven_switch_dispatch": len(switch_witnesses),
            "host_references": len(cross_image),
            "issues": dict(sorted(Counter(r["code"] for r in issue_rows).items())),
        },
        "ranges": range_rows, "references": reference_rows, "issues": issue_rows,
        "vendored_dispatch": [dispatch_witnesses[n] for n in sorted(dispatch_witnesses)],
        "switch_dispatch": [switch_witnesses[n] for n in sorted(switch_witnesses)],
        "config_comparisons": [asdict(c) for c in config_comparisons if c.image == image],
    }


def run(images: tuple[str, ...], *, output: Path | None = None) -> int:
    from scripts.kf.config_data import audit as compare_config_data
    from scripts.kf.roundtrip import link_labels

    manifest = load_manifest()
    extents = data_extents(manifest)
    comparisons = tuple(compare_config_data(images, modules=manifest.modules()))
    reports = []
    for image in images:
        ctx = Context(image)
        host = OVERLAY_SOURCES.get(image)
        host_functions = Context(host.host).idx.functions if host is not None else ()
        report = audit(image, ctx.idx.functions, extents, ctx.refs.references, ctx.img,
                       config_comparisons=comparisons,
                       link_labels=frozenset(link_labels(image).values()),
                       host_functions=host_functions, host_extents=extents)
        reports.append(report)
        summary = report["summary"]
        print(f"{image} known-reference closure: {summary['game_roots']} game roots, "
              f"{summary['reached_vendor_functions']} reached vendor functions, "
              f"{summary['source_owned_data_ranges']} source-owned / "
              f"{summary['config_only_data_ranges']} config-only data ranges")
        if summary["vendored_indirect_exempt"]:
            print(f"  vendored Sony indirect sites exempt (witnessed): "
                  f"{summary['vendored_indirect_exempt']} "
                  f"({summary['proven_vendored_dispatch']} with a value-chain proof)")
        if summary["host_references"]:
            print(f"  references into {OVERLAY_SOURCES[image].host} (owned there): "
                  f"{summary['host_references']}")
        if summary['config_matched_data_ranges']:
            print(f"  independent provider/retail matches: {summary['config_matched_data_ranges']} config ranges")
        for code, count in summary["issues"].items():
            print(f"  {code}: {count}")
        print("  exhaustive reachable-byte coverage is not yet proven")
    if output is not None:
        # Reports are build products, never curated inventory updates.
        resolved = output.resolve()
        if not resolved.is_relative_to(BUILD.resolve()):
            raise ValueError("reachability reports must be written under this worktree's build/")
        resolved.parent.mkdir(parents=True, exist_ok=True)
        resolved.write_text(json.dumps(reports, indent=2, sort_keys=True) + "\n")
    return int(any(report["issues"] for report in reports))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="kf verify reachability", description=__doc__)
    aliases = dict(IMAGE_NAMES)
    parser.add_argument("--image", action="append", choices=tuple(aliases))
    parser.add_argument("--output", type=Path,
                        help="write full witness/evidence JSON under build/")
    args = parser.parse_args(argv)
    images = tuple(aliases[i] for i in args.image) if args.image else tuple(IMAGE_LAYOUTS)
    return run(images, output=args.output)


if __name__ == "__main__":
    raise SystemExit(main())
