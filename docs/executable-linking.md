# Source-to-EXE build

Inside `nix develop`, run:

```sh
kf build
```

`kf link` is an alias for the same executable builder. There is one candidate
artifact chain:

```text
C source -> CPPPSX/CC1PSX (gcc-2.7.2-cdk) -> assembly -> ASPSX 2.79D -> DMPSX 3.0 -> Psy-Q OBJ
Psy-Q OBJ + SDK OBJ/LIB -> PSYLINK 2.73D -> CPE -> CPE2X 1.5 /CA -> PS-X EXE
```

The EXE is the unchanged output of CPE2X. Retail bytes are comparison inputs
only. The build does not synthesize object sections, force addresses from
`DATA()` claims, rewrite the CPE, patch the EXE, or copy missing retail bytes.
A failed phase removes the stale EXE. [The toolchain page](toolchain.md)
lists where each tool and library comes from.

DOSBox-X runs every tool at a fixed emulated rate (`DOS_CYCLES` in
`scripts/psxbuild/sdk.py`), so a DOS tool's uninitialized stack bytes cannot
vary with host load; `nix flake check` (`executable-determinism`) builds
every linked program in two trees and requires identical CPE and EXE bytes.
ASPSX, DMPSX, PSYLINK and CPE2X are all DOS programs; DOSBox-X starts each one
directly from a batch file, with no extender or loader staged.

A partial image, one whose source units claim some but not all of its
admitted game functions, is not linked: its code still calls and reads game
code no source provides, which PSYLINK cannot resolve. `kf build` reports it
as `partial` with the number of unclaimed game functions and does not fail;
its units are verified by the target relink and data gates of `kf check`
(`ST.EXE` is the first such image). Sources that claim no retail function,
such as the synthetic link controls, are always linked.

## Link model

`scripts/psxbuild/link.py` holds one `ImageLink` per program in
`IMAGE_LINKS`; an image without an entry fails with a readable error. Every
program shares one command-file layout:

```text
	org	$<load address>
text	group
bss	group	bss
	section	.rodata,text
	section	.rdata,text
	section	.text,text
	section	.ctors,text
	section	.dtors,text
	section	.data,text
	section	.sdata,text
	section	.sbss,bss
	section	.bss,bss
	include	"NONE3.OBJ"        ; overlays only
	include	"MALLOC.OBJ"       ; STR, TITLE, OPTXT, END only
	include	"U0000.OBJ"        ; game units in manifest order
	inclib	"<library>.LIB"    ; per image
	regs	pc=__SN_ENTRY_POINT
```

- **Load address.** From `config/images.toml`: `0x80010000` for PSX,
  `0x80012000` for TITLE, `0x80011000` for the others.
