/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * visibility table C and the save-game state block.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/menu.h>
#include <st/vis.h>

DATA(0x801c76f8, 0x4310, ".bss")
VisTableC vis_table_c;
DATA(0x801cba08, 0x1714, ".bss")
Data801cba08 DAT_801cba08;
