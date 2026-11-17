/*
 * ST.EXE player suffix, 0x8003e5e8..0x800443ec: ranged hits, camera,
 * equipment models, held items, movement and the per-frame update.
 * Split after player_hit to keep its nonexact extent from moving these
 * claims. Anonymous small-data literals and their two private statics
 * remain together under the -G8 profile.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
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
#include <st/effect.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/hud.h>
#include <st/light.h>
#include <st/menu.h>
#include <st/player.h>
#include <st/scratch.h>
#include <st/sound.h>
#include <st/vis.h>
#include <st/card.h>
#include <st/item.h>
#include <st/menu_main.h>

RODATA(0x80011cb4, 0x48)
JUMPTABLES(0x80011d3c, 0x1dc)
SDATA(0x8008802c, 0x10)
SDATA(0x80088040, 0x10)
SDATA(0x80088054, 0x10)

/* Hits the viewer with `damage` when it is within `range` of `pos`, scaled
 * down with the distance. */
ADDRESS(0x8003e5e8, 0x14c)
void func_8003e5e8(u16 *damage, u16 effect, long *pos, int min, int range, int kind)
{
    int d;

    if (kind == 0x8000) {
        d = func_8004a67c((VECTOR *)pos, range, (VECTOR *)&viewer.position.vx, 256, 1000);
    } else if (kind == 0x8001) {
        if (viewer.position.vy > pos[1]) {
            d = -9999999;
        } else {
            d = func_8004a67c((VECTOR *)pos, range, (VECTOR *)&viewer.position.vx, 256, 1000);
        }
    } else {
        d = func_8003ebfc(pos[0], pos[1], pos[2], range, kind);
        if (d == -1) {
            d = -9999999;
        }
    }
    if (d >= min) {
        player_hit(damage, 4096 - (d << 12) / range, effect, pos);
    }
}

/* The camera position and angles. */
ADDRESS(0x8003e734, 0x60)
void func_8003e734(VECTOR *pos, SVECTOR *rot)
{
    pos->vx = viewer.position.vx;
    pos->vz = viewer.position.vz;
    pos->vy = viewer.position.vy + viewer.unk2CE + viewer.unk2D2 - 1600;
    rot->vx = viewer.unk268;
    rot->vy = viewer.unk26A;
    rot->vz = viewer.unk26C;
}

/* Clears the status timers and restores the hit and magic points. */
ADDRESS(0x8003e794, 0xac)
void func_8003e794(void)
{
    viewer.unk190[0][0] = viewer.unk190[0][1] = viewer.unk190[1][0] = viewer.unk190[1][1] =
        viewer.unk190[2][0] = viewer.unk190[2][1] = viewer.unk190[3][0] = viewer.unk190[3][1] =
            viewer.unk190[4][0] = viewer.unk190[4][1] = viewer.unk190[5][0] =
                viewer.unk190[5][1] = viewer.unk190[6][0] = viewer.unk190[6][1] =
                    viewer.unk190[7][0] = viewer.unk190[7][1] = viewer.unk190[8][0] =
                        viewer.unk190[8][1] = viewer.unk190[9][0] = viewer.unk190[9][1] =
                            viewer.unk190[10][0] = viewer.unk190[10][1] =
                                viewer.unk190[11][0] = viewer.unk190[11][1] =
                                    viewer.unk190[12][0] = viewer.unk190[12][1] = 0;
    viewer.unk238 = 0;
    viewer.unk2 = viewer.unk18A = viewer.unk0;
    viewer.unk6 = viewer.unk18C = viewer.unk4;
    viewer.unk2C = 0;
    func_8003c054();
}

ADDRESS(0x8003e840, 0x34)
void func_8003e840(void)
{
    int i;

    for (i = 0; i < 5; i++) {
        viewer.unk1EC[i] = 0;
        viewer.unk20C[i] = 0;
    }
}

/* Resets the player for a new game. */
ADDRESS(0x8003e874, 0x1ac)
void func_8003e874(void)
{
    func_8003cae4();
    viewer.unk244 = 2048;
    viewer.unk34 = 0;
    viewer.unk2D6 = viewer.unk2D8 = viewer.unk2DA = viewer.unk2DC = 0;
    viewer.unk186 = 2;
    viewer.unk15E = 0;
    viewer.unk1E4 = 0;
    viewer.unk21E = 0;
    viewer.unk219 = viewer.unk21A = viewer.unk21B = 0;
    viewer.unk118[0] = viewer.unk118[1] = 0;
    func_8003ee38(0xffff, 0);
    func_8003ee38(0xffff, 1);
    viewer.unkE8[0] = viewer.unkE8[1] = viewer.unkE8[2] = viewer.unkE8[3] = 0;
    func_8003f12c(0xffff, 0, 0);
    func_8003f12c(0xffff, 1, 0);
    func_8003f12c(0xffff, 2, 0);
    func_8003f12c(0xffff, 3, 0);
    viewer.unk1D4[0] = 24;
    viewer.unk1D4[1] = 25;
    viewer.unk1D4[2] = 26;
    viewer.unk1D4[3] = 27;
    viewer.unk38[3] = 0;
    func_8003f12c(0, 0, 0);
    func_80055990();
    viewer.unk28 = 0;
    viewer.unk33 = 0;
    viewer.unk34 = 0;
    viewer.unk30 = 0;
    func_8003cedc();
    viewer.unk278.vx = 0;
    viewer.unk278.vy = 3072;
    viewer.unk278.vz = 0;
    viewer.position.vx = 0x3d000;
    viewer.position.vy = 0x405c0;
    viewer.position.vz = 0x3f000;
    viewer.unk1CC = viewer.unk1CE = viewer.unk1D0 = 0;
    func_8003e794();
    func_800595d4();
    viewer.unk22D = 61;
    viewer.unk22E = 64;
    viewer.unk22F = 63;
    viewer.unk22C = 0;
    viewer.unk230 = 0;
    viewer.unk232 = 1472;
    viewer.unk234 = 0;
    viewer.unk236 = 3072;
    func_8003e840();
}

ADDRESS(0x8003ea20, 0xb0)
void func_8003ea20(void)
{
    viewer.unk120[0] = DAT_80190f28[0];
    viewer.unk120[1] = DAT_80190f28[1];
    viewer.unkF8[0] = DAT_80199220[0];
    viewer.unkF8[1] = DAT_80199220[1];
    viewer.unkF8[2] = DAT_80199220[2];
    viewer.unkF8[3] = DAT_80199220[3];
    func_8003e874();
    DAT_801cd120.unk1 = 63;
    DAT_801cd120.unk0 = 63;
    DAT_801cd120.unk3 = 1;
    DAT_801cd120.unk2 = 1;
    DAT_801cd120.unk4 = 1;
    DAT_801cd120.unk5 = 1;
    DAT_801cd120.unk6 = 5;
    DAT_801cd120.unk7 = 30;
    DAT_801cd120.unk8 = 20;
    viewer.unk21C = 0;
}

ADDRESS(0x8003ead0, 0x28)
void func_8003ead0(void)
{
    viewer.unk2E4 = 0;
    viewer.unk2E8 = 0;
    viewer.unk2CA = 0;
    viewer.unk2CC = 0;
    viewer.unk2C8 = 0;
    viewer.unk21E = 1;
}

ADDRESS(0x8003eaf8, 0x3c)
void func_8003eaf8(void)
{
    viewer.unk23C = 0;
    viewer.unk21C = 1;
    viewer.unk239 = 0;
    viewer.unk2D4 = 0;
    viewer.unk218 = 0;
    func_8003ead0();
}

/* func_8003ebfc's distance to `pos` (height ignored) when the viewer is
 * within `limit` of facing `angle`, else -1. */
ADDRESS(0x8003eb34, 0xc8)
int func_8003eb34(VECTOR *pos, int angle, int range, int limit)
{
    int d;
    short a;

    d = func_8003ebfc(pos->vx, 0xffff, pos->vz, range, 0);
    if (d != -1) {
        a = (func_8004a1b4(viewer.position.vx - pos->vx, viewer.position.vz - pos->vz) - angle) & 0xfff;
        if (a > 2048) {
            a = 4096 - a;
        }
        if (a > limit) {
            d = -1;
        }
    }
    return d;
}

ADDRESS(0x8003ebfc, 0x50)
int func_8003ebfc(int x, int y, int z, int range, int kind)
{
    return func_8004a580((VECTOR *)&viewer.position.vx, x, y, z, range, 1000, kind);
}

ADDRESS(0x8003ec4c, 0x10)
void func_8003ec4c(volatile u8 *p)
{
    p[16] = 1;
}

/*
 * Uploads the TIM images in `data` (until a block without the TIM id): each
 * CLUT block at `clut` and each pixel block at `pix`, offset from the
 * TIM's own position unless `absolute`.
 */
/* Not exact: retail copies each TIM block header word 1, 0, 2 interleaved
 * with the absolute test; this copies them in order. */
