/*
 * ST.EXE player transitions, equipment wear and damage,
 * 0x8003c95c..0x8003e5e8. The constant and jump-table pools and private
 * screen-shake state remain together under the original -G8 profile.
 * The final player_hit claim precedes src/st/player_8003e5e8.c.
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

RODATA(0x80011c7c, 0x38)
JUMPTABLES(0x80011cfc, 0x40)

ADDRESS(0x8003c95c, 0x188)
void func_8003c95c(int r, int g, int b, int arg3, int arg4, int arg5)
{
    func_80015ad4();
    do {
        func_8001531c();
        func_800456f8();
    } while (area_state.unk0 != 0);
    area_state.unk4 = 99;
    area_state.unk5 = 99;
    area_state.unk6 = 99;
    area_state.unk7 = 99;
    area_state.unk8 = 99;
    func_80045044(r, g, b, 255, 255, 127, 127, 127, 0, 0, 0);
    do {
        func_8001531c();
        func_800456f8();
    } while (area_state.unk0 != 0);
    func_80015ad4();
    VSync(2);
    func_8003eaf8();
    lighting_load_defaults();
    func_80016508();
    lighting_build_matrices();
    func_800500ac(130, 4096, 4096, 0);
    func_80015ad4();
    area_state.unk18 = cd_state.unk4;
    func_80045044(255, 255, 255, arg3, arg4, 127, 127, 127, 0, 0, 0);
    lighting_clear_requests();
}

ADDRESS(0x8003cae4, 0xa4)
void func_8003cae4(void)
{
    int i;

    viewer.unk8 = 0;
    i = 14;
    while (i--) {
        viewer.unk9A[i] = 50;
        viewer.unk62[i] = 50;
        viewer.unk7E[i] = 0;
    }
    viewer.unk26 = 0;
    viewer.unk4 = 0;
    viewer.unk0 = 0;
    viewer.unk38[20] = 0xffff;
    viewer.unk38[19] = 0xffff;
    viewer.unk38[18] = 0xffff;
    viewer.unk38[4] = 0xffff;
    viewer.unk38[3] = 0xffff;
    viewer.unk38[2] = 0xffff;
    viewer.unk38[1] = 0xffff;
    viewer.unk38[0] = 0xffff;
    viewer.unk38[6] = 0xffff;
    viewer.unk38[11] = 0xffff;
    viewer.unk38[10] = 0xffff;
    viewer.unk38[9] = 0xffff;
    viewer.unk38[8] = 0xffff;
    viewer.unk38[7] = 0xffff;
    viewer.unk38[12] = 0xffff;
    viewer.unk38[5] = 0xffff;
    viewer.unk38[17] = 0xffff;
    viewer.unk38[16] = 0xffff;
    viewer.unk38[15] = 0xffff;
    viewer.unk38[14] = 0xffff;
    viewer.unk38[13] = 0xffff;
}

/*
 * Totals the bonuses of the equipped items (equipment slots other than 4..6,
 * 11, 12 and 15) into the viewer's derived stats.
 */
ADDRESS(0x8003cb88, 0x354)
void player_equipment_bonuses(void)
{
    int kind[4];
    int amount[3];
    int i;
    int j;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
    int a7;
    int a8;
    u16 *slot;
    Vis44 *item;

    slot = viewer.unk38;
    a1 = 0;
    a2 = 0;
    a3 = 0;
    a4 = 0;
    a5 = 0;
    a6 = 0;
    a7 = 0;
    a8 = 0;
    for (i = 0; i < 17; i++, slot++) {
        if (i >= 4 && i <= 6) {
            continue;
        }
        if (i == 11) {
            continue;
        }
        if (i == 12) {
            continue;
        }
        if (i == 15) {
            continue;
        }
        if (*slot == 0xffff) {
            continue;
        }
        item = &vis_table_c.unk0118[*slot];
        kind[0] = item->kind0;
        kind[1] = item->kind1;
        kind[2] = item->kind2;
        amount[0] = item->amount0;
        amount[1] = item->amount1;
        amount[2] = item->amount2;
        for (j = 0; j < 3; j++) {
            switch (kind[j]) {
            case 1:
                a1 += amount[j];
                if (a1 > 100) {
                    a1 = 100;
                }
                break;
            case 2:
                a2 += amount[j];
                if (a2 > 100) {
                    a2 = 100;
                }
                break;
            case 3:
                a3 += amount[j];
                if (a3 > 20) {
                    a3 = 20;
                }
                break;
            case 4:
                if (a4 >= 0) {
                    if (amount[j] == 1) {
                        a4 = 1;
                    } else if (amount[j] == 2) {
                        a4 = -1;
                    }
                }
                break;
            case 5:
                a5 += amount[j];
                if (a5 > 100) {
                    a5 = 100;
                }
                break;
            case 6:
                a6 += amount[j];
                if (a6 > 100) {
                    a6 = 100;
                }
                break;
            case 7:
                a7 += amount[j];
                if (a7 > 100) {
                    a7 = 100;
                }
                break;
            case 8:
                a8 = 1;
                break;
            }
        }
    }
    if (a1) {
        viewer.unk1C4 = 100 / a1;
    } else {
        viewer.unk1C4 = 0;
    }
    if (a2) {
        viewer.unk1C5 = 100 / a2;
    } else {
        viewer.unk1C5 = 0;
    }
    viewer.unk1C6 = a3 * 5;
    if (a4 < 0) {
        viewer.unk1C7 = -1;
    } else if (a4 > 0) {
        viewer.unk1C7 = 1;
    } else {
        viewer.unk1C7 = 0;
    }
    viewer.unk1C8 = 100 - a5 * 80 / 100;
    viewer.unk1C9 = a6;
    viewer.unk1CA = a7;
    viewer.unk1CB = a8;
}

