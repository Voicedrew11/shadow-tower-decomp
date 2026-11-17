/*
 * FDAT_372.BIN: its last handler (WIP module).
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

/* In area 1, while flag 20 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has started. */
ADDRESS(0x801ed9b4, 0x88)
void func_801ed9b4(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.unk3500[5];

    if (DAT_801cba08.unk1448[r->unk09] != 0
        && !(DAT_801cba08.unk1440 >> 20 & 1) && vis_table_a.unk6168 == 1) {
        DAT_801cba08.unk1448[r->unk09] = 2;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}