ADDRESS(0x8003ec5c, 0x1dc)
void func_8003ec5c(void *data, DVECTOR *pix, DVECTOR *clut, int absolute)
{
    TimBlock block;
    RECT rect;
    DVECTOR zero_clut;
    DVECTOR zero_pix;
    DVECTOR *cp;
    DVECTOR *pp;
    u_long *p;

    memset(&zero_clut, 0, sizeof(zero_clut));
    memset(&zero_pix, 0, sizeof(zero_pix));
    cp = &zero_clut;
    pp = &zero_pix;
    if (clut) {
        cp = clut;
    }
    p = data;
    if (pix) {
        pp = pix;
    }
next:
    if (*p++ == 0x10) {
        if (*p++ & 8) {
            block = *(TimBlock *)p;
            p += 3;
            if (!absolute) {
                rect.x = block.rect.x + cp->vx;
                rect.y = block.rect.y + cp->vy;
            } else {
                rect.x = cp->vx;
                rect.y = cp->vy;
            }
            rect.w = block.rect.w;
            rect.h = block.rect.h;
            LoadImage(&rect, p);
            p = (u_long *)((u8 *)p + (block.len - 12));
        }
        block = *(TimBlock *)p;
        p += 3;
        if (!absolute) {
            rect.x = block.rect.x + pp->vx;
            rect.y = block.rect.y + pp->vy;
        } else {
            rect.x = pp->vx;
            rect.y = pp->vy;
        }
        rect.w = block.rect.w;
        rect.h = block.rect.h;
        LoadImage(&rect, p);
        p = (u_long *)((u8 *)p + (block.len - 12));
        goto next;
    }
}

/* Loads item `id`'s parts and textures into item slot `slot` (0xffff:
 * empty), freeing the slot's old effects. */
/* Not exact: the slot-bit test compiles to srl/andi instead of
 * andi/sltu, and $s5 is saved later. */
ADDRESS(0x8003ee38, 0x2f4)
void func_8003ee38(u16 id, u8 slot)
{
    DVECTOR pix[2] = {{0, 0}, {64, 0}};
    DVECTOR clut[2] = {{0, 0}, {0, 6}};
    u8 item;
    StEffect *rec;
    int i;
    u8 *p;
    u16 n;
    u16 part;
    u_long size;

    item = 255;
    if (id != 0xffff) {
        item = id;
    }
    if (viewer.unk14C[slot] != 255) {
        rec = effects.slots;
        i = 128;
        while (i--) {
            if (rec->unk0 != 255 && ((u8)(rec->unk0 & 0x20) != 0) == slot) {
                if (rec->unk1 < 44) {
                    rec->unk0 = 255;
                }
                if (rec->unk1 >= 170 && rec->unk1 < 180) {
                    rec->unk0 = 255;
                }
            }
            rec++;
        }
    }
    viewer.unk14C[slot] = item;
    if (item != 255) {
        viewer.unk118[slot] = &effects.defs[item];
        func_80016030(1, item * 4 + 740, (u_long *)viewer.unk120[slot]);
        p = viewer.unk120[slot];
        n = *(u16 *)p;
        p += 4;
        part = slot * 8 + 96;
        while (n--) {
            size = *(u_long *)p;
            if (size >= 5) {
                world_state.unk834C[part] = p;
                func_800177a0((Model *)(p + ((u_long *)p)[2]), &pix[slot], &clut[slot], 0);
            }
            p += size;
            part++;
        }
        func_80016030(1, item * 4 + 741, DAT_80088c88);
        func_8003ec5c(DAT_80088c88, &pix[slot], &clut[slot], 0);
        DrawSync(0);
        func_80058a0c(1, item * 4 + 742, slot + 6);
    }
    viewer.unk188 = 30;
    viewer.unk182 = 0;
    viewer.unk184 = 0;
    viewer.unk13C[slot] = -1;
    viewer.unk128[slot] = 0;
    viewer.unk155[slot] = 255;
    func_80015ad4();
}

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
        viewer.unk130.halves[slot] = -1;
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
        if (viewer.unk130.halves[0] != -1) {
            return;
        }
        if (viewer.unk130.halves[1] >= 2048 && !viewer.unk14F) {
            return;
        }
        if (viewer.unk148[0] == 255) {
            return;
        }
        if (viewer.unk190[1][0] != 0) {
            return;
        }
        viewer.unk150[0] = value;
        viewer.unk130.halves[0] = 0;
        viewer.unk154 = 1;
        DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[8];
        viewer.unk130.halves[2] = viewer.unkE8[0]->unk6;
        viewer.unk138[0] = viewer.unkE8[0]->unk8;
        viewer.unk187 = 15;
    } else {
        if (viewer.unk130.halves[slot] != -1 || viewer.unk130.halves[0] != -1) {
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
        DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[9];
        viewer.unk150[slot] = value;
        viewer.unk130.halves[slot] = 0;
        viewer.unk154 = 1;
        viewer.unk130.halves[slot + 2] = viewer.unkE8[slot]->unk6;
        viewer.unk138[slot] = viewer.unkE8[slot]->unk8;
    }
}

/* Starts item slot `slot`'s use when both model slots are idle. */
ADDRESS(0x8003f670, 0x94)
void func_8003f670(u8 unused, u8 slot)
{
    Viewer *v;

    v = &viewer;
    if (*(s32 *)v->unk13C == -1 && v->unk130.word == -1 && v->unk14C[slot] != 255 &&
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
        pos->vx += viewer.position.vx;
        pos->vy += viewer.position.vy;
        pos->vz += viewer.position.vz;
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

/* The scratchpad model matrix (rotation and translation). */
#define PLAYER_MATRIX(sp) ((MATRIX *)&(sp)->part_pos)
/* Fires equipment slot `slot`'s effect of kind `kind`. va_start names
 * `kind`, so kind lives in its home slot: retail reads its low byte back
 * (lbu 128(sp)) for each effect request. */
ADDRESS(0x8003f910, 0x714)
void func_8003f910(int kind, int slot, ...)
{
    VECTOR pos;
    VECTOR pos2;
    SVECTOR rot;
    SVECTOR dir;
    SVECTOR aim;
    ScratchPad *sp;
    int count;
    int speed;
    int length;
    int turn;
    int i;
    int side;
    va_list ap;

    va_start(ap, kind);
    side = slot << 5;
    sp = SCRATCHPAD;
    count = viewer.unk157[slot] - 1;
    switch (kind) {
    case 0:
        speed = 4096;
        goto shoot;
    case 10:
        speed = 2048;
    shoot:
        rot.vx = viewer.unk270.vx;
        rot.vy = viewer.unk270.vy;
        rot.vz = viewer.unk270.vz;
        func_8003f704(700, &pos, &dir, &rot, slot);
        func_80036484(10, side | 0x42, kind, &pos, &dir, speed, &viewer.unk268);
        break;
    case 7:
        i = 2;
        while (i--) {
            length = i * 32 - 16;
            aim.vx = viewer.unk270.vx - 512;
            aim.vy = viewer.unk270.vy + length;
            aim.vz = viewer.unk270.vz;
            func_8003f704(700, &pos, &dir, &aim, slot);
            func_80036484(10, side | 0x42, kind, &pos, &dir, 1024, &viewer.unk268);
            viewer.unk160[slot].vx += 1024;
        }
        break;
    case 1:
        length = 256;
        turn = 0;
        speed = 1024;
        goto spray;
    case 3:
        length = 384;
        turn = count * 32 - 80;
        speed = 512;
        goto spray;
    case 9:
        length = 256;
        turn = 128 - rand() / 128;
        speed = 768;
        goto spray;
    case 6:
        length = 512;
        turn = 0;
        speed = 3072;
        goto spray;
    case 4:
        length = 384;
        turn = 0;
        speed = 8192;
        goto spray;
    case 12:
        length = 384;
        turn = 0;
        speed = 2048;
        goto spray;
    case 13:
        length = 320;
        turn = 0;
        speed = 8192;
        goto spray;
    case 15:
        length = 768;
        turn = 0;
        speed = 1024;
        goto spray;
    case 16:
        length = 320;
        turn = 0;
        speed = 1024;
    spray:
        rot.vx = viewer.unk270.vx;
        rot.vy = viewer.unk270.vy + turn;
        rot.vz = viewer.unk270.vz;
        func_8003f704(length, &pos, &dir, &rot, slot);
        rot.vx = viewer.unk270.vx;
        rot.vy = -viewer.unk270.vy;
        rot.vz = -viewer.unk270.vz;
        func_80036484(10, side | 0x42, kind, &pos, &dir, speed, &rot, count);
        break;
    case 44:
    case 45:
    case 46:
    case 47:
        anim_get_vertex(32, viewer.unk150[0], viewer.unkE8[0]->unk6, viewer.unkE8[0]->unk12, &dir);
        sp->model_t[0] = viewer.unkE8[slot]->unkC;
        sp->model_t[1] = viewer.unkE8[slot]->unkE;
        sp->model_t[2] = viewer.unkE8[slot]->unk10;
        RotMatrix(&viewer.unkE8[slot]->unk14, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos);
        dir.vx = pos.vx;
        dir.vy = pos.vy;
        dir.vz = pos.vz;
        rot = viewer.unk270;
        sp->model_t[0] = viewer.position.vx;
        sp->model_t[1] = viewer.position.vy;
        sp->model_t[2] = viewer.position.vz;
        RotMatrixZYX(&rot, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos);
        anim_get_vertex(32, viewer.unk150[0], viewer.unkE8[0]->unk6, viewer.unkE8[0]->unk14.pad, &dir);
        sp->model_t[0] = viewer.unkE8[slot]->unkC;
        sp->model_t[1] = viewer.unkE8[slot]->unkE;
        sp->model_t[2] = viewer.unkE8[slot]->unk10;
        RotMatrix(&viewer.unkE8[slot]->unk14, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos2);
        dir.vx = pos2.vx;
        dir.vy = pos2.vy;
        dir.vz = pos2.vz;
        rot = viewer.unk270;
        sp->model_t[0] = viewer.position.vx;
        sp->model_t[1] = viewer.position.vy;
        sp->model_t[2] = viewer.position.vz;
        RotMatrixZYX(&rot, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos2);
        func_8004a40c(pos.vx - pos2.vx, pos.vy - pos2.vy, pos.vz - pos2.vz, &rot);
        func_80049fe4(&rot, &dir);
        if (kind == 44) {
            func_8004a0f8(viewer.unk180 * 768 / 5120, &dir);
        } else {
            func_8004a0f8(viewer.unk180 * 1024 / 5120, &dir);
        }
        pos.vx = pos2.vx;
        pos.vy = pos2.vy + viewer.unk2CE + viewer.unk2D2;
        pos.vz = pos2.vz;
        func_80036484(10, side | 0x42, kind, &pos, &dir, &rot, viewer.unk180);
        break;
    }
}

/* Per hand: starts the queued magic, or recovers the second bar. */
ADDRESS(0x80040024, 0x288)
void func_80040024(u8 hand)
{
    int active;
    int rate = 36;

    if (viewer.unk13C[hand] != -1) {
        if (hand) {
            if (viewer.unk38[13] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[13])->unk28)
                func_80040f04(viewer.unk14C[hand], hand);
        } else {
            if (viewer.unk38[0] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[0])->unk28)
                func_80040f04(viewer.unk14C[0], 0);
        }
        viewer.unk13C[hand] = -1;
        return;
    }
    active = 0;
    if (viewer.unk38[0] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[0])->unk28)
        active = viewer.unk38[6] != 0xffff;
    if (viewer.unk38[13] != 0xffff && (vis_table_c.unk0118 + viewer.unk38[13])->unk28 &&
        viewer.unk38[12] != 0xffff)
        active = 1;
    if (!viewer.unk1CB || viewer.unk190[12][0] != 0) {
        if (!active)
            return;
        if (viewer.unk188 == 0) {
            viewer.unk182 += (viewer.unk62[8] / (viewer.unk62[8] + 1000) * rate + 36) << 1;
            if (viewer.unk182 > 5120)
                viewer.unk182 = 5120;
        } else {
            viewer.unk188--;
        }
    } else {
        if (viewer.unk182 - 128 > 0)
            viewer.unk182 -= 128;
        else
            viewer.unk182 = 0;
    }
}

