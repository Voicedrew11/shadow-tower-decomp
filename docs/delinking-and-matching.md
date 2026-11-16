# Delinking and objdiff matching

> **Inherited from the King's Field II (`SLPS-00069`) project.** Statements here
> about retail bytes, linker order, SDK versions or compiler probes describe that
> game. For Shadow Tower (`SLUS-00863`) they are hypotheses until re-proven.

The active [data matching campaign](data-matching-campaign.md) distinguishes
strict claimed-object comparison from complete reachable-data ownership and
linked-image closure. The historical first-pass results below do not establish
that closure.

Shadow Tower needs the same broad loop as Vostok/Gruntz—split retail bytes into
target objects, compile reconstruction objects, compare, and ratchet—but not
Vostok's actual delinker. Vostok consumes PE/COFF sections and a PDB. A PS-X EXE
is a flat linked MIPS load image with no retained relocation directory, and
this project has TSV evidence instead of a PDB.

The implementation is `scripts/kf/delink.py`. It consumes the manually owned
`functions.tsv`, `functions_vendored.tsv`, `data.tsv`,
`data_identities.tsv`, and `relocs.tsv`, carves retail bytes, and emits ELF32
little-endian MIPS target objects. These objects are comparison artifacts; the
historical linker still consumed native Psy-Q objects. Pure decode, validation,
and implicit-addend rules live in
`scripts/kf/relocations.py` and are shared with the semantic navigator, so a
reference shown as `validated` uses the same proof boundary as safe delinking.

## First-pass topology

The 43 images (seven programs and 36 FDAT overlays) are processed
independently, each under its `--image` key:

```text
PSX.EXE      + config/retail/*.tsv -> build/delink/psx      -> build/objdiff/psx
ST.EXE       + config/retail/*.tsv -> build/delink/st       -> build/objdiff/st
FDAT_012.BIN + config/retail/*.tsv -> build/delink/fdat_012 -> build/objdiff/fdat_012
...
```

An overlay's calls into `ST.EXE` are cross-image references: they stay
unmodeled targets until `ST.EXE`'s owner is linked against the overlay.

The target object format is ELF32, little-endian, `EM_MIPS`, MIPS-I/O32. It is
understood by GNU `mipsel` binutils and objdiff 3.7.3. Each initial object has a
`.text` section, function and referent symbols, and `.rel.text` entries using
the subset currently needed:

- `R_MIPS_26` for `j`/`jal`;
- `R_MIPS_HI16` plus `R_MIPS_LO16` for paired address formation; and
- `R_MIPS_32` only under the explicitly exploratory `all` policy.

The section extent comes from `size`; the function symbol size comes from
`body_size`. Thus known linker padding stays available to the object comparison
without being falsely scored as part of the function body.

## Why reconstructed relocations are conditional

The executable does not say which words the linker once relocated. The
14,908 rows in `relocs.tsv` are working claims from instruction forms, range
tests, and manual review; candidate, reviewed, and rejected status remain
explicit. The default policy applies only rows whose retail instruction bytes
independently validate the TSV claim:

- a candidate row must come from `reachable-code`; a manually reviewed MIPS26
  or HI16/LO16 row may retain its original `instruction-word` discovery channel;
- a J/JAL opcode and its decoded target must agree with the row;
- an external J/JAL target must be an admitted function start;
- a HI/LO pair must remain inside one function extent, consume the LUI register,
  decode to the recorded address, and use a sign-extending low instruction;
  rows may share a site only as one HI/LO family (same target: one `lui`
  feeding several lows, or `lui`s on converging paths feeding one low), which
  is emitted once per site in ELF REL order. That order is by site,
  except where another pair against the same symbol (with a different low
  half) sits between a HI16 and its own LO16: GNU ld completes a HI16 with
  the next same-symbol LO16, so that HI16 moves to just before its own
  (`mips_elf.rel_order`). The native object view applies the same rule,
  pairing each HI16 patch with the first later LO16 carrying its
  expression, because objdiff reports a HI16's addend from an immediately
  following LO16 only;
- an out-of-load HI/LO pair remains withheld unless a human has promoted that
  exact decoded pair to `status=reviewed`; this is the path for RAM/BSS symbols
  whose addresses are real but whose storage is absent from the PS-X EXE payload;
- candidate raw range-only pointer words and instruction-word scans are
  withheld; reviewed instruction-word rows still pass every byte/target check,
  while raw MIPS32 words remain exploratory-only; and
