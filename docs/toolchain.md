# SDK and source-to-EXE toolchain

The tools below were chosen by reproducing Shadow Tower's (`SLUS-00863`)
retail bytes. `PSX.EXE` is rebuilt byte-identical from `src/psx/main.c`
(see [executable linking](executable-linking.md)). That proves this chain
can produce the retail program; it does not prove these are the binaries
FromSoftware ran. Where a fact is only a probe, it says so.

## What each step uses

| Step | Tool | Source | Status |
| --- | --- | --- | --- |
| Preprocess, compile | `cpppsx-272cdk`, `cc1psx-272cdk`: Decompals old-gcc 0.17 `gcc-2.7.2-cdk`, a native rebuild of the SN Systems GCC "cygnus-2.7.2-970404 SN32" | GitHub release, SHA-256 pinned | `-O2 -G0` reproduces 14 sampled `ST.EXE` game functions and `-O2 -G8` reproduces `PSX.EXE`'s `main`; the original Win32 `CC1PSX.EXE` is not used |
| Assemble | SDevTC ASPSX 2.79D (1998-05-22, DOS): `BIN/DOS/ASPSXD.EXE` | Psy-Q/SDevTC 4.5 kit, Archive.org `ps1_sdks` | Reproduces the retail expansions; see below |
| GTE macros | Sony DMPSX 3.0 (DOS): `BIN/DOS/DMPSX.EXE`, run on every assembled object | 4.5 kit | Writes the retail COP2 command for every `INLINE_C.H` placeholder; see below |
| Link | PSYLINK 2.73D (DOS): `DOS/PSYLINKD.EXE` | SDevTC 2.4 CD (Japan, files dated 1998-06-11), redump in Archive.org `ps1_sdks` | Links `PSX.EXE` byte-identically |
| Convert | CPE2X 1.5 (1998-01-23, DOS) with `/CA` | 4.5 kit | Writes the whole retail header, including the "North America area" title |
| Libraries | Sony Runtime Library 4.2 `LIB/`, with `LIBPAD.LIB` from its bundled 4.2.1 pad patch | Archive.org, SHA-256 pinned | Every SDK object in all seven programs matches (below) |
| Overlay startup | `NONE3.OBJ` | Psy-Q 4.5 kit (identical in the 4.6 kit) | First object of STR, ST, TITLE, OPTXT, VS and END |
| Allocator | `MALLOC.OBJ` | Psy-Q Release 2.5 floppies | Linked after `NONE3.OBJ` in STR, TITLE, OPTXT and END |
| Headers | Runtime Library 4.2 `INCLUDE/`, staged as `include-lf/` (CRLF converted to LF) | as the libraries | `PSYQ_INCLUDE` |

All four SDK programs are DOS executables and run unchanged in the pinned
DOSBox-X. ASPSX 2.79D and PSYLINK 2.73D are Watcom C programs with the
FlashTek X-32VM DOS extender bound into the executable; DMPSX 3.0 (Borland
C++) and CPE2X 1.5 are 16-bit real-mode programs. No extender, Windows, Wine
or Win32 layer is staged, and no program is patched. They are staged in the
DOS root as `ASPSX.EXE`, `DMPSX.EXE`, `PSYLINK.EXE` and `CPE2X.EXE`.

The SN tool kits ship each tool as a Win32 program and as a DOS build (the
`D` suffix). Before this all-DOS chain, the Win32 ASPSX 2.79 and PSYLINK 2.74
ran under the HX DOS extender. The DOS builds give the same output:

- ASPSX 2.79D and the Win32 ASPSX 2.79 (`BIN/ASPSX.EXE` of the 4.5 kit,
  byte-identical to the 2.79 in mkst/maspsx's `aspsx-binaries`) write
  byte-identical objects for the 14 sampled functions below and for
  `PSX.EXE`'s unit.
- PSYLINK 2.73D links `PSX.EXE` byte-identically. For each of the seven
  link layouts, a test unit referencing every external of the layout's
  libraries was linked with `NONE3.OBJ`/`MALLOC.OBJ` as the layout says
  (958-1732 references each); STR, ST, TITLE, OPTXT and END linked with both
  linkers to identical CPE, SYM and MAP files and identical PS-X EXEs, and
  PSX and VS failed with the same diagnostics.
- The 4.5 kit's own DOS linker, `BIN/DOS/PSYLINKD.EXE` (2.74D, 1998-05-14),
  is an LX image whose MZ stub only prints "this is a FlashTek (register
  calling convention) executable": X-32VM's binder was never applied, and
  DOS/4GW refuses it. The kit's `BIN/DOS/PSYLINK.EXE` (2.37, 1996) faults on
  the 2.79 objects and Psy-Q 3.0's PSYLINK 1.29 cannot read the link
  script, so 2.73D from SDevTC 2.4 is used.

