/*
 * FDAT_382.BIN: its first two handlers and the scene word fdat_801edb04 (WIP module; a unit of its own until its neighbours in
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
#include <st/fdat_382.h>

/* Table A entry 32 beaten: 0, 1 this visit, 0xff gone (flag 24). */
DATA(0x801edb04, 0x4, ".bss")
int fdat_801edb04;

/* Not exact (one attempt, retries off; 72.7%): first divergence retail `lui a2,0x0`, this source `lui v0,0x0`. */
/* Sets up the area: with flags 21..23 all set entries 0..2 and 32 stand
 * (scene word 1), else they wait; flag 24 and record 1's stream counter
 * set the other words. */
ADDRESS(0x801ed4a0, 0xe8)
void func_801ed4a0(void)
{
    Vis212 *e32 = &vis_table_a.entries[32];
    Vis192 *r1 = &vis_table_a.unk3500[1];

    if ((DAT_801cba08.unk1440.word & 0xe00000) == 0xe00000) {
        vis_table_a.entries[32].unk00 = 100;
        vis_table_a.entries[2].unk00 = 100;
        vis_table_a.entries[1].unk00 = 100;
        vis_table_a.entries[0].unk00 = 100;
        e32->unk1C.bytes[3] = 1;
        fdat_801edafc = 1;
        e32->unk70 &= ~0x2000000;
    } else {
        vis_table_a.entries[32].unk00 = 0;
        vis_table_a.entries[2].unk00 = 0;
        vis_table_a.entries[1].unk00 = 0;
        vis_table_a.entries[0].unk00 = 0;
        fdat_801edafc = 0;
    }
    if (!DAT_801cba08.unk1440.bit.f24)
        fdat_801edb04 = 0;
    else
        fdat_801edb04 = 0xff;
    if (DAT_801cba08.unk1448[r1->unk09] >= 2)
        fdat_801edb00 = 0xff;
    else
        fdat_801edb00 = 0;
}

/* Not exact (one attempt, retries off; 87.0%): first divergence retail `bnez v0,<func_801ed588+0xd8>`, this source `bnez v0,<func_801ed588+0xd4>`. */
/* Runs the area: the viewer reaching z 0x3a800 wakes entry 32 (or sends
 * it, with entries 0 and 32 out of range); beaten, it changes the lighting
 * of four areas (flag 24) and leaves. */
ADDRESS(0x801ed588, 0x1c4)
void func_801ed588(void)
{
    Vis212 *e0 = vis_table_a.entries;
    Vis212 *e32 = &vis_table_a.entries[32];
    Vis192 *r1 = &vis_table_a.unk3500[1];

    if (fdat_801edb00 == 0 && viewer.position.vz < 0x3a800) {
        if (fdat_801edafc == 0) {
            DAT_801cba08.unk1448[r1->unk09] = 0;
            fdat_801ed388(e32, 1, 0);
        } else {
            DAT_801cba08.unk1448[r1->unk09] = 1;
            fdat_801ed388(e32, 0, 0);
            e32->unk1C.bytes[3] = 0;
            e0->unk1C.bytes[3] = 0;
        }
        e32->unk70 &= ~0x2000000;
        fdat_801edb00 = 0xff;
    }
    if (fdat_801edb04 == 0 && e32->unk2C == 0) {
        fdat_801edb04 = 1;
        lighting.area_mode[9] = 0x11;
        lighting.area_mode[8] = 0x11;
        lighting.area_mode[13] = 0x11;
        lighting.area_mode[38] = 0x11;
        DAT_801cba08.unk1440.word |= 0x1000000;
    }
    if (fdat_801edb04 == 1 && e32->unk05 == 3) {
        viewer.unk218 = 5;
        e32->unk70 |= 0x2000000;
        DAT_801cba08.unk1448[r1->unk09] = 2;
        fdat_801ed388(e32, 1, 0);
        fdat_801edb04 = 0xff;
        viewer.unk218 = 0;
    }
}
