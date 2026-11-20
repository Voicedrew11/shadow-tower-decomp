# Build and progress system

> The build and progress tooling comes from the King's Field II (`SLPS-00069`)
> project. Its toolchain and link model are Shadow Tower's own (see
> [the toolchain page](toolchain.md)); examples that name King's Field
> programs are kept where they illustrate a mechanism.

`kf build` is the source-to-EXE build: the gcc-2.7.2-cdk compiler emits
assembly, ASPSX 2.79D emits native Psy-Q objects, DMPSX 3.0 rewrites the GTE
placeholder words of `INLINE_C.H` in them, PSYLINK 2.73D links those
objects with the Runtime Library 4.2 inputs, and CPE2X 1.5 writes each
executable. See [the linking guide](executable-linking.md) for outputs and
provenance.
It does not read retail executable bytes, calculate match scores, or update
the README. Tool failures stop the build; reconstruction checks are separate.

At the command level, the build is a compiler, an assembler, and a linker:

```text
source files -> compiler -> assembler -> DMPSX -> object files
object files + SDK objects/libraries -> linker -> executable
```

The SDK linker writes an intermediate CPE file; its CPE2X utility wraps that
result in the PlayStation EXE format. The small linker command file lists the
inputs and their order, the executable load origin and entry point, and the
uninitialized sections. PSYLINK chooses individual symbol addresses and applies
relocations. `ADDRESS()` and `DATA()` annotations do not control placement.

