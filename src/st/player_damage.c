/*
 * ST.EXE player damage, resets and the item slots (WIP module; part of
 * src/st/player.c's translation unit).
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
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/player.h>
#include <st/scratch.h>
#include <st/vis.h>

RODATA(0x80011d1c, 0x20)
SDATA(0x8008802c, 0x10)

/* Takes `damage` hit points, pushes the viewer away from `from` and starts
 * the red fade and the screen shake. */
/* Not exact: unk2E2's sum adds the constant to rand() rather than to
 * `amount` (register choice only). */
ADDRESS(0x8003d788, 0x458)
void player_take_damage(int damage, int flags, long *from)
{
    SVECTOR dir;
    SVECTOR push;
    int hp;
    int scaled;
    int reach;
    int amount;
    short time;

    if (viewer.unk218 == 4 || damage == 0) {
        return;
    }
    player_wear_armour(damage, flags);
    hp = viewer.unk2 - damage;
    if (hp <= 0) {
        hp = 0;
    }
    viewer.unk2 = hp;
    PLAYER_CLAMP_HP();
    scaled = (damage << 12) / viewer.unk0;
    if (scaled > 950) {
        scaled = 950;
    }
    reach = scaled >> 2;
    if (from) {
        func_8004a390(viewer.x - from[0], viewer.y - from[1], viewer.z - from[2], &dir);
        if (reach < 80) {
            reach = 80;
        }
        if (reach > 450) {
            reach = 450;
        }
        func_80049f08(&dir, &push);
        func_8004a0f8(reach, &push);
        if (push.vy > 200) {
            push.vy = 200;
        } else if (push.vy < -200) {
            push.vy = -200;
        }
    } else {
        push.vx = push.vy = push.vz = 0;
    }
    time = (1300 - (scaled >> 1)) >> 2;
    if (time < 71) {
        time = 70;
    }
    if (viewer.unk1E) {
        viewer.unk2F4 = (damage * 5000 / viewer.unk0 + 500) / viewer.unk1E;
    } else {
        viewer.unk2F4 = 30;
    }
    if (viewer.unk2F4 < 0) {
        viewer.unk2F4 = -1;
    }
    if (viewer.unk2F4 > 30) {
        viewer.unk2F4 = 30;
    }
    func_80057a68(damage * 255 / viewer.unk0);
    amount = damage * 255 / viewer.unk0 + 32;
    if (amount > 128) {
        amount = 128;
    }
    viewer.unk2E2 = (rand() - 16384 + amount) >> 9;
    viewer.unk2DE += amount + (rand() >> 9);
    if (hp == 0) {
        player_kill((int)&push);
        viewer.unk2D6 = 3500;
        viewer.unk2D8 = time;
    } else if (from == NULL) {
        viewer.unk2D6 = 3500;
        viewer.unk2D8 = time;
    } else {
        switch ((dir.vy - viewer.unk26A + 256) >> 9 & 7) {
        case 0:
            dir.vx = -scaled;
            dir.vz = 0;
            break;
        case 1:
            dir.vx = -scaled;
            dir.vz = scaled;
            break;
        case 2:
            dir.vx = 0;
            dir.vz = scaled;
            break;
        case 3:
            dir.vx = scaled;
            dir.vz = scaled;
            break;
        case 4:
            dir.vx = scaled;
            dir.vz = 0;
            break;
        case 5:
            dir.vx = scaled;
            dir.vz = -scaled;
            break;
        case 6:
            dir.vx = 0;
            dir.vz = -scaled;
            break;
        case 7:
            dir.vx = -scaled;
            dir.vz = -scaled;
            break;
        }
        dir.vy = 0;
        func_80042c44(&push, &dir, time, damage);
    }
}

/*
 * Applies a hit: rolls the status effect against the viewer's resistance,
 * caps the status times, then turns the three physical and six elemental
 * damage values into hit points through the defence values.
 */
/* Not exact (8 bytes short): register allocation of the status-time
 * products and of the two damage sums. */
