/*
 * FDAT_182.BIN: a handler after fdat_801ed4a0 (WIP module; a unit of its own
 * until its neighbours in the run are exact).
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

/* In area 3, while flag 18 is clear, hands table A entry 32 to its exit. */
ADDRESS(0x801ed950, 0x7c)
void func_801ed950(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.unk3500[12];

    if (!(DAT_801cba08.unk1440 >> 18 & 1) && vis_table_a.unk6168 == 3) {
        DAT_801cba08.unk1448[r->unk09] = 2;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
