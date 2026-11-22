/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the menu's record counters and the CD state block.
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
#include <st/cd.h>
#include <st/menu.h>

DATA(0x800881f0, 0xe, ".bss")
s16 DAT_800881f0[7];
DATA(0x80088200, 0xa88, ".bss")
StCdState cd_state;
