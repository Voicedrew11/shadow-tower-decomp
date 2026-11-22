/*
 * ST.EXE: using an item (table C ids 266..313): status cures and boosts,
 * keys tried on the table B entries in reach, repairs, and the area's own
 * item handler (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/sound.h>
#include <st/card.h>
#include <st/menu.h>
#include <st/player.h>
#include <st/item.h>
#include <st/vis.h>

RODATA(0x8001274c, 0x108)

/* The use result: whether the item was used (1) or not (2), a member of
 * the save-game state block. */
#define USE_RESULT game_progress.item_use_result

/* Uses item `id` and leaves the use result. */
ADDRESS(0x80054a54, 0x848)
void item_use(int id)
{
    VECTOR pos;
    int i;
    int points;

    pos = viewer.position;
    USE_RESULT = 2;
    pos.vy += 1600;
    switch (id) {
    case 0x10a:
    case 0x10b:
        break;
    case 0x10c:
        if (!(viewer.status_flags & 0x30000)) {
            USE_RESULT = 1;
            viewer.unk190[5][1] = 3000;
            viewer.unk190[5][0] = 0;
            viewer.status_flags |= 0x10000;
        }
        break;
    case 0x10d:
        if (!(viewer.status_flags & 0x30000)) {
            USE_RESULT = 1;
            viewer.unk190[5][1] = 18000;
            viewer.unk190[5][0] = 0;
            viewer.status_flags |= 0x20000;
        }
        break;
    case 0x10e:
        if (area_state.area != 30 && viewer.save_area != 0xff) {
            viewer.save_warp_request = 1;
            viewer.submerged_acid = 0;
            viewer.submerged = 0;
            USE_RESULT = 1;
            area_state.hooks->item_used(id);
        }
        break;
    case 0x110:
    case 0x111:
    case 0x112:
    case 0x113:
    case 0x114:
    case 0x115:
    case 0x116:
    case 0x117:
    case 0x118:
        i = 0;
        vis_lists.position = pos;
        vis_lists.radius = 1056;
        vis_lists.height = 1600;
        vis_lists.velocity.vx = 0;
        vis_lists.velocity.vy = 0;
        vis_lists.velocity.vz = 0;
        vis_lists.speed = 0;
        vis_lists.heading = 0;
        vis_lists.tests = 0;
        while ((i = object_find_usable(i, &vis_lists)) != -1) {
            switch (object_try_key(&vis_table_b.entries[i], id)) {
            case 0:
                USE_RESULT = 2;
                goto done;
            case 1:
                if (id == 0x114)
                    area_state.hooks->item_used(i | 0x4000);
                else
                    play_unlock_sound();
                USE_RESULT = 1;
                if (id == 0x114 || id == 0x116)
                    goto used;
                goto done;
            case 3:
                play_cancel_sound();
                USE_RESULT = 1;
                goto done;
            }
            i++;
        }
        break;
    case 0x11c:
        if (viewer.hp < viewer.max_hp) {
            viewer.hp = viewer.max_hp;
            USE_RESULT = 1;
            sound_play(8, 128);
        }
        break;
    case 0x11d:
        if (viewer.mp < viewer.max_mp) {
            viewer.mp = viewer.max_mp;
            USE_RESULT = 1;
            sound_play(8, 128);
        }
        break;
    case 0x11e:
        if (viewer.status_flags & 7) {
            viewer.unk190[0][1] = 0;
            viewer.unk190[0][0] = 0;
            viewer.status_flags &= ~7;
            USE_RESULT = 1;
            sound_play(8, 128);
        }
        break;
    case 0x11f:
        if (viewer.status_flags & 0x18) {
            viewer.unk190[1][1] = 0;
            viewer.unk190[1][0] = 0;
            viewer.status_flags &= ~0x18;
            USE_RESULT = 1;
            sound_play(8, 128);
        }
        break;
    case 0x120:
        if (viewer.status_flags & 0x100) {
            viewer.unk190[4][1] = 0;
            viewer.unk190[4][0] = 0;
            viewer.status_flags &= ~0x100;
            USE_RESULT = 1;
            sound_play(8, 128);
        }
        break;
    case 0x122:
        if (!(viewer.status_flags & 0x40000)) {
            viewer.unk190[6][1] = 300;
            viewer.unk190[6][0] = 0;
            viewer.status_flags |= 0x40000;
            USE_RESULT = 1;
        }
        break;
    case 0x123:
        if (!(viewer.status_flags & 0x80000)) {
            viewer.unk190[7][1] = 3000;
            viewer.unk190[7][0] = 0;
            viewer.status_flags |= 0x80000;
            USE_RESULT = 1;
        }
        break;
    case 0x124:
        if (!(viewer.status_flags & 0x100000)) {
            viewer.unk190[8][1] = 600;
            viewer.unk190[8][0] = 0;
            viewer.status_flags |= 0x100000;
            USE_RESULT = 1;
        }
        break;
    case 0x125:
        if (!(viewer.status_flags & 0x200000)) {
            viewer.unk190[9][1] = 300;
            viewer.unk190[9][0] = 0;
            viewer.status_flags |= 0x200000;
            USE_RESULT = 1;
        }
        break;
    case 0x126:
        if (!(viewer.status_flags & 0x400000)) {
            viewer.unk190[10][1] = 6000;
            viewer.unk190[10][0] = 0;
            viewer.status_flags |= 0x400000;
            USE_RESULT = 1;
        }
        break;
    case 0x127:
        if (!(viewer.status_flags & 0x800000)) {
            viewer.unk190[11][1] = 3000;
            viewer.unk190[11][0] = 0;
            viewer.status_flags |= 0x800000;
            USE_RESULT = 1;
        }
        break;
    case 0x128:
        if (!(viewer.status_flags & 0x800)) {
            viewer.unk190[12][1] = 6000;
            viewer.unk190[12][0] = 0;
            viewer.status_flags |= 0x800;
            USE_RESULT = 1;
        }
        break;
    case 0x12e: {
        u16 *ids[12] = {
            &viewer.equipment[EQUIP_SLOT_RIGHT_RING], &viewer.equipment[EQUIP_SLOT_GLOVES],
                &viewer.equipment[EQUIP_SLOT_RIGHT_HAND], &viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE],
            &viewer.equipment[EQUIP_SLOT_HELMET], &viewer.equipment[EQUIP_SLOT_ARMOR],
                &viewer.equipment[EQUIP_SLOT_BOOTS], &viewer.equipment[EQUIP_SLOT_LEFT_RING],
            &viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR], &viewer.equipment[EQUIP_SLOT_LEFT_HAND],
                &viewer.equipment[EQUIP_SLOT_LEFT_RESERVE], NULL,
        };
        u16 **p;
        ItemType *c;
        int n;
        int base;

        /* 10 to 30 percent of each item's maximum, by how full both bars are */
        base = viewer.mp * 10 / viewer.max_mp + 10;
        points = viewer.hp * 10 / viewer.max_hp + base;
        for (p = ids; *p != NULL; p++) {
            if (**p != 0xffff) {
                c = &vis_table_c.types[**p];
                n = c->max_durability * points / 100;
                if (n == 0)
                    n = 1;
                if (c->durability + n > c->max_durability)
                    c->durability = c->max_durability;
                else
                    c->durability += n;
                USE_RESULT = 1;
            }
        }
        player_update_durability_warning();
        break;
    }
    case 0x129:
    case 0x12a:
    case 0x12b:
    case 0x12f:
    case 0x130:
        area_state.hooks->item_used(id);
        break;
    case 0x131:
        area_state.hooks->item_used(id);
        goto done;
    case 0x136:
        play_confirm_sound();
        USE_RESULT = menu_attribute_points_run(5);
        break;
    case 0x137:
        play_confirm_sound();
        USE_RESULT = menu_attribute_points_run(53);
        break;
    case 0x138:
        play_confirm_sound();
        USE_RESULT = menu_attribute_points_run(29);
        break;
    case 0x139:
        play_confirm_sound();
        USE_RESULT = menu_attribute_points_run(14);
        break;
    }
used:
    if (USE_RESULT == 1)
        item_take(id);
done:
    player_stop_motion();
    {
        /* the quick-use slots drop an item used up */
        u16 *slots[5] = {
            &viewer.equipment[EQUIP_SLOT_QUICK_1], &viewer.equipment[EQUIP_SLOT_QUICK_2],
                &viewer.equipment[EQUIP_SLOT_QUICK_3], &viewer.equipment[EQUIP_SLOT_QUICK_4],
            &viewer.equipment[EQUIP_SLOT_QUICK_5],
        };
        int slot;

        for (slot = 0; slot < 5; slot++) {
            if (*slots[slot] == id && !inventory.counts[*slots[slot]].count)
                *slots[slot] = 0xffff;
        }
    }
}
