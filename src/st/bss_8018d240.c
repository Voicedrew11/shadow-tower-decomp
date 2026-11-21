/*
 * ST.EXE: a data-only module (WIP) owning an unreferenced .bss hole between
 * lighting's end (0x8018d23c, then 8-aligned) and effects (0x8018d298).
 * Nothing references it, but PSYLINK leaves no hole between objects' COMMON
 * beyond each request's alignment, so some object allocated these bytes;
 * its identity is admitted from the layout alone and its unit stays
 * unresolved until the linked .SYM confirms the placement. Its spelling
 * falls below lighting's bucket and above effects', so it is an object of
 * its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

DATA(0x8018d240, 0x58, ".bss")
u8 DAT_8018d240[0x58];
