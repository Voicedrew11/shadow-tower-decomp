/*
 * VS.EXE: a data-only module (WIP) defining the 0x80063330-0x8006333f block of the
 * first .sbss run. It links before the code (link = "first"): the start-up
 * module reads part of the run with lui pairs, so it cannot have allocated
 * it, and an earlier object did (docs/executable-linking.md). PSYLINK
 * allocates each object's small COMMON after the previous object's, in
 * symbol-hash bucket order; the run splits into 16-byte blocks whose
 * address names ascend in bucket, one object each. Every code unit that
 * reads a word $gp-relative keeps a tentative definition, which merges here.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/sound.h>

DATA(0x80063330, 0x4, ".sbss")
s32 DAT_80063330[1];

DATA(0x80063334, 0x4, ".sbss")
s32 DAT_80063334;

DATA(0x80063338, 0x4, ".sbss")
s32 DAT_80063338;

/* Unreferenced (layout evidence only). */
DATA(0x8006333c, 0x2, ".sbss")
u16 DAT_8006333c;

DATA(0x8006333e, 0x2, ".sbss")
u16 DAT_8006333e;
