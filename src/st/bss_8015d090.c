/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * the graphics state and the menu's record words.
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
#include <st/gfx.h>
#include <st/menu.h>

DATA(0x8015d090, 0x1ee1a, ".bss")
WorldState world_state;
DATA(0x8017beb0, 0x1c, ".bss")
s32 DAT_8017beb0[7];