- **Read-only data before `.text`.** Every program begins with its game
  units' constant pools (`.rodata`), then `.rdata`: the units' switch tables
  followed by the SDK libraries' read-only data. The compiler writes both
  kinds of game data to `.rdata`; the build moves the constants (see
  [the build system](build-system.md#unit-manifest-and-address-claims)).
  `ST.EXE` is the evidence: every unit's constants precede every unit's
  tables, each in link order. The overlays' text starts at the entry point.
- **`.ctors`/`.dtors`.** LIBSN `SNMAIN` declares them. Placed after
  `.text`, both are empty at `.data`'s start, which is where retail
  `PSX.EXE`'s `__main` walks the constructor list (`0x80010350`). Left
  ungrouped they land at the origin and two words of `PSX.EXE` differ.
- **Startup.** `PSX.EXE` takes `__SN_ENTRY_POINT` from LIBSN `SNMAIN`,
  pulled by the `regs` reference; it is linked after LIBAPI's BIOS stubs and
  before LIBAPI `InitHeap`, its own dependency. The six other programs start
  with `NONE3.OBJ`, which clears the bss group, sets `$gp` and calls
  `main`.
- **Allocator.** STR, TITLE, OPTXT and END link the Release 2.5
  `MALLOC.OBJ` directly after `NONE3.OBJ`, before their game code. ST and VS
  link no allocator object.
- **Libraries.** The `inclib` order per image is in
  [the toolchain page](toolchain.md#library-attribution) and `IMAGE_LINKS`.
  No program links `CARD.OBJ` explicitly; LIBCARD members come from the
  archive.
- **Converter.** `CPE2X /CA <NAME>.CPE`. CPE2X 1.5 writes every header
  field, so no DOS session layout is reproduced.

### PSYLINK archive rules

PSYLINK, not the build script, places SDK library members. These rules were
established for PSYLINK 1.29 in the King's Field II project; the Shadow
Tower retail member groups follow the same pattern; `PSX.EXE`, `STR.EXE`,
`TITLE.EXE` and `END.EXE` have been relinked with 2.73D byte-identical:

- Archives are visited in `inclib` order. The groups of members from each
  archive follow one another in the linked text in that order.
- After it takes a member, the linker searches again from the first archive.
  A dependency in an archive listed earlier is placed directly after the
  member that needs it (`PSX.EXE`'s LIBAPI `InitHeap` follows LIBSN
  `SNMAIN`). A dependency in an archive listed later waits for that
  archive's group.
- Inside one archive, members follow the linker's own symbol-table order,
  which is neither archive order nor first-reference order.

So the SND members that call SPU interleave with the SPU group when LIBSPU
is listed first, which is the shape of every retail program here.

## FDAT overlays

The 36 `FDAT_nnn.BIN` images are raw entries of `ST\COM\FDAT.T`, not PS-X
EXEs. `ST.EXE` loads one at `0x801EB560`, the end of its bss, and calls into
it through the handler table at its first byte; the overlay calls `ST.EXE`'s
functions (its own libraries included) and reads `ST.EXE`'s data in place.
`kf build` links a fully claimed overlay with `OVERLAY_LINK`:

- the same command file at origin `0x801EB560`, with no startup object,
  allocator, library or `regs` line;
- every symbol the overlay's objects reference without defining becomes a
  command-file `name equ $address` line. `executable.host_symbols` takes the
  addresses from the host's identities (the target relink's address book,
  `roundtrip.address_book`, with an unresolved `DAT_xxxxxxxx` spelling its
  literal address) and refuses a name that is not a unique `ST.EXE` identity
  outside the overlay's window;
- PSYLINK's pure binary output (`/p`, written as `<NAME>.RAW`) replaces the
  CPE and CPE2X; and
- `pack_archive_entry` frames those bytes as the archive does: zeros up to
  the sector multiple that leaves room for one trailing word, and that word,
  `0x12345678` plus the 32-bit sum of every preceding word. All 36 retail
  entries satisfy that rule; the packing step only appends the frame and
  never rewrites linked bytes.

An overlay's read-only data is not separated: its compiler output keeps
every `.rdata` block in cc1 order under one section name
(`sdk.keep_read_only_together`), and the unit's `RODATA()` claim covers that
whole run, switch tables included (an overlay unit may not claim
`JUMPTABLES()`). The retail evidence is the overlays themselves:
`FDAT_032.BIN` puts a `RECT` initializer right after its 0x1cc-byte handler
tables and its switch table after four alignment bytes, `FDAT_242.BIN` has a
second run of `RECT` initializers after its switch table, and the code of
`FDAT_442.BIN`, which has neither, starts directly after the tables at
`0x801EB72C` with no pool padding.

A partially claimed overlay is reported `partial` like `ST.EXE`; its units
are verified by the target relink and data gates. `kf link --compare-only`
compares an overlay's file bytes strictly (header size 0) and keeps it out of
the island report and the README executable block.

## PSX.EXE

`src/psx/main.c` reconstructs `main` (`0x80010068`, `0xe8` bytes): it
loads `cdrom:<NAME>.EXE;1` with the BIOS `Load`, clears the EXEC stack
fields, calls `Exec`, then reads the next program's index from the byte at
`0x800103f0`. With the `probe-gcc272cdk-o2-g8` profile and the link above,
the whole 4096-byte `PSX.EXE`, header included, equals retail
(`dd01da6a...821e`). The linked layout:

| Range | Contents |
| --- | --- |
| `80010000-80010067` | `.rdata`: the six program paths |
| `80010068-8001014f` | `main` |
| `80010150-800101bf` | LIBAPI `Load`, `Exec`, `_96_init`, `_96_remove`, `EnterCriticalSection`, `ExitCriticalSection` |
| `800101c0-8001033f` | LIBSN `SNMAIN`: `__SN_ENTRY_POINT`, `__main`, `__do_global_dtors` |
| `80010340-8001034f` | LIBAPI `InitHeap` |
| `80010350-80010393` | `.data`: the path table, `SNMAIN`'s section words, LIBSN's `_stacksize` and `_ramsize` |
| `80010394-800103a3` | `.sdata`: `$gp` base; the program index and three byte pointers |
| `800103a8-800103ef` | `.sbss` (`SNMAIN`) and `.bss` (the EXEC header) |

`0x800103f0`-`0x800103f8` lie just past the linked bss. Nothing in
`PSX.EXE` defines them; `main` reaches them through initialized pointers and
one direct store, so the source spells the addresses. Whether the original
spelled numbers or link-time symbols is unknown.

The objdiff score of `main` is 98.97%: the retail target object has no
curated relocations or data claims yet, so its references compare as raw
words. `kf check` reports the unit's `.rodata`, `.data` and `.sdata` as
unclaimed and its EXEC header as an unplaced COMMON request until the data
identities are admitted.

## END.EXE

`src/end/main.c` reconstructs END.EXE's fifteen game functions as one unit
(`probe-gcc272cdk-o2-g8`); with the `IMAGE_LINKS` entry above the whole
129,024-byte file equals retail. The link confirmed these placement rules for
PSYLINK 2.73D:

- The 12 zero bytes at `0x8001d280` are the tail of the last LIBGPU text
  contribution, not game `.data`; the unit's `.data` starts at `0x8001d28c`.
- `.sbss` and `.bss` are allocated object by object in link order: each
  object's fixed reservations (its `.lcomm` statics, 16-byte aligned in
  `.bss` by ASPSX), then its exported COMMON in symbol-hash bucket order.
  END's movie buffers follow the 56-byte `dec` static directly, which only
  COMMON allows; their spellings are chosen inside the bucket order.
- GCC writes all of a unit's data directives before its text, so every
  small symbol the unit defines is `$gp`-relative; the one absolute store to
  END's own `.sbss` slot `0x8002fd7c` is spelled as a fixed address.

## STR.EXE

`src/str/main.c` reconstructs STR.EXE's twenty game functions as one unit
(`probe-gcc272cdk-o2-g8`); with the `IMAGE_LINKS` entry above the whole
141,312-byte file equals retail. It is END.EXE's closest relative: the same
movie player (24-bit only, with a global end frame) and CD helpers, plus a
LIBPAD controller module. The link confirmed END's rules and adds:

- GCC writes a unit's constants (strings, initializers) before its code. The
  unit's constant pool is `0x80011000`-`0x8001110f`, its last four bytes the
  zero tail that ends a pool on an eight-byte boundary; the one switch table
  (`pad_update_state`) follows at `0x80011110`, before LIBCD's `.rdata`.
- `.sdata` ends at `0x800328d4`; `.sbss` starts 8-aligned at `0x800328d8`
  with MALLOC.OBJ's 32 bytes, then the unit's statics from `0x800328f8`.
- The ten exported COMMON buffers follow the 56-byte `dec` static in
  hash-bucket order, each 8-aligned; their spellings are WIP names chosen
  inside that order (`vlc_buffer` 40, `ring_buffer` 148, `blend_buffer` 234,
  `tim_buf` 237, `display` 253, `poison_image` 262, `pad_data` 310,
  `cdl_file` 314, `pad_info` 328, `image_rgb` 422), and LIBCD's
  `StCdIntrFlag` follows them.

## TITLE.EXE

`src/title/main.c` reconstructs TITLE.EXE's eighteen game functions as one
unit (`probe-gcc272cdk-o2-g8`); with its `IMAGE_LINKS` entry the whole
96,256-byte file equals retail. TITLE.EXE alone loads at `0x80012000`.
`main` loads `\ST\TITLE\TITLE.MIC`, streams `\ST\TITLE\TITLE.XA` and runs
the menu, then writes the next program: 2 (`OPTXT.EXE`) for a new game,
3 (`ST.EXE`) with 2 in the mailbox byte `0x800103f1` for Continue, 5
(`VS.EXE`) for versus, and 0 (`STR.EXE`) when the title times out. The
link confirmed END's placement rules again:

- `.sdata` holds the two byte pointers and the unit's literals
  (`0x80028abc`-`0x80028aeb`); `.sbss` starts at `0x80028af0`, after four
  alignment bytes, with MALLOC.OBJ's 32 bytes, then the unit's `pad_align`; `.bss` starts
  at `0x80028b18` with MALLOC.OBJ's `SearchPoint` and `TopAllocArea`.
- The unit's six buffers are COMMON, allocated from `0x80028b28` in
  hash-bucket order (`sound_bank`, `file_buffer`, `display`, `pad_data`,
  `menu`, `pad_state`); the spellings are WIP names inside that order.
- The switch table of `pad_update_state` follows `main`'s literals, the
  first of the image's `.rdata`.

The relink also places sixteen library functions the FID census left as
game candidates (LIBSND, LIBSPU and LIBPAD members; see
`functions_vendored.tsv`, confidence `relink-map-sdk42`).

## OPTXT.EXE

`src/optxt/main.c` reconstructs OPTXT.EXE's eighteen game functions as one
unit (`probe-gcc272cdk-o2-g8`) and `src/optxt/state.c` defines the opening's
state word; with the `IMAGE_LINKS` entry above the whole 61,440-byte file
equals retail. The link confirmed the END.EXE rules above and these:

- The unit's `.data` (the text-page table) starts right after the last LIBPAD
  text at `0x8001df50`; its `.sbss` statics follow `MALLOC.OBJ`'s 32-byte
  `.sbss`, and `.bss` starts with `MALLOC.OBJ`'s 16 bytes.
- Exported COMMON objects are 8-byte aligned in bucket order: the text
  buffer, `display`, the 0x4c-byte pad buffers (`pad_data`) and the pad
  status.
- The opening's state word `0x8001f7ec`, the word after the unit's `.sbss`,
  is read and written only with absolute `lui` pairs, which no symbol of the
  unit gets: ASPSX sees a unit's own small data before its text and
  addresses it through `$gp`. It is an `int` global of another object
  (`src/optxt/state.c`): `main.c` only declares it, and PSYLINK places the
  small COMMON at the end of `.sbss`, after `main.c`'s statics.

## PS-X EXE header

`KERNEL.H` defines the 0x88-byte header as follows:

- 0x00: the eight-byte key `PS-X EXE`.
- 0x08: two reserved words.
- 0x10: the fifteen-word `EXEC`: `pc0`, `gp0`, `t_addr`, `t_size`, the
  data and BSS extents, `s_addr`, `s_size`, and the save area `sp`, `fp`,
  `gp`, `ret` and `base`.
- 0x4c: a 60-byte title.

The rest of the 2048-byte sector is zero padding. All seven retail headers
have zero reserved words, `gp0`, data/BSS extents and save area, `s_addr`
`801ffff0`, and the title `Sony Computer Entertainment Inc. for North
America area` followed by zeros. CPE2X 1.5 (`/CA`) writes exactly that; the
two CPE2X 1.3 builds that King's Field II used write only the Japan title
and leave stack residue in the title tail.

## Game data placement

Each object contributes one run per output section in link order, so a
datum's section and its unit's link position fix its address. The facts
below were established in the King's Field projects (GCC 2.5.7, ASPSX 1.07,
PSYLINK 1.17/1.29); they are starting hypotheses for the gcc-2.7.2-cdk,
ASPSX 2.79D and PSYLINK 2.73D chain:

