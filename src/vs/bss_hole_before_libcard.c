/*
 * VS.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * the last game object (DAT_801ce698's end) and LIBCARD's .bss (0x801d24a8). Nothing references it, but PSYLINK leaves no hole between
 * objects' COMMON beyond each request's alignment, so some object allocated
 * these bytes; its identity is admitted from the layout alone and its unit
 * stays unresolved until the linked .SYM confirms the placement.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

DATA(0x801d1ba8, 0x900, ".bss")
u8 DAT_801d1ba8[0x900];
