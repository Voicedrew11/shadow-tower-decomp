/*
 * VS.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * vis_table_c's end and pad_buffers. Nothing references it, but PSYLINK leaves no hole between
 * objects' COMMON beyond each request's alignment, so some object allocated
 * these bytes; its identity is admitted from the layout alone and its unit
 * stays unresolved until the linked .SYM confirms the placement.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

DATA(0x80196010, 0x1d0, ".bss")
u8 DAT_80196010[0x1d0];
