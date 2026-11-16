"""The shared compiler and native SDK assembler used by every source build."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

from .dmpsx import (dmpsx_command, merge as merge_dmpsx, stage as stage_dmpsx,
                    verify as verify_dmpsx, view as dmpsx_view)

C_COMPILERS = {
    # Decompals rebuild of the SN Systems GCC 2.7.2 ("cygnus-2.7.2-970404
    # SN32") PSX compiler: the Shadow Tower default.
    "gcc272cdk-native": ("cpppsx-272cdk", "cc1psx-272cdk"),
    "gcc257-native": ("cpppsx-257", "cc1psx-257"),
    "gcc260-native": ("cpppsx-260", "cc1psx-260"),
    # The kit's extensionless CC1PSX (GCC 2.4.1) runs under DOSBox. Its
    # preprocessing and the source-size probe use the native 2.5.7 pair: both
    # are optimizer-independent and see the same headers and MIPS ABI.
    "gcc241-kit": ("cpppsx-257", "cc1psx-257"),
}
DOS_C_COMPILERS = frozenset({"gcc241-kit"})


def run_command(arguments, *, environment=None):
    result = subprocess.run(arguments, capture_output=True, env=environment, check=False)
    if result.returncode:
        detail = (result.stderr or result.stdout).decode(errors="replace")
        raise RuntimeError(f"command failed ({result.returncode}): {detail}")
    return result.stdout


# Real-mode hardware interrupts push their frames onto the running program's
# stack, so stack bytes a DOS tool never initializes depend on where the timer
# IRQ landed. CPE2X copies such bytes into the reserved EXE header words
# (offsets 8..9 change with the emulated CPU rate). `cycles=max` retunes the
# rate from host load, which made those bytes vary between builds; a fixed
# rate ties the IRQ schedule to the executed instruction count instead.
DOS_CYCLES = 'fixed 100000'


def dos_run(root: Path, commands: list[str], phase: str, settings: tuple[str, ...] = ()) -> None:
    batch = phase.upper() + '.BAT'
    (root / batch).write_bytes(('\r\n'.join(commands) + '\r\n').encode('ascii'))
    environment = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
                       XDG_CONFIG_HOME=str(root / 'dosbox-config'))
    configuration = [argument for setting in settings for argument in ('-set', setting)]
    result = subprocess.run(['dosbox-x', '-silent', '-fastlaunch', '-set', 'sdl output=surface',
                             '-set', f'cpu cycles={DOS_CYCLES}', *configuration,
                             '-c', f'mount c "{root}"', '-c', 'c:', '-c', batch, '-exit'],
                            cwd=root, env=environment, capture_output=True, timeout=600)
    (root / (phase + '-dosbox.log')).write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f'DOSBox failed during {phase}: {result.returncode}')


def tool_succeeded(root: Path, log: str, product: str, magic: bytes) -> None:
    transcript = (root / log).read_text(errors='replace')
    if not re.search(r'(?m)^0 error\(s\)', transcript):
        errors = [line.strip() for line in transcript.splitlines() if 'Error' in line]
        detail = errors[0] if errors else next(iter(transcript.strip().splitlines()), 'no diagnostic output')
        raise RuntimeError(f'{log}: {len(errors)} native tool errors; first: {detail}')
    path = root / product
    if not path.is_file() or not path.read_bytes().startswith(magic):
        raise RuntimeError(f'{log}: native tool produced no valid {product}')


# ASPSX 2.79D is the DOS build of SDevTC ASPSX 2.79 (the 4.5 kit's
# BIN/DOS/ASPSXD.EXE). Like PSYLINK 2.73D and CPE2X 1.5 it is a plain DOS
# program whose FlashTek X-32VM extender is bound into the executable, so
# DOSBox-X runs it directly and no extender files are staged.
ASPSX_VERSION = '2.79'


def aspsx_command(stem: str, small_data: int) -> str:
    return f'ASPSX.EXE -g -G{small_data} -o {stem}.OBJ {stem}.S > {stem}.TXT'


def rewrite_gte(root: Path, stems: tuple[str, ...]) -> dict[str, int]:
    """Run DMPSX 3.0 on each assembled object; return the GTE words rewritten.

    Psy-Q builds ran DMPSX on the object after `ccpsx -c`. It rewrites
    INLINE_C.H's placeholder words and must leave an object without them
    byte-identical; dmpsx.py explains the view it is given and the checks.
    """
    if not stems:
        return {}
    stage_dmpsx(Path(os.environ['PSYQ_DMPSX']), root)
    objects = {stem: (root / (stem + '.OBJ')).read_bytes() for stem in stems}
    for stem, data in objects.items():
        (root / (stem + '.GTE')).write_bytes(dmpsx_view(data))
        (root / (stem + '.GTO')).unlink(missing_ok=True)
    dos_run(root, [dmpsx_command(stem) for stem in stems], 'gte')
    rewritten = {}
    for stem, data in objects.items():
        output = root / (stem + '.GTO')
        if not output.is_file():
            raise RuntimeError(f'{stem}.OBJ: DMPSX wrote no {output.name}')
        result = merge_dmpsx(data, output.read_bytes(), stem + '.OBJ')
        rewritten[stem] = verify_dmpsx(data, result, stem + '.OBJ')
        (root / (stem + '.OBJ')).write_bytes(result)
        for suffix in ('.GTE', '.GTO'):
            (root / (stem + suffix)).unlink()
    return rewritten


# Retail ST.EXE lays out every game unit's constant pool (string literals,
# aggregate initializers, const data) before any unit's switch tables, each
# group in link order, and the SDK libraries' read-only data after the tables
# (STR.EXE and TITLE.EXE show their unit's table directly before LIBSND's
# `.rdata`). The compiler writes both groups to `.rdata`, so a unit's tables
# would sit beside its own constants. The constants therefore move to their own
# `.rodata` section, which the link script places before `.rdata`; the tables,
# like the SDK's read-only data, stay in `.rdata`. Which mechanism separated
# them in the original build is unknown; this models the observed layout.
CONSTANT_SECTION = '.rodata'
JUMP_TABLE_LABEL = re.compile(r'\$L\d+:')
JUMP_TABLE_ROW = re.compile(r'\.(?:gp)?word\s+\$L\d+')


def separate_read_only(assembly: str) -> str:
    """Move every non-switch-table `.rdata` block of cc1 output to `.rodata`.

    GCC emits a switch table as `.rdata`, `.align 3`, a `$Ln:` label and one
    `.word $Lm` row per case; any other `.rdata` block is a constant.
    """
    lines = assembly.split('\n')
    moved = False
    for index, line in enumerate(lines):
        if line.strip() != '.rdata':
            continue
        following = [item.strip() for item in lines[index + 1:index + 4]]
        if (len(following) == 3 and following[0] in ('.align\t3', '.align 3')
                and JUMP_TABLE_LABEL.fullmatch(following[1])
                and JUMP_TABLE_ROW.fullmatch(following[2])):
            continue
        lines[index] = line.replace('.rdata', f'.section\t{CONSTANT_SECTION}')
        moved = True
    if moved:
        # A unit's constant pool ends on an eight-byte boundary: STR.EXE's
        # switch table follows its 0x10c-byte pool after four zero bytes, and
        # every ST.EXE pool boundary is eight-aligned. The trailing directive
        # makes ASPSX pad the section; nothing follows it.
        lines[-1:] = [lines[-1], f'\t.section\t{CONSTANT_SECTION}', '\t.align\t3', '']
    return '\n'.join(lines)


def dos_text(path: Path) -> None:
    # The DOS reader needs CRLF. No directives, instructions or literals change.
    path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))


def compile_c(source: Path, root: Path, stem: str, *, compiler: str,
              optimization: str, small_data: int, include_dirs: tuple[Path, ...],
              cc1_flags: tuple[str, ...] = (), defines: tuple[str, ...] = (),
              cc1_override: Path | None = None,
              environment: dict[str, str] | None = None, run=run_command) -> dict:
    cpp, cc1 = C_COMPILERS[compiler]
    preprocessed, assembly = root / (stem + '.I'), root / (stem + '.S')
    includes = [argument for path in include_dirs for argument in ('-I', str(path.resolve()))]
    # The name becomes the assembly's `.file`; older ASPSX builds hang on long
    # ones, so a checkout-relative spelling keeps deep worktrees buildable.
    cpp_command = [cpp, '-lang-c', '-undef', '-nostdinc', *includes,
                   *(f'-D{value}' for value in defines), os.path.relpath(source.resolve())]
    preprocessed.write_bytes(run(cpp_command))
    cc1_command = [str(cc1_override) if cc1_override else cc1, '-quiet', '-g', f'-{optimization}',
                   f'-G{small_data}', *cc1_flags, str(preprocessed), '-o', str(assembly)]
    probe = {}
    if compiler in DOS_C_COMPILERS:
        if cc1_override is not None:
            raise ValueError(f'{compiler} has no cc1 override')
        stage_kit_gcc241(root / 'CC1.EXE')
        assembly.unlink(missing_ok=True)
        dos_command = ['CC1', '-quiet', '-g', f'-{optimization}', f'-G{small_data}', *cc1_flags,
                       preprocessed.name, '-o', assembly.name]
        dos_run(root, [' '.join([*dos_command, '>', stem + '.CC1'])], 'cc1')
        if not assembly.is_file() or not assembly.stat().st_size:
            log = root / (stem + '.CC1')
            detail = log.read_text(errors='replace').strip() if log.is_file() else ''
            raise RuntimeError(f'kit CC1PSX produced no assembly: {detail or "no output"}')
        probe = {'size_probe_command': cc1_command}
        cc1_command = dos_command
    else:
        run(cc1_command, environment=environment)
    text = assembly.read_bytes().decode('latin-1')
    assembly.write_bytes(separate_read_only(text).encode('latin-1'))
    dos_text(assembly)
    return {'object': stem + '.OBJ', 'assembly': stem + '.S', 'log': stem + '.TXT',
            'preprocessor_command': cpp_command, 'compiler_command': cc1_command, **probe,
            'assembler_command': aspsx_command(stem, small_data),
            'dmpsx_command': dmpsx_command(stem),
            'assembly_sha256': hashlib.sha256(assembly.read_bytes()).hexdigest()}


def stage_kit_gcc241(target: Path) -> None:
    """Write the kit's GCC 2.4.1 cc1 as a stubbed DOS executable.

    The Psy-Q 3.0 kit ships CC1PSX 2.4.1 as a bare go32 COFF image beside
    the go32-stubbed 2.6.0 CC1PSX.EXE. Prefixing that stub to the 2.4.1 image
    makes it runnable under DOSBox; neither compiler image is modified.
    """
    compiler = Path(os.environ['PSYQ_SDK']) / 'COMPILER'
    stubbed = (compiler / 'CC1PSX.EXE').read_bytes()
    image = (compiler / 'CC1PSX').read_bytes()
    last_page_bytes, pages = struct.unpack_from('<HH', stubbed, 2)
    stub_size = pages * 512 - ((512 - last_page_bytes) if last_page_bytes else 0)
    i386_coff = b'\x4c\x01'
    if (stubbed[:2] != b'MZ' or stubbed[stub_size:stub_size + 2] != i386_coff
            or image[:2] != i386_coff):
        raise RuntimeError('unexpected kit CC1PSX layout; cannot stage GCC 2.4.1')
    target.write_bytes(stubbed[:stub_size] + image)


def compile_classic(source: Path, root: Path, stem: str, *, include_dirs: tuple[Path, ...],
                    defines: tuple[str, ...] = (), **options) -> dict:
    """Use GCC's original MIPS headers and the driver definitions they require."""
    if options.get('compiler') != 'gcc257-native':
        raise ValueError('classic requires GCC 2.5.7 and its original headers')
    headers = Path(os.environ['PSYQ_C_INCLUDE'])
    provenance = {name: hashlib.sha256((headers / name).read_bytes()).hexdigest()
                  for name in ('stdarg.h', 'va-mips.h')}
    # gcc.c supplies the version; config/mips/psx.h selects little-endian MIPS.
    driver_defines = ('__GNUC__=2', '__GNUC_MINOR__=5', '__mips__', '__MIPSEL__')
    result = compile_c(source, root, stem, include_dirs=(headers, *include_dirs),
                       defines=(*driver_defines, *defines), **options)
    return {**result, 'compiler_headers': provenance}


def assemble(root: Path, stem: str, small_data: int) -> bytes:
    return assemble_many(root, (stem,), small_data)[stem]


def assemble_many(root: Path, stems: tuple[str, ...], small_data: int) -> dict[str, bytes]:
    shutil.copyfile(os.environ['PSYQ_ASPSX'], root / 'ASPSX.EXE')
    for stem in stems:
        dos_text(root / (stem + '.S'))
        (root / (stem + '.OBJ')).unlink(missing_ok=True)
    dos_run(root, [aspsx_command(stem, small_data) for stem in stems], 'asm')
    for stem in stems:
        tool_succeeded(root, stem + '.TXT', stem + '.OBJ', b'LNK\x02')
    rewrite_gte(root, stems)
    return {stem: (root / (stem + '.OBJ')).read_bytes() for stem in stems}
