/*
 * ST.EXE: a data-only module (WIP) defining the 0x80088190-0x8008819f part of the
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
#include <st/menu.h>
#include <st/text.h>


DATA(0x80088190, 0x2, ".sbss")
u16 DAT_80088190;

DATA(0x80088192, 0x2, ".sbss")
u16 DAT_80088192;

/* Unreferenced (layout evidence only). */
DATA(0x80088194, 0x4, ".sbss")
s32 DAT_80088194;

/* Unreferenced (layout evidence only). */
DATA(0x80088198, 0x6, ".sbss")
u16 DAT_80088198[3];

DATA(0x8008819e, 0x2, ".sbss")
u16 DAT_8008819e;
