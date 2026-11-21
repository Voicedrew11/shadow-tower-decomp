/*
 * ST.EXE: a data-only module (WIP) defining the 0x800881b0-0x800881b7 part of the
 * first .sbss run. It links before the code (link = "first"): the -G8
 * modules that use these words (st.ui, st.menu, st.equip_menu, ...) read
 * most of them with lui pairs, so none of them allocated the run, while
 * st.loop, the second object, reads DAT_800881a4 inside it $gp-relative.
 * PSYLINK allocates each object's small COMMON after the previous
 * object's, in symbol-hash bucket order (docs/executable-linking.md); the
 * address names split the run into four ascending blocks, one object each.
 * A code unit that reads a word $gp-relative keeps a tentative
 * definition, which merges here.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>


/* Unreferenced (layout evidence only). */
DATA(0x800881b0, 0x8, ".sbss")
u8 DAT_800881b0[8];