/* Steps hand `hand`'s swing or cast for its held item kind: the swing
 * timer viewer.unk130.halves[hand] against its hit window (unk130[hand + 2]) and
 * its end (unk138[hand]), the stamina bar viewer.unk17E, the hit search
 * over vis_table_a's entries and the item wear. The unused aggregates
 * keep retail's frame layout (sv at 24, v at 56, req at 104, p at 144).
 * Not exact (12 bytes long): register allocation differs from the start
 * (hand reaches $s6 through a copy in $t1, the hit search keeps one more
 * pointer copy, the 0..30 case's cost and limit swap $s1/$s2); the CFG,
 * calls and constants match. */
ADDRESS(0x800402ac, 0xc58)
void func_800402ac(u8 hand)
{
    ViewerModel *rec;
    SVECTOR unused0;
    SVECTOR sv;
    VECTOR unused1;
    SVECTOR unused2;
    VECTOR v;
    MATRIX unused3;
    VisRequest req;
    VECTOR p;
    Vis212 *e;
    Vis212 *hit;
    int cost;
    int max;
    int d;
    int i;
    int r;
    int hr;
    u32 best;
    short dmg;
    int reach;
    int inc;
    int kind;
    Vis192 *def;

    rec = viewer.unkE8[hand];
    switch (viewer.unk148[hand]) {
    case 36 ... 54:
        if (viewer.unk2DE != 0) {
            viewer.unk2DE >>= 1;
        }
        if (viewer.unk2E2 != 0) {
            viewer.unk2E2 >>= 1;
        }
        if (viewer.unk2E0 != 0) {
            viewer.unk2E0 -= viewer.unk2E0 / 20;
        }
        if (viewer.unk130.halves[hand] == -1) {
            break;
        }
        cost = rec->unk4 + rec->unk4 * viewer.unk62[1] / 1600;
        if (viewer.unk130.halves[hand ^ 1] != -1) {
            viewer.unk14F = 1;
        }
        if (viewer.unk14F) {
            viewer.unk130.halves[hand] -= cost;
        } else if ((!(DAT_801cd180.unk2 & DAT_801cd180.unk22[8]) && hand == 0) ||
                   (!(DAT_801cd180.unk2 & DAT_801cd180.unk22[9]) && hand != 0)) {
            viewer.unk130.halves[hand] -= cost;
            viewer.unk189 = 0;
        } else if (viewer.unk130.halves[hand + 2] - cost < viewer.unk130.halves[hand] &&
                   viewer.unk130.halves[hand] <= viewer.unk130.halves[hand + 2]) {
            viewer.unk189 = 1;
            sv.vx = 0;
            sv.vy = 0;
            sv.vz = rec->unk2 << 1;
            ApplyMatrix(&viewer.unk298, &sv, &v);
            v.vy = v.vy * (3600 - rec->unk2) / 1800;
            v.vx /= 4;
            v.vy /= 4;
            v.vz /= 4;
            d = 0xffff;
            i = 4;
            p.vx = viewer.position.vx;
            p.vy = viewer.position.vy;
            p.vz = viewer.position.vz;
            while (i--) {
                p.vx += v.vx;
                p.vy += v.vy;
                p.vz += v.vz;
                req.unk4 = p;
                req.unk1C = 1024;
                req.unk1E = 1024;
                r = func_8004bb88((VisLists *)&req, 4);
                if (r != -1) {
                    d = viewer.unk26A - (short)vis_table_a.entries[r].unk56;
                    if (d > 2048) {
                        d -= 4096;
                    }
                    if (d < -2048) {
                        d += 4096;
                    }
                }
            }
            if (d != 0xffff) {
                viewer.unk2E0 += (d - viewer.unk2E0) / 4;
            }
        } else if (viewer.unk154) {
            viewer.unk154 = 0;
        } else {
            viewer.unk130.halves[hand] += cost;
        }
        if ((u16)viewer.unk130.halves[hand] > 4095) {
            viewer.unk130.halves[hand] = -1;
            viewer.unk14F = 0;
            viewer.unk2DE = 0;
            viewer.unk2E0 = 0;
            viewer.unk2E2 = 0;
        }
        break;
    case 31 ... 34:
        if (viewer.unk130.halves[hand] != -1) {
            cost = rec->unk4 + rec->unk4 * viewer.unk62[1] / 1600;
            if (viewer.unk21F) {
                if (viewer.unk138[hand] - cost >= viewer.unk130.halves[hand]) {
                    viewer.unk130.halves[hand] -= cost;
                } else {
                    viewer.unk130.halves[hand] += cost;
                    if (viewer.unk130.halves[hand + 2] < viewer.unk130.halves[hand] &&
                        viewer.unk130.halves[hand] <= viewer.unk130.halves[hand + 2] + cost) {
                        kind = viewer.unk148[hand];
                        func_800582f0((viewer.unk1D4[hand] - 24) / 2 + 22, 128);
                        viewer.unk130.halves[hand] -= cost;
                        func_80040f04(kind + 13, hand);
                        viewer.unk130.halves[hand] += cost;
                    }
                }
            } else if ((!(DAT_801cd180.unk2 & DAT_801cd180.unk22[8]) && hand == 0) ||
                       (!(DAT_801cd180.unk2 & DAT_801cd180.unk22[9]) && hand != 0)) {
                viewer.unk21F = 1;
                viewer.unk180 = viewer.unk17E;
                viewer.unk17E = 0;
            } else if (viewer.unk130.halves[hand + 2] - cost < viewer.unk130.halves[hand] &&
                       viewer.unk130.halves[hand] <= viewer.unk130.halves[hand + 2]) {
                viewer.unk17E += (viewer.unk62[1] * 96 / 800 + 96) * 2;
                if (viewer.unk17E > 5120) {
                    viewer.unk17E = 5120;
                }
            } else if (viewer.unk130.halves[hand] == 0 && viewer.unk154) {
                viewer.unk154 = 0;
                viewer.unk21F = 0;
            } else {
                viewer.unk17E += viewer.unk62[1] * 96 / 800 + 96;
                if (viewer.unk17E > 5120) {
                    viewer.unk17E = 5120;
                }
                viewer.unk130.halves[hand] += cost;
                if (viewer.unk138[hand] - cost < viewer.unk130.halves[hand] &&
                    viewer.unk130.halves[hand] <= viewer.unk138[hand]) {
                    func_800582f0(viewer.unk1D4[hand], 128);
                    player_wear_item(1, &viewer.unk38[3]);
                }
            }
            if ((u16)viewer.unk130.halves[hand] > 4095) {
                viewer.unk130.halves[hand] = -1;
                viewer.unk14E = 0;
                viewer.unk21F = 0;
            }
            viewer.unk130.halves[hand ^ 1] = viewer.unk130.halves[hand];
        } else {
            viewer.unk187 = 0;
        }
        break;
    case 0 ... 30:
        if (viewer.unk130.halves[hand] != -1) {
            cost = rec->unk4 + rec->unk4 * viewer.unk62[1] / 1600;
            max = rec->unk8;
            if (DAT_801cd180.unk4 & DAT_801cd180.unk22[8]) {
                DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[8];
            }
            if (viewer.unk14E && !(viewer.unk130.halves[hand + 2] - cost < viewer.unk130.halves[hand])) {
                viewer.unk130.halves[hand] -= cost;
            } else if (viewer.unk154) {
                viewer.unk180 = viewer.unk17E;
                viewer.unk17E = 0;
                viewer.unk154 = 0;
            } else {
                viewer.unk130.halves[hand] += cost;
            }
            if (viewer.unk138[hand] - cost < viewer.unk130.halves[hand] &&
                viewer.unk130.halves[hand] <= viewer.unk138[hand]) {
                func_800582f0(viewer.unk1D4[hand], 128);
                if (viewer.unk138[hand] >= max) {
                    viewer.unk138[hand] = 5000;
                } else {
                    viewer.unk138[hand] = viewer.unk138[hand];
                }
            }
            if (viewer.unk148[hand] < 36 || viewer.unk148[hand] > 254) {
                if (viewer.unk130.halves[hand + 2] - cost < viewer.unk130.halves[hand] &&
                    viewer.unk130.halves[hand] <= viewer.unk130.halves[hand + 2]) {
                    e = vis_table_a.entries;
                    best = -1;
                    hit = NULL;
                    hr = 0;
                    dmg = 0;
                    reach = viewer.unkE8[hand]->unk2;
                    vis_table_a.unk6144 = 0;
                    do {
                        if (e->unk00 != 255) {
                            func_8004c03c(e);
                            if (e->unk05 == 1 && e->unk1C.bytes[3] != 1 && e->unk2C != 0 &&
                                (r = func_8004db9c(e, reach)) != 0 && e->unk3C < best) {
                                best = e->unk3C;
                                hit = e;
                                hr = r;
                            }
                        }
                        e++;
                    } while (++vis_table_a.unk6144 < 64);
                    if (hit != NULL) {
                        dmg = func_8004def8(hit, hr, viewer.unk180, viewer.unkBA, 0);
                        func_80026a88(dmg);
                    }
                    if (hr != 0 && dmg != -1) {
                        def = &vis_table_a.unk3500[hit->unk02];
                        if (dmg * 4 < def->unk32) {
                            viewer.unk21B += 3;
                        } else if (dmg * 2 < def->unk32) {
                            viewer.unk21B += 2;
                        } else if (dmg < def->unk32) {
                            viewer.unk21B += 1;
                        }
                        if (viewer.unk21B >= 7) {
                            viewer.unk21B -= 7;
                            player_wear_item(1, &viewer.unk38[3]);
                        }
                    }
                }
                if ((u16)viewer.unk130.halves[hand] > 4095) {
                    viewer.unk130.halves[hand] = -1;
                    viewer.unk14E = 0;
                }
            }
        } else {
            viewer.unk14E = 0;
            if (!(DAT_801cd180.unk2 & DAT_801cd180.unk22[8])) {
                if (viewer.unk187 == 0) {
                    inc = viewer.unk62[1] / (viewer.unk62[1] + 1000) * 96 + 96;
                    if (viewer.unk38[16] != 0xffff && viewer.unk130.halves[1] >= 0) {
                        inc >>= 1;
                    }
                    viewer.unk17E += inc * 2;
                    if (viewer.unk17E > 5120) {
                        viewer.unk17E = 5120;
                    }
                } else {
                    viewer.unk187--;
                }
            }
        }
        break;
    case 255: /* no item: an empty out-of-range label; with it GCC dispatches
               * by the compare tree retail has instead of a jump table */
        break;
    }
}

