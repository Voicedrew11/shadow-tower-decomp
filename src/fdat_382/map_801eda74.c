/*
 * FDAT_382.BIN: its last handler (WIP module; a unit of its own until its
 * neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* In area 0, while flag 24 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has reached 2. */
ADDRESS(0x801eda74, 0x88)
void func_801eda74(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.unk3500[1];

    if (DAT_801cba08.unk1448[r->unk09] >= 2 && !(DAT_801cba08.unk1440 >> 24 & 1)
        && vis_table_a.unk6168 == 0) {
        DAT_801cba08.unk1448[r->unk09] = 3;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
