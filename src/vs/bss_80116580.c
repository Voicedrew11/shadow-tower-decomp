/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the menu sound bank header and the sound tables.
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

DATA(0x80116580, 0x1000, ".bss")
u_char DAT_80116580[0x1000];
DATA(0x80117580, 0x18, ".bss")
char DAT_80117580[24];
DATA(0x80117598, 0xe, ".bss")
u16 DAT_80117598[7];
DATA(0x801175a8, 0x160, ".bss")
char DAT_801175a8[0x160];