/* Starts hand `hand` casting magic `magic` if it is idle and the magic
 * points cover the cost: per-magic cast timers and launch offsets, then
 * the hand's item wears unless the magic is 44..47. */
ADDRESS(0x80040f04, 0x24c)
void func_80040f04(u8 magic, u8 hand)
{
    StMagic *rec;
    int cost;
    SVECTOR unused;

    if (viewer.unk155[hand] != 255 || magic == 255) {
        return;
    }
    rec = (StMagic *)&effects.defs[magic];
    cost = rec->unk2;
    if (viewer.unk6 < cost) {
        return;
    }
    viewer.unk6 -= cost;
    viewer.unk155[hand] = magic;
    switch (magic) {
    case 0:
    case 10:
        viewer.unk15B[hand] = 1;
        viewer.unk157[hand] = 1;
        viewer.unk159[hand] = 1;
        viewer.unk160[hand].vx = 0;
        viewer.unk160[hand].vy = 50;
        viewer.unk160[hand].vz = 0;
        break;
    case 7:
        viewer.unk15B[hand] = 1;
        viewer.unk157[hand] = 1;
        viewer.unk159[hand] = 1;
        viewer.unk160[hand].vx = -512;
        viewer.unk160[hand].vy = -1024;
        viewer.unk160[hand].vz = 0;
        break;
    case 3:
        viewer.unk157[hand] = 4;
        viewer.unk159[hand] = 8;
        viewer.unk160[hand].vx = 0;
        viewer.unk160[hand].vy = 200;
        viewer.unk160[hand].vz = 0;
        goto fire;
    case 9:
        viewer.unk157[hand] = 3;
        viewer.unk159[hand] = 4;
        viewer.unk160[hand].vx = 0;
        viewer.unk160[hand].vy = 200;
        viewer.unk160[hand].vz = 0;
        goto fire;
    case 12:
        viewer.unk157[hand] = 2;
        viewer.unk159[hand] = 4;
        viewer.unk160[hand].vx = 0;
        viewer.unk160[hand].vy = 200;
        viewer.unk160[hand].vz = 0;
        goto fire;
    case 1:
    case 4:
    case 6:
    case 13:
    case 15:
    case 16:
        viewer.unk157[hand] = 1;
        viewer.unk159[hand] = 1;
        viewer.unk160[hand].vx = 0;
        viewer.unk160[hand].vy = 200;
        viewer.unk160[hand].vz = 0;
    fire:
        viewer.unk15B[hand] = 1;
        break;
    case 44:
    case 45:
    case 46:
    case 47:
        viewer.unk15B[hand] = 1;
        viewer.unk157[hand] = 1;
        viewer.unk159[hand] = 1;
        break;
    }
    viewer.unk118[hand] = rec;
    switch (magic) {
    case 44:
    case 45:
    case 46:
    case 47:
        break;
    default:
        if (hand) {
            player_wear_item(1, &viewer.unk38[13]);
        } else {
            player_wear_item(1, &viewer.unk38[0]);
        }
    }
}

/* Moves the viewer `speed` units along heading `angle`, sliding along the
 * walls func_80049544 reports (at most three tries), and keeps the motion
 * in viewer.unk260.
 * Not exact: the setup block schedules the viewer position copy after
 * vel.vz and the unk1C/unk1E stores (retail copies first, keeping the
 * vel.vz product in $t1); the rest matches. */
