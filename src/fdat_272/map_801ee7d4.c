/*
 * FDAT_272.BIN: the scene's second part (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ef934, 0x4, ".bss")
int fdat_801ef934;

/* Entries 51 and 50 play animations 3 and 0 to the end (51 once 50 is half
 * through), table B entry 19 steps from state 11 to 12, and the view
 * pitches up 16 a frame for 16 frames. */
ADDRESS(0x801ee7d4, 0x174)
void func_801ee7d4(void)
{
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis88 *d19 = &vis_table_b.entries[19];
    Vis212 *e50;
    int n;

    fdat_801eb7b4(e51, 3);
    e50 = e51 - 1;
    fdat_801eb7b4(e50, 0);
    e51->unk2A = 0;
    e50->unk2A = 0;
    do {
        fdat_801eba4c(e50);
        fdat_801eba4c(e51);
        e50->unk2A += e50->unkB0;
        if ((short)e50->unk2A > 0x800)
            e51->unk2A += e51->unkB0;
        if ((short)e51->unk2A >= 0x1000)
            e51->unk2A = 0xfff;
        if ((short)e50->unk2A >= 0x1000)
            e50->unk2A = 0xfff;
        func_80026dd4(NULL, NULL, 0);
    } while ((short)e51->unk2A != 0xfff);
    if (d19->unk0C == 11)
        d19->unk0C = 12;
    n = 16;
    while (n--) {
        viewer.unk278.vx += 16;
        viewer.unk268 = viewer.unk278.vx + viewer.unk280 + viewer.unk288 + viewer.unk290;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
}