- rejected, overlapping, malformed, cross-fragment, or unsupported candidates
  are withheld with a machine-readable reason.

An internal `j label` is not relocation-free. MIPS J instructions encode an
absolute target within the current 256 MiB region, not a PC-relative distance.
GNU `as` emits `R_MIPS_26` against the local `.text` section, with the label
offset in the instruction field. The target-object writer reproduces that
behavior.

Likewise, MIPS ELF uses REL rather than RELA: the addend lives inside the
instruction or data word. The delinker rewrites linked absolute fields back to
object addends before recording the relocation. A normal signed HI/LO pair uses
the carry-adjusted high half `(addend + 0x8000) >> 16`. Inferred `lui` + `ori`
pairs are withheld by the safe policy because ELF HI16/LO16 semantics do not
faithfully model an unsigned low half in every address range.

Curated BSS aggregates supply the owner symbol and extent for this calculation.
When a reviewed row names that semantic owner but targets an interior field,
the emitted relocation keeps the owner symbol and encodes `target - owner` as
its implicit addend. This lets reconstructed C use a typed struct global rather
than introducing a fake symbol for each referenced field.

The available policies are:

| Policy | Meaning |
| --- | --- |
| `safe` | default; validated reachable-code candidates plus manually reviewed MIPS26/HI16/LO16 rows from any discovery channel |
| `reviewed` | same structural checks, but only rows manually promoted to `status=reviewed` |
| `all` | exploratory; also admits supported raw/instruction-word candidates after byte validation |

`all` is useful for experiments, not for establishing a baseline.

Each run records applied and withheld rows, with per-row reasons, in
`build/delink/*/relocations_withheld.tsv`. A HI16/LO16 candidate produces two
ELF relocation entries; a reviewed `mips_gprel16` row produces one
`R_MIPS_GPREL16` against the image's startup `$gp` (`config/images.toml`).

