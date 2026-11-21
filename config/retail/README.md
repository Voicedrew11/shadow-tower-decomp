# Retail executable censuses

These TSVs are the admitted, manually curated work-in-progress model of the
Shadow Tower `SLUS-00863` executables listed in `../images.toml`:

- `functions.tsv`: current function starts and explicit WIP extents;
- `functions_vendored.tsv`: provider/library ownership layered over those
  structural functions;
- `data.tsv`: defined data plus `unclassified` gaps;
- `relocs.tsv`: reconstructed relocation candidates and their review status;
- `function_identities.tsv`: source-level names, owners, actions, return types,
  and named parameters for all carveable non-vendored functions;
- `data_identities.tsv`: source-level names, extents, storage, linkage scope,
  types, and owners for initialized globals/statics and referenced BSS;
- `data_sections.tsv`: reviewed initialized section assignments;
- `structures.tsv` and `structure_fields.tsv`: target structure layouts with
  their evidence.

Each image is a separate linked program or overlay, so every row carries an
`image`. The six programs reuse the 0x80011000 load region, and the 36
`FDAT_nnn.BIN` overlays all load at 0x801EB560 inside `ST.EXE`'s address
space: never merge rows on address alone. Overlay rows use file offsets
relative to the overlay image (it has no PS-X EXE header).
All files start empty, with their headers only. Seed them with
`kf inventory propose` and admit rows by review; see
`docs/function-and-data-inventory.md`.
