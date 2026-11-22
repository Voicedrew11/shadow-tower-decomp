/*
 * VS.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * vs_gfx_state's end and vis_table_c (0x80191d00). Nothing references it, but PSYLINK leaves no hole between
 * objects' COMMON beyond each request's alignment, so some object allocated
 * these bytes; its identity is admitted from the layout alone and its unit
 * stays unresolved until the linked .SYM confirms the placement.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

DATA(0x8018ef60, 0x2da0, ".bss")
u8 DAT_8018ef60[0x2da0];
