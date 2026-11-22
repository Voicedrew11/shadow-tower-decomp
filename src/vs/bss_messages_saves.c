/*
 * VS.EXE: a data-only module (WIP) defining game .bss:
 * the message tables and the card saves.
 * PSYLINK allocates each object's tentative definitions (COMMON) after the
 * previous object's, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/menu.h>
#include <vs/card.h>

DATA(0x80196230, 0xa800, ".bss")
VsMessages DAT_80196230[2];
DATA(0x801a0a30, 0x50, ".bss")
char DAT_801a0a30[2][40];
DATA(0x801a0a80, 0x3b10, ".bss")
VsSave DAT_801a0a80[2];
