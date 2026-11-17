/*
 * FDAT_272.BIN: entry 32's exit (WIP module; a unit of its own until its neighbours in
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

/* Entry 32 plays animation 9 (the ground shakes through its middle part
 * and it starts moving near the end), then walks off along x to 0x3f000. */
ADDRESS(0x801ef07c, 0x144)
void func_801ef07c(void)
{
    Vis212 *e = &vis_table_a.entries[32];

    fdat_801eb7b4(e, 9);
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        if (e->unk2A > 0xa00 && e->unk2A < 0xdfc)
            fdat_801eb814(0x400, 0x80);
        if ((short)e->unk2A > 0xdfc)
            e->unk74.vx += 0x40;
        func_80026dd4(NULL, NULL, 0);
    } while ((short)e->unk2A != 0xfff);
    fdat_801eb7b4(e, 1);
    do {
        fdat_801eba4c(e);
        e->unk2A += e->unkB0;
        if ((short)e->unk2A > 0x1000)
            e->unk2A = 0;
        e->unk74.vx += 0x40;
        func_80026dd4(NULL, NULL, 0);
    } while (e->unk74.vx <= 0x3f000);
}