- GCC 2.5.7's C front end emits every initialized file-scope or local static,
  const or not, at its declaration even when nothing reads it, and keeps
  unused non-const tentative statics as `.lcomm` requests. The images retain
  such leftovers (the word after the map-object jump tables, the bytes after
  the identity matrix, the eleventh menu label row, KF1's palette rectangles,
  small-data words and four card `.sbss` slots); each is a curated
  unreferenced identity rather than padding.
- A jump table is emitted after `.align 3`, which ASPSX applies relative to
  the object's `.rdata` start; table offsets therefore show which object an
  unexplained word belongs to. Constant pools link separately, before every
  table (`.rodata`), each ending eight-byte aligned.
- Under `-G8`, initialized data of at most eight bytes goes to `.sdata` and
  private tentative data of at most eight bytes to `.sbss`. Unlike ASPSX
  1.07, ASPSX 2.79 also addresses that data `$gp`-relative, so `-G` changes
  code as well as placement.
- PSYLINK places fixed `.bss` reservations in link order, then allocates
  exported COMMON by symbol-hash bucket, `(len + byte sum) & 511`, as decoded
  in the KF1 project. The native build follows that order exactly, so a game
  COMMON object reaches its retail address only if its source name falls in
  the bucket interval its SDK neighbours bound. `kf common-order` reports
  which curated names do not. Curated identities only bound that interval
  loosely: unidentified SDK COMMON (from the linked `.SYM`) tightens it, and
  retail may hold game COMMON nothing references. Names chosen to fit are WIP spellings,
  not recovered ones; a source shared by two programs spells per-image
  names under a define where the two intervals do not overlap.