ADDRESS(0x8003cedc, 0x4c)
void func_8003cedc(void)
{
    player_equipment_bonuses();
    func_8005a3a8(0);
    PLAYER_CLAMP_HP();
}

ADDRESS(0x8003cf28, 0x6c)
void player_add_hp(int delta)
{
    int hp;

    hp = viewer.unk2 + delta;
    if (hp <= 0) {
        hp = 0;
        player_kill(hp);
    } else if (viewer.unk0 < hp) {
        hp = viewer.unk0;
    }
    viewer.unk2 = hp;
}

ADDRESS(0x8003cf94, 0x48)
void player_add_mp(int delta)
{
    int mp;

    mp = viewer.unk6 + delta;
    if (mp <= 0) {
        mp = 0;
    } else if (viewer.unk4 < mp) {
        mp = viewer.unk4;
    }
    viewer.unk6 = mp;
}

ADDRESS(0x8003cfdc, 0x54)
void func_8003cfdc(int id)
{
    int v;

    v = vis_table_c.unk0118[id].unk28;
    if (v != 0) {
        if (v < 4) {
            viewer.unk2C |= 0x1000000;
        }
    }
}

/* Clears the durability-warning flag, then sets it again if any worn item
 * (weapons and armour slots) is nearly broken. */
ADDRESS(0x8003d030, 0xd4)
void func_8003d030(void)
{
    u16 *slots[10] = {
        &viewer.unk38[0], &viewer.unk38[2],  &viewer.unk38[3],  &viewer.unk38[8],
        &viewer.unk38[9], &viewer.unk38[10], &viewer.unk38[13], &viewer.unk38[16],
    };
    u16 **p;

    p = slots;
    viewer.unk2C &= ~0x1000000;
    for (; *p; p++) {
        if (**p != 0xffff) {
            func_8003cfdc(**p);
        }
    }
}

/* Takes `amount` durability from the item in equipment slot `slot`; a
 * broken item is unequipped (except in slots 0 and 13) and its model
 * dropped. */
ADDRESS(0x8003d104, 0x184)
void player_wear_item(int amount, u16 *slot)
{
    int left;

    if (*slot == 0xffff) {
        return;
    }
    left = vis_table_c.unk0118[*slot].unk28;
    if (left == 0) {
        return;
    }
    left -= amount;
    if (left <= 0) {
        func_800582f0(4, 128);
        vis_table_c.unk0118[*slot].unk28 = 0;
        if (slot != &viewer.unk38[0] && slot != &viewer.unk38[13]) {
            *slot = 0xffff;
        }
        if (slot == &viewer.unk38[3]) {
            if (viewer.unk148[0] >= 31 && viewer.unk148[0] <= 34) {
                func_8003f12c(0xffff, 1, 0);
            }
            func_8003f12c(0xffff, 0, 0);
        } else if (slot == &viewer.unk38[16]) {
            func_8003f12c(0xffff, 1, 0);
        } else if (slot == &viewer.unk38[4]) {
            func_8003f12c(0xffff, 2, 0);
        } else if (slot == &viewer.unk38[17]) {
            func_8003f12c(0xffff, 3, 0);
        }
        player_equipment_bonuses();
        func_8005a3a8(0);
        PLAYER_CLAMP_HP();
    } else {
        vis_table_c.unk0118[*slot].unk28 = left;
    }
    func_8003d030();
}