The build uses ASPSX's native objects, after DMPSX, for PSYLINK; the
comparison tools translate that same format to ELF only because objdiff
needs it. Every assembled object goes through DMPSX (`rewrite_gte` in
`scripts/psxbuild/sdk.py`; DMPSX 3.0 is given only the object's `.text` code,
see [the toolchain page](toolchain.md#dmpsx)), and the result is checked
against ASPSX's object: one without GTE placeholders must come back
byte-identical, and in one with them only those `.text` words may change,
each into a COP2 command. No maspsx or GNU assembler output enters either
path.

`kf analyze` drives the incremental Ninja graph for inspection. Source
compilation uses the same native SDK path; ELF objects are read-only views
of its LNK outputs. Retail delinking and the progress ledger are comparison
tools and supply no bytes or layout to the executable build.

## Unit manifest and ADDRESS() claims

`config/units.toml` is the only source-enrolment mechanism. A unit has a stable
ID, one of the seven executable names in `config/images.toml`, a repo-relative source, and a
named complete profile. It carries no address: every function a source
reconstructs claims its retail address in the source itself,

```c
#include <kf/lib/address.h>

ADDRESS(0x8001499c, 0x38)
void game_shutdown(void)
```

`ADDRESS()` expands to nothing; `scripts/kf/model.py` reads the claims and the
manifest loader binds them (`build/gen/bindings.tsv` records the result). The
binding rules are structural, in the spirit of Gruntz's `RVA()`:

- a normal claim must name an admitted, non-vendored, non-fragmented function
  of the unit's image, spelled exactly as `function_identities.tsv` names it,
  so a labelled function is never called by an address-derived name in source,
  and its size must equal the admitted retail body size, so the claim and the
  census cannot drift apart silently;
- a unit explicitly marked `scope = "vendored"` may claim only provider-owned
  functions. It remains in objdiff as a strict source-verification unit, but
  every function must stay exactly 100% and none enters progress or banking;
- claims inside one source ascend by address, and a unit owns every admitted
  function between its first and last claim, so a source file is a contiguous
  run of the linked image (address-order incrementalism);
- units are listed in ascending address order within each image, so the
  manifest itself reads as the recovered link order; a data-only unit may
  opt out with `link = "first"`: it is listed at the start of its image's
  block and links before the code units (the object that allocated VS.EXE's
  first .sbss run preceded the start-up module, which reads part of that run
  with lui pairs); and
- an address is claimed by at most one unit.

A unit may also claim its read-only contributions, each once:

```c
RODATA(0x8001235c, 0x178)
JUMPTABLES(0x80011490, 0x18)
```

The compiler writes a unit's constants (string literals, aggregate
initializers, const data) and its switch jump tables to `.rdata`. Retail
`ST.EXE` lays out every unit's constants before any unit's tables, each group
in link order, with the SDK libraries' read-only data after the tables; the
build therefore moves each constant block to `.rodata`
(`separate_read_only` in `scripts/psxbuild/sdk.py`) and the link script
places `.rodata` before `.rdata`. A unit's constant pool ends on an eight-byte
boundary (its zero tail is part of the claim). `RODATA()` claims the
constants, `JUMPTABLES()` the tables; each is a contiguous slice of the image.
The delinker carves them into the module object's `.rodata` and `.rdata`,
rewrites entries that point into the unit's own code into `.text`-relative
words, and resolves the code's references to section offsets, so objdiff
compares tables and literals too. Source literals must therefore appear in
retail order, which the address-ordered functions give for free; a string
that lives inside the range is spelled as the literal the original used, not
as a named extern.

An FDAT overlay unit is the exception: its read-only data stays in cc1 order
under one section (`keep_read_only_together`, selected by
`retail.read_only_mode` for every overlay image, and for a PS-X EXE whose
`config/images.toml` row sets `read_only = "together"`), without the pool padding,
and its single `RODATA()` claim covers constants and switch tables together
(see [the linking guide](executable-linking.md#fdat-overlays) for the retail
evidence). Pointers in that run into the unit's own code are rewritten like
jump-table rows; a pointer into another unit's code needs a reviewed
`mips32_candidate` relocation row.

A unit compiled with a small-data profile (`small_data = 8`) may also claim
its anonymous `.sdata` constants, one ascending range per run between its
named small data:

```c
SDATA(0x8006d6a4, 0xb)
```

GCC's MIPS `SELECT_SECTION` (2.5.7 and 2.7.2 alike) sends every constant of at most `-G` bytes
to `.sdata`, so a short literal (or a local `char buf[2] = " "` initializer)
is an `.sdata` member, while the same text initializing a larger array keeps
the array's type and stays in `.rdata`. ASPSX 2.79 addresses such data with
one `$gp`-relative instruction (native patch type 100, `R_MIPS_GPREL16` in the
ELF view), so `-G8` changes code as well as placement. The delinker packs the
claimed ranges among the unit's `.sdata` `DATA()` claims in address order and
resolves code references to them as `.sdata` offsets; a range never becomes a
symbol. Small statics land in `.sbss` and resolve through that section symbol;
declare their request in `data_reservations.tsv`. `PSX.EXE`'s `main` is the
first `-G8` unit: its program index and three byte pointers are the
`.sdata` run at `0x80010394`, the address `SNMAIN` loads into `$gp`.

A unit with several claims is a module: a translation-unit hypothesis whose
target object is the whole run carved as one section. Address proximity alone
does not prove the original file boundary, so module names stay WIP (a class
name, with an address suffix while a class is split by unreconstructed gaps)
and modules merge as gaps are filled.

### DATA() claims

A module owns a global by defining it under a `DATA()` claim; every other unit
declares the same global `extern`:

```c
DATA(0x80057b0c, 0x4)
static u32 frame_pacer_vsync_count = 0;
```

The claim binds the declarator to the `data_identities.tsv` row at that
address: the name and size must match, the storage must be `load` or `bss`, a
datum is claimed by one unit only, and the claims of each section ascend
inside a source (GCC writes a section's data in source order; a unit's
`.sbss` statics may precede its later `.sdata` words).
Data need not be contiguous with the unit's code. The bindings file records
data claims with `kind=data`.

The delinker carves claimed `load` data from the retail image into the module
object's `.data` section (claims packed in order, each at the alignment its
retail address implies) and claimed `bss` data as `.bss` sizes, with
`STT_OBJECT` symbols whose binding follows the identity's `scope` (`static`
becomes a local symbol). Relocation candidates sited inside claimed data go
through the shared validator; under the safe policy the raw pointer words of
tables remain withheld (`data:` reasons in `relocations_withheld.tsv`), so a
pointer table compares by bytes only until its rows are reviewed.

On the compiled side, the native assembler preserves initialized storage,
private BSS, and exported COMMON requests as distinct records. The ELF reader
preserves those classes: COMMON remains `SHN_COMMON` with its native requested
extent and has no assigned section offset. An equal request does not establish
its final address; only PSYLINK supplies executable placement.

