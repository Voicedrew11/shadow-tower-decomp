/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the equipment
 * screen's saved ids and the id grid state. PSYLINK allocates each object's
 * tentative definitions (COMMON) after the previous object's, starting
 * eight-aligned, in symbol-hash bucket order (docs/executable-linking.md),
 * so the definitions here are listed in address order and their spellings
 * ascend in bucket; every user declares them extern. The spelling
 * equip_menu_saved_ids sorts below DAT_801e0d20 in that order (buckets 93
 * and 312), so it starts an object of its own.
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

DATA(0x801e0d28, 0xc, ".bss")
u16 equip_menu_saved_ids[6];
DATA(0x801e0d38, 0x14, ".bss")
ArrangeListState arrange_list_state;
