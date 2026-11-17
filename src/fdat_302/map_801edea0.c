/*
 * FDAT_302.BIN: the hall scene and the word fdat_801ee47c (WIP module; a unit of its own until its neighbours in
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

/* Table A entry 5's walk: 0, 1 walking, 0xff gone; defined here only
 * because each map unit places one COMMON word. */
DATA(0x801ee47c, 0x4, ".bss")
int fdat_801ee47c;

void func_800500ac(int arg0, int arg1, int arg2, int arg3);
void func_8004e324(void);
void func_80042c10(void);

/* Not exact (one attempt, retries off; 93.9%): first divergence retail `lbu a0,11(s0)`, this source `lbu v0,11(s0)`. */
/* The hall scene: fades out, puts the viewer at the hall's start, fades
 * in with table A entry 0 awake (loading its model if needed); unless
 * record 0's stream counter has started it is handed to its exit. */
ADDRESS(0x801edea0, 0x1f0)
void func_801edea0(void)
{
    Vis212 *e = vis_table_a.entries;
    int index = e->unk02;
    int slot = e->unk0B + 128;
    Vis192 *r = &vis_table_a.unk3500[index];

    func_80057c54();
    func_800500ac(2, 0, 0x1000, 0x100);
    func_8004e2d8();
    viewer.y = 0x3f9c0;
    viewer.x = 0x45800;
    viewer.z = 0x40000;
    viewer.unk278.vy = 0;
    viewer.unk278.vx = 0;
    viewer.unk218 = 0;
    viewer.unk28C = 0;
    viewer.unk28A = 0;
    viewer.unk288 = 0;
    *(SVECTOR *)&viewer.unk268 = viewer.unk278;
    viewer.unk218 = 5;
    func_800500ac(2, 0x1000, 0x1000, 0);
    func_8004e324();
    fdat_801ed410(e);
    if (world_state.unk834C[slot] == NULL)
        func_800237a8(2, index * 5, slot, e->unk0C & 0xf);
    e->unk36 = -1;
    e->unk1C.bytes[2] &= 0xf2;
    fdat_801eb7b4(e, 7);
    vis_table_a.unk6168 = 0;
    func_80015ad4();
    func_80057c54();
    func_800500ac(2, 0x1000, 0, -0x100);
    func_80057a3c();
    if (DAT_801cba08.unk1448[r->unk09] == 0) {
        fdat_801ed388(e, 0, 0);
        e->unk70 &= ~0x2000000;
    }
    vis_table_a.unk6138 = e;
    vis_table_a.unk6134 = r;
    fdat_801ed4a0();
    func_80042c10();
}
