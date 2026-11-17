/*
 * FDAT_372.BIN: its second handler (WIP module; a unit of its own until its neighbours in
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

/* src/fdat_372/map_801ed4a0.c (a static of FDAT_132.BIN has the name) */
extern int fdat_801eda3c;

/* Until record 5's stream counter starts, the viewer passing z 0x4c000
 * sends table A entry 32; beaten, it changes the lighting of four areas
 * (flag 20) and leaves. */
ADDRESS(0x801ed510, 0x178)
void func_801ed510(void)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[5];

    if (DAT_801cba08.unk1448[r->unk09] == 0 && viewer.z > 0x4c000) {
        viewer.unk218 = 5;
        fdat_801ed388(e, 0, 0);
        e->unk64 = 1;
        viewer.unk218 = 0;
        e->unk70 &= ~0x2000000;
        r->unk1C &= ~0x180;
    }
    if (fdat_801eda3c == 0 && e->unk2C == 0) {
        fdat_801eda3c = 1;
        lighting.area_mode[10] = 0x11;
        lighting.area_mode[21] = 0x11;
        lighting.area_mode[32] = 0x11;
        lighting.area_mode[37] = 0x11;
        DAT_801cba08.unk1440.word |= 0x100000;
    }
    if (fdat_801eda3c == 1 && e->unk05 == 3) {
        viewer.unk218 = 5;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 0);
        fdat_801eda3c = 0xff;
        viewer.unk218 = 0;
    }
}
