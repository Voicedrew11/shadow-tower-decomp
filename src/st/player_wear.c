/*
 * ST.EXE player equipment durability and the damage screen shake (WIP
 * module; part of src/st/player.c's translation unit).
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

RODATA(0x80011c7c, 0x38)

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
/* Not exact: `wear` and the table cursor swap $s1/$s2, and the armour
 * loop constants are set up in another order. */
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
    u16 **p;

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
    i = 4;
    p = &armour[3];
    while (i--) {
        if (**p != 0xffff) {
            player_wear_item(i != pick ? wear : wear * 2, *p);
        }
        p--;
    }
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