ADDRESS(0x80041150, 0x4dc)
void func_80041150(int angle, int speed)
{
    VECTOR vel;
    VECTOR d;
    SVECTOR n;
    int hits;
    int tries;
    int slid;
    int slid_a;
    int slid_b;
    int ang;
    int dy;

    slid_a = 0;
    slid_b = 0;
    slid = 0;
    tries = 0;
    vel.vx = rcos(angle) * speed / 4096;
    vel.vz = rsin(angle) * speed / 4096;
    vis_lists.unk4.vx = viewer.position.vx;
    vis_lists.unk4.vy = viewer.position.vy;
    vis_lists.unk4.vz = viewer.position.vz;
    vis_lists.unk4.pad = viewer.position.pad;
    vis_lists.unk1C = 256;
    vis_lists.unk1E = 1600;
    vel.vy = viewer.unk2D4;
    vis_lists.unk0 = 1074;
loop:
    viewer.unk260.vz = vel.vz + viewer.unk1D0;
    viewer.unk260.vx = vel.vx + viewer.unk1CC;
    viewer.unk260.vy = vel.vy;
    vis_lists.unk14 = viewer.unk260;
    vis_lists.unk20 = ratan2(viewer.unk260.vz, viewer.unk260.vx);
    vis_lists.unk22 = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
    vis_lists.unk47CE = 0;
    vis_lists.unk47D0 = 0;
    vis_lists.unk47D2 = 0;
    hits = func_80049544(&vis_lists);
    if (hits == 0) {
    move:
        viewer.position.vx += vel.vx;
        viewer.position.vy += vel.vy;
        viewer.position.vz += vel.vz;
        goto done;
    }
    if (tries++ >= 3) {
        goto done;
    }
    if (hits & 1) {
        if (!(hits & ~1)) {
            goto move;
        }
    }
    if (hits & 2) {
        vel.vx = (1 - (vis_lists.unk3C - 256)) * vis_lists.unk8C / 4096;
        vel.vz = (1 - (vis_lists.unk3C - 256)) * vis_lists.unk90 / 4096;
        if (slid_a) {
            goto move;
        }
        slid_a = 1;
        ang = ratan2(vis_lists.unk90, vis_lists.unk8C);
        vis_lists.unk78 = (short)ang;
        hits = speed * rcos(angle - (ang + 1024)) / 4096;
        vis_lists.unk44C6 = hits;
    } else if (hits & 8) {
        vel.vx = (1 - (vis_lists.unk44 - 256)) * vis_lists.unk9C / 4096;
        vel.vz = (1 - (vis_lists.unk44 - 256)) * vis_lists.unkA0 / 4096;
        if (slid_b) {
            goto move;
        }
        slid_b = 1;
        ang = ratan2(vis_lists.unkA0, vis_lists.unk9C);
        vis_lists.unk78 = (short)ang;
        hits = speed * rcos(angle - (ang + 1024)) / 4096;
        vis_lists.unk44C6 = hits;
    } else if (hits & 0x50) {
        func_800492f4((VECTOR *)&viewer.position.vx);
        d.vx = viewer.position.vx - vis_lists.unkAC.vx;
        d.vy = 0;
        d.vz = viewer.position.vz - vis_lists.unkAC.vz;
        VectorNormalS(&d, &n);
        dy = vis_lists.unkBC - vis_lists.unk44C0 - 256;
        if (hits & 0x10) {
            dy = vis_lists.unkBC - vis_lists.unk44C0 - 768;
        }
        dy = 1 - dy;
        vel.vx = dy * n.vx / 4096;
        vel.vz = dy * n.vz / 4096;
        if (slid) {
            goto done;
        }
        slid = 1;
        ang = ratan2(d.vz, d.vx);
        vis_lists.unk78 = ang;
        hits = speed * rcos(angle - (ang + 1024)) / 4096;
        vis_lists.unk44C6 = hits;
    } else {
        goto done;
    }
    if (hits != 0) {
        vel.vx += rcos(ang + 1024) * hits / 4096;
        vel.vz += rsin(ang + 1024) * hits / 4096;
    }
    goto loop;
done:
    viewer.unk1CC += vis_lists.unk47CE / 4;
    viewer.unk1D0 += vis_lists.unk47D2 / 4;
    viewer.unk260.vx = vel.vx;
    viewer.unk260.vy = vel.vy;
    viewer.unk260.vz = vel.vz;
}

/* The coasting vertical offset func_8004162c halves each frame. */
DATA(0x8008803c, 0x2, ".sdata")
static s16 DAT_8008803c = 0;

/* Coasts the viewer on its push offsets while it is not walking (sliding
 * along walls as func_80041150 does), then steps the vertical speed
 * against the floor, the head bob and the stamina, and records the
 * visible record the viewer stands in.
 * Not exact: as in func_80041150, the setup block schedules the viewer
 * position copy after the unk1C/unk1E stores (retail copies first); the
 * rest matches. */
ADDRESS(0x8004162c, 0x720)
void func_8004162c(void)
{
    VECTOR vel;
    int hits;
    int push;
    int slid_a;
    int slid_b;
    int ang;
    int speed;
    short heading;
    int d;
    int s;

    memset(&vel, 0, sizeof(vel));
    slid_b = 0;
    slid_a = 0;
    if (viewer.unk2C8 == 0 || viewer.unk21E != 0) {
        vel.vx = viewer.unk1CC;
        vel.vz = viewer.unk1D0;
        vis_lists.unk4.vx = viewer.position.vx;
        vis_lists.unk4.vy = viewer.position.vy;
        vis_lists.unk4.vz = viewer.position.vz;
        vis_lists.unk4.pad = viewer.position.pad;
        vis_lists.unk1C = 256;
        vis_lists.unk1E = 1600;
        vel.vy = viewer.unk2D4;
        vis_lists.unk0 = 1074;
        speed = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
        heading = ratan2(vel.vz, vel.vx);
    loop:
        vis_lists.unk20 = 9999;
        vis_lists.unk14.vx = vel.vx;
        vis_lists.unk14.vy = vel.vy;
        vis_lists.unk14.vz = vel.vz;
        vis_lists.unk22 = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
        vis_lists.unk47CE = 0;
        vis_lists.unk47D0 = 0;
        vis_lists.unk47D2 = 0;
        hits = func_80049544(&vis_lists);
        if (hits == 0) {
        move:
            viewer.position.vx += vel.vx;
            viewer.position.vy += vel.vy;
            viewer.position.vz += vel.vz;
            goto done;
        }
        if (hits & 1) {
            viewer.position.vx += vel.vx;
            viewer.position.vy += vel.vy;
            viewer.position.vz += vel.vz;
        }
        if (hits & 2) {
            vel.vx = (1 - (vis_lists.unk3C - 256)) * vis_lists.unk8C / 4096;
            vel.vz = (1 - (vis_lists.unk3C - 256)) * vis_lists.unk90 / 4096;
            if (slid_a) {
                goto move;
            }
            slid_a = 1;
            ang = ratan2(vis_lists.unk90, vis_lists.unk8C);
            vis_lists.unk78 = ang;
            push = speed * rcos(heading - (ang + 1024)) / 4096;
            vis_lists.unk44C6 = push;
        } else if (hits & 8) {
            vel.vx = (1 - (vis_lists.unk44 - 256)) * vis_lists.unk9C / 4096;
            vel.vz = (1 - (vis_lists.unk44 - 256)) * vis_lists.unkA0 / 4096;
            if (slid_b) {
                goto move;
            }
            slid_b = 1;
            ang = ratan2(vis_lists.unkA0, vis_lists.unk9C);
            vis_lists.unk78 = ang;
            push = speed * rcos(heading - (ang + 1024)) / 4096;
            vis_lists.unk44C6 = push;
        } else {
            goto done;
        }
        if (push != 0) {
            vel.vx += rcos(ang + 1024) * push / 4096;
            vel.vz += rsin(ang + 1024) * push / 4096;
        }
        goto loop;
    done:
        viewer.unk1CC += vis_lists.unk47CE / 4;
        viewer.unk1D0 += vis_lists.unk47D2 / 4;
        viewer.unk260.vx = vel.vx;
        viewer.unk260.vy = vel.vy;
        viewer.unk260.vz = vel.vz;
    }
    func_8003bdb0();
    d = vis_lists.unk2C - 1600;
    if (d < 0) {
        if (viewer.unk2D4 > 0) {
            func_80057b48(viewer.unk2D4);
            viewer.unk2D4 >>= 2;
        } else if (viewer.unk2D4 < 0) {
            if (d >= -1024) {
                viewer.unk2E4 = 0;
            }
            viewer.unk2D4 -= 48;
            if (vis_lists.unk2C - viewer.unk2D4 > 1600) {
                viewer.unk2D4 = vis_lists.unk2C - 1600;
            }
        } else if (d >= -31) {
            viewer.unk2D4 -= d;
        } else {
            if (d >= -1024) {
                viewer.unk2E4 = 0;
            }
            viewer.unk2D4 -= 16;
        }
    } else if (d > 0) {
        if (viewer.unk2D4 > 0) {
            if (d < 1025) {
                viewer.unk2E4 = 0;
            }
            if ((vis_lists.unk74 & ~15) == 0) {
                viewer.unk2D4 += 48;
            }
        } else if (viewer.unk2D4 < 0) {
            viewer.unk2D4 /= 2;
        } else if (d < 32) {
            viewer.unk2D4 += d;
        } else {
            if (d < 1025) {
                viewer.unk2E4 = 0;
            }
            viewer.unk2D4 += 16;
        }
    } else {
        viewer.unk239 = 0;
        viewer.unk2D4 = 0;
    }
    if (viewer.unk2D4 > 128) {
        DAT_8008803c = (viewer.unk2D4 - 128) / 2;
    } else if (viewer.unk2D4 < -128) {
        DAT_8008803c = (viewer.unk2D4 + 128) / 2;
    } else {
        DAT_8008803c /= 2;
    }
    viewer.unk290 = DAT_8008803c;
    if (viewer.unk239 == 0) {
        if (DAT_801cd120.unk2) {
            viewer.unk2D0 = (viewer.unk2D0 + viewer.unk2C8) & 0xfff;
            s = rsin(viewer.unk2D0) >> 5;
            if (s < 0) {
                s = -s;
            }
            viewer.unk2CE = s - (s >> 2);
        } else {
            viewer.unk2CE = 0;
        }
        if (viewer.unk239 == 0 && viewer.unk2D4 < 256) {
            if (viewer.unk2C8 < viewer.unk2E4 >> 2) {
                viewer.unk244 -= viewer.unk2C8 * 3;
            } else if (viewer.unk2C8 < viewer.unk2E4 >> 1) {
                viewer.unk244 -= viewer.unk2C8 * 3 / 2;
            } else {
                viewer.unk244 -= viewer.unk2C8;
            }
            if (viewer.unk244 < 0) {
                viewer.unk244 = 2048;
                func_80057bac(viewer.unk2C8);
            }
        }
    }
    func_8003be10();
    if (vis_lists.unk44C8 != -1) {
        viewer.unk170 = viewer.unk174;
        viewer.unk174 = &vis_records[vis_lists.unk44C8];
        viewer.unk178 = vis_lists.unk44C8;
    }
}

