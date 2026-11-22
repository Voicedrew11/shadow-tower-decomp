/*
 * VS.EXE: a data-only module (WIP) defining the 0x80063300-0x8006330f block of the
 * first .sbss run. It links before the code (link = "first"): the start-up
 * module reads part of the run with lui pairs, so it cannot have allocated
 * it, and an earlier object did (docs/executable-linking.md).
 * PSYLINK allocates each object's small COMMON after the previous
 * object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md); the run splits at eight-byte boundaries
 * into blocks whose spellings ascend in bucket, one object each.
 * A code unit that reads a word $gp-relative keeps a tentative
 * definition, which merges here.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/sound.h>

DATA(0x80063300, 0x4, ".sbss")
VsFighter *current_fighter;
DATA(0x80063304, 0x4, ".sbss")
s32 DAT_80063304;
DATA(0x80063308, 0x4, ".sbss")
s32 DAT_80063308;
/* Unreferenced (layout evidence only). */
DATA(0x8006330c, 0x4, ".sbss")
s32 DAT_8006330c;