ADDRESS(0x8003dbe0, 0xa08)
void player_hit(u16 *damage, u16 scale, u16 effect, long *from)
{
    u16 bonus[6];
    int physical;
    int elemental;
    u16 *defence;
    u16 *extra;
    int k300;
    int k75;
    int k600;
    int resist;
    int base;
    int limit300;
    int limit75;
    int limit600;
    u16 attack;
    u16 magic;
    Vis44 *shield;
    int k;
    int d;
    int v;
    int i;

    physical = elemental = 0;
    defence = viewer.unkD8;
    memset(bonus, 0, sizeof(bonus));
    extra = bonus;
    if (effect != 0) {
        switch (effect) {
        case 8:
        case 16:
        case 24:
        case 32:
        case 64:
        case 96:
        case 128:
        case 256:
            resist = (u16)(viewer.unkB6 / 20) + viewer.unk62[13] * 3 / 80 + viewer.unkD8[5] * 60 / 255;
            if (101 - resist < rand() * 100 / 32768) {
                goto limits;
            }
            break;
        case 7:
            resist = (u16)(viewer.unkB6 / 20) + viewer.unk62[12] * 3 / 80 + viewer.unkD8[2] * 60 / 255;
            if (101 - resist < rand() * 100 / 32768) {
                goto limits;
            }
            break;
        }
        switch (effect) {
        case 128:
            if (!(viewer.unk2C & 0x80)) {
                viewer.unk190[3][1] = 300;
                viewer.unk190[3][0] = 0;
                viewer.unk2C |= 0x80;
                func_8005a3a8(0);
                PLAYER_CLAMP_HP();
            } else {
                viewer.unk190[3][0] = 20;
                viewer.unk190[3][1] = 300;
            }
            break;
        case 8:
        case 16:
        case 24:
            if (!(viewer.unk2C & 0x18)) {
                viewer.unk190[1][1] = 75;
                viewer.unk190[1][0] = 0;
                viewer.unk2C |= 8;
            } else if (viewer.unk2C & 8) {
                viewer.unk190[1][1] = 75;
                viewer.unk190[1][0] = 0;
                viewer.unk2C |= 0x10;
            } else {
                viewer.unk190[1][0] = 20;
                viewer.unk190[1][1] = 75;
            }
            break;
        case 32:
        case 64:
        case 96:
            if (!(viewer.unk2C & 0x60)) {
                viewer.unk190[2][1] = 600;
                viewer.unk190[2][0] = 0;
                viewer.unk2C |= 0x60;
            } else {
                viewer.unk190[2][0] = 20;
                viewer.unk190[2][1] = 600;
            }
            break;
        case 512:
            player_add_hp(-(rand() / 256));
            func_800582f0(9, 128);
            break;
        case 256:
            if (!(viewer.unk2C & 0x100)) {
                viewer.unk190[4][1] = 6000;
                viewer.unk190[4][0] = 0;
                viewer.unk2C |= 0x100;
                func_8005a3a8(0);
                PLAYER_CLAMP_HP();
            } else {
                viewer.unk190[4][0] = 20;
                viewer.unk190[4][1] = 6000;
            }
            break;
        case 1:
        case 2:
        case 4:
        case 7:
            if (!(viewer.unk2C & 0x100000)) {
                if (viewer.unk2C & 7) {
                    if (viewer.unk190[0][0] >= 220 && viewer.unk190[0][1] != 0) {
                        viewer.unk190[0][0] = 20;
                        viewer.unk190[0][1] = 600;
                        if (viewer.unk2C & 2) {
                            viewer.unk2C |= 4;
                        }
                        if (viewer.unk2C & 1) {
                            viewer.unk2C |= 2;
                        }
                    }
                } else {
                    viewer.unk190[0][1] = 600;
                    viewer.unk190[0][0] = 0;
                    viewer.unk2C |= 1;
                }
            }
            break;
        case 1024:
            viewer.unk2C |= 0x400;
            if (viewer.unk23C % 40 == 0) {
                func_800582f0(7, 128);
            }
            viewer.unk2DA = 3500;
            viewer.unk2DC = 128;
            break;
        case 16384:
            viewer.unk189 = 0;
            viewer.unk14F = 1;
            break;
        }
    limits:
        k300 = 300;
        k75 = 75;
        k600 = 600;
        base = (112 - viewer.unk20) * 50 / 59 + 50;
        limit300 = base * k300 / 100;
        limit75 = base * k75 / 100;
        limit600 = base * k600 / 100;
        limit300 = limit300 * viewer.unk1C8 / 100;
        limit75 = limit75 * viewer.unk1C8 / 100;
        limit600 = limit600 * viewer.unk1C8 / 100;
        if (limit300 < viewer.unk190[3][1]) {
            viewer.unk190[3][1] = limit300;
        }
        if (limit75 < viewer.unk190[1][1]) {
            viewer.unk190[1][1] = limit75;
        }
        if (limit600 < viewer.unk190[2][1]) {
            viewer.unk190[2][1] = limit600;
        }
        if (limit600 < viewer.unk190[0][1]) {
            viewer.unk190[0][1] = limit600;
        }
    }
    if (viewer.unk189 != 0 && viewer.unk38[16] != 0xffff) {
        attack = viewer.unk12 + viewer.unk16;
        shield = &vis_table_c.unk0118[viewer.unk38[16]];
        magic = viewer.unk14 + viewer.unk18;
        if (shield->kind2 >= 9) {
            bonus[shield->kind2 - 9] = shield->amount2 >> 1;
        }
    } else {
        attack = viewer.unk12;
        magic = viewer.unk14;
    }
    k = attack ? attack : 1;
    for (i = 0; i < 3; i++, damage++) {
        if (*damage) {
            d = *damage;
            physical += d * 32500 / (k * (650 - d));
        }
    }
    for (i = 0; i < 6; i++, damage++, defence++, extra++) {
        if (*damage == 0) {
            continue;
        }
        d = *damage;
        if ((viewer.unk2C & 0x80000) && i == 0) {
            d >>= 2;
        }
        if ((viewer.unk2C & 0x800000) && i == 3) {
            d >>= 2;
        }
        if ((viewer.unk2C & 0x100000) && i == 2) {
            continue;
        }
        k = magic + *defence + *extra;
        if (k == 0) {
            k = 1;
        }
        v = d * 32500 / (k * (650 - d));
        if (i != 2 && v == 0) {
            v = 1;
        }
        elemental += v;
    }
    player_take_damage((physical + elemental) * scale >> 12, effect, from);
}