- A controlled PSYLINK 2.73D link of two `-G8` ASPSX objects (`.lcomm` and
  `.comm` only) showed the per-object order: object A's `.sbss` static, then
  A's small COMMON in bucket order (later-interned first within a bucket),
  then B's static and B's new COMMON; a name both request is allocated once,
  in A's batch. Each request is aligned to the largest power of two not
  above its size, capped at 8 (sizes 1, 2, 3, 4, 6, 8 at 1, 2, 2, 4, 4, 8);
  requests over 8 bytes go to `.bss` the same way. One object's batch is
  therefore contiguous. VS.EXE's `.sbss` run `0x800632e8`-`0x80063381`
  interleaves words the start-up module (`vs.main`) reads `$gp`-relative
  (so it requests them as COMMON) with words it reads only absolutely
  (`0x80063334`, `0x80063374`, so it does not request them), so an earlier
  object allocated that run and `vs.main`'s requests merge into it.
- `kf verify data` accepts a unit's COMMON placement only when the current
  link's `.SYM` (`scripts/kf/linked_symbols.py`) puts every request at its
  claim, and the link report's input hashes all match the working tree.
- A Sony library member's COMMON (LIBCD's `StCdIntrFlag` in STR.EXE and
  END.EXE) has no source unit and no initialized bytes to compare. The
  known-reference closure (`scripts/kf/data_reachability.py`) counts such a
  curated `bss` identity as compared (`sdk-bss-link-placed`) when no compiled
  game unit defines or requests the name and that same current `.SYM` puts
  it at the identity's address with no other symbol inside its extent.

