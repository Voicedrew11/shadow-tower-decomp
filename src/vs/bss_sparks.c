/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the hit-spark
 * particles. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern. The spelling sparks sorts below file_row_clipped in that order
 * (buckets 154 and 167), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/effect.h>

DATA(0x801c4a58, 0x9c40, ".bss")
VsSpark sparks[2000];