/* Hits the viewer with `damage` when it is within `range` of `pos`, scaled
 * down with the distance. */
ADDRESS(0x8003e5e8, 0x14c)
void func_8003e5e8(u16 *damage, u16 effect, long *pos, int min, int range, int kind)
{
    int d;

    if (kind == 0x8000) {
        d = func_8004a67c(pos, range, &viewer.x, 256, 1000);
    } else if (kind == 0x8001) {
        if (viewer.y > pos[1]) {
            d = -9999999;
        } else {
            d = func_8004a67c(pos, range, &viewer.x, 256, 1000);
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
    pos->vx = viewer.x;
    pos->vz = viewer.z;
    pos->vy = viewer.y + viewer.unk2CE + viewer.unk2D2 - 1600;
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
    viewer.x = 0x3d000;
    viewer.y = 0x405c0;
    viewer.z = 0x3f000;
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
int func_8003eb34(long *pos, int angle, int range, int limit)
{
    int d;
    short a;

    d = func_8003ebfc(pos[0], 0xffff, pos[2], range, 0);
    if (d != -1) {
        a = (func_8004a1b4(viewer.x - pos[0], viewer.z - pos[2]) - angle) & 0xfff;
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
    return func_8004a580(&viewer.x, x, y, z, range, 1000, kind);
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
    Data8018e318 *rec;
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
        rec = DAT_8018e318;
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
        viewer.unk118[slot] = &DAT_8018d298[item];
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
