/*
 * ST.EXE: the save-data image writer and reader and the area's per-frame
 * hook (WIP module, 0x800562d4..0x8005738c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/save.h>

#define BIT(i) (1 << ((i) % 32))

/* Fills the save image; `fresh` stores the start position and no area. */
ADDRESS(0x800562d4, 0x7cc)
void save_pack_data(StSaveData *save, int fresh)
{
    int i;
    GroundItem *e32;
    GroundItem *e48;
    ItemType *r;

    if (fresh)
        save->loaded_ids[0] = save->loaded_ids[1] = save->loaded_ids[2] = save->loaded_ids[3] = save->loaded_ids[4] = 0;
    else
        *(StBytes5 *)save->loaded_ids = ((StAreaBytes *)&area_state)->loaded_ids;
    copy_words(save->creatures_gone, game_progress.creatures_gone[0], 0x60);
    copy_words(save->ground_items_gone, game_progress.ground_items_gone[0], 0x30);
    copy_words(save->objects_removed, game_progress.objects_removed[0], 0x180);
    copy_words(save->objects_unlocked, game_progress.objects_unlocked[0], 0x180);
    copy_words(save->objects_finished, game_progress.objects_finished[0], 0x180);
    copy_words(save->event_flags, &game_progress.event_flags.word, 2);
    copy_words(save->talk_counts, (u32 *)game_progress.talk_counts, 12);
    copy_words(save->areas_visited, (u32 *)game_progress.areas_visited, 12);
    copy_words(save->creature_seen_bits, game_progress.creature_bits_and_vs_data, 5);
    save->ground_item_counts[0] = vis_table_c.ground_item_counts[2];
    save->ground_item_counts[1] = vis_table_c.ground_item_counts[3];
    save->ground_item_counts[2] = vis_table_c.ground_item_counts[4];
    save->ground_item_counts[3] = vis_table_c.ground_item_counts[5];
    copy_words(save->collected_bits, vis_table_c.collected_bits, 9);
    e32 = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++, e32++) {
        if (i < vis_table_c.ground_item_counts[2]) {
            save->ground_items[i].item = e32->item;
            save->ground_items[i].block = e32->block;
            save->ground_items[i].position[0] = e32->position.vx;
            save->ground_items[i].position[1] = e32->position.vy;
            save->ground_items[i].position[2] = e32->position.vz;
            save->ground_items[i].rotation[0] = e32->rot_x;
            save->ground_items[i].rotation[1] = e32->facing;
            save->ground_items[i].rotation[2] = e32->rot_z;
            save->ground_items[i].state = e32->state;
            save->ground_items[i].area = e32->area;
        }
    }
    e48 = &vis_table_c.entries[48];
    for (i = 0; i < 16; i++, e48++) {
        if (i < vis_table_c.ground_item_counts[3]) {
            save->ground_items[16 + i].item = e48->item;
            save->ground_items[16 + i].block = e48->block;
            save->ground_items[16 + i].position[0] = e48->position.vx;
            save->ground_items[16 + i].position[1] = e48->position.vy;
            save->ground_items[16 + i].position[2] = e48->position.vz;
            save->ground_items[16 + i].rotation[0] = e48->rot_x;
            save->ground_items[16 + i].rotation[1] = e48->facing;
            save->ground_items[16 + i].rotation[2] = e48->rot_z;
            save->ground_items[16 + i].state = e48->state;
            save->ground_items[16 + i].area = e48->area;
        }
    }
    save->status_flags = viewer.status_flags;
    save->play_seconds = viewer.play_seconds;
    save->hp = viewer.hp;
    save->mp = viewer.mp;
    save->sp_total = viewer.sp_total;
    save->creatures_killed = viewer.creatures_killed;
    save->unk1800 = viewer.unk26;
    save->cunes_found = viewer.cunes_found;
    save->items_found = viewer.items_found;
    save->unk1803 = viewer.unk32;
    copy_halfwords((u16 *)save->attributes, (u16 *)viewer.attributes, 14);
    copy_halfwords((u16 *)save->attribute_bonuses, (u16 *)viewer.attribute_bonuses, 14);
    copy_halfwords(save->base_attributes, viewer.base_attributes, 14);
    copy_words(save->save_point, (u32 *)&viewer.save_area, 3);
    save->status_timers[0][0] = viewer.unk190[0][0];
    save->status_timers[0][1] = viewer.unk190[0][1];
    save->status_timers[1][0] = viewer.unk190[1][0];
    save->status_timers[1][1] = viewer.unk190[1][1];
    save->status_timers[2][0] = viewer.unk190[2][0];
    save->status_timers[2][1] = viewer.unk190[2][1];
    save->status_timers[3][0] = viewer.unk190[3][0];
    save->status_timers[3][1] = viewer.unk190[3][1];
    save->status_timers[4][0] = viewer.unk190[4][0];
    save->status_timers[4][1] = viewer.unk190[4][1];
    save->status_timers[5][0] = viewer.unk190[5][0];
    save->status_timers[5][1] = viewer.unk190[5][1];
    save->status_timers[6][0] = viewer.unk190[6][0];
    save->status_timers[6][1] = viewer.unk190[6][1];
    save->status_timers[7][0] = viewer.unk190[7][0];
    save->status_timers[7][1] = viewer.unk190[7][1];
    save->status_timers[8][0] = viewer.unk190[8][0];
    save->status_timers[8][1] = viewer.unk190[8][1];
    save->status_timers[9][0] = viewer.unk190[9][0];
    save->status_timers[9][1] = viewer.unk190[9][1];
    save->status_timers[10][0] = viewer.unk190[10][0];
    save->status_timers[10][1] = viewer.unk190[10][1];
    save->status_timers[11][0] = viewer.unk190[11][0];
    save->status_timers[11][1] = viewer.unk190[11][1];
    save->status_timers[12][0] = viewer.unk190[12][0];
    save->status_timers[12][1] = viewer.unk190[12][1];
    save->wear[0] = viewer.shield_wear;
    save->wear[1] = viewer.armour_wear;
    save->wear[2] = viewer.weapon_wear;
    if (fresh) {
        save->position.vx = 0x3d000;
        save->position.vy = 0x405c0;
        save->position.vz = 0x3f000;
        save->view_rot.vx = 0;
        save->view_rot.vy = 3072;
        save->view_rot.vz = 0;
    } else {
        save->position = *(VECTOR *)&viewer.position.vx;
        save->view_rot = viewer.view_rot;
    }
    save->settings[0] = game_settings.bgm_volume;
    save->settings[1] = game_settings.sfx_volume;
    save->settings[2] = game_settings.walk_bob;
    save->settings[3] = game_settings.show_status;
    save->settings[4] = game_settings.subtitles;
    save->settings[5] = game_settings.vibration;
    if (game_progress.save_id == -1) {
        /* A save id from the vsync counter (low half) and rand() (high). */
        ((u16 *)&game_progress.save_id)[0] = VSync(-1);
        ((u16 *)&game_progress.save_id)[1] = rand();
    }
    save->save_id = game_progress.save_id;
    copy_words(save->creature_killed_bits, &game_progress.creature_bits_and_vs_data[5], 5);
    copy_words(save->vs_items_lost, &game_progress.creature_bits_and_vs_data[10], 0x4f);
    copy_words(save->inventory_counts, (u32 *)&inventory, 0x4f);
    copy_words(save->placed_bits, vis_table_c.placed_bits, 10);
    save->equipment[EQUIP_SLOT_RIGHT_RING] = viewer.equipment[EQUIP_SLOT_RIGHT_RING];
    save->equipment[EQUIP_SLOT_RIGHT_BRACELET] = viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET];
    save->equipment[EQUIP_SLOT_GLOVES] = viewer.equipment[EQUIP_SLOT_GLOVES];
    save->equipment[EQUIP_SLOT_RIGHT_HAND] = viewer.equipment[EQUIP_SLOT_RIGHT_HAND];
    save->equipment[EQUIP_SLOT_RIGHT_RESERVE] = viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE];
    save->equipment[EQUIP_SLOT_QUICK_1] = viewer.equipment[EQUIP_SLOT_QUICK_1];
    save->equipment[EQUIP_SLOT_RIGHT_MAGIC] = viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC];
    save->equipment[EQUIP_SLOT_AMULET] = viewer.equipment[EQUIP_SLOT_AMULET];
    save->equipment[EQUIP_SLOT_HELMET] = viewer.equipment[EQUIP_SLOT_HELMET];
    save->equipment[EQUIP_SLOT_ARMOR] = viewer.equipment[EQUIP_SLOT_ARMOR];
    save->equipment[EQUIP_SLOT_BOOTS] = viewer.equipment[EQUIP_SLOT_BOOTS];
    save->equipment[EQUIP_SLOT_QUICK_3] = viewer.equipment[EQUIP_SLOT_QUICK_3];
    save->equipment[EQUIP_SLOT_LEFT_MAGIC] = viewer.equipment[EQUIP_SLOT_LEFT_MAGIC];
    save->equipment[EQUIP_SLOT_LEFT_RING] = viewer.equipment[EQUIP_SLOT_LEFT_RING];
    save->equipment[EQUIP_SLOT_LEFT_BRACELET] = viewer.equipment[EQUIP_SLOT_LEFT_BRACELET];
    save->equipment[EQUIP_SLOT_GLOVES_MIRROR] = viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR];
    save->equipment[EQUIP_SLOT_LEFT_HAND] = viewer.equipment[EQUIP_SLOT_LEFT_HAND];
    save->equipment[EQUIP_SLOT_LEFT_RESERVE] = viewer.equipment[EQUIP_SLOT_LEFT_RESERVE];
    save->equipment[EQUIP_SLOT_QUICK_5] = viewer.equipment[EQUIP_SLOT_QUICK_5];
    save->equipment[EQUIP_SLOT_QUICK_2] = viewer.equipment[EQUIP_SLOT_QUICK_2];
    save->equipment[EQUIP_SLOT_QUICK_4] = viewer.equipment[EQUIP_SLOT_QUICK_4];
    save->vs_lineup[0] = game_progress.creature_bits_and_vs_data[89];
    save->vs_lineup[1] = game_progress.creature_bits_and_vs_data[90];
    save->vs_lineup[2] = game_progress.creature_bits_and_vs_data[91];
    r = vis_table_c.types;
    for (i = 0; i < 314; i++, r++)
        save->item_durability[i] = r->durability;
    copy_words(save->area_light_modes, (u32 *)lighting.area_mode, 24);
    copy_halfwords(save->shop_ids, inventory.shop_ids, 14);
    *(StHalves7 *)save->shop_stock = *(StHalves7 *)inventory.shop_stock;
    copy_halfwords(save->buttons, pad_state.buttons, 12);
}

