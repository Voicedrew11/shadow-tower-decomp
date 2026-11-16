# Function, global, and static identities

The structural census says where linked code and data exist. The identity
layer records the current source-level interpretation without changing delink
symbols or pretending that WIP names are original symbols:

- `config/retail/functions.tsv` lists every admitted function start and WIP
  extent, and `data.tsv` accounts for every other loaded byte (defined data
  or an `unclassified` gap);
- `config/retail/functions_vendored.tsv` layers Sony/Psy-Q ownership over the
  structural rows;
- `config/retail/function_identities.tsv` has exactly one row for each
  carveable non-vendored function;
- `config/retail/data_identities.tsv` tracks initialized objects and referenced
  BSS objects that may become C globals, file-local statics, or
  function-local statics;
- `config/retail/structures.tsv` and `structure_fields.tsv` track complete
  target sizes and every field offset/extent, including opaque ranges; and
- address-derived `func_` and `DAT_` names are stable unresolved identities.

## Current census

| Image | Functions | Psy-Q | Game |
| :---- | --------: | ----: | ---: |
| `PSX.EXE` | 11 | 10 | 1 |
| `STR.EXE` | 322 | 302 | 20 |
| `ST.EXE` | 1,139 | 457 | 682 |
| `TITLE.EXE` | 387 | 369 | 18 |
| `OPTXT.EXE` | 283 | 265 | 18 |
| `VS.EXE` | 660 | 406 | 254 |
| `END.EXE` | 276 | 261 | 15 |
| 36 `FDAT_nnn.BIN` | 943 (23–37 each) | 0 | 943 |
| **Total** | **4,021** | **2,070** | **1,951** |

PSX.EXE `main` is reconstructed (`src/psx/main.c`) with its six data
identities and reviewed relocations, STR.EXE's twenty game functions
(`src/str/main.c`), TITLE.EXE's eighteen (`src/title/main.c`), OPTXT.EXE's
eighteen (`src/optxt/main.c`, seventeen exact) and END.EXE's fifteen
(`src/end/main.c`) with their data; every other game identity is
address-only with a candidate signature from the static MIPS live-in
heuristic (`kf inventory seed`). The tables are WIP, and the rules below
say how they grew.

### Function starts

Starts come from `kf-function-audit` and `kf-function-propose`, rerun until
the proposal stops changing (three passes from the entry-only census). Each
row's `confidence` names the rule that admitted it:

- `entry`: the PS-X EXE header pc0;
- `mips-frame-carve`: a stack frame with `ra` saved and a matching stack
  restore at the return. Shadow Tower's compiler restores the stack before
  the return (`addiu sp; jr ra; nop`); loads scheduled above the frame
  allocation start at the previous function's return;
- `mips-return-carve`: valid code between one return and the next;
- `mips-call-carve`: a `jal` target decoded from admitted code that lies in
  unowned bytes, carved to its first unconditional transfer (BIOS A0/B0
  stubs end in `jr $t2`); and
- `mips-call-split`: a `jal` target inside an admitted extent whose preceding
  code cannot fall through into it.

A `jal` target up to 0x40 bytes of straight-line code above an admitted
start moves that start back (the row's note records it). After the last
pass every `jal` target decoded from admitted code lands on an admitted
start, except 18 interior entries (GTE assembly with two entry points in
ST `0x80075acc`, `0x80075ecc`, `0x8007636c`, plus ST `0x8006887c` and VS
`0x80035f94`), which stay unsplit for review. Every overlay's 32 header
handler pointers land on admitted starts. Pointer-only (`referenced-code`)
candidates, mostly jump-table cases, are not admitted.

### Psy-Q ownership

`kf-vendored-seed` layers two channels over the structural rows. First,
relocation-masked function IDs built from the libraries the build links
(Runtime Library 4.2 with the 4.2.1 LIBPAD, the 4.5 kit's `NONE3.OBJ` and
Release 2.5 `MALLOC.OBJ`; `kf-fid-census` builds the same corpus): 743 admitted
rows are `fid-sdk42`, of which 28 are `MALLOC.OBJ` and 6 `NONE3.OBJ` (their
`source_version` names the standalone object). Then `ghidra_psx_ldr`
signature sets fill starts the IDs leave open, preferred set first
(`--signature-set`, default 420): 1,263 `psyq420-signature`, 13
`psyq420-signature-ambiguous`, and 6 `psyq410-signature` rows for the
eight-byte `NONE3` `__main`, too short for a function ID. Each confidence
names its evidence and release.

END.EXE `0x8001670c` (`StSetMask`, LIBCD member `C_010`) is too short for a
function ID or signature; the byte-identical native relink maps it there, so
its row's confidence is `relink-map-sdk42`. The same holds for STR.EXE
`0x80016c3c` (`StSetMask`) and for eleven LIBPAD starts the census had left
as game code: `PADMAIN`'s exported `_padSetVsyncParam`, `_padInitSioMode` and
`_padWaitRXready` and three static functions, and all five static functions
of `PADIF` (whose only export is the `padIntFunc` data table); the
byte-identical STR.EXE relink places each at its member offset. The
byte-identical TITLE.EXE relink places sixteen more the census had left as
game candidates: `SpuInit` (LIBSPU `S_I`), `SsQuit`, `_SsSndPlay`,
`SsUtReverbOn`, `_SsVmDamperOff` (LIBSND), three exports and three statics of
LIBPAD `PADMAIN`, and the five `PADIF` statics that `padIntFunc` points at.
OPTXT.EXE's same eleven LIBPAD functions (0x5e0 bytes of text between
`PADCMD` and `PADPORTD`) are placed by the native relink whose bytes outside
`play_opening_text` all equal retail.

