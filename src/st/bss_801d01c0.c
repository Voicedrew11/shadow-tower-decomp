/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * sound bank buffers 2..13 and 15, the sound state block and the equipment screen state.
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
#include <st/equip_menu.h>

DATA(0x801d01c0, 0x4000, ".bss")
u8 DAT_801d01c0[4][0x1000];
DATA(0x801d41c0, 0x4000, ".bss")
u8 DAT_801d41c0[4][0x1000];
DATA(0x801d81c0, 0x1000, ".bss")
u8 DAT_801d81c0[0x1000];
DATA(0x801d91c0, 0x4000, ".bss")
u8 DAT_801d91c0[4][0x1000];
DATA(0x801dd1c0, 0x3b28, ".bss")
Data801dd1c0 DAT_801dd1c0;
DATA(0x801e0ce8, 0x20, ".bss")
Data801e0ce8 DAT_801e0ce8;
