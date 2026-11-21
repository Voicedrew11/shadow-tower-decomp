/*
 * ST.EXE: a data-only module (WIP) defining game .bss:
 * an unreferenced block, the option screens' equipment ids, the id grid
 * state and the message table.
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
#include <st/idgrid.h>
#include <st/menu.h>

/* Unreferenced (layout evidence only). */
DATA(0x801e0d20, 0x8, ".bss")
u8 DAT_801e0d20[0x8];
DATA(0x801e0d28, 0xc, ".bss")
u16 DAT_801e0d28[6];
DATA(0x801e0d38, 0x14, ".bss")
IdGridState DAT_801e0d38;
DATA(0x801e0d50, 0x5400, ".bss")
MenuMessages menu_messages;