Carving the whole image is not the same as selecting decomp work. Functions in
`functions_vendored.tsv` are excluded when objdiff projects are generated; the
current per-image counts are in
[the inventory](function-and-data-inventory.md#current-census).

Vendored objects exist only to preserve
executable topology, provider evidence, symbol identities, and call relocation
targets. They do not count as source reconstruction or progress.

## MIPS analysis implications

R3000 code has no universal function marker. A conventional non-leaf function
often adjusts `$sp`, saves `$ra`, and restores both before `jr $ra`, but leaf
functions, tail calls, stubs, and hand-written SDK assembly can omit most of
that shape. Ghidra provides the main CFG baseline; the independent carver adds
stack-frame and clean return-delimited candidates. Both remain WIP evidence.

Every branch and jump has a delay slot: the instruction immediately after it
executes before control transfers. A load also has a one-instruction delay on
the original R3000A before its result can safely be consumed. Function carving,
source reconstruction, and instruction scheduling must preserve those slots;
the instruction following `jr $ra` is commonly part of the function.

Direct `jal` calls expose their target in the instruction. `jalr` calls through
a register, so the callee may come from a function-pointer table, switch table,
callback structure, or value assembled earlier. “The table” is not one special
global—it means any data object holding code addresses. A function referenced
only this way may remain undiscovered until those data rows and their consumers
are typed. This is why raw pointer candidates are retained even though the safe
object pass does not emit them yet.

Address formation such as `lui` followed by `addiu`, `lw`, or `sw` is toolable,
but not self-proving: immediates can also be numeric constants, and compiler
scheduling can separate the pair. The TSV records the inference channel and
confidence so later review can promote or reject it without losing evidence.

Known first-pass omissions are:

- no admitted `R_MIPS_GPREL16` census yet. JP game code has no `$gp`-relative
  access; `-G8` units model small-data placement only (`.sdata` claims,
  `SDATA()` literal ranges and `.sbss` statics, see `build-system.md`);
- no data sections assigned to translation units;
- no reconstructed fragmented-function ranges; and
- no final native Psy-Q object/link-order reproduction.

The evidence currently points to C plus hand-written/runtime assembly and has
not identified a C++ runtime, RTTI, exceptions, or vtable corpus. That is a
working language attribution, not proof that every original source file was C.

## Audit products

Every image directory contains:

| File | Meaning |
| --- | --- |
| `objects.tsv` | carved objects and their function identity |
| `relocations_used.tsv` | every emitted relocation, symbol, and implicit addend |
| `relocations_withheld.tsv` | every in-scope candidate not used, with reason |
| `functions_withheld.tsv` | functions that could not be represented safely |
| `objects/*.o` | synthetic target MIPS ELF objects, one per function |
| `modules/*.o` | one object per manifested unit: its contiguous run of functions carved as a single `.text` with per-function symbols and rebased section-relative addends, plus `.data`/`.bss` for the globals the unit claims with `DATA()` |

Candidates in data rather than a current function extent are also retained in
the withheld audit. They will move into data/translation-unit objects once
source ownership is curated.

## Commands

Given a hash-identical extraction containing all four executables:

```sh
nix develop
kf init --retail-dir /path/to/retail
kf build
kf analyze
kf status
```

`kf init` stores the validated absolute path in ignored `build/local.toml`.
An explicit `--retail-dir` takes precedence, followed by `KF_RETAIL_DIR`, then
the local file. `kf configure` validates `config/units.toml` and emits the
incremental Ninja graph. The normal commands are:

| Command | Behavior |
| --- | --- |
| `kf analyze [all\|base\|target\|compare\|verify]` | configure if needed, then build all or selected `--image` targets; `--retail-dir` overrides local configuration |
| `kf build [--image I]` | compile and link the selected source-to-EXE chain with native SDK tools |
| `kf try --unit ID [--source FILE]` | compile one unit into a scratch object, compare full-extent instruction/relocation listings, and report paired CFG clues for differing functions without updating recorded matches |
| `kf match [--unit ID]` | build, identify content-changed base objects, and summarize scores |
| `kf status [--json] [--all]` | report current state without building or writing |
| [`kf casts [--kind K] [--scope S] [--list\|--json]`](cast-audit.md) | parse every selected target-C unit variant and count written C-style casts once by spelling location, including shared macros and headers |
| [`kf enums [--value N] [--duplicates] [--json]`](enum-comparison.md) | compare evaluated enum names across source/image variants as leads for semantic review |
| [`kf literals [--domains] [--member TEXT] [--value N] [--output PATH]`](enum-domain-plan.md) | census every written integer literal and enum constant by semantic sink (field, parameter, return, case subject, index, mask, shift, arithmetic, declaration), join sinks connected by value flow into candidate enum domains with KF1 counterparts, and reconcile all written numeric tokens |
| [`kf parameters [--all] [--json]`](parameter-inventory.md) | inventory enum/record argument names, declarations and definitions in both modern and retail views for individual semantic review |
| [`kf resources census FILE --offset N --length N --stride N --field-offset N --encoding E [--json]`](resource-census.md) | count an explicitly identified binary record field with row and byte references |
| `kf check [--strict]` | fail on data mismatches/incomplete comparisons, known-reference ownership gaps, unfaithful target relink/placement, non-exact vendored source verification, unchanged-input regressions, lost banked rows, or invalid/stale reports |
| `kf verify roundtrip [--image I] [--unit ID] [--output PATH]` | relink manifested target ELF sections at claim-derived addresses with pinned GNU ld and compare all initialized bytes with verified retail |
| `kf bank [--unit ID] [--dirty]` | manually update all fresh scores, or only selected units when every selected function is exactly 100% |

Status separates eligible, manifested, compiled, scored, and exact functions.
The default exact threshold is strictly `100%`. `--loose` is available on
status and match for a `99.995%` navigation threshold, but the ledger always
stores the full unrounded value.

The delinker verifies each full executable hash and PS-X header before reading
any bytes. A focused iteration can select an image and function:

```sh
kf-delink \
  --exe-dir /path/to/retail \
  --image PSX.EXE \
  --va 0x80010068
```

Use `kf analyze` to refresh the shared project after focused delinking.

The generated objdiff project pairs every target object with:

```text
build/objdiff/<image>/base/<same target-object filename>
```

Only non-vendored objects become progress units. `vendored_excluded.tsv`
records every provider-owned per-function target omitted from progress. An
explicit `scope = "vendored"` manifest unit may still pair a reconstructed
provider module for strict 100% source verification; it is compiled and
checked but excluded from the denominator and bank. Until a game
reconstruction exists, `pairings.tsv` records a non-vendored function as
`unstarted`; it does not enter the objdiff project. A manifested source whose
build object is absent is separately recorded as `manifest-missing-base`.
Regenerate the project after adding a base object, then run:

```sh
kf-objdiff-report --project-dir build/objdiff
objdiff
```

One report and GUI project groups units under `psx/`, `game/`, and `open/`.
Each unit still compares its own image-local base and target objects, so
equal addresses or symbol names in different overlays do not collide.
`kf status` and the data gates select image-qualified units from the shared
report; progress continues to exclude vendored functions.

## Native source objects and comparison views

The executable and matching commands share this compilation path:

```text
C source -> CPPPSX/CC1PSX -> ASPSX -> DMPSX -> native Psy-Q OBJ
```

`kf build` passes the native objects and original SDK inputs to PSYLINK and
CPE2X. `kf analyze` reads native objects into ELF inspection views for objdiff.
Those views preserve actual section payloads, symbols, relocation expressions,
and unplaced COMMON requests; they do not generate the executable.

`kf-compile` exposes the same native compilation primitive for focused tests.
It writes an adjacent `.OBJ`, `.S`, and `.o.json` alongside the ELF view. A C
invocation must state its optimization profile. Use `kf try --unit ID` for a
manifested unit so its compiler flags, include paths and defines are shared
with the complete executable build. Historical compiler attribution remains
open; both the GCC 2.5.7 and 2.6.0 profiles are explicit probes.

Translation-unit hypotheses are expressed directly by the sources: a unit
source claims one contiguous run of functions with `ADDRESS()` and the globals
it owns with `DATA()`, and is compiled once against the module object carved
for that run (see `build-system.md`). Function symbols in target objects
follow `function_identities.tsv`, and data symbols follow
`data_identities.tsv`, so reconstructed source uses curated names and
address-derived names remain the explicit spelling of unresolved identities.
Claimed load data is carved by bytes; the relocation candidates inside it are
raw pointer words that the safe policy still withholds, so pointer tables
compare byte-for-byte until their rows are reviewed.

## Validation

The default build also checks target-object roundtrips through the independent
GNU MIPS linker. A packed section must admit one retail base for all its owned
symbols; the linker may not scatter it or override defined symbols to hide a
layout conflict. Relinked initialized bytes are compared in full, and BSS is
checked as uninitialized storage. A unit's single global COMMON request whose exact `DATA()` claim
is the only one in its section is placed at that claim: in `.bss`, or in
`.sbss` for a small request of a `-G8` unit. A unit whose `.sdata` holds
only its `SDATA()` literal range places that section at the range's start. Synthetic tails imitating the probe assembler
are not appended to retail claims: source alignment/extent differences remain
real comparison failures. This is not yet a linked reconstruction or a check
of all config-only reachable storage.

`kf check` applies the target-relink and known-reference closure gates only to
images that at least one source unit claims; an image without units has
nothing to relink, and its census-wide closure is the curation worklist
(`kf verify reachability --image KEY`). The OK line says how many images were
gated. Inside reached Sony functions, only indirect transfers proved from the
member's own instructions (BIOS `li $t2, 0xA0/0xB0/0xC0; jr $t2` stubs and
startup table loops with a link-time zero count) are exempt; they are listed
as `vendored_dispatch` witness rows in the reachability report. In a game
function, a compiled `switch` jump is proved from its own bounded value
chain (`sltiu`/`beqz` bound, `lui`/`addiu` table address, `sll`/`addu`/`lw`,
`jr`) when every table word is an instruction start inside that function;
the CFG's `switch_table` rule (`scripts/kf/sema/cfg.py`) is the one prover,
and each such jump is a `switch_dispatch` witness row. A Sony member's
switch jump gets no exemption from it.

Both objdiff front ends are built from the same pinned 3.7.3 source with
`patches/objdiff-strict-data.patch`. Native section scores include anonymous
bytes, padding, ordered relocation rows and BSS extents; report data totals
also account for extra base storage. `tests/objdiff_data_smoke.py` verifies
positive and negative MIPS controls in both directions through `diff` and
`report generate`. See the [data campaign](data-matching-campaign.md) for the
remaining ownership and linked-image boundary; a native data score is not
complete reachable-byte coverage.

`tests/objdiff_mips_smoke.py` uses only synthetic functions. One checks ordinary
instruction matching; a second 100% calibration covers an external JAL, a
local `.text` J relocation, and an external HI16/LO16 pair with a nonzero
addend. No vendored retail routine is presented as decompiled source. The normal
Python tests also parse the ELF headers, symbols, relocation types, function
extent, MIPS26 addend, and HI16/LO16 carry behavior.
`tests/compiler_mips_smoke.py` compiles a simple C function with GCC 2.6.0,
passes its assembly through native ASPSX, and verifies the real object and
its ELF inspection view. It also covers widths, private BSS, exported COMMON,
function order, native division checks and return delay slots.

Run the complete checks with:

```sh
nix flake check
```
