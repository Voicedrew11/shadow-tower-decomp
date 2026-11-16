"""Link PlayStation programs with the Psy-Q SDK."""

from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from .sdk import dos_run, tool_succeeded
from .sdk_compat import library_input


@dataclass(frozen=True)
class ImageLink:
    """How one Shadow Tower program is linked.

    `libraries` is the `inclib` order. PSYLINK visits the archives in this
    order and returns to the first one after every member it takes, so the
    groups of members from each archive follow one another in the linked text
    in this order, and a member's dependencies on an archive listed earlier
    follow it directly. Archives no member is taken from have no effect.
    `startup` puts NONE3.OBJ first; `allocator` puts MALLOC.OBJ after it.
    Both precede the game units.
    """

    libraries: tuple[str, ...]
    startup: bool = True
    allocator: bool = False
    evidence: str = ''


# The retail member groups, read from whole-object masked matches of the
# Runtime Library 4.2 archives against each image's text (relocation fields
# wildcarded); see docs/executable-linking.md. Libraries listed only where a
# member of theirs is linked. LIBC2 members that retail links are
# byte-identical to LIBC ones, so LIBC2 is not listed.
IMAGE_LINKS = {
    # LIBAPI stubs (Load, Exec, _96_init, _96_remove, Enter/ExitCriticalSection)
    # follow main; LIBSN SNMAIN (the entry point) follows them, then its
    # InitHeap dependency from the earlier-listed LIBAPI.
    'PSX.EXE': ImageLink(('LIBAPI', 'LIBSN'), startup=False,
                         evidence='retail PSX.EXE byte-identical'),
    'STR.EXE': ImageLink(('LIBCD', 'LIBSPU', 'LIBSND', 'LIBPRESS', 'LIBETC', 'LIBAPI', 'LIBC',
                          'LIBDS', 'LIBGPU', 'LIBPAD'), allocator=True,
                         evidence='retail member groups CD SND/SPU PRESS ETC API C DS GPU PAD'),
    'ST.EXE': ImageLink(('LIBCARD', 'LIBCD', 'LIBSPU', 'LIBSND', 'LIBGTE', 'LIBETC', 'LIBAPI',
                         'LIBC', 'LIBGPU', 'LIBPAD'),
                        evidence='retail member groups CARD CD SPU/SND GTE ETC API C GPU PAD'),
    'TITLE.EXE': ImageLink(('LIBCD', 'LIBSPU', 'LIBSND', 'LIBETC', 'LIBAPI', 'LIBC', 'LIBGPU',
                            'LIBPAD'), allocator=True,
                           evidence='retail member groups CD SND/SPU ETC API C GPU PAD'),
    'OPTXT.EXE': ImageLink(('LIBCD', 'LIBSPU', 'LIBSND', 'LIBETC', 'LIBAPI', 'LIBC', 'LIBGPU',
                            'LIBPAD'), allocator=True,
                           evidence='retail member groups CD SND/SPU ETC API C GPU PAD'),
    'VS.EXE': ImageLink(('LIBCARD', 'LIBCD', 'LIBSPU', 'LIBSND', 'LIBSN', 'LIBGTE', 'LIBETC',
                         'LIBAPI', 'LIBC', 'LIBGPU'),
                        evidence='retail member groups CARD CD SPU/SND SN GTE ETC API C GPU'),
    'END.EXE': ImageLink(('LIBCD', 'LIBSPU', 'LIBSND', 'LIBPRESS', 'LIBETC', 'LIBAPI', 'LIBC',
                          'LIBDS', 'LIBGPU'), allocator=True,
                         evidence='retail member groups CD SND/SPU PRESS ETC API C DS GPU'),
}
ENTRY = '__SN_ENTRY_POINT'
# Psy-Q 4.x overlay startup (identical in the 4.5 and 4.6 kits): clears the
# bss group, sets $gp and calls main. Retail text starts with it at the entry.
STARTUP = 'NONE3.OBJ'
STARTUP_SHA256 = '063efc868877ca84fcbe1fdc3afe9656e6c7933cb68156e08d04d35b76166899'
MALLOC_OBJECT_SHA256 = '628e405fd0e3acfff2ce9d4a15d481f0aa36398c14e9eae0a82b7ff0a86a74c9'
# PSYLINK 2.73D, the DOS build of the linker from the SDevTC 2.4 CD.
LINKER_SHA256 = '2014db126ceaea6d287067ea3f829eea1745836fca570f22ef379eaff7832dd2'
# CPE2X 1.5 writes every header field; /CA selects the "North America area"
# title of the retail SLUS-00863 headers.
CONVERTER_SHA256 = '777caca88327cd18d4f92b43439575c1759381690d49f14177805a94b8ecbef6'
CONVERTER_REGION = '/CA'
# All programs lay out .rdata before .text, and the empty .ctors/.dtors that
# LIBSN SNMAIN declares between .text and .data: SNMAIN's __main walks the
# constructor list from .data's start in retail PSX.EXE.
SECTIONS = ('text\tgroup', 'bss\tgroup\tbss',
            '\tsection\t.rdata,text', '\tsection\t.text,text',
            '\tsection\t.ctors,text', '\tsection\t.dtors,text',
            '\tsection\t.data,text', '\tsection\t.sdata,text',
            '\tsection\t.sbss,bss', '\tsection\t.bss,bss')


