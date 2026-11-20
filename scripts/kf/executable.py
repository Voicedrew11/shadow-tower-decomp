"""Compile C and link original Psy-Q objects/libraries without output rewriting."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import struct

from scripts.kf.delink import load_catalog
from scripts.kf.sdk import compile_c
from scripts.psxbuild.sdk import ASPSX_VERSION
from scripts.psxbuild.link import build_image as link_image
from scripts.kf.executable_diff import compare_executables, render_html
from scripts.kf.graph import IncludeScanner
from scripts.kf.manifest import Manifest, Profile, Unit, load as load_manifest
from scripts.kf.paths import BUILD, REPO, RETAIL_CONFIG
from scripts.kf.readme import refresh_executable
from scripts.kf.retail import (
    IMAGE_KEYS, IMAGE_LAYOUTS, IMAGE_NAMES, OVERLAY_SOURCES, image_key, read_only_mode,
)
from scripts.kf.sema.image import RetailImage


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compile_unit(unit: Unit, profile: Profile, root: Path, index: int) -> dict:
    if profile.language != 'c' or profile.aspsx_version != ASPSX_VERSION:
        raise ValueError(f'{unit.unit}: executable build requires C and ASPSX {ASPSX_VERSION}')
    if profile.optimization is None:
        raise ValueError(f'{unit.unit}: missing compiler optimization profile')
    result = compile_c(
        unit.source_path, root, f'U{index:04d}', compiler=profile.compiler,
        optimization=profile.optimization, small_data=profile.small_data,
        include_dirs=(REPO / 'include', REPO / 'vendor/include', Path(os.environ['PSYQ_INCLUDE'])),
        cc1_flags=profile.cc1_flags, defines=unit.defines,
        read_only=read_only_mode(unit.image))
    dependencies = [unit.source, *IncludeScanner().headers(unit.source)]
    return {**result, 'unit': unit.unit, 'source': unit.source,
            'source_sha256': {name: file_hash(REPO / name) for name in dependencies}}


def cpe_loads(data: bytes) -> tuple[int | None, list[tuple[int, bytes]]]:
    """Read native CPE records for verification only; never produce output bytes."""
    if not data.startswith(b'CPE\x01'):
        raise ValueError('expected native CPE v1 output')
    position, entry, loads = 4, None, []
    while position < len(data):
        tag = data[position]
        position += 1
        if tag == 0:
            if position != len(data):
                raise ValueError('bytes after native CPE end record')
            return entry, loads
        if tag == 8:
            size = 1
        elif tag == 3:
            size = 6
            if position + size <= len(data):
                register, value = struct.unpack_from('<HI', data, position)
                if register == 0x90:
                    entry = value
        elif tag == 1:
            if position + 8 > len(data):
                raise ValueError('truncated CPE load record')
            address, length = struct.unpack_from('<II', data, position)
            size = 8 + length
            if position + size <= len(data):
                loads.append((address, data[position + 8:position + size]))
        else:
            raise ValueError(f'unsupported native CPE record {tag}')
        if position + size > len(data):
            raise ValueError('truncated native CPE record')
        position += size
    raise ValueError('missing native CPE end record')


def unclaimed_game_functions(name: str, units: list[Unit]) -> list[int]:
    """Admitted game functions of the image that no unit's claimed run covers.

    Sources that claim no retail function (free-form controls) are not a
    partial reconstruction of the census, so they report nothing.
    """
    if not any(unit.functions for unit in units):
        return []
    claimed = {function.va for unit in units for function in unit.functions}
    return [function.va for function in load_catalog(RETAIL_CONFIG).functions[name]
            if not function.provider and function.va not in claimed]


def host_symbols(name: str, objects: list[Path]) -> dict[str, int]:
    """Absolute addresses of the host symbols an overlay's objects reference.

    Every name the objects reference without defining must be one identity of
    the overlay's host program outside the overlay's own window (the same
    address book the target relink uses); anything else fails the link.
    """
    from scripts.kf.lnk import read as read_object
    from scripts.kf.roundtrip import ADDRESS_NAME, address_book

    defined, referenced = set(), set()
    for path in objects:
        native = read_object(path.read_bytes())
        for symbol in native.symbols.values():
            (referenced if symbol.section is None and not symbol.common else defined).add(
                symbol.name)
    host = OVERLAY_SOURCES[name].host
    book = address_book(host)
    layout = IMAGE_LAYOUTS[name]
    result, missing = {}, []
    for symbol in sorted(referenced - defined):
        addresses = book.get(symbol, set())
        address_name = ADDRESS_NAME.fullmatch(symbol)
        if not addresses and address_name:
            # An unresolved DAT_ identity spells its literal address, as in
            # the target relink (roundtrip.externals).
            addresses = {int(address_name.group(1), 16)}
        if len(addresses) != 1 or layout.contains(next(iter(addresses))):
            missing.append(symbol)
            continue
        result[symbol] = next(iter(addresses))
    if missing:
        raise ValueError(f'{name}: references with no unique {host} identity: '
                         + ', '.join(missing))
    return result


def build_image(name: str, manifest: Manifest, root: Path) -> dict:
    units = [u for u in manifest.units if u.image == name and u.scope != 'vendored']
    if not units:
        # An image no source claims yet has nothing to link; that is not a failure.
        root.mkdir(parents=True, exist_ok=True)
        report = {'image': name, 'linked': False, 'phase': 'not-started', 'units': []}
        (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
        return report
    unclaimed = unclaimed_game_functions(name, units)
    if unclaimed:
        # A partial reconstruction still calls and reads game code no source
        # provides, so PSYLINK cannot resolve it. Its units are verified by the
        # target relink and data gates instead; that is not a link failure.
        root.mkdir(parents=True, exist_ok=True)
        report = {'image': name, 'linked': False, 'phase': 'partial',
                  'units': [unit.unit for unit in units],
                  'unclaimed_game_functions': len(unclaimed)}
        (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
        return report
    report = link_image(
        name, root, units,
        lambda unit, output, index: compile_unit(unit, manifest.profiles[unit.profile], output, index),
        repo=REPO, load_address=IMAGE_LAYOUTS[name].load_address,
        host_symbols=((lambda objects: host_symbols(name, objects))
                      if name in OVERLAY_SOURCES else None))
    report.update(output_rewritten=False, retail_payload_inputs=[],
                  game_execution_tested=False, historical_toolchain_proven=False)
    (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def compare(actual: bytes, image: RetailImage) -> dict:
    expected = image.data
    # A PS-X EXE carries a 2048-byte header; a headerless overlay is all load.
    header = image.layout.header_size
    common = min(len(actual), len(expected))
    differences = [i for i in range(common) if actual[i] != expected[i]]
    differences.extend(range(common, max(len(actual), len(expected))))
    first = differences[0] if differences else None
    first_load = next((i for i in differences if i >= header), None)
    return {'file_equal': actual == expected, 'retail_size': len(expected), 'linked_size': len(actual),
            'retail_sha256': hashlib.sha256(expected).hexdigest(),
            'linked_sha256': hashlib.sha256(actual).hexdigest(),
            'differing_bytes': len(differences), 'first_difference_file_offset': first,
            'first_difference_va': image.load_start + first - header
            if first is not None and first >= header else None,
            'first_load_difference_file_offset': first_load,
            'first_load_difference_va': image.load_start + first_load - header
            if first_load is not None else None,
            'first_load_difference_retail': expected[first_load:first_load+16].hex()
            if first_load is not None else None,
            'first_load_difference_linked': actual[first_load:first_load+16].hex()
            if first_load is not None else None,
            'header_differing_bytes': sum(i < header for i in differences),
            'load_differing_bytes': sum(i >= header for i in differences)}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--image', choices=IMAGE_KEYS, action='append')
    parser.add_argument('--compare-only', action='store_true',
                        help='compare existing native EXEs without rebuilding or changing them')
    args = parser.parse_args(argv)
    names = [IMAGE_NAMES[key] for key in args.image] if args.image else list(IMAGE_LAYOUTS)
    try:
        manifest = None if args.compare_only else load_manifest()
        verified = {name: RetailImage.load(name) for name in names} if args.compare_only else {}
        reports = []
        filename = 'fuzzy-comparison.json' if args.compare_only else 'build.json'
        for name in names:
            root = BUILD / 'link' / image_key(name)
            report = ({'image': name, 'mode': 'compare-existing', 'executable': str(root / name)}
                      if args.compare_only else build_image(name, manifest, root))
            if args.compare_only:
                actual = (root / name).read_bytes()
                report['comparison'] = compare(actual, verified[name])
                diff = report['comparison']
                print(f'{name}: existing EXE has {diff["linked_size"]} bytes; '
                      f'{diff["differing_bytes"]} file bytes differ from retail')
                if name in OVERLAY_SOURCES:
                    # The island report models PS-X EXE load areas; an
                    # overlay keeps only the strict file comparison.
                    report['layout_tolerant'] = None
                    (root / filename).write_text(json.dumps(report, indent=2) + '\n')
                    reports.append(report)
                    continue
                report['layout_tolerant'] = compare_executables(verified[name].data, actual)
                fuzzy = report['layout_tolerant']
                print(f'  island-aligned byte similarity: {fuzzy["byte_similarity_percent"]:.2f}%; '
                      f'nonzero bytes: {fuzzy["nonzero_byte_similarity_percent"]:.2f}%; '
                      f'{fuzzy["island_count"]} islands (heuristic, not exactness)')
                html = root / 'fuzzy-comparison.html'
                html.write_text(render_html(name, diff, fuzzy))
                print(f'  report: {html}')
            elif report['linked']:
                print(f'{name}: built {report["executable"]}')
            elif report['phase'] == 'not-started':
                print(f'{name}: no source units yet; link skipped')
            elif report['phase'] == 'partial':
                print(f'{name}: partial reconstruction ({report["unclaimed_game_functions"]} '
                      'game functions unclaimed); native link skipped')
            else:
                print(f'{name}: {report["phase"]} failed; see {root}: {report["error"]}')
            (root / filename).write_text(json.dumps(report, indent=2) + '\n')
            reports.append(report)
        (BUILD / 'link').mkdir(parents=True, exist_ok=True)
        (BUILD / 'link' / filename).write_text(json.dumps(reports, indent=2) + '\n')
        if args.compare_only:
            refresh_executable(BUILD / 'link')
        return int(any(not report.get('linked', args.compare_only)
                       and report.get('phase') not in {'not-started', 'partial'}
                       for report in reports))
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    raise SystemExit(main())