### Runtime smoke test

`kf-run-candidate` substitutes linked executables into a copy of the raw retail
Mode 2 disc (`KF_CANDIDATE_IMAGES` selects which ones, for controlled hybrid
tests). The retail BIN must match the disc SHA-256 in
`config/images.toml`. Every
changed sector's EDC and P/Q ECC is regenerated; `kf-run-retail` boots the
unmodified disc.

Existing source-to-EXE outputs can be compared again without rebuilding them:

```sh
kf link --compare-only
```

The PSYLINK controls use original Psy-Q objects and libraries to test section
alignment and BSS allocation.

## Layout-tolerant executable comparison

`kf link --compare-only` compares moved byte regions in existing outputs and
refreshes the second generated README block, `executable-score`:

```sh
kf link --compare-only
```

The command accepts `--image psx|str|st|title|optxt|vs|end`. The README keeps one row per image
and a byte-weighted total for the available images. It reads saved reports
only when their candidate hashes match EXEs still on disk and their retail
hashes match the configured retail identities. Missing, changed or failed
outputs receive no cached score. The executable and function blocks share
the same update lock and preserve each other.

The **Data modules exact** column shows complete passing source data owners
out of all compared owners, using the same strict gate as `kf verify data`.
A module counts only when every initialized section, relocation, BSS layout
and retail section placement passes. Exact functions or equal initializer
bytes alone do not make its data exact. Separate config-owned SDK contributions
are excluded; these object checks do not measure the generated EXE's similarity.
`kf analyze`/`kf check` and `kf bank` also refresh this column. Missing comparison
artifacts or objects older than their source, headers, inventories or build
inputs show `—`, independently of executable-score availability.

