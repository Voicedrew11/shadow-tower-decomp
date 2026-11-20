/*
 * VS.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * DAT_801961e0's end (0x80196224, then 8-aligned) and DAT_80196230. Nothing references it, but PSYLINK leaves no hole between
 * objects' COMMON beyond each request's alignment, so some object allocated
 * these bytes; its identity is admitted from the layout alone and its unit
 * stays unresolved until the linked .SYM confirms the placement.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

DATA(0x80196228, 0x8, ".bss")
u8 DAT_80196228[0x8];
