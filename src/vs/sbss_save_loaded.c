/*
 * VS.EXE: a data-only module (WIP) defining the 0x80063380-0x80063387 block of the
 * first .sbss run. It links before the code (link = "first"): the start-up
 * module reads part of the run with lui pairs, so it cannot have allocated
 * it, and an earlier object did (docs/executable-linking.md).
 * PSYLINK allocates each object's small COMMON after the previous
 * object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md); the run splits at eight-byte boundaries
 * into blocks whose spellings ascend in bucket, one object each.
 * A code unit that reads a word $gp-relative keeps a tentative
 * definition, which merges here.
 * The two bytes and the unreferenced DAT_80063388 after them were
 * vs.menu's COMMON batch, but in bucket order DAT_80063388 (232) sorts
 * before them (389 and 390). Defined here and in vs.sbss_hole_after_save_loaded,
 * first-linked objects allocate all three at the end of the run, and
 * vs.menu's tentative definitions merge into them.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/sound.h>

DATA(0x80063380, 0x1, ".sbss")
u8 save_loaded_1p;
DATA(0x80063381, 0x1, ".sbss")
u8 save_loaded_2p;