| Environment variable | Contents |
| --- | --- |
| `PSYQ_LIB` | Runtime Library 4.2 archives (LIBPAD from the 4.2.1 patch) |
| `PSYQ_INCLUDE` | Runtime Library 4.2 headers with LF line endings |
| `PSYQ_ASPSX` | ASPSX 2.79D |
| `PSYQ_DMPSX` | DMPSX 3.0 |
| `PSYQ_PSYLINK` | PSYLINK 2.73D |
| `PSYQ_CPE2X`, `PSYQ_STARTUP_OBJ` | CPE2X 1.5, NONE3.OBJ |
| `PSYQ_MALLOC_OBJ` | Release 2.5 MALLOC.OBJ |
| `PSYQ_SDK` | the Psy-Q 3.0 kit inherited from King's Field II, kept only for the `gcc241-kit` probe; `kf-vendored-seed` and `kf-fid-census` read the Runtime Library 4.2 archives plus `NONE3.OBJ` and `MALLOC.OBJ` |
| `PSYQ_C_INCLUDE` | GCC 2.5.7's `stdarg.h`/`va-mips.h`, used by `compile_classic` |

## Library attribution

The evidence is a whole-object comparison: each object's `.text` from a
candidate archive, with the bits its relocation records patch wildcarded, is
searched for in every image. Compared were the Psy-Q 3.0 kit, Runtime
Library 4.2 with its two 4.2.1 patches, the Psy-Q 4.5 and 4.6 kit libraries,
and the Release 2.5 MALLOC.OBJ. The SDK region of each program (from the
first SDK object after the game code to the end of `.text`) is covered
without gaps by objects of these inputs:

- Runtime Library 4.2 archives. Many objects match 4.2 and none of the other
  kits compared: LIBCD `EVENT`, `SYS`, `BIOS` and `CDREAD`, LIBGPU `SYS` and
  `EXT`, LIBETC `INTR_VB`, LIBAPI `C114`, LIBC `A63`, LIBPRESS `LIBPRESS`
  and `VLC`, LIBCARD `INIT` and `PATCH`, and several LIBSND members. The
  retail RCS strings are those of 4.2: `sys.c` 1.140 (1998-01-12), `bios.c`
  1.86, `intr.c` 1.76.
- LIBPAD from 4.2.1 (`LIB/42PATCH/J421PD.ZIP`): `PADMAIN`, `PADIF` and
  `WAITRC2` match only the patched archive.
- `NONE3.OBJ` and `MALLOC.OBJ` as above. Runtime Library 4.2 ships neither.

Ghidra's Psy-Q signature sets had suggested older LIBSND/LIBSPU members
(`SSINIT`, `S_M_UTIL`, `S_M_INIT`, `VS_VTC`, `PLAY`), a 4.3+ `WAITRC2` and an
older MALLOC; the whole-object search finds the 4.2, 4.2.1 and Release 2.5
objects in all of those places. The short signatures matched by chance.
Runtime Libraries 4.0, 4.1 and 4.3 were not compared.

| Image | SDK groups in linked order (`inclib` order in `scripts/psxbuild/link.py`) |
| --- | --- |
| `PSX.EXE` | LIBAPI stubs, LIBSN `SNMAIN` (the entry point), LIBAPI `InitHeap` |
| `STR.EXE` | CD, SND/SPU, PRESS, ETC, API, C, DS, GPU, PAD |
| `ST.EXE` | CARD, CD, SPU/SND, GTE, ETC, API, C, GPU, PAD |
| `TITLE.EXE`, `OPTXT.EXE` | CD, SND/SPU, ETC, API, C, GPU, PAD |
| `VS.EXE` | CARD, CD, SPU/SND, SN, GTE, ETC, API, C, GPU |
| `END.EXE` | CD, SND/SPU, PRESS, ETC, API, C, DS, GPU |

