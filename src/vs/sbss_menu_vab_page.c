/*
 * VS.EXE: a data-only module (WIP) defining the 0x80063358-0x8006335f block of the
 * first .sbss run. It links before the code (link = "first"): the start-up
 * module reads part of the run with lui pairs, so it cannot have allocated
 * it, and an earlier object did (docs/executable-linking.md).
 * PSYLINK allocates each object's small COMMON after the previous
 * object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md); the run splits at eight-byte boundaries
 * into blocks whose spellings ascend in bucket, one object each.
 * A code unit that reads a word $gp-relative keeps a tentative
 * definition, which merges here.
 * The spelling DAT_80063358 sorts level with camera_speed in that order
 * (buckets 229 and 229), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/sound.h>

/* Unreferenced (layout evidence only). */
DATA(0x80063358, 0x2, ".sbss")
u16 DAT_80063358;
DATA(0x8006335a, 0x2, ".sbss")
s16 DAT_8006335a;
DATA(0x8006335c, 0x2, ".sbss")
u16 DAT_8006335c;
