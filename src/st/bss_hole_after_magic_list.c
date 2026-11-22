/*
 * ST.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * equip_magic_list's end and equip_menu_saved_ids (0x801e0d28). Nothing
 * references it, but PSYLINK leaves no hole between objects' COMMON beyond
 * each request's alignment, so some object allocated these bytes; its
 * identity is admitted from the layout alone and its unit stays unresolved
 * until the linked .SYM confirms the placement.
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