Native section-relative relocations can also change an objdiff score without
changing resolved instructions. For example, in the SLPS-00017 project
`player_warp_to_floor_entry` scored 99.876540%: ASPSX records `.data + 14` where the earlier ELF
path retained `floor_entry_cells - 2`. The table begins at `.data + 16`, so
the expressions agree. An independent comparison resolves all 81 words,
six calls and thirteen address references identically to retail. This does
not waive the strict 100% banking rule or justify changing the source to
influence the inspector's symbol choice.

Private names and function records come from SDK debug metadata. Claimed C
object sizes are measured with the same target compiler and ASPSX in an
auxiliary sizeof query. That query supplies inspection metadata only: neither
its table nor `DATA()` expectations alter the actual native object. The reader
never appends `.size`, `.type`, or allocation directives to game assembly.

## Incremental graph

`kf configure` writes `build/build.ninja`. Each image has `base-IMAGE`,
`target-IMAGE`, `compare-IMAGE`, `verify-IMAGE`, and `all-IMAGE` aliases.
Compilation and delinking keep separate image outputs. Comparison synchronizes
all seven images into `build/objdiff/objdiff.json` and one native `report.json`;
`--image` scopes verification and status to the selected image. Objdiff groups
units under one directory per image (`psx/`, `st/`, ...), while inventories, relocations, and
progress identities remain keyed by image.

The graph tracks retail executables, curated TSVs, source files, transitive
repo-local headers, manifest profiles, analysis scripts, and a generated
toolchain identity. The delink edge also depends on the manifest and unit
sources because module target objects are carved from their claims. Target objects, reconstruction objects, TSVs, projects,
and empty reports are written only when content changes. Removing a unit
prunes its orphan base object at configure time.

The graph's own inputs (scanned sources and headers, scripts and inventories)
are listed in `build/build.ninja.d`, the generator edge's depfile, so editing
any of them regenerates the graph. Ninja cannot regenerate past a deleted file
that a compile or delink edge still names, so `kf analyze`/`kf match` first
reconfigure whenever a depfile entry no longer exists (or the depfile is
absent, as in graphs written before it).

## Status and banking

The status universe is every contiguous, non-vendored function: currently
the admitted rows of `config/retail/functions.tsv` (`kf status` prints them). Progress is counted per function even
when several functions share a module unit; the objdiff report lists each
function inside its unit. Only manifested units with real base objects enter
objdiff. This keeps an absent reconstruction distinct from a
real 0% comparison and prevents zero-total dummy objects from reporting 100%.

The committed `config/match_baseline.tsv` is keyed by `(image, va)`. Its input
fingerprint includes source, transitive local headers, the complete profile,
and toolchain identity. An unchanged fingerprint below its best score is a
regression. A changed fingerprint is reported separately; `--strict` also
gates it against the historical best. A banked identity that disappears from
the report is a loss.

Each compiled base object records that fingerprint beside it
(`<object>.inputs`). When a unit's current fingerprint differs, or the report
is older than its objects, `kf status` starts with a `STALE:` line, marks the
image row `STALE`, and warns which units changed: the numbers shown are the
last analysis, not the current sources. `kf check` and `kf bank` treat the
same condition as a failure.

Only `kf bank` mutates the ledger. It refuses stale reports and dirty build
inputs unless `--dirty` is explicit. `kf bank --unit ID` updates only the
selected unit, requires every selected function to be exactly 100%, and
preserves all unrelated baseline rows. Normal builds never update committed
progress automatically.

## Generated README status

The compact status block at the top of `README.md` is derived state, not
hand-written documentation. `scripts/kf/readme.py` renders it from the same
strict-100% snapshot as `kf status` and replaces only the text between
`<!-- match-score:start -->` and `<!-- match-score:end -->`.

`kf check` refreshes the block, and `kf analyze` ends in that check. The check
also requires every vendored source-verification function to remain 100% and
compares every owned `.data`, `.rodata`, `.rdata`, and `.bss` contribution against the
retail-delinked object by default. It fails on any byte, exact object-section
extent (including assembler-owned writable/switch-table zero tails materialized
by the delinker),
relocation type/referent, ownership, or comparison-artifact mismatch. `kf bank` also
refreshes it after updating the manual ledger. Concurrent per-image checks
serialize snapshot and replacement through `build/gen/readme.lock`; content is
written atomically and only when it changes. README generation never changes
`config/match_baseline.tsv`.

## Shared sources across images

Shadow Tower's FDAT overlays repeat a common set of functions in every entry,
and the programs may share game code. A source is written once with the
claims of its primary image; another image reuses it with a unit that binds
by name (the names below are illustrative):

