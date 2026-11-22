/*
 * VS.EXE: a data-only module (WIP) owning an unreferenced eight bytes at
 * the end of the first .sbss run (0x80063388-0x8006338f), before the
 * next module's .sbss statics (0x80063390). Nothing references them; the
 * identity is admitted from the layout alone. They were vs.menu's, after
 * its two save bytes, but DAT_80063388 sorts below save_loaded_2p in
 * PSYLINK's bucket order (buckets 232 and 390), so it is an object of its
 * own, linked first after vs.sbss_save_loaded; its batch starts
 * eight-aligned (docs/executable-linking.md).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>

/* Unreferenced (layout evidence only). */
DATA(0x80063388, 0x8, ".sbss")
u8 DAT_80063388[8];