ADDRESS(0x80041d4c, 0x354)
void func_80041d4c(void)
{
    if (DAT_801cd180.unk2 & DAT_801cd180.unk22[2]) {
        viewer.unk2CA += viewer.unk2E8 >> 2;
        if (viewer.unk2E8 < viewer.unk2CA)
            viewer.unk2CA = viewer.unk2E8;
    } else if (DAT_801cd180.unk2 & DAT_801cd180.unk22[3]) {
        viewer.unk2CA -= viewer.unk2E8 >> 2;
        if (viewer.unk2CA < -viewer.unk2E8)
            viewer.unk2CA = -viewer.unk2E8;
    } else if (viewer.unk2CA > 0) {
        viewer.unk2CA -= viewer.unk2E8 >> 2;
        if (viewer.unk2CA < 0)
            viewer.unk2CA = 0;
    } else if (viewer.unk2CA < 0) {
        viewer.unk2CA += viewer.unk2E8 >> 2;
        if (viewer.unk2CA > 0)
            viewer.unk2CA = 0;
    }
    viewer.unk278.vy = (viewer.unk278.vy + viewer.unk2CA) & 0xfff;
    if (DAT_801cd180.unk2 & DAT_801cd180.unk22[6]) {
        if (DAT_801cd180.unk2 & DAT_801cd180.unk22[7]) {
            if (viewer.unk278.vx > 2048) {
                int limit = (4224 - viewer.unk278.vx) >> 2;

                viewer.unk2CC += 8;
                if (limit < viewer.unk2CC) {
                    viewer.unk2CC = limit;
                    if (limit == 0)
                        viewer.unk2CC = 4224 - viewer.unk278.vx;
                }
            } else if (viewer.unk278.vx < 2048) {
                int limit = -((viewer.unk278.vx - 128) >> 2);

                viewer.unk2CC -= 8;
                if (viewer.unk2CC < limit) {
                    viewer.unk2CC = limit;
                    if (limit == 0) {
                        s16 offset = viewer.unk278.vx - 128;

                        viewer.unk2CC = -offset;
                    }
                }
            }
        } else {
            viewer.unk2CC -= 3;
            if (viewer.unk2CC < -32)
                viewer.unk2CC = -32;
        }
    } else if (DAT_801cd180.unk2 & DAT_801cd180.unk22[7]) {
        viewer.unk2CC += 3;
        if (viewer.unk2CC > 32)
            viewer.unk2CC = 32;
    } else if (viewer.unk2CC > 0) {
        viewer.unk2CC -= 3;
        if (viewer.unk2CC < 0)
            viewer.unk2CC = 0;
    } else if (viewer.unk2CC < 0) {
        viewer.unk2CC += 3;
        if (viewer.unk2CC > 0)
            viewer.unk2CC = 0;
    }
    if (viewer.unk2CC > 0) {
        viewer.unk278.vx = (viewer.unk278.vx + viewer.unk2CC) & 0xfff;
        if (func_8004a194(viewer.unk278.vx, 700))
            viewer.unk278.vx = 700;
    } else if (viewer.unk2CC < 0) {
        viewer.unk278.vx = (viewer.unk278.vx + viewer.unk2CC) & 0xfff;
        if (func_8004a194(3396, viewer.unk278.vx))
            viewer.unk278.vx = 3396;
    }
}

/* Walks the viewer from the held buttons: accelerates the walk velocity
 * (viewer.unk2B8) towards viewer.unk2E4 or lets it decay, turns it by the
 * yaw and moves the viewer along it (func_80041150); eases the push
 * offsets viewer.unk1CC..unk1D0 back and adds the area's drift. */
ADDRESS(0x800420a0, 0x580)
void func_800420a0(void)
{
    SVECTOR sv;
    VECTOR v;
    VECTOR sq;
    MATRIX m;
    SVECTOR rot;
    int ang;

    if (DAT_801cd180.unk2 & DAT_801cd180.unk22[0]) {
        v.vz = viewer.unk2B8.vz + (viewer.unk2E4 >> 2);
        if (v.vz <= viewer.unk2E4) {
            viewer.unk2B8.vz = v.vz;
        } else {
            viewer.unk2B8.vz = viewer.unk2E4;
        }
    } else if (DAT_801cd180.unk2 & DAT_801cd180.unk22[1]) {
        v.vz = viewer.unk2B8.vz - (viewer.unk2E4 >> 2);
        if (v.vz >= -viewer.unk2E4) {
            viewer.unk2B8.vz = v.vz;
        } else {
            viewer.unk2B8.vz = -viewer.unk2E4;
        }
    } else if (viewer.unk2B8.vz > 0) {
        viewer.unk2B8.vz -= viewer.unk2E4 >> 3;
        if (viewer.unk2B8.vz < 0) {
            viewer.unk2B8.vz = 0;
        }
    } else if (viewer.unk2B8.vz < 0) {
        viewer.unk2B8.vz += viewer.unk2E4 >> 3;
        if (viewer.unk2B8.vz > 0) {
            viewer.unk2B8.vz = 0;
        }
    }
    if ((DAT_801cd180.unk2 & DAT_801cd180.unk22[5]) && !(DAT_801cd180.unk2 & DAT_801cd180.unk22[4])) {
        v.vx = viewer.unk2B8.vx + (viewer.unk2E4 >> 2);
        if (v.vx <= viewer.unk2E4) {
            viewer.unk2B8.vx = v.vx;
        } else {
            viewer.unk2B8.vx = viewer.unk2E4;
        }
    } else if ((DAT_801cd180.unk2 & DAT_801cd180.unk22[4]) && !(DAT_801cd180.unk2 & DAT_801cd180.unk22[5])) {
        v.vx = viewer.unk2B8.vx - (viewer.unk2E4 >> 2);
        if (v.vx >= -viewer.unk2E4) {
            viewer.unk2B8.vx = v.vx;
        } else {
            viewer.unk2B8.vx = -viewer.unk2E4;
        }
    } else if (viewer.unk2B8.vx > 0) {
        viewer.unk2B8.vx -= viewer.unk2E4 >> 3;
        if (viewer.unk2B8.vx < 0) {
            viewer.unk2B8.vx = 0;
        }
    } else if (viewer.unk2B8.vx < 0) {
        viewer.unk2B8.vx += viewer.unk2E4 >> 3;
        if (viewer.unk2B8.vx > 0) {
            viewer.unk2B8.vx = 0;
        }
    }
    gte_ldlvl(&viewer.unk2B8);
    gte_sqr0();
    gte_stlvnl(&sq);
    viewer.unk2C8 = SquareRoot0(sq.vx + sq.vz);
    if (viewer.unk2C8 == 0) {
        v.vz = 0;
        v.vx = 0;
    } else {
        if (viewer.unk2B8.vx < 0) {
            sq.vx = -sq.vx;
        }
        if (viewer.unk2B8.vz < 0) {
            sq.vz = -sq.vz;
        }
        v.vx = sq.vx / viewer.unk2C8;
        v.vz = sq.vz / viewer.unk2C8;
    }
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = -viewer.unk278.vy;
    RotMatrix(&rot, &m);
    sv.vx = v.vx;
    sv.vy = v.vy;
    sv.vz = v.vz;
    gte_SetRotMatrix(&m);
    gte_ldv0(&sv);
    gte_rtv0();
    gte_stlvnl(&v);
    ang = ratan2(v.vz, v.vx);
    gte_ldlvl(&v);
    gte_sqr0();
    gte_stlvnl(&sq);
    viewer.unk2C8 = SquareRoot0(sq.vx + sq.vz);
    viewer.unk1CC -= viewer.unk1CC / 4;
    viewer.unk1CE -= viewer.unk1CE / 4;
    viewer.unk1D0 -= viewer.unk1D0 / 4;
    if (viewer.unk1CC > -4 && viewer.unk1CC < 4) {
        viewer.unk1CC = 0;
    }
    if (viewer.unk1CE > -4 && viewer.unk1CE < 4) {
        viewer.unk1CE = 0;
    }
    if (viewer.unk1D0 > -4 && viewer.unk1D0 < 4) {
        viewer.unk1D0 = 0;
    }
    viewer.unk1CC += vis_table_b.unk71A4 / 4;
    viewer.unk1D0 += vis_table_b.unk71A8 / 4;
    viewer.unk1CC += vis_table_a.unk6160 / 2;
    viewer.unk1D0 += vis_table_a.unk6164 / 2;
    if (viewer.unk2C8 != 0 && viewer.unk21E == 0) {
        func_80041150(ang, viewer.unk2C8);
    }
}

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
 * The paired timer word must be -1 before either equipment request runs.
 */