/* Restores the state save_pack_data saved. */
ADDRESS(0x80056aa0, 0x8bc)
void save_unpack_data(StSaveData *save)
{
    int i;
    int area;
    int bit;
    GroundItem *entry;
    ItemType *r;

    ((StAreaBytes *)&area_state)->loaded_ids = *(StBytes5 *)save->loaded_ids;
    copy_words(game_progress.creatures_gone[0], save->creatures_gone, 0x60);
    copy_words(game_progress.ground_items_gone[0], save->ground_items_gone, 0x30);
    copy_words(game_progress.objects_removed[0], save->objects_removed, 0x180);
    copy_words(game_progress.objects_unlocked[0], save->objects_unlocked, 0x180);
    copy_words(game_progress.objects_finished[0], save->objects_finished, 0x180);
    copy_words(&game_progress.event_flags.word, save->event_flags, 2);
    copy_words((u32 *)game_progress.talk_counts, save->talk_counts, 12);
    copy_words((u32 *)game_progress.areas_visited, save->areas_visited, 12);
    copy_words(game_progress.creature_bits_and_vs_data, save->creature_seen_bits, 5);
    for (area = 0; area < 48; area++) {
        game_progress.area_kills[area] = 0;
        for (i = 0; i < 64; i++) {
            if ((game_progress.creatures_gone[area][i / 32] & BIT(i)) && i < 48)
                game_progress.area_kills[area]++;
        }
    }
    vis_table_c.ground_item_counts[2] = save->ground_item_counts[0];
    vis_table_c.ground_item_counts[3] = save->ground_item_counts[1];
    vis_table_c.ground_item_counts[4] = save->ground_item_counts[2];
    vis_table_c.ground_item_counts[5] = save->ground_item_counts[3];
    copy_words(vis_table_c.collected_bits, save->collected_bits, 9);
    entry = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++, entry++) {
        if (i < vis_table_c.ground_item_counts[2]) {
            entry->item = save->ground_items[i].item;
            entry->block = save->ground_items[i].block;
            entry->position.vx = save->ground_items[i].position[0];
            entry->position.vy = save->ground_items[i].position[1];
            entry->position.vz = save->ground_items[i].position[2];
            entry->rot_x = save->ground_items[i].rotation[0];
            entry->facing = save->ground_items[i].rotation[1];
            entry->rot_z = save->ground_items[i].rotation[2];
            entry->state = save->ground_items[i].state;
            entry->area = save->ground_items[i].area;
            r = &vis_table_c.types[entry->item];
            *(u32 *)entry->anim_slot = 0;
            entry->unk20 = r->flags;
            entry->prev_state = 0;
            entry->draw_flags = 0;
            entry->bounces = 0;
            entry->vertical_speed = 0;
            entry->depth_bias = r->unk5[1];
        } else {
            entry->item = 0xffff;
            entry->state = 0;
        }
    }
    entry = &vis_table_c.entries[48];
    for (i = 0; i < 16; i++, entry++) {
        if (i < vis_table_c.ground_item_counts[3]) {
            entry->item = save->ground_items[16 + i].item;
            entry->block = save->ground_items[16 + i].block;
            entry->position.vx = save->ground_items[16 + i].position[0];
            entry->position.vy = save->ground_items[16 + i].position[1];
            entry->position.vz = save->ground_items[16 + i].position[2];
            entry->rot_x = save->ground_items[16 + i].rotation[0];
            entry->facing = save->ground_items[16 + i].rotation[1];
            entry->rot_z = save->ground_items[16 + i].rotation[2];
            entry->state = save->ground_items[16 + i].state;
            entry->area = save->ground_items[16 + i].area;
            r = &vis_table_c.types[entry->item];
            *(u32 *)entry->anim_slot = 0;
            entry->unk20 = r->flags;
            entry->prev_state = 0;
            entry->draw_flags = 0;
            entry->bounces = 0;
            entry->vertical_speed = 0;
            entry->depth_bias = r->unk5[1];
        } else {
            entry->item = 0xffff;
            entry->state = 0;
        }
    }
    viewer.status_flags = save->status_flags;
    viewer.play_seconds = save->play_seconds;
    viewer.hp = save->hp;
    viewer.mp = save->mp;
    viewer.sp_total = save->sp_total;
    viewer.creatures_killed = save->creatures_killed;
    viewer.unk26 = save->unk1800;
    viewer.cunes_found = save->cunes_found;
    viewer.items_found = 0;
    for (bit = 0; bit < 266; bit++) {
        if (vis_table_c.collected_bits[bit / 32] & BIT(bit))
            viewer.items_found++;
    }
    viewer.unk32 = save->unk1803;
    copy_halfwords((u16 *)viewer.attributes, (u16 *)save->attributes, 14);
    copy_halfwords((u16 *)viewer.attribute_bonuses, (u16 *)save->attribute_bonuses, 14);
    copy_halfwords(viewer.base_attributes, save->base_attributes, 14);
    copy_words((u32 *)&viewer.save_area, save->save_point, 3);
    viewer.unk190[0][0] = save->status_timers[0][0];
    viewer.unk190[0][1] = save->status_timers[0][1];
    viewer.unk190[1][0] = save->status_timers[1][0];
    viewer.unk190[1][1] = save->status_timers[1][1];
    viewer.unk190[2][0] = save->status_timers[2][0];
    viewer.unk190[2][1] = save->status_timers[2][1];
    viewer.unk190[3][0] = save->status_timers[3][0];
    viewer.unk190[3][1] = save->status_timers[3][1];
    viewer.unk190[4][0] = save->status_timers[4][0];
    viewer.unk190[4][1] = save->status_timers[4][1];
    viewer.unk190[5][0] = save->status_timers[5][0];
    viewer.unk190[5][1] = save->status_timers[5][1];
    viewer.unk190[6][0] = save->status_timers[6][0];
    viewer.unk190[6][1] = save->status_timers[6][1];
    viewer.unk190[7][0] = save->status_timers[7][0];
    viewer.unk190[7][1] = save->status_timers[7][1];
    viewer.unk190[8][0] = save->status_timers[8][0];
    viewer.unk190[8][1] = save->status_timers[8][1];
    viewer.unk190[9][0] = save->status_timers[9][0];
    viewer.unk190[9][1] = save->status_timers[9][1];
    viewer.unk190[10][0] = save->status_timers[10][0];
    viewer.unk190[10][1] = save->status_timers[10][1];
    viewer.unk190[11][0] = save->status_timers[11][0];
    viewer.unk190[11][1] = save->status_timers[11][1];
    viewer.unk190[12][0] = save->status_timers[12][0];
    viewer.unk190[12][1] = save->status_timers[12][1];
    viewer.shield_wear = save->wear[0];
    viewer.armour_wear = save->wear[1];
    viewer.weapon_wear = save->wear[2];
    viewer.position = save->position;
    viewer.view_rot = save->view_rot;
    game_settings.bgm_volume = save->settings[0];
    game_settings.sfx_volume = save->settings[1];
    game_settings.walk_bob = save->settings[2];
    game_settings.show_status = save->settings[3];
    game_settings.subtitles = save->settings[4];
    game_settings.vibration = save->settings[5];
    game_progress.save_id = save->save_id;
    copy_words(&game_progress.creature_bits_and_vs_data[5], save->creature_killed_bits, 5);
    copy_words(&game_progress.creature_bits_and_vs_data[10], save->vs_items_lost, 0x4f);
    copy_words((u32 *)&inventory, save->inventory_counts, 0x4f);
    copy_words(vis_table_c.placed_bits, save->placed_bits, 10);
    viewer.equipment[EQUIP_SLOT_RIGHT_RING] = save->equipment[EQUIP_SLOT_RIGHT_RING];
    viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET] = save->equipment[EQUIP_SLOT_RIGHT_BRACELET];
    viewer.equipment[EQUIP_SLOT_GLOVES] = save->equipment[EQUIP_SLOT_GLOVES];
    viewer.equipment[EQUIP_SLOT_RIGHT_HAND] = save->equipment[EQUIP_SLOT_RIGHT_HAND];
    viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] = save->equipment[EQUIP_SLOT_RIGHT_RESERVE];
    viewer.equipment[EQUIP_SLOT_QUICK_1] = save->equipment[EQUIP_SLOT_QUICK_1];
    viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC] = save->equipment[EQUIP_SLOT_RIGHT_MAGIC];
    viewer.equipment[EQUIP_SLOT_AMULET] = save->equipment[EQUIP_SLOT_AMULET];
    viewer.equipment[EQUIP_SLOT_HELMET] = save->equipment[EQUIP_SLOT_HELMET];
    viewer.equipment[EQUIP_SLOT_ARMOR] = save->equipment[EQUIP_SLOT_ARMOR];
    viewer.equipment[EQUIP_SLOT_BOOTS] = save->equipment[EQUIP_SLOT_BOOTS];
    viewer.equipment[EQUIP_SLOT_QUICK_3] = save->equipment[EQUIP_SLOT_QUICK_3];
    viewer.equipment[EQUIP_SLOT_LEFT_MAGIC] = save->equipment[EQUIP_SLOT_LEFT_MAGIC];
    viewer.equipment[EQUIP_SLOT_LEFT_RING] = save->equipment[EQUIP_SLOT_LEFT_RING];
    viewer.equipment[EQUIP_SLOT_LEFT_BRACELET] = save->equipment[EQUIP_SLOT_LEFT_BRACELET];
    viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR] = save->equipment[EQUIP_SLOT_GLOVES_MIRROR];
    viewer.equipment[EQUIP_SLOT_LEFT_HAND] = save->equipment[EQUIP_SLOT_LEFT_HAND];
    viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] = save->equipment[EQUIP_SLOT_LEFT_RESERVE];
    viewer.equipment[EQUIP_SLOT_QUICK_5] = save->equipment[EQUIP_SLOT_QUICK_5];
    viewer.equipment[EQUIP_SLOT_QUICK_2] = save->equipment[EQUIP_SLOT_QUICK_2];
    viewer.equipment[EQUIP_SLOT_QUICK_4] = save->equipment[EQUIP_SLOT_QUICK_4];
    game_progress.creature_bits_and_vs_data[89] = save->vs_lineup[0];
    game_progress.creature_bits_and_vs_data[90] = save->vs_lineup[1];
    game_progress.creature_bits_and_vs_data[91] = save->vs_lineup[2];
    r = vis_table_c.types;
    for (i = 0; i < 314; i++, r++)
        r->durability = save->item_durability[i];
    copy_words((u32 *)lighting.area_mode, save->area_light_modes, 24);
    copy_halfwords(inventory.shop_ids, save->shop_ids, 14);
    *(StHalves7 *)inventory.shop_stock = *(StHalves7 *)save->shop_stock;
    copy_halfwords(pad_state.buttons, save->buttons, 12);
}

ADDRESS(0x8005735c, 0x30)
void area_run_frame_hook(void)
{
    area_state.hooks->frame();
}