```toml
[[unit]]
unit = "fdat_012.common"
image = "FDAT_012.BIN"
source = "src/lib/fdat_common.c"
profile = "probe-gcc272cdk-o2-g0"
bind = "name"
defines = ["ST_FDAT_012"]
```

- `bind = "name"` resolves every `ADDRESS()` and `DATA()` claim through the
  unit image's own `function_identities.tsv` / `data_identities.tsv` rows by
  the definition name, so addresses stay image-qualified. The claimed size
  must still equal that image's body size; a size mismatch means the images
  do not share the body and the difference has to be expressed in source.
  `RODATA()`, `JUMPTABLES()` and `SDATA()` ranges are image-specific and
  cannot be name-bound.
- `defines` adds `-DNAME` (or `-DNAME=value`) to the preprocessor run for
  that unit only. Use it for constants that provably differ between the
  images, not to fork whole bodies silently.

When the shared functions are only part of each image's unit, or sit in a
different order around image-specific code, the shared run goes into a
`src/lib/*.inc` fragment instead: its definitions stack one
`ADDRESS_AT("ST", ...)` / `ADDRESS_AT("VS", ...)` claim per image, and each
image's unit source includes it (`#include "../lib/name.inc"`) at its place
in that image's contiguous run. The fragment must be contiguous and in the
same order in every image; it may not own `RODATA()`, `JUMPTABLES()` or
`SDATA()` (the including unit claims its image's ranges), and its text is
shared verbatim, so every name it spells must be the same identity in both
images. VS.EXE's copies of ST.EXE's animation node-list and VRAM helpers
are the first users (`src/lib/anim_*.inc`, `src/lib/vram.inc`); VS copies
whose callees or globals carry address-derived (image-specific) names, or
whose structures have VS layouts, stay separate sources under `src/vs/`.

A fragment may instead carry plain `ADDRESS()` claims and be included by
sources that each bind by name. A unit's own source, one that no other unit
lists, may bind by name and still claim its image's `RODATA()`,
`JUMPTABLES()` and `SDATA()` ranges: the ranges belong to that one image,
while the fragment's functions resolve through the image's identities. The
FDAT overlays use this: the 21 functions every overlay starts with sit at
different addresses in seven of the 36 overlays and carry one shared
identity name (`fdat_<FDAT_002.BIN address>`) in all of them. Each overlay
has three units: the shared source `src/lib/fdat_common_a.c` (the first ten
functions), its own `src/fdat_nnn/main.c` (the handler table and
`fdat_801ebec4`'s switch table as one `RODATA()` run, and the fragment
`src/lib/fdat_common.inc` with `fdat_801ebec4` and `fdat_801ec6a4`), and the
shared `src/lib/fdat_common_b.c` (the last nine). The boundaries are WIP:
they keep the two not-yet-exact functions `fdat_801eba4c` and
`fdat_801ec6a4` from moving the switch-table and handler-table pointers
(a non-exact function ends its unit).

The overlays' own handlers after those 21 functions are units named by
their first function, `fdat_nnn.map_<address>` in
`src/fdat_nnn/map_<address>.c`, each a contiguous run of exact handlers.
Handlers that several overlays share are fragments named like the common
functions: `src/lib/fdat_801ed6e4.inc` (19 overlays) calls the overlay's
move picker, which is the shared `fdat_801ed4a0` in 14 of them and an
overlay's own variant in five; a source names the variant by defining
`FDAT_PICK_MOVE` before the include. `src/lib/fdat_801ed4a0.inc` (21
overlays) is not yet exact, and no unit includes it until it is, because
its switch table sits in the overlay's read-only run and its entries would
move with the code.

An overlay's handlers keep their state in words right after its code (in
the archive entry's zero tail). They are `.bss` `DATA()` claims of the
first map unit that uses them, with FDAT data identities (`fdat_<address>`).
A unit's single word is a global (a COMMON request placed at its claim) so
that later units of the overlay can declare it `extern`; two or more words
in one unit are statics with `fixed` rows in `data_reservations.tsv`, since
ASPSX packs `.lcomm` requests without alignment and retail keeps them in
consecutive words. A map handler's own read-only data, such as
`FDAT_032.BIN`'s `RECT` initializer, is a `RODATA()` claim of its unit for
that slice of the overlay's read-only run.
