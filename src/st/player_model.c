/*
 * ST.EXE player equipment models and effect placement (WIP module; part of
 * src/st/player.c's translation unit). func_8003f910 after it stays
 * unclaimed: it is not exact and its jump table (0x80011d3c) would diverge.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <kf/lib/null.h>
#include <RAND.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/anim.h>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/player.h>
#include <st/scratch.h>
#include <st/vis.h>

RODATA(0x80011cb4, 0x20)

/* Loads equipment `id`'s model and textures into model slot `slot` (0xffff:
 * empty); a two-handed model (31..34) also fills the paired slot. */
ADDRESS(0x8003f12c, 0x3c0)
void func_8003f12c(u16 id, u8 slot, u8 reload)
{
    DVECTOR pix[4] = {{0, 0}, {64, 0}, {0, 128}, {64, 128}};
    DVECTOR clut[4] = {{0, 0}, {0, 2}, {0, 4}, {0, 6}};
    ScratchPad *sp;
    Vis44 *item;
    u8 model;
    u8 cur;
    u8 paired;
    u8 other;
    u8 pair;

    paired = 0;
    other = 0;
    sp = SCRATCHPAD;
    if (id != 0xffff) {
        item = &vis_table_c.unk0118[id];
        model = item->unk4;
    } else {
        model = 255;
    }
    viewer.unk187 = 15;
    viewer.unk17E = 0;
    viewer.unk180 = 0;
    if (model != 255 && (cur = viewer.unk148[slot]) == model && reload) {
        func_80016030(1, cur * 4 + 485, DAT_80088c88);
        func_8003ec5c(DAT_80088c88, &pix[slot], &clut[slot], 0);
        DrawSync(0);
    }
    viewer.unk148[slot] = model;
    if (model >= 31 && model <= 34) {
        paired = 1;
        other = model + 29;
    }
    if (model != 255) {
        viewer.unkE8[slot] = &DAT_801b5220[model];
        if (paired) {
            pair = slot ^ 1;
            viewer.unkE8[pair] = &DAT_801b5220[other];
            func_80016030(1, model * 4 + 484, viewer.unkF8[slot]);
            func_80016030(1, other * 4 + 484, viewer.unkF8[pair]);
            anim_register(slot + 32, viewer.unkF8[slot]);
            func_800177a0((Model *)sp->model, &pix[slot], &clut[slot], 0);
            anim_register(pair + 32, viewer.unkF8[pair]);
        } else {
            func_80016030(1, model * 4 + 484, viewer.unkF8[slot]);
            anim_register(slot + 32, viewer.unkF8[slot]);
            func_800177a0((Model *)sp->model, &pix[slot], &clut[slot], 1);
        }
        func_80016030(1, model * 4 + 485, DAT_80088c88);
        func_8003ec5c(DAT_80088c88, &pix[slot], &clut[slot], 0);
        DrawSync(0);
        func_80058a0c(1, model * 4 + 486, viewer.unk1D4[slot] - 22);
    }
    if (slot < 2) {
        viewer.unk130[slot] = -1;
    }
    viewer.unk108[slot] = 0;
    viewer.unk14E = 0;
    viewer.unk14F = 0;
    viewer.unk2DE = 0;
    viewer.unk2E0 = 0;
    viewer.unk2E2 = 0;
    viewer.unk189 = 0;
    func_80015ad4();
}

/* Starts model slot `slot`'s action animation when it is idle. */
ADDRESS(0x8003f4ec, 0x184)
void func_8003f4ec(u8 value, u8 slot)
{
    if (slot == 0) {
        if (viewer.unk130[0] != -1) {
            return;
        }
        if (viewer.unk130[1] >= 2048 && !viewer.unk14F) {
            return;
        }
        if (viewer.unk148[0] == 255) {
            return;
        }
        if (viewer.unk190[1][0] != 0) {
            return;
        }
        viewer.unk150[0] = value;
        viewer.unk130[0] = 0;
        viewer.unk154 = 1;
        DAT_801cd180.unk4 &= ~DAT_801cd180.unk32;
        viewer.unk130[2] = viewer.unkE8[0]->unk6;
        viewer.unk138[0] = viewer.unkE8[0]->unk8;
        viewer.unk187 = 15;
    } else {
        if (viewer.unk130[slot] != -1 || viewer.unk130[0] != -1) {
            return;
        }
        if (viewer.unk13C[slot] != -1 || viewer.unk13C[0] != -1) {
            return;
        }
        if (viewer.unk148[slot] == 255) {
            return;
        }
        if (viewer.unk190[1][0] != 0) {
            return;
        }
        DAT_801cd180.unk4 &= ~DAT_801cd180.unk34;
        viewer.unk150[slot] = value;
        viewer.unk130[slot] = 0;
        viewer.unk154 = 1;
        viewer.unk130[slot + 2] = viewer.unkE8[slot]->unk6;
        viewer.unk138[slot] = viewer.unkE8[slot]->unk8;
    }
}

/* Starts item slot `slot`'s use when both model slots are idle. */
ADDRESS(0x8003f670, 0x94)
void func_8003f670(u8 unused, u8 slot)
{
    Viewer *v;

    v = &viewer;
    if (*(s32 *)v->unk13C == -1 && *(s32 *)v->unk130 == -1 && v->unk14C[slot] != 255 &&
        !(v->unk2C & 0x10) && (!v->unk1CB || v->unk190[12][0] != 0)) {
        v->unk13C[slot] = 0;
        v->unk184 = v->unk182;
        v->unk182 = 0;
        v->unk188 = 30;
    }
}

/*
 * Places an equipment effect: `pos` gets slot `slot`'s offset turned by the
 * viewer's matrix, and `dir` a `length`-long direction along `rot` (or the
 * viewer's facing when `rot` is null).
 */
ADDRESS(0x8003f704, 0x20c)
int func_8003f704(int length, VECTOR *pos, SVECTOR *dir, SVECTOR *rot, int slot)
{
    if (pos) {
        gte_SetRotMatrix(&viewer.unk298);
        gte_ldv0(&viewer.unk160[slot]);
        gte_rtv0();
        gte_stlvnl(pos);
        pos->vx += viewer.x;
        pos->vy += viewer.y;
        pos->vz += viewer.z;
    }
    if (rot) {
        if (dir) {
            MATRIX m;
            SVECTOR v;

            memset(&v, 0, sizeof(v));
            v.vz = 4096;
            RotMatrixZYX(rot, &m);
            gte_SetRotMatrix(&m);
            gte_ldv0(&v);
            gte_rtv0();
            gte_stsv(dir);
            func_8004a0f8(length, dir);
        }
    } else if (dir) {
        SVECTOR v;

        memset(&v, 0, sizeof(v));
        v.vz = 4096;
        gte_SetRotMatrix(&viewer.unk298);
        gte_ldv0(&v);
        gte_rtv0();
        gte_stsv(dir);
        func_8004a0f8(length, dir);
    }
    return 0;
}
