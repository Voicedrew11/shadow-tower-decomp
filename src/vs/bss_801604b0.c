/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the sound banks, the teams and the graphics state.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>
#include <vs/sound.h>
#include <vs/menu.h>

DATA(0x801604b0, 0x2008, ".bss")
VsSoundBank DAT_801604b0[2];
DATA(0x801624b8, 0x18, ".bss")
VsTeam DAT_801624b8[2];
DATA(0x801624d0, 0x2ca90, ".bss")
VsWorldState vs_gfx_state;
