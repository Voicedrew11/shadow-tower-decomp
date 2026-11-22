/*
 * VS.EXE: a data-only module (WIP) defining game .bss: the SPU voice status
 * bytes. PSYLINK allocates each object's tentative definitions (COMMON)
 * after the previous object's, starting eight-aligned, in symbol-hash
 * bucket order (docs/executable-linking.md), and every user declares it
 * extern. The spelling voice_status sorts below menu_vab_head in that order
 * (buckets 293 and 331), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>

DATA(0x80117580, 0x18, ".bss")
char voice_status[24];