ADDRESS(0x80042900, 0x29c)
void func_80042900(void)
{
    if (DAT_801cd180.unk14 == 2) {
        DAT_801cd180.unk14 = 0;
        if (viewer.unk130.word == -1 && viewer.unk38[4] != 0xffff) {
            func_80042620(0, 2);
            func_8003d030();
        }
    }
    if (DAT_801cd180.unk14 == 3) {
        DAT_801cd180.unk14 = 0;
        if (viewer.unk130.word == -1 && viewer.unk38[17] != 0xffff) {
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
        if (viewer.unk187 == 0 && (DAT_801cd180.unk4 & DAT_801cd180.unk22[8]))
            func_8003f4ec(0, 0);
        if (DAT_801cd180.unk4 & DAT_801cd180.unk22[9])
            func_8003f4ec(0, 1);
    }
}

ADDRESS(0x80042b9c, 0x74)
void func_80042b9c(void)
{
    u8 mode = viewer.unk218;

    if (mode != 4 && (DAT_801cd180.unk4 & DAT_801cd180.unk22[10]) && !DAT_801cd180.unk12) {
        DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[10];
        if (mode == 0)
            func_80055374(&viewer.position.vx, &viewer.unk278);
    }
}

ADDRESS(0x80042c10, 0x34)
void func_80042c10(void)
{
    viewer.unk218 = 0;
    viewer.unk280.halves[0] = viewer.unk280.halves[1] = viewer.unk284 = 0;
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

/* The per-frame player update: the status timers, opening the pause menu
 * and the map, the walk and stamina limits, the state machine of
 * viewer.unk218 (walking, using an item, dying, falling into a pit), the
 * floor damage and the status damage over time, the view angles, the
 * swings and the hit/magic point recovery.
 * Not exact: input-owner and masked-button scheduling, a status scalar's
 * address temporary, and the later damage/wear loop temporaries differ. */
ADDRESS(0x80042d74, 0x1678)
void func_80042d74(void)
{
    RECT r;
    StAnimFile *file;
    StEffect *fx;
    int n;
    int i;
    int k;
    int d;

    player_update_status(1);
    if (viewer.unk218 == 0 || viewer.unk218 == 4 || viewer.unk218 == 16 || viewer.unk218 == 1) {
        if (((DAT_801cd180.unk4 & 1) || DAT_801cd180.unk14 == 1) && viewer.unk130.word == -1 &&
            viewer.unk218 != 1 && viewer.unk218 != 4 && !(viewer.unk2C & 0x10) &&
            area_state.unk0 == 0 && vis_table_a.unk6169 == 0 && area_state.unk18 == -1) {
            Vis88 *entry;
            int left;

            i = 16;
            lighting.unkA = 1;
            func_80057cc8();
            func_80042cfc();
            func_80057a3c();
            func_80057a68(0);
            entry = vis_table_b.entries;
            left = 240;
            while (left--) {
                if (entry->unk05 == 236 && entry->unk54 != -1) {
                    func_8005850c(entry->unk54, entry->unk4A.snd.unk4A + 1108);
                    entry->unk54 = -1;
                }
                entry++;
            }
            viewer.unk21D = 0;
            viewer.unk2B = 0;
            viewer.unk2A = 0;
            func_80058ef4();
            if (DAT_801cd120.unk3 == 0) {
                func_8003e840();
            }
            if (viewer.unk2A) {
                func_8003e840();
                func_80057a3c();
                DAT_801cd180.unk10 = 1;
                func_80016508();
                viewer.unk2D6 = viewer.unk2D8 = viewer.unk2DA = viewer.unk2DC = 0;
                func_800500ac(2, 4096, 4096, 0);
                func_800500ac(2, 4096, 4096, 0);
                func_80015ad4();
                func_8003c0e0();
                vis_table_a.unk6168 = 254;
                vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] =
                    vis_table_a.unk6158[3] = 255;
                func_80057a68(0);
                fx = effects.slots;
                k = 128;
                while (k--) {
                    fx->unk0 = 255;
                    fx++;
                }
                func_8003c95c(area_state.unk4, area_state.unk4, area_state.unk4, area_state.unk7,
                              area_state.unk8, 255);
                func_80015ad4();
                func_8003ead0();
            } else {
                if (viewer.unk21D) {
                    while (i--) {
                        anim_get_file(i + 128, &file);
                        if (file) {
                            func_80015230(file);
                        }
                    }
                    i = 128;
                    while (i--) {
                        anim_get_file(i + 588, &file);
                        if (file) {
                            func_80015230(file);
                        }
                    }
                }
                player_equipment_bonuses();
                func_8003d030();
                {
                    world_state.draw[0].clip.w = 0;
                    world_state.draw[0].clip.h = 0;
                    world_state.draw[1].clip.w = 0;
                    world_state.draw[1].clip.h = 0;
                    r.x = 0;
                    r.y = world_state.frame ? 0 : 240;
                    r.w = 320;
                    r.h = 240;
                    MoveImage(&r, 0, world_state.frame * 240);
                    DrawSync(0);
                    player_update_status(0);
                    func_80026afc(0, 0);
                    player_update_status(0);
                    func_80015ad4();
                    world_state.draw[0].clip.w = 320;
                    world_state.draw[0].clip.h = 240;
                    world_state.draw[1].clip.w = 320;
                    world_state.draw[1].clip.h = 240;
                }
            }
            func_80057a3c();
            lighting.unkA = 2;
        }
        if (viewer.unk2B) {
            func_80052c24();
        }
        if ((DAT_801cd180.unk4 & 8) && viewer.unk218 != 4 && area_state.unk0 == 0 &&
            vis_table_a.unk6169 == 0 && area_state.unk18 == -1) {
            lighting.unkA = 1;
            func_80057cc8();
            DAT_801cd180.unk4 &= ~8;
            func_80057a3c();
            func_80057a68(0);
            func_800631ac();
            func_80057a3c();
            lighting.unkA = 2;
        }
        viewer.unk2E4 = 192;
        viewer.unk2E8 = 32;
        if (!(DAT_801cd180.unk2 & (DAT_801cd180.unk22[0] | DAT_801cd180.unk22[1]))) {
            viewer.unk2E8 = 40;
        }
        if (viewer.unk190[3][0] != 0) {
            viewer.unk2E4 = 96;
        }
        if (viewer.unk190[1][0] != 0) {
            viewer.unk2E4 = 0;
            if (viewer.unk2C & 0x10) {
                viewer.unk2E8 = 0;
            }
            if (viewer.unk190[1][0] % 20 == 0) {
                func_800582f0(6, 128);
            }
        }
        if (viewer.unk22 > viewer.unk24) {
            int limit = viewer.unk2E4 - (viewer.unk22 - viewer.unk24) / 5;

            if (limit < 0) {
                viewer.unk2E4 = 0;
            } else {
                viewer.unk2E4 = limit;
            }
        }
        switch (viewer.unk218) {
        case 0:
            func_80042900();
            if (area_state.unk0 != 1 && vis_table_a.unk6169 == 0) {
                func_80041d4c();
                func_800420a0();
                func_8004162c();
                viewer.unk21E = 0;
                if ((DAT_801cd180.unk4 & DAT_801cd180.unk22[11]) && DAT_801cd180.unk12 == 0 &&
                    (viewer.unk38[5] != 0xffff || viewer.unk38[19] != 0xffff ||
                     viewer.unk38[11] != 0xffff || viewer.unk38[20] != 0xffff ||
                     viewer.unk38[18] != 0xffff)) {
                    func_80057cc8();
                    viewer.unk218 = 1;
                    DAT_801cd180.unk4 &= DAT_801cd180.unk22[11];
                    func_80057a3c();
                }
            }
            func_80042b9c();
            break;
        case 1:
            func_80042900();
            if (area_state.unk0 == 1 || vis_table_a.unk6169) {
                goto status;
            }
            func_80041d4c();
            func_800420a0();
            func_8004162c();
            viewer.unk21E = 0;
            if (DAT_801cd180.unk4 & DAT_801cd180.unk22[8]) {
                func_80057d10();
                DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[8];
                if (++viewer.unk15E >= 5) {
                    viewer.unk15E = 0;
                }
            } else if (DAT_801cd180.unk4 & DAT_801cd180.unk22[9]) {
                func_80057d10();
                DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[9];
                if (--viewer.unk15E < 0) {
                    viewer.unk15E = 4;
                }
            } else {
                if ((DAT_801cd180.unk4 & DAT_801cd180.unk22[11]) && DAT_801cd180.unk12 == 0) {
                    func_80057cec();
                    viewer.unk218 = 0;
                    DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[11];
                    DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[10];
                } else if ((DAT_801cd180.unk4 & DAT_801cd180.unk22[10]) && DAT_801cd180.unk12 == 0) {
                    int ids[5] = {viewer.unk38[5], viewer.unk38[19], viewer.unk38[11],
                                  viewer.unk38[20], viewer.unk38[18]};

                    viewer.unk218 = 0;
                    DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[8];
                    DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[11];
                    DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[9];
                    DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[10];
                    if (ids[viewer.unk15E] != 0xffff) {
                        viewer.unk2B = 0;
                        func_80054a54(ids[viewer.unk15E]);
                        if (DAT_801cd0e0 == 1) {
                            func_80057cc8();
                            if (ids[viewer.unk15E] == 288) {
                                func_8005a3a8(0);
                            }
                            if (DAT_801c7590.counts[ids[viewer.unk15E]].count == 0) {
                                RECT icon = {896, 256, 24, 24};

                                switch (viewer.unk15E) {
                                case 0:
                                    viewer.unk38[5] = 0xffff;
                                    break;
                                case 1:
                                    viewer.unk38[19] = 0xffff;
                                    break;
                                case 2:
                                    viewer.unk38[11] = 0xffff;
                                    break;
                                case 3:
                                    viewer.unk38[20] = 0xffff;
                                    break;
                                case 4:
                                    viewer.unk38[18] = 0xffff;
                                    break;
                                }
                                icon.y = viewer.unk15E * 24 + 256;
                                ClearImage(&icon, 0, 0, 0);
                            }
                        } else if (DAT_801cd0e0 == 2) {
                            func_80057d34();
                        }
                    } else {
                        func_80057d34();
                    }
                } else {
                    goto status;
                }
                func_80057a3c();
            }
        status:
            func_80042b9c();
            break;
        case 4:
            if (viewer.unk286 < 17) {
                func_80021994(255, 255, 255, viewer.unk286 << 8, 2);
                viewer.unk280.halves[0] += viewer.unk17A;
                if (viewer.unk286 == 16) {
                    area_state.unkC->unk34();
                    func_80021994(255, 255, 255, viewer.unk286 << 8, 2);
                }
            } else {
                func_80021994(255, 255, 255, 4096, 2);
                if (viewer.unk286 == 20) {
                    RECT clear = {0, 0, 320, 480};

                    anim_slots_free_idle();
                    anim_sets_free_idle();
                    anim_slots_free_all();
                    anim_sets_free_all();
                    fx = effects.slots;
                    k = 128;
                    while (k--) {
                        fx->unk0 = 255;
                        fx++;
                    }
                    for (;;) {
                        if (func_80063260() == 0) {
                            Vis88 *b;

                            b = vis_table_b.entries;
                            func_80016508();
                            viewer.unk2D6 = viewer.unk2D8 = viewer.unk2DA = viewer.unk2DC = 0;
                            func_800500ac(2, 4096, 4096, 256);
                            n = 240;
                            while (n--) {
                                if (b->unk05 == 236 && b->unk54 != -1) {
                                    func_8005850c(b->unk54, b->unk4A.snd.unk4A + 1108);
                                    b->unk54 = -1;
                                }
                                b++;
                            }
                            DAT_800881a4 = 2;
                            world_state.unk1EE15 = 0;
                            break;
                        }
                        func_8003e840();
                        ClearImage(&clear, 0, 0, 0);
                        DrawSync(0);
                        func_80058dc4(0, 1);
                        if (func_80062c28(1) == 1) {
                            Vis88 *b;

                            b = vis_table_b.entries;
                            n = 240;
                            while (n--) {
                                if (b->unk05 == 236 && b->unk54 != -1) {
                                    func_8005850c(b->unk54, b->unk4A.snd.unk4A + 1108);
                                    b->unk54 = -1;
                                }
                                b++;
                            }
                            func_80057a3c();
                            func_80058ed4();
                            func_80016508();
                            viewer.unk2D6 = viewer.unk2D8 = viewer.unk2DA = viewer.unk2DC = 0;
                            func_800500ac(2, 0, 4096, 256);
                            func_80015ad4();
                            func_8003c0e0();
                            vis_table_a.unk6168 = 254;
                            vis_table_a.unk6158[0] = vis_table_a.unk6158[1] =
                                vis_table_a.unk6158[2] = vis_table_a.unk6158[3] = 255;
                            func_80057a68(0);
                            func_8003c95c(area_state.unk4, area_state.unk4, area_state.unk4,
                                          area_state.unk7, area_state.unk8, 255);
                            func_80015ad4();
                            func_8003ead0();
                            break;
                        }
                        func_80057a3c();
                        func_80058ed4();
                        ClearImage(&clear, 0, 0, 0);
                        DrawSync(0);
                    }
                }
            }
            viewer.unk286++;
            break;
        case 16:
            func_80042900();
            func_80041d4c();
            func_800420a0();
            if (viewer.unk280.word == 0 && viewer.unk284 == 0) {
                func_80042c10();
            }
            func_8004162c();
            viewer.unk21E = 0;
            func_80042b9c();
            break;
        case 5:
        case 6:
            func_8003ead0();
            vis_table_b.unk71A4 = vis_table_b.unk71A6 = vis_table_b.unk71A8 = 0;
            vis_lists.unk47CE = vis_lists.unk47D0 = vis_lists.unk47D2 = 0;
            vis_table_a.unk6160 = vis_table_a.unk6162 = vis_table_a.unk6164 = 0;
            viewer.unk1CC = viewer.unk1CE = viewer.unk1D0 = 0;
            break;
        }
        {
            u16 hit[9];
            long from[4];
            u16 hit2[9];
            long from2[4];
            u16 hit3[9];
            long from3[4];

            if ((vis_lists.unk47DC & 0x10) && vis_lists.unk30 < 1601) {
                memset(hit, 0, sizeof(hit));
                memset(from, 0, sizeof(from));
                player_hit(hit, 4096, 7, from);
            }
            if ((vis_lists.unk47DC & 0x20) && vis_lists.unk30 < 1601) {
                memset(hit2, 0, sizeof(hit2));
                memset(from2, 0, sizeof(from2));
                player_hit(hit2, 4096, 1024, from2);
            }
            if ((vis_lists.unk47DC & 8) && vis_lists.unk30 < 1601) {
                memset(hit3, 0, sizeof(hit3));
                hit3[3] = 150;
                memset(from3, 0, sizeof(from3));
                player_hit(hit3, 4096, 0, from3);
            }
            if (viewer.unk190[0][0] != 0) {
                d = 0;
                if (viewer.unk2C & 4) {
                    d = -256;
                } else if (viewer.unk2C & 2) {
                    d = -64;
                } else if (viewer.unk2C & 1) {
                    d = -16;
                }
                if (viewer.unk190[0][0] % 20 == 0) {
                    player_add_hp(d);
                    func_800582f0(5, 128);
                }
            }
            if (viewer.unk190[4][0] != 0 && viewer.unk190[4][0] % 20 == 0) {
                player_add_hp(-1);
                player_add_mp(-1);
            }
            if (viewer.unk24 < viewer.unk22 && viewer.unk23C % 20 == 0) {
                player_add_hp(viewer.unk24 / 10 - viewer.unk22 / 10);
            }
            if ((viewer.unk2C & 0x400) || viewer.unk2F0) {
                if (viewer.unk23C % 20 == 0) {
                    d = (viewer.unkD8[3] - 256) / 4;
                    if (viewer.unk2C & 0x800000) {
                        player_add_hp(d / 2);
                    } else {
                        player_add_hp(d);
                    }
                }
                if (viewer.unk23C % 60 == 0) {
                    u16 *slots[9] = {&viewer.unk38[8], &viewer.unk38[9], &viewer.unk38[10],
                                     &viewer.unk38[3], &viewer.unk38[16], &viewer.unk38[2],
                                     &viewer.unk38[0], &viewer.unk38[13], NULL};

                    if (viewer.unk2F0) {
                        if (viewer.unk2C & 0x800000) {
                            for (i = 0; i < 8; i++) {
                                if (*slots[i] != 0xffff) {
                                    player_wear_item(1, slots[i]);
                                }
                            }
                        } else {
                            for (i = 0; i < 8; i++) {
                                if (*slots[i] != 0xffff) {
                                    player_wear_item(2, slots[i]);
                                }
                            }
                        }
                    } else if (viewer.unk2C & 0x800000) {
                        if (*slots[2] != 0xffff) {
                            player_wear_item(1, slots[2]);
                        }
                    } else if (*slots[2] != 0xffff) {
                        player_wear_item(2, slots[2]);
                    }
                }
                viewer.unk2C &= ~0x400;
            }
            player_update_shake();
            viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
            viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
            viewer.unk26C = viewer.unk278.vz + viewer.unk284 + viewer.unk28C + viewer.unk294;
            viewer.unk288 = viewer.unk28A = viewer.unk28C = 0;
            viewer.unk270.vx = 4096 - viewer.unk268;
            viewer.unk270.vy = 4096 - viewer.unk26A;
            viewer.unk270.vz = 4096 - viewer.unk26C;
            RotMatrixZYX(&viewer.unk270, &viewer.unk298);
            vis_lists.unk47CE /= 2;
            vis_lists.unk47D0 /= 2;
            vis_lists.unk47D2 /= 2;
            if (vis_lists.unk47CE >= -4 && vis_lists.unk47CE < 4) {
                vis_lists.unk47CE = 0;
            }
            if (vis_lists.unk47D0 >= -4 && vis_lists.unk47D0 < 4) {
                vis_lists.unk47D0 = 0;
            }
            if (vis_lists.unk47D2 >= -4 && vis_lists.unk47D2 < 4) {
                vis_lists.unk47D2 = 0;
            }
            func_800402ac(0);
            func_800402ac(1);
            func_80040024(0);
            func_80040024(1);
            if (viewer.unk218 != 4) {
                if (viewer.unk2 * 2 < viewer.unk0 && viewer.unk1C4 && viewer.unk23C % viewer.unk1C4 == 0) {
                    player_add_hp(1);
                }
                if (viewer.unk6 * 2 < viewer.unk4 && viewer.unk1C5 && viewer.unk23C % viewer.unk1C5 == 0) {
                    player_add_mp(1);
                }
            }
            viewer.unk23C++;
        }
    }
}