The report is a content heuristic called `unique-byte-islands-v1`, not a
reimplementation of objdiff or a claim of function identity:

1. Find exact 16-byte windows that occur once in each load area. Coalesce
   overlapping windows with the same displacement, then select the longest
   nonoverlapping runs. Each byte can be assigned at most once in each image.
2. Extend selected runs through equal, unused neighboring bytes. Also keep
   exact file-boundary prefixes and suffixes, so repeated padding does not
   require a unique seed.
3. Join adjacent anchors only when they are neighbors in both images. Each
   intervening gap is at most 128 bytes. A bounded byte-sequence comparison
   accepts gaps with at least 60% similarity, or edits of at most 16 bytes
   per side when the two flanking exact runs supply at least 85% similarity
   across that local span. Unchanged bytes inside an accepted gap earn
   credit; its edited, inserted or deleted bytes do not.
4. Sum equal byte pairs across all selected islands. Report similarity as
   `200 * equal_pairs / (retail_load_bytes + candidate_load_bytes)`. All
   unpaired bytes remain in the denominator. The nonzero score uses the
   same formula with zero-valued bytes removed from both counts.

Islands may move or reorder, but no address, opcode, register, immediate,
constant or data byte is masked. A changed pointer still loses byte credit;
similar pointer encodings do not prove the same referent. Short or repetitive
regions without suitable anchors can remain unpaired even when shared.
The algorithm is conservative and deterministic, rather than an optimal
global alignment or an instruction-semantic comparison.

The island scores cover the complete EXE load areas, including sector
padding, and exclude the 2048-byte headers. The separate fixed-offset
comparison still covers the complete files and is the only executable
equality check. Neither island similarity nor island coverage changes
function scores, banking criteria or the full data-layout gates.

Normal builds write tool commands and input hashes into each image's
`build.json`. They do not load retail EXEs, compare output bytes, or update
the README. Explicit comparison-only runs write `fuzzy-comparison.json` and
`fuzzy-comparison.html`, preserving the build report and executable.
The standalone HTML contains a movement
map, filterable islands with both addresses and sizes, and the largest
unanchored regions. The JSON retains all islands, complementary unanchored
ranges, parameters, counts and input hashes. These generated files stay
under `build/link/<image>/` and are not committed.

## Verification and limits

| Property | Current status |
| --- | --- |
| PSYLINK emits each program | Wired through `kf build` for all seven and the 36 FDAT overlays (raw `/p` output, archive frame); an image links once its units claim every game function |
| Candidate artifact path | Direct source-to-EXE with no output rewriting |
| Byte-identical retail images | `PSX.EXE`, `STR.EXE`, `TITLE.EXE` and `END.EXE` (whole files, headers included) |
| Library half of the overlays | Object-by-object attribution only (see the toolchain page); not yet relinked |
| Historical toolchain identity | Open: the compiler is a rebuild and the exact ASPSX, PSYLINK and CPE2X builds are the earliest available ones that reproduce retail |
| Boot | Not yet attempted |

An SDK mismatch does not by itself prove that an executable is unplayable,
and a successful link does not establish playability: private object
layouts, data, callbacks and initialization behaviour can differ. The
hash-verified retail disc remains the runtime reference.

`NativeBuildControls` in `tests/test_executable.py` link synthetic programs
for PSX and for overlays with and without the allocator. They check the
object and `inclib` order, the startup at the entry point, read-only data before
`.text`, `$gp` at the first `.sdata` byte, bss exclusion, the CPE2X 1.5
header, unchanged archives, and rejection of unresolved references, an
unknown image, or stale output. They read no retail file.
