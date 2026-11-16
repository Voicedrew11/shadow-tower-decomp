/*
 * ST.EXE: swapping the items between the hands, the per-frame item and
 * magic requests, and small viewer state setters (WIP module inside the
 * translation unit that also holds 0x8003cb88..0x80042d74).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/player.h>

SDATA(0x80088040, 0x10)

/*
 * Swaps hand `a`'s item with slot `b`, twice for a two-handed item.
 * Not exact: the per-slot base registers are assigned in another order
 * (retail loads x[b] before x[a] in each swap).
 */
ADDRESS(0x80042620, 0x2e0)
void func_80042620(u8 a, u8 b)
{
    SVECTOR unused0 = {0, 128, 0, -128};
    SVECTOR unused1 = {0, 4, 0, -4};
    int n;
    u16 id;
    void *p;
    u8 c;

    n = 1;
    if ((viewer.unk38[3] != 0xffff && vis_table_c.unk0118[viewer.unk38[3]].unk26 == 9) ||
        (viewer.unk38[4] != 0xffff && vis_table_c.unk0118[viewer.unk38[4]].unk26 == 9))
        n = 2;
    do {
        if (a == 0) {
            id = viewer.unk38[3];
            viewer.unk38[3] = viewer.unk38[4];
            viewer.unk38[4] = id;
        } else {
            id = viewer.unk38[16];
            viewer.unk38[16] = viewer.unk38[17];
            viewer.unk38[17] = id;
        }
        p = viewer.unkE8[a];
        viewer.unkE8[a] = viewer.unkE8[b];
        viewer.unkE8[b] = p;
        p = viewer.unkF8[a];
        viewer.unkF8[a] = viewer.unkF8[b];
        viewer.unkF8[b] = p;
        c = viewer.unk148[a];
        viewer.unk148[a] = viewer.unk148[b];
        viewer.unk148[b] = c;
        if (viewer.unk148[a] != 0xff)
            anim_register(a + 32, viewer.unkF8[a]);
        if (viewer.unk148[b] != 0xff)
            anim_register(b + 32, viewer.unkF8[b]);
        if (viewer.unk108[a])
            ((StAnimSlot *)viewer.unk108[a])->unk2 = 0xffff;
        if (viewer.unk108[b])
            ((StAnimSlot *)viewer.unk108[b])->unk2 = 0xffff;
        c = viewer.unk1D4[a];
        viewer.unk1D4[a] = viewer.unk1D4[b];
        viewer.unk1D4[b] = c;
        if (a == 0) {
            viewer.unk17E = 0;
            a = 1;
            b = 3;
        } else {
            a = 0;
            b = 2;
        }
    } while (--n);
    if (viewer.unk148[0] >= 31 && viewer.unk148[0] < 35)
        anim_register(33, viewer.unkF8[1]);
    if (viewer.unk148[2] >= 31 && viewer.unk148[2] < 35)
        anim_register(35, viewer.unkF8[3]);
    viewer.unk108[0] = 0;
    viewer.unk108[1] = 0;
    viewer.unk108[2] = 0;
    viewer.unk108[3] = 0;
    func_8003cedc();
    viewer.unk18A = 0xffff;
}

/*
 * Consumes the frame's item request, then steps the queued magic.
 * Not exact: retail tests both hands' unk130 halfwords as one word
 * (lw 0x130 against -1) where this reads unk130[0] with lh.
 */
ADDRESS(0x80042900, 0x29c)
void func_80042900(void)
{
    if (DAT_801cd180.unk14 == 2) {
        DAT_801cd180.unk14 = 0;
        if (viewer.unk130[0] == -1 && viewer.unk38[4] != 0xffff) {
            func_80042620(0, 2);
            func_8003d030();
        }
    }
    if (DAT_801cd180.unk14 == 3) {
        DAT_801cd180.unk14 = 0;
        if (viewer.unk130[0] == -1 && viewer.unk38[17] != 0xffff) {
            func_80042620(1, 3);
            func_8003d030();
        }
    }
    if (DAT_801cd180.unk14 == 4) {
        DAT_801cd180.unk14 = 0;
        if (viewer.unk182 >= 0x1400)
            func_8003f670(0, 0);
    }
    if (DAT_801cd180.unk14 == 5) {
        DAT_801cd180.unk14 = 0;
        if (viewer.unk182 >= 0x1400)
            func_8003f670(0, 1);
    }
    if (viewer.unk155[0] != 0xff && --viewer.unk15B[0] == 0) {
        func_8003f910(viewer.unk155[0], 0);
        if (--viewer.unk157[0] == 0)
            viewer.unk155[0] = 0xff;
        else
            viewer.unk15B[0] = viewer.unk159[0];
    }
    if (viewer.unk155[1] != 0xff && --viewer.unk15B[1] == 0) {
        func_8003f910(viewer.unk155[1], 1);
        if (--viewer.unk157[1] == 0)
            viewer.unk155[1] = 0xff;
        else
            viewer.unk15B[1] = viewer.unk159[1];
    }
    if (viewer.unk218 != 1) {
        if (viewer.unk187 == 0 && (DAT_801cd180.unk4 & DAT_801cd180.unk32))
            func_8003f4ec(0, 0);
        if (DAT_801cd180.unk4 & DAT_801cd180.unk34)
            func_8003f4ec(0, 1);
    }
}

ADDRESS(0x80042b9c, 0x74)
void func_80042b9c(void)
{
    u8 mode = viewer.unk218;

    if (mode != 4 && (DAT_801cd180.unk4 & DAT_801cd180.unk36) && !DAT_801cd180.unk12) {
        DAT_801cd180.unk4 &= ~DAT_801cd180.unk36;
        if (mode == 0)
            func_80055374(&viewer.x, &viewer.unk278);
    }
}

ADDRESS(0x80042c10, 0x34)
void func_80042c10(void)
{
    viewer.unk218 = 0;
    viewer.unk280 = viewer.unk282 = viewer.unk284 = 0;
    viewer.unk288 = viewer.unk28A = viewer.unk28C = 0;
    viewer.unk290 = viewer.unk292 = viewer.unk294 = 0;
}

DATA(0x80088050, 0x4, ".sdata")
static s32 DAT_80088050 = 0;

ADDRESS(0x80042c44, 0xa8)
void func_80042c44(SVECTOR *arg0, SVECTOR *arg1, s16 arg2, int arg3)
{
    viewer.unk218 = 16;
    viewer.unk2D6 = 3500;
    viewer.unk2D8 = arg2;
    if (viewer.unk0 < arg3 * 10)
        viewer.unk14E = 1;
    if (viewer.unk38[16] != 0xffff && viewer.unk189 && DAT_80088050 != viewer.unk23C) {
        func_800582f0(viewer.unk1D4[1], 128);
        DAT_80088050 = viewer.unk23C;
    }
}

ADDRESS(0x80042cec, 0x10)
void func_80042cec(void)
{
    viewer.unk218 = 2;
}

/* Frees the animation slots and objects with two state bytes held. */
ADDRESS(0x80042cfc, 0x78)
void func_80042cfc(void)
{
    u8 *flags = (u8 *)&DAT_801cd120;
    u8 a = flags[3];
    u8 b = flags[5];

    flags[3] = 0;
    func_80026afc(0, 0);
    anim_slots_free_idle();
    anim_sets_free_idle();
    anim_slots_free_all();
    anim_sets_free_all();
    flags[3] = a;
    flags[5] = b;
}