Only `PSX.EXE` is linked from source so far, so the other orders are read
from retail, not yet reproduced by PSYLINK.

## Compiler

The old-gcc `gcc-2.7.2-cdk` cc1 identifies itself as `cygnus-2.7.2-970404
SN32.3.7.0004 (SonyPSX)` and defaults to `-msoft-float -mcpu=3000 -mgas
-fgnu-linker`; the profiles pass only `-O2` and `-G`. It fills delay slots
itself (`.set noreorder` around each filled branch) and spells relocated
operands as `%hi()`/`%lo()`.

The sampled `ST.EXE` game code is `-G0`. `PSX.EXE` reads its words
`$gp`-relative, so its unit uses `-G8` (`-G4` gives the same bytes).
`END.EXE` game code also addresses `$gp`. Its first three functions
(`0x80011fa8`, `0x80011ffc`, `0x800120d0`: a `ClearImage` of the 1024x512
frame, a CD sector read with retries, and a `\ST\` file load) compile
exactly, relocation fields masked, at `-O2 -G8` or `-G0`; only `-G8` puts
their `RECT` initializer template and the `"\\ST\\"` and `";1"` literals in
`.sdata`, which is where retail has them (`0x8002fd18`-`0x8002fd2b`, after
the text). These are scratch results, not yet units.

GCC 2.5.7 and 2.6.0 rebuilds (`cc1psx-257`, `cc1psx-260`) and the 3.0
kit's GCC 2.4.1 (`gcc241-kit`) remain selectable as probes; no unit uses
them, and the King's Field II profiles that encoded their per-unit flags are
removed. `compile_classic` (the standalone source export) still requires
GCC 2.5.7.

## Assembler

The retail code shows a Psy-Q 4.x-era ASPSX in `.set reorder` mode: `li` as
`addiu rd,$0,imm`, `move` as `addu rd,rs,$0`, the checked `div` expansion
with `break 7`/`break 6`, a `nop` between `mflo` and a dependent
instruction, and no delay-slot filling of its own. Tested against 14 sampled
`ST.EXE` functions compiled by the cdk compiler, with relocation fields
masked:

| ASPSX | Result |
| --- | --- |
| 1.07, 2.08 (Psy-Q 3.0), 2.30, 2.34, 2.56 (Psy-Q 4.0) | reject the `%hi`/`%lo` operands |
| 2.79D (DOS, used) and 2.79 (Win32 under HX) | 14/14 exact, byte-identical objects |

2.67, 2.77, 2.81 and 2.86 were not tried, so the exact retail
version between 2.56 and 2.86 remains open. ASPSX 2.79 assembles `-G8`
small-data accesses as single `$gp`-relative instructions (patch type 100);
the ELF view translates them to `R_MIPS_GPREL16`.

No maspsx or GNU assembler processes game source.

## DMPSX

Runtime Library 4.2's `INLINE_C.H` (identical to the 4.5 kit's but for its
`$PSLibId$` line; "Macro definitions of DMPSX version 3") writes each GTE
command as an inline `.word` that is not yet an instruction: opcode SPECIAL,
function field 0x3f, the command in bits 6-15 (`gte_nclip` is `0x117f`), and
for `gte_mvmva` its sf/mx/v/cv/lm operands in bits 18-26. The plain macros
put `nop; nop` before the word; the `_b` forms omit them. Sony's DMPSX
replaces the words with COP2 commands in the assembled object, the step Psy-Q
users ran on each object after `ccpsx -c`; the 4.5 kit's `PSYQ.INI` and CCPSX
do not invoke it.

The chain runs the kit's DOS `BIN/DOS/DMPSX.EXE` (version 3.0, "Copyright
1995, 1996", Borland C++) after ASPSX on every assembled object, for the
linked units and for the per-unit comparison objects (`rewrite_gte` in
`scripts/psxbuild/sdk.py`). DMPSX copies an object record by record and
rewrites only the code records of `.text`. Version 3.0 reads an XDEF record
(tag 12) as ten bytes plus its name, where the LNK v2 record ASPSX 2.79 writes
has eight; the kit's Win32 DMPSX 3.05 (1998-09-07) reads eight. Run on a whole
ASPSX 2.79 object, 3.0 therefore loses its place after the first XDEF and
depends on where it lands: of 70 small control objects without any
placeholder, 51 came back changed, most with their last one to three bytes
missing and some with a placeholder-shaped byte group inside a debug record
rewritten. It is given instead a view of the object holding only the `.text`
section declarations and code records, in their original order and sizes,
which it reads correctly: `DMPSX.EXE U0000.GTE -o U0000.GTO`. The rewritten
code bytes are copied back into the ASPSX object, and the build fails unless
the result differs from the ASPSX object only in `.text` placeholder words,
each now a COP2 command; DMPSX reports its own errors (an undefined command,
too few delay slots before one) only on the DOS console. Units without
placeholders, which is every unit of `PSX`, `STR`, `TITLE`, `OPTXT` and `END`,
come back byte-identical, and so does any object without them.

`tests/test_dmpsx.py` compiles every type 2 macro of `INLINE_C.H` (65 commands,
with and without the leading `nop`s, plus `gte_mvmva` operand sets) and checks
each against the GTE encoding. Retail game code (functions not in
`functions_vendored.tsv`; the FDAT overlays have no COP2 commands) uses these:

| Command | Word | `INLINE_C.H` macro | `ST.EXE` | `VS.EXE` |
| --- | --- | --- | ---: | ---: |
| NCLIP | `4b400006` | `gte_nclip` | 27 | 17 |
| NCDS | `4ae80413` | `gte_ncds` | 23 | 6 |
| OP sf=1 | `4b78000c` | `gte_op12` | 21 | 0 |
| MVMVA sf=1 RT·IR | `4a49e012` | `gte_rtir` | 18 | 12 |
| NCCS | `4b08041b` | `gte_nccs` | 16 | 6 |
| DPCS | `4a780010` | `gte_dpcs` | 14 | 0 |
| MVMVA sf=1 RT·V0+TR | `4a480012` | `gte_rt`, `gte_rtv0tr` | 13 | 4 |
| MVMVA sf=1 RT·V0 | `4a486012` | `gte_rtv0` | 9 | 0 |
| NCCT | `4b18043f` | `gte_ncct` | 8 | 4 |
| NCDT | `4af80416` | `gte_ncdt` | 6 | 4 |
| RTPS | `4a180001` | `gte_rtps` | 6 | 11 |
| SQR sf=0 | `4aa00428` | `gte_sqr0` | 2 | 0 |
| RTPT | `4a280030` | `gte_rtpt` | 1 | 18 |

`ST.EXE` has 164 such commands; 150 follow the macro's two `nop`s and 14
follow other instructions directly (the `_b` forms after a separate load). All are type 2 macros
of `INLINE_C.H`, so no other header is needed for them. `GTEMAC.H`'s compound
macros (`gte_RotTransPers` and the like) are built from the `INLINE_C.H` ones
and emit the same placeholders; `INLINE_O.H` stages every address through
`$12`, which the retail loads (straight off the operand register, for example
`lwc2 $6,0($2)`) do not; `INLINE_A.H` is for assembler sources. The other COP2 traffic (`lwc2`/`swc2`, `mtc2`/`mfc2`,
`ctc2`/`cfc2`) comes from the type 1 and type 3 macros, which are ordinary
instructions that ASPSX assembles.

## Analysis tools

`nix develop` also provides Ghidra/PyGhidra with the PSX loader, little-endian
MIPS GNU binutils, psy-k, objdiff, DOSBox-X, disc-image utilities and the
normal C/C++/Python build tools. Ghidra plugin packaging is documented
separately in [`ghidra.md`](ghidra.md).

Splat, Rabbitizer, and spimdisasm use the separately locked Python environment
because the latter two are not available in the pinned Nixpkgs revision. The
shell wrappers keep both setup and execution on that project lock:

```sh
kf-python-sync
splat --help
```

The first command is the one-time `uv sync --frozen --no-install-project`
step; `splat` delegates to `uv run --frozen` thereafter.

No generated SDK binary, retail game image, or executable belongs in Git.
