/*
 * FDAT_262.BIN: its per-entry handler, entry 48's scene and the word fdat_801ee370 (WIP module; a unit of its own until its neighbours in
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

/* Entry 48: 0 asleep, 0xfd hit, 0xfe awake, 0xff gone (flags 29, 30). */
DATA(0x801ee370, 0x4, ".bss")
int fdat_801ee370;

void func_801edc18(void);

/* The current entry's handler: the animation steps as fdat_801ed6e4 does,
 * kind 1 replaying animation 10 while entry 48 sleeps. */
ADDRESS(0x801edec4, 0xf0)
void func_801edec4(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 1 && (fdat_801ee370 == 0 || fdat_801ee370 == 0xfd)) {
            fdat_801eb7b4(e, 10);
        } else {
            e->unk1A = 0xff;
            func_801edc18();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

/*
 * Entry 48 plays animation 15 and lifts its arm a frame at a time; table C
 * item 5 rides on vertex 0x160 until the arm is up. Then the view settles
 * and the item drops.
 */
ADDRESS(0x801edfb4, 0x18c)
void func_801edfb4(void)
{
    Vis212 *e = &vis_table_a.entries[48];
    Vis48 *c = &vis_table_c.entries[5];
    SVECTOR v;
    SVECTOR unused;

    fdat_801eb7b4(e, 15);
    do {
        e->unk2A += e->unkB0;
        if ((s16)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        anim_get_vertex(e->unk0B + 128, e->unk08, (s16)e->unk2A, 0x160, &v);
        c->unk08.vx = e->unk74.vx - v.vz;
        c->unk08.vy = e->unk74.vy + v.vy;
        c->unk08.vz = e->unk74.vz + v.vx;
        func_80026dd4(NULL, &viewer.unk278, 0);
    } while ((s16)e->unk2A != 0xfff);
    c->unk22 = 2;
    viewer.unk268 = viewer.unk278.vx + viewer.unk280 + viewer.unk288 + viewer.unk290;
    viewer.unk26A = viewer.unk278.vy + viewer.unk282 + viewer.unk28A + viewer.unk292;
    viewer.unk26C = viewer.unk278.vz + viewer.unk284 + viewer.unk28C + viewer.unk294;
    viewer.unk270.vx = 4096 - viewer.unk268;
    viewer.unk270.vy = 4096 - viewer.unk26A;
    viewer.unk270.vz = 4096 - viewer.unk26C;
    RotMatrixZYX(&viewer.unk270, &viewer.unk298);
    func_8005335c(c, 1);
}