/* Wears the shield (slot 16) while it is raised, and the armour. */
ADDRESS(0x8003d288, 0x30c)
void player_wear_armour(int damage, u16 flags)
{
    u16 *armour[4] = {&viewer.unk38[8], &viewer.unk38[9], &viewer.unk38[10], &viewer.unk38[2]};
    int wear;
    int n;
    int shield;
    int pick;
    int i;
    u8 a;
    u8 b;

    if (viewer.unk189 != 0 && viewer.unk38[16] != 0xffff) {
        if (damage * 20 < viewer.unk0) {
            viewer.unk219++;
        } else {
            n = damage * 10 / viewer.unk0 + 1;
            if (n <= 0) {
                return;
            }
            viewer.unk219 += n * 2;
            viewer.unk21A += n;
        }
        a = viewer.unk219;
        b = viewer.unk21A;
        shield = a / 5;
        wear = b / 5;
        viewer.unk219 = a - shield * 5;
        viewer.unk21A = b - wear * 5;
        if ((flags & 0x400) && !(viewer.unk2C & 0x800000)) {
            shield <<= 1;
            wear <<= 1;
        }
        if (flags & 0x1000) {
            shield *= 5;
            wear *= 5;
        }
        player_wear_item(shield, &viewer.unk38[16]);
        pick = -1;
    } else {
        if (damage * 20 < viewer.unk0) {
            return;
        }
        n = damage * 10 / viewer.unk0 + 1;
        if (n <= 0) {
            return;
        }
        viewer.unk21A += n * 2;
        wear = viewer.unk21A / 5;
        viewer.unk21A -= wear * 5;
        if ((flags & 0x400) && !(viewer.unk2C & 0x800000)) {
            wear <<= 1;
        }
        if (flags & 0x1000) {
            wear *= 5;
        }
        pick = rand() * 5 >> 15;
        if (pick >= 5) {
            pick = 4;
        }
    }
    i = 3;
    do {
        if (*armour[i] != 0xffff) {
            if (i == pick)
                player_wear_item(wear * 2, armour[i]);
            else
                player_wear_item(wear, armour[i]);
        }
    } while (--i != -1);
}

/* The screen shake offsets player_update_shake adds to the view angles. */
DATA(0x800881d0, 0x4, ".sbss")
static short shake[2];

/* Steps the screen shake started by player_take_damage. */
ADDRESS(0x8003d594, 0x1f4)
void player_update_shake(void)
{
    if (viewer.unk2F4 < 0) {
        return;
    }
    if (viewer.unk2F4 % 3 == 0) {
        shake[0] = (rand() - 16384) >> ((30 - viewer.unk2F4) / 5 + 5);
        shake[1] = (rand() - 16384) >> ((30 - viewer.unk2F4) / 5 + 5);
    } else {
        shake[0] >>= 1;
        shake[1] >>= 1;
    }
    if (shake[0] < -2 || shake[0] > 2) {
        viewer.unk288 += shake[0];
    }
    if (shake[1] < -2 || shake[1] > 2) {
        viewer.unk28C += shake[1];
    }
    if (viewer.unk288 > 2048 && viewer.unk288 < 4032) {
        viewer.unk288 = 4032;
    }
    if (viewer.unk288 > 64 && viewer.unk288 < 2048) {
        viewer.unk288 = 64;
    }
    if (viewer.unk28C > 2048 && viewer.unk28C < 4032) {
        viewer.unk28C = 4032;
    }
    if (viewer.unk28C > 64 && viewer.unk28C < 2048) {
        viewer.unk28C = 64;
    }
    viewer.unk2F4--;
}

/* Takes `damage` hit points, pushes the viewer away from `from` and starts
 * the red fade and the screen shake. */
ADDRESS(0x8003d788, 0x458)
void player_take_damage(int damage, int flags, long *from)
{
    SVECTOR dir;
    SVECTOR push;
    int hp;
    int scaled;
    int reach;
    int amount;
    int random;
    int shake_amount;
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
        func_8004a390(viewer.position.vx - from[0], viewer.position.vy - from[1], viewer.position.vz - from[2], &dir);
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
    random = rand();
    shake_amount = amount - 16384;
    shake_amount += random;
    viewer.unk2E2 = shake_amount >> 9;
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
/* Not exact: status-cap products and damage sums still use different
 * registers. The shared row-0 cap has its own value before comparison. */
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
    int limit0;
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
        limit0 = limit600;
        if (limit300 < viewer.unk190[3][1]) {
            viewer.unk190[3][1] = limit300;
        }
        if (limit75 < viewer.unk190[1][1]) {
            viewer.unk190[1][1] = limit75;
        }
        if (limit600 < viewer.unk190[2][1]) {
            viewer.unk190[2][1] = limit600;
        }
        if (limit0 < viewer.unk190[0][1]) {
            viewer.unk190[0][1] = limit0;
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
