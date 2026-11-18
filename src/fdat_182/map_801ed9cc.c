/*
 * FDAT_182.BIN: the gate scene (WIP module; a unit of its own until its neighbours in
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

#include <st/gfx.h>
#include <st/sound.h>
#include <st/pad.h>
#include <st/bank.h>
#include <st/cd.h>

/* 1 while the area 3 exit scene is armed, 0xff once played; defined here
 * only because each map unit places one COMMON word (src/fdat_182/
 * main.c uses it). */
DATA(0x801edc54, 0x4, ".bss")
int fdat_801edc54;

void func_800500ac(int arg0, int arg1, int arg2, int arg3);
void func_8004e324(void);
void func_80042c10(void);

/*
 * The gate scene of func_801ed604's event: fades out, puts the viewer at
 * gate 0 or 1 facing in, fades in; at gate 0 table A entry 32 wakes
 * (loading its model if needed) and, unless record 12's stream counter has
 * started, is handed to its exit; otherwise it is set to run.
 */
ADDRESS(0x801ed9cc, 0x284)
void func_801ed9cc(int gate)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[12];
    int index;
    int slot;

    func_80057c54();
    func_800500ac(2, 0, 0x1000, 0x100);
    func_8004e2d8();
    if (gate == 0) {
        viewer.position.vx = 0x50800;
        viewer.position.vy = 0x425c0;
        viewer.position.vz = 0x3bc00;
        viewer.unk278.vy = 0x800;
    } else {
        viewer.position.vx = 0x41800;
        viewer.position.vy = 0x401c0;
        viewer.position.vz = 0x37000;
        viewer.unk278.vy = 0;
    }
    viewer.unk218 = 0;
    viewer.unk28C = 0;
    viewer.unk28A = 0;
    viewer.unk288 = 0;
    viewer.unk278.vx = 0;
    *(SVECTOR *)&viewer.unk268 = viewer.unk278;
    viewer.unk218 = 5;
    func_800500ac(2, 0x1000, 0x1000, 0);
    func_8004e324();
    if (gate == 0 && e->unk05 != 3 && e->unk05 != 1) {
        index = e->unk02;
        slot = e->unk0B + 128;
        fdat_801ed410(e);
        if (world_state.unk834C[slot] == NULL)
            func_800237a8(2, index * 5, slot, e->unk0C & 0xf);
        e->unk36 = -1;
        e->unk1C.bytes[2] &= 0xf2;
        fdat_801eb7b4(e, 9);
        vis_table_a.unk6168 = 3;
    }
    func_80015ad4();
    func_80057c54();
    func_800500ac(2, 0x1000, 0, -0x100);
    func_80057a3c();
    if (gate == 0 && DAT_801cba08.unk1448[r->unk09] == 0) {
        fdat_801ed388(e, 0, 0);
        e->unk70 &= ~0x2000000;
    } else {
        e->unk44 = 0x80;
    }
    vis_table_a.unk6138 = e;
    vis_table_a.unk6134 = r;
    fdat_801ed4a0();
    func_80042c10();
    fdat_801edc50 = 0;
}
