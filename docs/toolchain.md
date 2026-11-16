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
| Assemble | SDevTC ASPSX 2.79 (1998-05-22, Win32) | mkst/maspsx `aspsx-binaries.tar.gz` | Reproduces the retail expansions; see below |
| Link | PSYLINK 2.74 (1998-05-14, Win32) | Psy-Q/SDevTC 4.5 kit, Archive.org `ps1_sdks` | Links `PSX.EXE` byte-identically |
| Convert | CPE2X 1.5 (1998-01-23, DOS) with `/CA` | same kit | Writes the whole retail header, including the "North America area" title |
| Libraries | Sony Runtime Library 4.2 `LIB/`, with `LIBPAD.LIB` from its bundled 4.2.1 pad patch | Archive.org, SHA-256 pinned | Every SDK object in all seven programs matches (below) |
| Overlay startup | `NONE3.OBJ` | Psy-Q 4.5 kit (identical in the 4.6 kit) | First object of STR, ST, TITLE, OPTXT, VS and END |
| Allocator | `MALLOC.OBJ` | Psy-Q Release 2.5 floppies | Linked after `NONE3.OBJ` in STR, TITLE, OPTXT and END |
| Headers | Runtime Library 4.2 `INCLUDE/`, staged as `include-lf/` (CRLF converted to LF) | as the libraries | `PSYQ_INCLUDE` |

ASPSX 2.79 and PSYLINK 2.74 are Win32 console programs. Every DOS and
Win32 tool runs in the same pinned DOSBox-X; the Win32 ones are started
through the HX DOS extender (HXRT 2.23 from its GitHub release: `HDPMI32`
as the resident DPMI host, `DPMILD32` as the PE loader, and the `DKRNL32`,
`DUSER32` and `DGDI32` API layers). No program is patched and no Windows or
Wine runtime is involved.

| Environment variable | Contents |
| --- | --- |
| `PSYQ_LIB` | Runtime Library 4.2 archives (LIBPAD from the 4.2.1 patch) |
| `PSYQ_INCLUDE` | Runtime Library 4.2 headers with LF line endings |
| `PSYQ_ASPSX` | ASPSX 2.79 |
| `PSYQ_PSYLINK`, `PSYQ_CPE2X`, `PSYQ_STARTUP_OBJ` | PSYLINK 2.74, CPE2X 1.5, NONE3.OBJ |
| `PSYQ_MALLOC_OBJ` | Release 2.5 MALLOC.OBJ |
| `PSYQ_HX` | the five HX files above |
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
| 1.07, 2.08 (Psy-Q 3.0), 2.30, 2.34, 2.56 (Psy-Q 4.0, DOS) | reject the `%hi`/`%lo` operands |
| 2.79 (Win32 under HX, and the 4.5 kit's DOS-extended 2.79D) | 14/14 exact |

2.67, 2.77, 2.81 and 2.86 (Win32) were not tried, so the exact retail
version between 2.56 and 2.86 remains open. ASPSX 2.79 assembles `-G8`
small-data accesses as single `$gp`-relative instructions (patch type 100);
the ELF view translates them to `R_MIPS_GPREL16`.

No maspsx or GNU assembler processes game source.

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
