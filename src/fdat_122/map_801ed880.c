/*
 * FDAT_122.BIN: its last two handlers (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Arms table A entries 32..34 and resets those that are not active. */
ADDRESS(0x801ed880, 0x78)
void func_801ed880(void)
{
    vis_table_a.entries[32].unk00 = vis_table_a.entries[33].unk00 = vis_table_a.entries[34].unk00 = 100;
    if (vis_table_a.entries[32].unk05 == 0)
        fdat_801ed410(&vis_table_a.entries[32]);
    if (vis_table_a.entries[33].unk05 == 0)
        fdat_801ed410(&vis_table_a.entries[33]);
    if (vis_table_a.entries[34].unk05 == 0)
        fdat_801ed410(&vis_table_a.entries[34]);
}

/* Restarts script 0 of an entry of record 0x35 that is running script 1. */
ADDRESS(0x801ed8f8, 0x40)
void func_801ed8f8(Vis212 *e)
{
    if (e->unk26 == 0x35 && e->unk18 == 1)
        fdat_801eb7b4(e, 0);
}
