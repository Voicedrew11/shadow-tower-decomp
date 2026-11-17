/*
 * FDAT_252.BIN: its second handler (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

/* Until the scene ran, the viewer standing on record 0x33 or 0x36 sends
 * table A entry 32 off (record 8 loses its range bits); otherwise the entry
 * keeps state 0x80. */
ADDRESS(0x801ed4ec, 0xc8)
void func_801ed4ec(void)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[8];

    if (fdat_801ed8cc == 0) {
        if (viewer.unk178 == 0x33 || viewer.unk178 == 0x36) {
            viewer.unk218 = 5;
            fdat_801ed388(e, 0, 0);
            e->unk64 = 1;
            fdat_801ed8cc = 1;
            viewer.unk218 = 0;
            e->unk70 &= ~0x2000000;
            r->unk1C &= ~0x180;
        } else {
            e->unk44 = 0x80;
        }
    }
}