def image_link(name: str) -> ImageLink:
    try:
        return IMAGE_LINKS[name]
    except KeyError:
        raise ValueError(f'{name}: no link model; add it to IMAGE_LINKS in '
                         f'scripts/psxbuild/link.py (known: {", ".join(IMAGE_LINKS)})') from None


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _pinned_object(source: Path, expected: str, root: Path, name: str, provenance: str) -> dict:
    if file_hash(source) != expected:
        raise ValueError(f'{source}: expected the pinned {name} (sha256 {expected[:12]}...)')
    shutil.copyfile(source, root / name)
    return {'file': name, 'path': str(source), 'sha256': expected, 'provenance': provenance}


def build_image(name, root, units, compile_one, *, repo, load_address) -> dict:
    """Compile and link a program using its source units and SDK libraries."""
    root = root.resolve()
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    report = {'image': name, 'linked': False, 'phase': 'compile', 'units': [],
              'libraries': [], 'startup': None, 'allocator': None, 'tools': {}}
    try:
        layout = image_link(name)
        tools = {'ASPSX.EXE': Path(os.environ['PSYQ_ASPSX']),
                 'PSYLINK.EXE': Path(os.environ['PSYQ_PSYLINK']),
                 'CPE2X.EXE': Path(os.environ['PSYQ_CPE2X'])}
        for tool, source in tools.items():
            shutil.copyfile(source, root / tool)
            report['tools'][tool] = {'path': str(source), 'sha256': file_hash(source)}
        if report['tools']['PSYLINK.EXE']['sha256'] != LINKER_SHA256:
            raise ValueError(f'{tools["PSYLINK.EXE"]}: expected the pinned PSYLINK 2.73D')
        if report['tools']['CPE2X.EXE']['sha256'] != CONVERTER_SHA256:
            raise ValueError(f'{tools["CPE2X.EXE"]}: expected the pinned CPE2X 1.5')
        if not units:
            raise ValueError(f'{name}: no C source units')
        for index, unit in enumerate(units):
            report['units'].append(compile_one(unit, root, index))
        explicit = []
        if layout.startup:
            report['startup'] = _pinned_object(
                Path(os.environ['PSYQ_STARTUP_OBJ']), STARTUP_SHA256, root, STARTUP,
                'Psy-Q 4.x kit LIB/NONE3.OBJ; retail text starts with it at the entry')
            explicit.append(STARTUP)
        if layout.allocator:
            report['allocator'] = _pinned_object(
                Path(os.environ['PSYQ_MALLOC_OBJ']), MALLOC_OBJECT_SHA256, root, 'MALLOC.OBJ',
                'Psy-Q Release 2.5 object; retail text follows NONE3.OBJ, relocations masked')
            explicit.append('MALLOC.OBJ')
        report['phase'] = 'assemble'
        assembler_commands = [u['assembler_command'] for u in report['units']
                              if 'assembler_command' in u]
        if assembler_commands:
            dos_run(root, assembler_commands, 'asm')
        for unit in report['units']:
            if 'assembler_command' in unit:
                tool_succeeded(root, unit['log'], unit['object'], b'LNK\x02')
            elif not (root / unit['object']).read_bytes().startswith(b'LNK\x02'):
                raise ValueError('compiler produced no native object')
            unit['object_sha256'] = file_hash(root / unit['object'])
        for library in layout.libraries:
            filename = library + '.LIB'
            source = Path(os.environ['PSYQ_LIB']) / filename
            data, corrections = library_input(filename, source.read_bytes())
            (root / filename).write_bytes(data)
            report['libraries'].append({
                'file': filename, 'path': str(source), 'sha256': file_hash(source),
                'link_input_sha256': hashlib.sha256(data).hexdigest(),
                'corrections': corrections,
            })
        report['phase'] = 'link'
        stem = name.removesuffix('.EXE')
        objects = [*explicit, *(u['object'] for u in report['units'])]
        commands = [f'\torg\t${load_address:08x}', *SECTIONS,
                    *(f'\tinclude\t"{obj}"' for obj in objects),
                    *(f'\tinclib\t"{library["file"]}"' for library in report['libraries']),
                    f'\tregs\tpc={ENTRY}']
        (root / 'LINK.LNK').write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
        report['linker_command'] = (
            f'PSYLINK.EXE /c /m @LINK.LNK,{stem}.CPE,{stem}.SYM,{stem}.MAP > LINK.TXT')
        dos_run(root, [report['linker_command']], 'link')
        tool_succeeded(root, 'LINK.TXT', stem + '.CPE', b'CPE\x01')
        report['phase'] = 'convert'
        report['converter_command'] = f'CPE2X {CONVERTER_REGION} {stem}.CPE > CONVERT.TXT'
        dos_run(root, [report['converter_command']], 'convert')
        executable = root / name
        if not executable.is_file():
            raise RuntimeError('CPE2X produced no executable; see CONVERT.TXT')
        actual = executable.read_bytes()
        if len(actual) < 2048 or actual[:8] != b'PS-X EXE':
            raise ValueError('CPE2X produced an invalid executable')
        report.update(linked=True, phase='complete', executable=str(executable),
                      cpe=str(root / (stem + '.CPE')))
    except (KeyError, OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
        (root / name).unlink(missing_ok=True)
        report['error'] = str(error)
    (root / 'build.json').write_text(json.dumps(report, indent=2) + '\n')
    return report