Not admitted: the overlays' two-instruction default handler (`jr ra; nop`),
matched only by an old LIBSND `VM_VIB` signature, since the overlays link no
SDK code; and `__builtin_delete` (LIBSN `_OP_DELE`) at ST `0x800152f8` and
VS `0x8002e6fc`, an exact 4.2 function ID that sits inside game code; it
stays game until its placement is explained.

## Ownership board

`kf verify board` reports **unresolved data ownership** separately from the
informational **raw DAT_ identities** count. An identity has an established
owner when its complete extent is covered by an image- and storage-matching
source `RODATA`/`DATA` claim, or by a supported/proven identity with an owner
and recorded evidence. A name alone, a candidate annotation, address proximity,
or a pointer into SDK code does not establish ownership. SDK-owned data follows
the same evidence rule. Counts describe inventory candidates, not necessarily
the number of original C objects.

Both data inventory counts are informational: discovering new data must not
fail the source cleanliness ratchet. `--update` saves only ratcheted metrics.
For the remaining work list, run `kf verify board --data`; use
`kf verify board --data all` to include each accepted owner and its evidence.
These commands read curated inputs and source claims without binary heuristics
or changes to the inventory.

## Review loop

```sh
kf-function-audit --exe-dir retail      # build/function-audit/candidates.tsv
kf-function-propose                     # build/function-audit/proposed-retail/
kf-vendored-seed --exe-dir retail       # build/vendored-seed/functions_vendored.tsv
kf inventory check
kf inventory propose
kf inventory ghidra --image st --image vs
kf inventory propose-ghidra

kf sema --image st addr 0x80077104
kf sema --image fdat_012 xref 0x801ed498 --confirmed-only
kf sema --image st disasm 0x80014320 --blocks
```

Every generator writes below `build/`; diff its output against
`config/retail` and admit rows by hand.

`kf inventory propose` writes ignored review files:

- `build/function-inventory/evidence.tsv` combines MIPS live-in signature
  hints, callers, callees, indirect calls, strings, data references, and basic
  instruction-shape counts for every non-vendored function;
- `build/function-inventory/data-evidence.tsv` lists confirmed/candidate users,
  read/write/address counts, and tentative scope/owner hints for every tracked
  datum;
- `build/function-inventory/relocs-bss.tsv` proposes decoded MIPS HI/LO pairs
  from non-vendored code to admitted BSS extents.

A datum with one user receives a `function-static` hint and that function as
an owner hint; the adjacent confidence column distinguishes confirmed from
candidate xrefs. This is prioritization evidence only: another xref may still
be missing, and local linkage cannot be reconstructed from one use alone.
Multiple users produce `shared`, not `global`: without reliable TU ownership,
xrefs cannot distinguish external linkage from a file-local static used by
several functions.

`kf inventory ghidra` keeps persistent, ignored projects under
`build/ghidra-inventory`. Its JSON contains the inferred prototype, structured
parameter storage, decompiled C, and non-flow memory references for each
non-vendored function. Ghidra results are proposals because analysis can merge
functions, infer the wrong width, or manufacture a misleading default name.
They must be checked against retail instructions, delay slots, callers,
callees, relocation referents, and the pinned headers before admission.
`kf inventory propose-ghidra` merges those inferred prototypes into an ignored
review TSV while preserving supported/proven signatures; it also never edits
the curated identity file. It also writes `ghidra-data-xrefs.tsv`, grouping
Ghidra's memory references by target and marking each as admitted data/text,
scratchpad, hardware, or unowned RAM. Unowned RAM and zero-admitted-site groups
are queues for missing global/static extents and relocation review.
Its companion `data-identities.ghidra.tsv` proposes non-overlapping BSS extents
only for RAM addresses directly read or written by an instruction. Mere scalar
constants, pointer-only parameters, heap initialization addresses, scratchpad,
and hardware references are not auto-admitted.

## Curation rules

- Never key an identity by VA alone; `image` plus `va` is the key. The FDAT
  overlays all share one window, and their calls into `ST.EXE` are
  cross-image references.
- Preserve an address-derived name until evidence supports a semantic one.
- Record semantic argument names only after checking call sites and the body.
- Do not create an interior global when the address is a field or element of a
  larger owned object; refine the owner's datatype and extent instead.
- `scope=static` means file-local linkage and `scope=function-static` means a
  function-local static. Neither may be promoted solely from xref count.
- Keep generated reports under `build/`; only reviewed edits belong in the
  curated TSVs.

The inventories are intentionally iterative. Missing functions, relocations,
data extents, and xrefs can invalidate earlier candidates without invalidating
the workflow. Only PSX.EXE `main`'s, STR.EXE's, TITLE.EXE's, OPTXT.EXE's and END.EXE's relocations are admitted so far.
