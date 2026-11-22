/*
 * VS.EXE: a data-only module (WIP) defining game .bss: an unreferenced
 * halfword array and the sequence attribute table. PSYLINK allocates each
 * object's tentative definitions (COMMON) after the previous object's,
 * starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md), so the definitions here are listed in
 * address order and their spellings ascend in bucket; every user declares
 * them extern. The spelling DAT_80117598 sorts below voice_status in that
 * order (buckets 235 and 293), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>

DATA(0x80117598, 0xe, ".bss")
u16 DAT_80117598[7];
DATA(0x801175a8, 0x160, ".bss")
char seq_attr_table[0x160];
