# Forty-three images in one repository

This project is one reverse-engineering effort with 43 independent
decompilation targets: seven linked PS-X EXE programs and 36 code overlays.
`config/images.toml` is the authority for each image's file, hash and
layout.

| Target | Retail role | Load window | Functions (game / Psy-Q) |
| --- | --- | --- | ---: |
| `PSX.EXE` | boot file; program sequencer | `0x80010000..0x80010800` | 1 / 10 |
| `STR.EXE` | program index 0, run first | `0x80011000..0x80033000` | 32 / 290 |
| `ST.EXE` | main game program (index 3) | `0x80011000..0x80088800` | 682 / 457 |
| `TITLE.EXE` | program index 1 | `0x80012000..0x80029000` | 34 / 353 |
| `OPTXT.EXE` | program index 2 | `0x80011000..0x8001f800` | 29 / 254 |
| `VS.EXE` | program index 5 | `0x80011000..0x80063800` | 259 / 401 |
| `END.EXE` | program index 4 | `0x80011000..0x80030000` | 16 / 260 |
| `FDAT_nnn.BIN` (36) | `ST.EXE` code overlays | `0x801EB560` + `0x2800`–`0x4800` | 943 / 0 (23–37 each) |

Counts are the current WIP census (`config/retail/functions.tsv` and
`functions_vendored.tsv`); see
[function-and-data-inventory.md](function-and-data-inventory.md). Indices are
the slots of `PSX.EXE`'s program table; other roles are not yet established.

The six programs occupy overlapping RAM at different times; `PSX.EXE` stays
resident below them and runs them in turn. The FDAT overlays are not PS-X
EXEs: they are raw entries of `ST\COM\FDAT.T`, loaded by `ST.EXE` one at a
time at `0x801EB560`, the end of `ST.EXE`'s `.bss`. They therefore share
`ST.EXE`'s address space: their `jal` targets outside the overlay are
`ST.EXE` functions, and an overlay's file offset is its RAM offset from
`0x801EB560` (there is no 0x800-byte header).

Every authoritative address is keyed by `(image, va)`. The same virtual
address names unrelated functions in different programs, and all 36
overlays use the same window. A symbol, relocation, or function must never
be joined across images by address alone, and a call from an overlay into
`ST.EXE` is a cross-image reference.

The repository shares everything that should be shared:

- one pinned SDK and the GNU/maspsx analysis route;
- platform headers, Sony library declarations, and reconstructed types;
- analysis scripts and Ghidra configuration;
- evidence and naming conventions; and
- source files proven byte-identical between images (the FDAT overlays
  repeat a common set of functions in every entry).

`src/` holds one directory per image key (`psx/`, `st/`, `fdat_012/`, ...)
plus `lib/` for sources linked into more than one image (`ADDRESS_AT`
claims). Linking an overlay is not modeled yet: `kf build` refuses source
units for an overlay image until its output format is.

## Address and naming rules

The structural inventory in `config/retail/functions.tsv` remains the source
of function boundaries. `functions_vendored.tsv` overlays Sony/Psy-Q provider
names without replacing that structure. The delinker selects names in this
order:

1. a structural function name;
2. a vendored-function name at the same `(image, va)`; or
3. a deterministic `func_<va>` placeholder.

Duplicate preferred names within one image receive an address suffix. They
do not need suffixes merely because another image uses the same name.

Data and relocation ownership will eventually be assigned to translation
units. That ownership is not inferable from the linked image alone, so the
initial pipeline carves one object per contiguous function. It is a
calibration topology, not a claim about the original files. Provider-
identified Sony/Psy-Q functions never become reconstruction or objdiff
units.

## Matching tiers

There are two different goals:

1. **Object match.** Compile reconstructed C or assembly, compare its MIPS ELF
   object to the carved target object, and iterate quickly with objdiff.
2. **Retail link match.** Reconstruct original Psy-Q object boundaries, library
   extraction, link order, linker script/options, overlay loading, and PS-X EXE
   packaging with the native tools.

The first tier can progress while the exact compiler and link topology remain
under investigation. A 100% function score proves the chosen object model and
code bytes for that comparison; it does not by itself prove the final Psy-Q
link or the historical translation-unit boundary.

See [delinking-and-matching.md](delinking-and-matching.md) for the implemented
first-tier workflow.
