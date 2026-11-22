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
        save->unk4[0] = save->unk4[1] = save->unk4[2] = save->unk4[3] = save->unk4[4] = 0;
    else
        *(StBytes5 *)save->unk4 = ((StAreaBytes *)&area_state)->unk4;
    copy_words(save->unkC, game_progress.unk0[0], 0x60);
    copy_words(save->unk18C, game_progress.unk180[0], 0x30);
    copy_words(save->unk24C, game_progress.unk240[0], 0x180);
    copy_words(save->unk84C, game_progress.unk840[0], 0x180);
    copy_words(save->unkE4C, game_progress.unkE40[0], 0x180);
    copy_words(save->unk144C, &game_progress.unk1440.word, 2);
    copy_words(save->unk1454, (u32 *)game_progress.unk1448, 12);
    copy_words(save->unk1484, (u32 *)game_progress.unk1478, 12);
    copy_words(save->unk14B4, game_progress.unk14A8, 5);
    save->unk14C8[0] = vis_table_c.ground_item_counts[2];
    save->unk14C8[1] = vis_table_c.ground_item_counts[3];
    save->unk14C8[2] = vis_table_c.ground_item_counts[4];
    save->unk14C8[3] = vis_table_c.ground_item_counts[5];
    copy_words(save->unk14CC, vis_table_c.collected_bits, 9);
    e32 = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++, e32++) {
        if (i < vis_table_c.ground_item_counts[2]) {
            save->unk14F0[i].unk00 = e32->item;
            save->unk14F0[i].unk02 = e32->block;
            save->unk14F0[i].unk04[0] = e32->position.vx;
            save->unk14F0[i].unk04[1] = e32->position.vy;
            save->unk14F0[i].unk04[2] = e32->position.vz;
            save->unk14F0[i].unk10[0] = e32->rot_x;
            save->unk14F0[i].unk10[1] = e32->facing;
            save->unk14F0[i].unk10[2] = e32->rot_z;
            save->unk14F0[i].unk16 = e32->state;
            save->unk14F0[i].unk17 = e32->area;
        }
    }
    e48 = &vis_table_c.entries[48];
    for (i = 0; i < 16; i++, e48++) {
        if (i < vis_table_c.ground_item_counts[3]) {
            save->unk14F0[16 + i].unk00 = e48->item;
            save->unk14F0[16 + i].unk02 = e48->block;
            save->unk14F0[16 + i].unk04[0] = e48->position.vx;
            save->unk14F0[16 + i].unk04[1] = e48->position.vy;
            save->unk14F0[16 + i].unk04[2] = e48->position.vz;
            save->unk14F0[16 + i].unk10[0] = e48->rot_x;
            save->unk14F0[16 + i].unk10[1] = e48->facing;
            save->unk14F0[16 + i].unk10[2] = e48->rot_z;
            save->unk14F0[16 + i].unk16 = e48->state;
            save->unk14F0[16 + i].unk17 = e48->area;
        }
    }
    save->unk17F0 = viewer.status_flags;
    save->unk17F4 = viewer.play_seconds;
    save->unk17F8 = viewer.hp;
    save->unk17FA = viewer.mp;
    save->unk17FC = viewer.sp_total;
    save->unk17FE = viewer.creatures_killed;
    save->unk1800 = viewer.unk26;
    save->unk1801 = viewer.cunes_found;
    save->unk1802 = viewer.items_found;
    save->unk1803 = viewer.unk32;
    copy_halfwords((u16 *)save->unk1804, (u16 *)viewer.attributes, 14);
    copy_halfwords((u16 *)save->unk1820, (u16 *)viewer.attribute_bonuses, 14);
    copy_halfwords(save->unk183C, viewer.base_attributes, 14);
    copy_words(save->unk1870, (u32 *)&viewer.save_area, 3);
    save->unk187C[0][0] = viewer.unk190[0][0];
    save->unk187C[0][1] = viewer.unk190[0][1];
    save->unk187C[1][0] = viewer.unk190[1][0];
    save->unk187C[1][1] = viewer.unk190[1][1];
    save->unk187C[2][0] = viewer.unk190[2][0];
    save->unk187C[2][1] = viewer.unk190[2][1];
    save->unk187C[3][0] = viewer.unk190[3][0];
    save->unk187C[3][1] = viewer.unk190[3][1];
    save->unk187C[4][0] = viewer.unk190[4][0];
    save->unk187C[4][1] = viewer.unk190[4][1];
    save->unk187C[5][0] = viewer.unk190[5][0];
    save->unk187C[5][1] = viewer.unk190[5][1];
    save->unk187C[6][0] = viewer.unk190[6][0];
    save->unk187C[6][1] = viewer.unk190[6][1];
    save->unk187C[7][0] = viewer.unk190[7][0];
    save->unk187C[7][1] = viewer.unk190[7][1];
    save->unk187C[8][0] = viewer.unk190[8][0];
    save->unk187C[8][1] = viewer.unk190[8][1];
    save->unk187C[9][0] = viewer.unk190[9][0];
    save->unk187C[9][1] = viewer.unk190[9][1];
    save->unk187C[10][0] = viewer.unk190[10][0];
    save->unk187C[10][1] = viewer.unk190[10][1];
    save->unk187C[11][0] = viewer.unk190[11][0];
    save->unk187C[11][1] = viewer.unk190[11][1];
    save->unk187C[12][0] = viewer.unk190[12][0];
    save->unk187C[12][1] = viewer.unk190[12][1];
    save->unk18B0[0] = viewer.shield_wear;
    save->unk18B0[1] = viewer.armour_wear;
    save->unk18B0[2] = viewer.weapon_wear;
    if (fresh) {
        save->unk1858.vx = 0x3d000;
        save->unk1858.vy = 0x405c0;
        save->unk1858.vz = 0x3f000;
        save->unk1868.vx = 0;
        save->unk1868.vy = 3072;
        save->unk1868.vz = 0;
    } else {
        save->unk1858 = *(VECTOR *)&viewer.position.vx;
        save->unk1868 = viewer.view_rot;
    }
    save->unk18B4[0] = game_settings.bgm_volume;
    save->unk18B4[1] = game_settings.sfx_volume;
    save->unk18B4[2] = game_settings.walk_bob;
    save->unk18B4[3] = game_settings.show_status;
    save->unk18B4[4] = game_settings.subtitles;
    save->unk18B4[5] = game_settings.vibration;
    if (game_progress.unk16E0 == -1) {
        /* A save id from the vsync counter (low half) and rand() (high). */
        ((u16 *)&game_progress.unk16E0)[0] = VSync(-1);
        ((u16 *)&game_progress.unk16E0)[1] = rand();
    }
    save->unk0 = game_progress.unk16E0;
    copy_words(save->unk18BC, &game_progress.unk14A8[5], 5);
    copy_words(save->unk18D0, &game_progress.unk14A8[10], 0x4f);
    copy_words(save->unk1A0C, (u32 *)&inventory, 0x4f);
    copy_words(save->unk1B48, vis_table_c.placed_bits, 10);
    save->unk1B70[0] = viewer.equipment[0];
    save->unk1B70[1] = viewer.equipment[1];
    save->unk1B70[2] = viewer.equipment[2];
    save->unk1B70[3] = viewer.equipment[3];
    save->unk1B70[4] = viewer.equipment[4];
    save->unk1B70[5] = viewer.equipment[5];
    save->unk1B70[6] = viewer.equipment[6];
    save->unk1B70[7] = viewer.equipment[7];
    save->unk1B70[8] = viewer.equipment[8];
    save->unk1B70[9] = viewer.equipment[9];
    save->unk1B70[10] = viewer.equipment[10];
    save->unk1B70[11] = viewer.equipment[11];
    save->unk1B70[12] = viewer.equipment[12];
    save->unk1B70[13] = viewer.equipment[13];
    save->unk1B70[14] = viewer.equipment[14];
    save->unk1B70[15] = viewer.equipment[15];
    save->unk1B70[16] = viewer.equipment[16];
    save->unk1B70[17] = viewer.equipment[17];
    save->unk1B70[18] = viewer.equipment[18];
    save->unk1B70[19] = viewer.equipment[19];
    save->unk1B70[20] = viewer.equipment[20];
    save->unk1B9C[0] = game_progress.unk14A8[89];
    save->unk1B9C[1] = game_progress.unk14A8[90];
    save->unk1B9C[2] = game_progress.unk14A8[91];
    r = vis_table_c.types;
    for (i = 0; i < 314; i++, r++)
        save->unk1BA8[i] = r->durability;
    copy_words(save->unk1CE4, (u32 *)lighting.area_mode, 24);
    copy_halfwords(save->unk1D44, inventory.shop_ids, 14);
    *(StHalves7 *)save->unk1D60 = *(StHalves7 *)inventory.shop_stock;
    copy_halfwords(save->unk1D6E, pad_state.buttons, 12);
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

    ((StAreaBytes *)&area_state)->unk4 = *(StBytes5 *)save->unk4;
    copy_words(game_progress.unk0[0], save->unkC, 0x60);
    copy_words(game_progress.unk180[0], save->unk18C, 0x30);
    copy_words(game_progress.unk240[0], save->unk24C, 0x180);
    copy_words(game_progress.unk840[0], save->unk84C, 0x180);
    copy_words(game_progress.unkE40[0], save->unkE4C, 0x180);
    copy_words(&game_progress.unk1440.word, save->unk144C, 2);
    copy_words((u32 *)game_progress.unk1448, save->unk1454, 12);
    copy_words((u32 *)game_progress.unk1478, save->unk1484, 12);
    copy_words(game_progress.unk14A8, save->unk14B4, 5);
    for (area = 0; area < 48; area++) {
        game_progress.unk16E4[area] = 0;
        for (i = 0; i < 64; i++) {
            if ((game_progress.unk0[area][i / 32] & BIT(i)) && i < 48)
                game_progress.unk16E4[area]++;
        }
    }
    vis_table_c.ground_item_counts[2] = save->unk14C8[0];
    vis_table_c.ground_item_counts[3] = save->unk14C8[1];
    vis_table_c.ground_item_counts[4] = save->unk14C8[2];
    vis_table_c.ground_item_counts[5] = save->unk14C8[3];
    copy_words(vis_table_c.collected_bits, save->unk14CC, 9);
    entry = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++, entry++) {
        if (i < vis_table_c.ground_item_counts[2]) {
            entry->item = save->unk14F0[i].unk00;
            entry->block = save->unk14F0[i].unk02;
            entry->position.vx = save->unk14F0[i].unk04[0];
            entry->position.vy = save->unk14F0[i].unk04[1];
            entry->position.vz = save->unk14F0[i].unk04[2];
            entry->rot_x = save->unk14F0[i].unk10[0];
            entry->facing = save->unk14F0[i].unk10[1];
            entry->rot_z = save->unk14F0[i].unk10[2];
            entry->state = save->unk14F0[i].unk16;
            entry->area = save->unk14F0[i].unk17;
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
            entry->item = save->unk14F0[16 + i].unk00;
            entry->block = save->unk14F0[16 + i].unk02;
            entry->position.vx = save->unk14F0[16 + i].unk04[0];
            entry->position.vy = save->unk14F0[16 + i].unk04[1];
            entry->position.vz = save->unk14F0[16 + i].unk04[2];
            entry->rot_x = save->unk14F0[16 + i].unk10[0];
            entry->facing = save->unk14F0[16 + i].unk10[1];
            entry->rot_z = save->unk14F0[16 + i].unk10[2];
            entry->state = save->unk14F0[16 + i].unk16;
            entry->area = save->unk14F0[16 + i].unk17;
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
    viewer.status_flags = save->unk17F0;
    viewer.play_seconds = save->unk17F4;
    viewer.hp = save->unk17F8;
    viewer.mp = save->unk17FA;
    viewer.sp_total = save->unk17FC;
    viewer.creatures_killed = save->unk17FE;
    viewer.unk26 = save->unk1800;
    viewer.cunes_found = save->unk1801;
    viewer.items_found = 0;
    for (bit = 0; bit < 266; bit++) {
        if (vis_table_c.collected_bits[bit / 32] & BIT(bit))
            viewer.items_found++;
    }
    viewer.unk32 = save->unk1803;
    copy_halfwords((u16 *)viewer.attributes, (u16 *)save->unk1804, 14);
    copy_halfwords((u16 *)viewer.attribute_bonuses, (u16 *)save->unk1820, 14);
    copy_halfwords(viewer.base_attributes, save->unk183C, 14);
    copy_words((u32 *)&viewer.save_area, save->unk1870, 3);
    viewer.unk190[0][0] = save->unk187C[0][0];
    viewer.unk190[0][1] = save->unk187C[0][1];
    viewer.unk190[1][0] = save->unk187C[1][0];
    viewer.unk190[1][1] = save->unk187C[1][1];
    viewer.unk190[2][0] = save->unk187C[2][0];
    viewer.unk190[2][1] = save->unk187C[2][1];
    viewer.unk190[3][0] = save->unk187C[3][0];
    viewer.unk190[3][1] = save->unk187C[3][1];
    viewer.unk190[4][0] = save->unk187C[4][0];
    viewer.unk190[4][1] = save->unk187C[4][1];
    viewer.unk190[5][0] = save->unk187C[5][0];
    viewer.unk190[5][1] = save->unk187C[5][1];
    viewer.unk190[6][0] = save->unk187C[6][0];
    viewer.unk190[6][1] = save->unk187C[6][1];
    viewer.unk190[7][0] = save->unk187C[7][0];
    viewer.unk190[7][1] = save->unk187C[7][1];
    viewer.unk190[8][0] = save->unk187C[8][0];
    viewer.unk190[8][1] = save->unk187C[8][1];
    viewer.unk190[9][0] = save->unk187C[9][0];
    viewer.unk190[9][1] = save->unk187C[9][1];
    viewer.unk190[10][0] = save->unk187C[10][0];
    viewer.unk190[10][1] = save->unk187C[10][1];
    viewer.unk190[11][0] = save->unk187C[11][0];
    viewer.unk190[11][1] = save->unk187C[11][1];
    viewer.unk190[12][0] = save->unk187C[12][0];
    viewer.unk190[12][1] = save->unk187C[12][1];
    viewer.shield_wear = save->unk18B0[0];
    viewer.armour_wear = save->unk18B0[1];
    viewer.weapon_wear = save->unk18B0[2];
    viewer.position = save->unk1858;
    viewer.view_rot = save->unk1868;
    game_settings.bgm_volume = save->unk18B4[0];
    game_settings.sfx_volume = save->unk18B4[1];
    game_settings.walk_bob = save->unk18B4[2];
    game_settings.show_status = save->unk18B4[3];
    game_settings.subtitles = save->unk18B4[4];
    game_settings.vibration = save->unk18B4[5];
    game_progress.unk16E0 = save->unk0;
    copy_words(&game_progress.unk14A8[5], save->unk18BC, 5);
    copy_words(&game_progress.unk14A8[10], save->unk18D0, 0x4f);
    copy_words((u32 *)&inventory, save->unk1A0C, 0x4f);
    copy_words(vis_table_c.placed_bits, save->unk1B48, 10);
    viewer.equipment[0] = save->unk1B70[0];
    viewer.equipment[1] = save->unk1B70[1];
    viewer.equipment[2] = save->unk1B70[2];
    viewer.equipment[3] = save->unk1B70[3];
    viewer.equipment[4] = save->unk1B70[4];
    viewer.equipment[5] = save->unk1B70[5];
    viewer.equipment[6] = save->unk1B70[6];
    viewer.equipment[7] = save->unk1B70[7];
    viewer.equipment[8] = save->unk1B70[8];
    viewer.equipment[9] = save->unk1B70[9];
    viewer.equipment[10] = save->unk1B70[10];
    viewer.equipment[11] = save->unk1B70[11];
    viewer.equipment[12] = save->unk1B70[12];
    viewer.equipment[13] = save->unk1B70[13];
    viewer.equipment[14] = save->unk1B70[14];
    viewer.equipment[15] = save->unk1B70[15];
    viewer.equipment[16] = save->unk1B70[16];
    viewer.equipment[17] = save->unk1B70[17];
    viewer.equipment[18] = save->unk1B70[18];
    viewer.equipment[19] = save->unk1B70[19];
    viewer.equipment[20] = save->unk1B70[20];
    game_progress.unk14A8[89] = save->unk1B9C[0];
    game_progress.unk14A8[90] = save->unk1B9C[1];
    game_progress.unk14A8[91] = save->unk1B9C[2];
    r = vis_table_c.types;
    for (i = 0; i < 314; i++, r++)
        r->durability = save->unk1BA8[i];
    copy_words((u32 *)lighting.area_mode, save->unk1CE4, 24);
    copy_halfwords(inventory.shop_ids, save->unk1D44, 14);
    *(StHalves7 *)inventory.shop_stock = *(StHalves7 *)save->unk1D60;
    copy_halfwords(pad_state.buttons, save->unk1D6E, 12);
}

ADDRESS(0x8005735c, 0x30)
void area_run_frame_hook(void)
{
    area_state.hooks->frame();
}
