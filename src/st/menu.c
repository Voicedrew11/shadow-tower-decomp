/*
 * ST.EXE: the pause menu and the screens it runs (WIP module,
 * 0x80058c04..0x80063638): the menu entry and page loop, the equipment
 * pictures and attribute totals, the figure recomputation, the equip info,
 * creature book, arrange items (icon view) and item use pages, the attribute point
 * screen, the shops, the area list, the options and the memory card
 * screens. One translation unit (-G8): its constant pool (.rodata
 * 0x800128d0..0x80013738) has no 8-byte break between functions, its
 * switch tables (0x80013738..0x80013880) follow, and GCC merged the screen
 * RECT {0, 0, 320, 480} (.sdata 0x800880ac) between fade_screen_to_black and
 * menu_save_prompt_run. The pool starts after st.sound_bank's switch table, so
 * the unit starts at 0x80058c04; src/st/menu_equip_open.c is a unit of its own
 * (its RECT literals repeat this unit's instead of merging with them).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/vis.h>
#include <st/effect.h>
#include <st/menu.h>
#include <st/hud.h>
#include <st/area.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/sound.h>
#include <st/menu_main.h>
#include <st/stats.h>
#include <st/card.h>
#include <st/pad.h>
#include <st/save.h>
#include <st/item.h>

RODATA(0x800128d0, 0xfb0)
SDATA(0x800880a4, 0x3f)

/* The CD file buffer (cd_file_buffer) follows cd_state; these screens spell
 * it from cd_state's base, which GCC then shares with cd_state.load_cursor. */
#define CD_BUFFER ((u_long *)(&cd_state + 1))

ADDRESS(0x80058c04, 0x20)
void menu_store_screen(void)
{
    gfx_store_screen();
}

ADDRESS(0x80058c24, 0x28)
void menu_restore_screen(void)
{
    equip_load_item_slot_pictures();
    gfx_restore_buffers();
}

/* Loads member 0 of archive 3: the menu messages, then the menu image. */
ADDRESS(0x80058c4c, 0x7c)
void menu_load_main_messages(void)
{
    u_long *image;

    image = cd_file_buffer;
    cd_read_member(3, 0, image);
    copy_words((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (cd_file_buffer[0] + 4));
    tim_load_images_at(image + 1, &MENU_SCRATCH->image_vram[0], &MENU_SCRATCH->image_vram[1], 1);
}

ADDRESS(0x80058cc8, 0x68)
void menu_show_loading_screen(void)
{
    RECT rect = {576, 384, 64, 16};
    DVECTOR size = {576, 480};

    ResetGraph(1);
    loading_screen_show();
    loading_screen_end();
}

/* Reloads the menu and the viewer's equipment models after the menu. */
ADDRESS(0x80058d30, 0x94)
void menu_reload_assets(void)
{
    loading_screen_show();
    menu_load_main_messages();
    area_reload_images();
    player_load_equipment_model(viewer.equipment[3], 0, 1);
    player_load_equipment_model(viewer.equipment[16], 1, 1);
    player_load_equipment_model(viewer.equipment[4], 2, 1);
    player_load_equipment_model(viewer.equipment[17], 3, 1);
    player_load_magic(viewer.equipment[6], 0);
    player_load_magic(viewer.equipment[12], 1);
    loading_screen_end();
}

/* Opens a menu screen: the VRAM positions, the menu loads and the HUD. */
ADDRESS(0x80058dc4, 0x110)
void card_screen_open(int arg0, int arg1)
{
    MenuScratch *sp = MENU_SCRATCH;
    u_long *image;

    sp->image_vram[0].vx = 960;
    sp->image_vram[0].vy = 256;
    sp->image_vram[1].vx = 560;
    sp->image_vram[1].vy = 495;
    sp->image_vram[2].vx = 896;
    sp->image_vram[2].vy = 256;
    sp->image_vram[3].vx = 560;
    sp->image_vram[3].vy = 497;
    sp->image_vram[4].vx = 768;
    sp->image_vram[4].vy = 256;
    cd_wait_idle();
    image = cd_file_buffer;
    cd_read_member(3, 0, image);
    copy_words((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (cd_file_buffer[0] + 4));
    tim_load_images_at(image + 1, &sp->image_vram[0], &sp->image_vram[1], 1);
    menu_store_screen();
    sp->font_mode = 0;
    SCRATCHPAD->pitch_sin = 0;
    if (arg0)
        sp->mode = 256;
    else
        sp->mode = 1;
    DrawSync(0);
}

ADDRESS(0x80058ed4, 0x20)
void card_screen_close(void)
{
    menu_restore_screen();
}

/* The pause menu: runs the selected page until one closes the menu. */
ADDRESS(0x80058ef4, 0x310)
void menu_main_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u_long *image;

    sp->image_vram[0].vx = 960;
    sp->image_vram[0].vy = 256;
    sp->image_vram[1].vx = 560;
    sp->image_vram[1].vy = 495;
    sp->image_vram[2].vx = 896;
    sp->image_vram[2].vy = 256;
    sp->image_vram[3].vx = 560;
    sp->image_vram[3].vy = 497;
    sp->image_vram[4].vx = 768;
    sp->image_vram[4].vy = 256;
    cd_wait_idle();
    image = cd_file_buffer;
    cd_read_member(3, 0, image);
    copy_words((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (cd_file_buffer[0] + 4));
    tim_load_images_at(image + 1, &sp->image_vram[0], &sp->image_vram[1], 1);
    menu_store_screen();
    sp->main_row = 0;
    pad_clear_input();
    DrawSync(0);
    for (;;) {
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            break;
        }
        if (pad_state.pressed & 0x1000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            sp->font_mode = 0;
            menu_area_info_run();
        }
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            if ((s16)sp->main_row == 0) {
                sp->font_mode = 0;
                if (menu_use_item_run() == 1)
                    break;
            } else if ((s16)sp->main_row == 1) {
                sp->font_mode = 0;
                menu_equip_run();
            } else if ((s16)sp->main_row == 2) {
                viewer.free_anims_after_menu = 1;
                sp->font_mode = 1;
                if (pad_state.held & 0x800)
                    menu_arrange_icons_run();
                else
                    menu_arrange_list_run();
            } else if ((s16)sp->main_row == 3) {
                viewer.free_anims_after_menu = 1;
                sp->font_mode = 1;
                menu_creature_book_run();
            } else if ((s16)sp->main_row == 4) {
                viewer.free_anims_after_menu = 1;
                sp->font_mode = 1;
                menu_equip_info_run();
            } else if ((s16)sp->main_row == 5) {
                sp->font_mode = 0;
                menu_options_run();
            } else if ((s16)sp->main_row == 6) {
                sp->mode = 1;
                sp->font_mode = 0;
                sp->show_backdrop = 1;
                if (card_screen_run(1) == 1)
                    break;
            } else if ((s16)sp->main_row == 7) {
                sp->font_mode = 0;
                if (menu_end_game_run() == 1)
                    break;
            }
        }
        if (pad_state.pressed_repeat & 0x40) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->main_row++;
            if ((s16)sp->main_row >= 8)
                sp->main_row = 0;
        }
        if (pad_state.pressed_repeat & 0x10) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->main_row--;
            if ((s16)sp->main_row < 0)
                sp->main_row = 7;
        }
        sp->font_mode = 0;
        frame_rate_update();
        menu_main_draw();
    }
    pad_clear_input();
    menu_restore_screen();
}

/*
 * Loads and uploads the picture of equipment slot `slot` (none: its empty
 * picture); a two-handed weapon also copies its second half.
 */
ADDRESS(0x80059204, 0x3d0)
void equip_load_slot_picture(int slot)
{
    DVECTOR pos[21] = {
        {740, 152}, {796, 152}, {852, 152}, {932, 0},   {908, 152}, {964, 152}, {740, 124},
        {740, 180}, {796, 180}, {640, 124}, {640, 189}, {852, 180}, {788, 124}, {908, 180},
        {964, 180}, {852, 152}, {932, 73},  {796, 208}, {852, 208}, {908, 208}, {964, 208},
    };
    RECT rect;
    u16 *id = &viewer.equipment[slot];
    int part = -1;
    int none;
    int first;
    int file;
    u_long *image;
    u_long *data;

    cd_state.load_cursor = (u_long *)(&cd_state + 1);
    switch (slot) {
    case 0:
    case 13:
        first = 7;
        none = 202;
        break;
    case 1:
    case 14:
        first = 8;
        none = 238;
        break;
    case 2:
    case 15:
        first = 4;
        none = 124;
        break;
    case 3:
        first = 1;
        none = 0;
        part = 0;
        break;
    case 4:
        first = 1;
        none = 0;
        part = 1;
        break;
    case 16:
        first = 6;
        none = 166;
        part = 0;
        break;
    case 17:
        first = 6;
        none = 166;
        part = 1;
        break;
    case 7:
        first = 9;
        none = 259;
        break;
    case 8:
        first = 2;
        none = 72;
        break;
    case 9:
        first = 3;
        none = 93;
        if (*id != 0xffff && vis_table_c.types[*id].category == 10)
            part = 0;
        break;
    case 10:
        first = 5;
        none = 145;
        break;
    case 5:
    case 11:
    case 18:
    case 19:
    case 20:
        first = 10;
        none = 275;
        break;
    case 6:
    case 12:
        first = 977;
        none = 976;
        break;
    default:
        return;
    }
    if (slot == 6 || slot == 12)
        file = *id != 0xffff ? *id + first : none;
    else
        file = (*id != 0xffff ? *id + first : none) * 3 + 2;
    cd_read_member(7, file, cd_state.load_cursor);
    image = cd_state.load_cursor;
    if (part != -1) {
        if (part == 1)
            image = (u_long *)((u8 *)image + (image[0] + 4));
        data = image + 1;
    } else {
        data = image;
    }
    tim_load_images_at(data, &pos[slot], NULL, 1);
    if (*id != 0xffff && vis_table_c.types[*id].category == 9) {
        if (slot == 3) {
            rect.x = pos[3].vx;
            rect.y = pos[3].vy;
            rect.w = 92;
            rect.h = 73;
            DrawSync(0);
            MoveImage(&rect, pos[16].vx, pos[16].vy);
        } else if (slot == 4) {
            rect.x = pos[4].vx;
            rect.y = pos[4].vy;
            rect.w = 56;
            rect.h = 28;
            DrawSync(0);
            MoveImage(&rect, pos[17].vx, pos[17].vy);
        }
    } else if (*id != 0xffff && vis_table_c.types[*id].category == 10) {
        image = (u_long *)((u8 *)image + (image[0] + 4));
        tim_load_images_at(image + 1, &pos[2], NULL, 1);
    }
}

/* Uploads the pictures of the five quick item slots, 24 lines apart. */
ADDRESS(0x800595d4, 0x188)
void equip_load_item_slot_pictures(void)
{
    u32 ids[5];
    DVECTOR pos = {896, 256};
    RECT rect;
    int used;
    int i;
    int size;

    i = used = 0;
    ids[0] = viewer.equipment[5];
    ids[1] = viewer.equipment[19];
    ids[2] = viewer.equipment[11];
    ids[3] = viewer.equipment[20];
    ids[4] = viewer.equipment[18];
    cd_state.load_cursor = (u_long *)(&cd_state + 1);
    for (i = 0; i < 5; i++) {
        if (used + 2048 > 0x12000) {
            DrawSync(0);
            cd_state.load_cursor = (u_long *)(&cd_state + 1);
            used = 0;
        }
        if (ids[i] != 0xffff) {
            size = cd_read_member(6, vis_table_c.types[ids[i]].unk5[0] + 400, cd_state.load_cursor)
                   << 11;
            tim_load_images_at(cd_state.load_cursor, &pos, NULL, 1);
            used += size;
            cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
        } else {
            rect.x = pos.vx;
            rect.y = pos.vy;
            rect.w = rect.h = 24;
            ClearImage(&rect, 0, 0, 0);
        }
        pos.vy += 24;
    }
}

/*
 * Applies the two-handed rules after the selected slot changed (from the
 * slot copy at scratch +0x292 when `redraw` is 0): a two-handed weapon
 * empties the other hand's slots; with `redraw` 1 the changed pictures are
 * reloaded.
 */
ADDRESS(0x8005975c, 0x590)
void equip_apply_slot_change(int redraw)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (redraw == 0)
        *(EquipmentSlots *)sp->saved_equipment = *MENU_SLOTS;
    if (viewer.equipment[9] != 0xffff && vis_table_c.types[viewer.equipment[9]].category == 10) {
        if (sp->cursor == 2 || sp->cursor == 16 || sp->cursor == 9 || sp->cursor == 10
            || sp->cursor == 11) {
            viewer.equipment[2] = 0xffff;
            viewer.equipment[9] = 0xffff;
            viewer.equipment[10] = 0xffff;
            if (redraw == 1) {
                equip_load_slot_picture(2);
                equip_load_slot_picture(9);
                equip_load_slot_picture(10);
            }
        }
    }
    if (viewer.equipment[3] != 0xffff && vis_table_c.types[viewer.equipment[3]].category == 9) {
        if (sp->cursor == 3 || sp->cursor == 17) {
            viewer.equipment[3] = 0xffff;
            viewer.equipment[16] = 0xffff;
            if (redraw == 1) {
                equip_load_slot_picture(3);
                equip_load_slot_picture(16);
            }
        }
    }
    if (viewer.equipment[4] != 0xffff && vis_table_c.types[viewer.equipment[4]].category == 9) {
        if (sp->cursor == 4 || sp->cursor == 18) {
            viewer.equipment[4] = 0xffff;
            viewer.equipment[17] = 0xffff;
            if (redraw == 1) {
                equip_load_slot_picture(4);
                equip_load_slot_picture(17);
            }
        }
    }
    if (sp->choice_id != -1) {
        if (vis_table_c.types[sp->choice_id].category == 10) {
            viewer.equipment[2] = 0xffff;
            viewer.equipment[10] = 0xffff;
            if (redraw == 1) {
                equip_load_slot_picture(2);
                equip_load_slot_picture(10);
            }
        }
        if (vis_table_c.types[sp->choice_id].category == 9) {
            if (sp->cursor == 3) {
                viewer.equipment[16] = 0xffff;
                if (redraw == 1)
                    equip_load_slot_picture(16);
            } else if (sp->cursor == 4) {
                viewer.equipment[17] = 0xffff;
                if (redraw == 1)
                    equip_load_slot_picture(17);
            }
        }
    }
    if (sp->cursor == 2 || sp->cursor == 16) {
        if (sp->choice_id == -1) {
            u16 *hand = &viewer.equipment[2];

            *hand = 0xffff;
            hand[13] = 0xffff;
        } else {
            viewer.equipment[2] = sp->choice_id;
        }
        if (redraw == 1)
            equip_load_slot_picture(2);
    } else if (sp->cursor < 10) {
        if (sp->choice_id == -1)
            MENU_SLOTS->ids[sp->cursor] = 0xffff;
        else
            MENU_SLOTS->ids[sp->cursor] = sp->choice_id;
        if (redraw == 1)
            equip_load_slot_picture(sp->cursor);
    } else {
        if (sp->choice_id == -1)
            *(MENU_SLOTS->ids - 1 + sp->cursor) = 0xffff;
        else
            *(MENU_SLOTS->ids - 1 + sp->cursor) = sp->choice_id;
        if (redraw == 1)
            equip_load_slot_picture(sp->cursor - 1);
    }
    if (sp->cursor != 0 && sp->cursor != 14)
        return;
    if (sp->cursor == 0)
        viewer.equipment[6] = 0xffff;
    else
        viewer.equipment[12] = 0xffff;
    if (redraw == 1) {
        DrawSync(0);
        equip_load_slot_picture(sp->cursor == 0 ? 6 : 12);
    }
}

/* Restores the equipment slots from the scratch copy. */
ADDRESS(0x80059cec, 0xd4)
void equip_restore_slots(void)
{
    *MENU_SLOTS = *(EquipmentSlots *)MENU_SCRATCH->saved_equipment;
}

/*
 * Totals the attribute rows from the viewer's figures and equipment, and
 * fills one record per shown equipment slot with its figures and kind.
 */
ADDRESS(0x80059dc0, 0x5ac)
void player_total_attributes(void)
{
    MenuAttrScratch *at = MENU_ATTR;
    u16 *id;
    ItemType *rec;
    int i;
    int j;

    for (i = 0; i < 14; i++) {
        at->bonus_changes[i] = 0;
        at->base_attributes[i] = viewer.base_attributes[i];
        at->prev_attribute_bonuses[i] = viewer.attribute_bonuses[i];
        at->attribute_bonuses[i] = 0;
    }
    id = viewer.equipment;
    for (i = 0; i < 11; i++) {
        if (*id != 0xffff) {
            for (j = 0; j < 14; j++)
                at->attribute_bonuses[j] += vis_table_c.types[*id].attributes[j];
        }
        if (i == 3)
            id += 4;
        else if (i == 7)
            id += 3;
        else if (i == 9)
            id += 2;
        else
            id += 1;
    }
    if (viewer.status_flags & 0x80) {
        at->attribute_bonuses[1] >>= 1;
        at->base_attributes[1] >>= 1;
    }
    for (i = 0; i < 14; i++)
        at->bonus_changes[i] = at->attribute_bonuses[i] - at->prev_attribute_bonuses[i];
    for (i = 0; i < 14; i++) {
        at->attributes[i] = at->base_attributes[i] + at->attribute_bonuses[i];
        if (at->attributes[i] >= 1000)
            at->attributes[i] = 999;
        if (at->attributes[i] < 0)
            at->attributes_clamped[i] = 0;
        else
            at->attributes_clamped[i] = at->attributes[i];
        if (at->attribute_bonuses[i] < 0)
            at->attribute_bonuses_clamped[i] = 0;
        else
            at->attribute_bonuses_clamped[i] = at->attribute_bonuses[i];
    }
    id = viewer.equipment;
    at->next_rec = at->recs;
    at->equip_weight = 0;
    for (i = 0; i < 21; i++, id++) {
        if ((u32)(i - 4) < 3 || i == 11 || i == 12 || i == 15 || i >= 17)
            continue;
        if (*id == 0xffff) {
            fill_words((u32 *)at->next_rec, 0, 8);
        } else {
            rec = &vis_table_c.types[*id];
            at->next_rec->weight = rec->weight;
            for (j = 0; j < 14; j++)
                at->next_rec->attributes[j] = rec->attributes[j];
            at->next_rec->element_amount = 0;
            at->next_rec->element = 0;
            if (rec->kind2 >= 9) {
                at->next_rec->element = rec->kind2 - 9;
                at->next_rec->element_amount = rec->amount2;
            }
            if (i != 4 && i != 17)
                at->equip_weight += rec->weight;
        }
        at->next_rec++;
    }
    if (viewer.equipment[6] != 0xffff) {
        at->right_magic_attr = viewer.equipment[6] / 3;
        at->right_magic_bonus = *(at->right_magic_attr + effects.defs[viewer.equipment[6]].element_damage);
    } else {
        at->right_magic_attr = 0xffff;
        at->right_magic_bonus = 0;
    }
    if (viewer.equipment[12] != 0xffff) {
        at->left_magic_attr = viewer.equipment[12] / 3;
        at->left_magic_bonus = *(at->left_magic_attr + effects.defs[viewer.equipment[12]].element_damage);
    } else {
        at->left_magic_attr = 0xffff;
        at->left_magic_bonus = 0;
    }
    for (i = 0; i < 6; i++) {
        at->armour_elements[i] = 0;
        at->accessory_elements[i] = 0;
    }
    at->accessory_elements[at->recs[0].element] += at->recs[0].element_amount;
    at->accessory_elements[at->recs[8].element] += at->recs[8].element_amount;
    at->accessory_elements[at->recs[1].element] += at->recs[1].element_amount;
    at->accessory_elements[at->recs[9].element] += at->recs[9].element_amount;
    at->accessory_elements[at->recs[4].element] += at->recs[4].element_amount;
    at->armour_elements[at->recs[5].element] += at->recs[5].element_amount;
    at->armour_elements[at->recs[6].element] += at->recs[6].element_amount;
    at->armour_elements[at->recs[2].element] += at->recs[2].element_amount;
    at->armour_elements[at->recs[7].element] += at->recs[7].element_amount;
    at->armour_elements[at->recs[1].element] += at->recs[1].element_amount;
    at->armour_elements[at->recs[9].element] += at->recs[9].element_amount;
}

/* Copies the two recomputed figure rows into the viewer. */
ADDRESS(0x8005a36c, 0x3c)
void player_store_attributes(void)
{
    StatsScratch *sp = STATS_SCRATCH;
    int i;

    for (i = 0; i < 14; i++) {
        viewer.attributes[i] = sp->attributes[i];
        viewer.attribute_bonuses[i] = sp->attribute_bonuses[i];
    }
}

/*
 * Recomputes the viewer's derived figures from the attribute rows
 * player_total_attributes totals (after equip_apply_slot_change applies a slot change when
 * `apply` is set), then restores the slots (`apply`) or copies the rows
 * into the viewer.
 */
ADDRESS(0x8005a3a8, 0x6d8)
void player_compute_derived_stats(int apply)
{
    MenuAttrScratch *at = MENU_ATTR;
    u16 b6;
    u16 b8;
    u16 *p = NULL;
    u16 min;
    u16 max;
    int i;

    if (apply)
        equip_apply_slot_change(0);
    player_total_attributes();
    viewer.max_hp = at->attributes_clamped[0] * 5 + at->attributes_clamped[1] + at->attributes_clamped[2] + at->attributes_clamped[3]
                  + at->attributes_clamped[4] + at->attributes_clamped[5] + at->attributes_clamped[6];
    viewer.max_mp = at->attributes_clamped[7] * 5 + at->attributes_clamped[8] + at->attributes_clamped[9] + at->attributes_clamped[10]
                  + at->attributes_clamped[11] + at->attributes_clamped[12] + at->attributes_clamped[13];
    viewer.physical_base = viewer.max_hp / 80;
    viewer.magic_base = viewer.max_mp / 80;
    if (viewer.status_flags & 0x100) {
        b6 = viewer.physical_base;
        viewer.physical_base = b6 / 2;
        b8 = viewer.magic_base;
        viewer.magic_base = b8 / 2;
    }
    if (viewer.max_hp == 0)
        viewer.max_hp = 1;
    if (viewer.max_mp == 0)
        viewer.max_mp = 1;
    if (viewer.physical_base == 0)
        viewer.physical_base = 1;
    if (viewer.magic_base == 0)
        viewer.magic_base = 1;
    if (viewer.equipment[3] != 0xffff) {
        viewer.weapon_attack[0] = viewer.physical_base + (at->attribute_bonuses_clamped[4] >> 1);
        viewer.weapon_attack[1] = viewer.physical_base + (at->attribute_bonuses_clamped[5] >> 1);
        viewer.weapon_attack[2] = viewer.physical_base + (at->attribute_bonuses_clamped[6] >> 1);
        p = viewer.weapon_element_attack;
        for (i = 0; i < 6; i++) {
            if (at->recs[3].element == i) {
                *p = viewer.magic_base + (at->recs[3].element_amount >> 1);
                if (at->recs[1].element == i)
                    *p += at->recs[1].element_amount >> 2;
                if (at->recs[9].element == i)
                    *p += at->recs[9].element_amount >> 2;
            } else {
                *p = 0;
            }
        }
    } else {
        viewer.weapon_attack[0] = viewer.weapon_attack[1] = viewer.weapon_attack[2] = viewer.weapon_element_attack[0] = viewer.weapon_element_attack[1]
            = viewer.weapon_element_attack[2] = viewer.weapon_element_attack[3] = viewer.weapon_element_attack[4] = viewer.weapon_element_attack[5] = 0;
    }
    if (at->right_magic_attr != 0xffff)
        viewer.magic_attack_right = viewer.magic_base + at->right_magic_bonus + at->attributes_clamped[at->right_magic_attr + 11] / 6;
    else
        viewer.magic_attack_right = 0;
    if (at->left_magic_attr != 0xffff)
        viewer.magic_attack_left = viewer.magic_base + at->left_magic_bonus + at->attributes_clamped[at->left_magic_attr + 11] / 6;
    else
        viewer.magic_attack_left = 0;
    viewer.armour_defence = viewer.physical_base + ((at->attributes_clamped[2] - at->recs[10].attributes[2]) >> 2);
    viewer.magic_defence = viewer.magic_base + ((at->attributes_clamped[9] - at->recs[10].attributes[9]) >> 2);
    viewer.shield_defence = at->recs[10].attributes[2];
    viewer.shield_magic_defence = at->recs[10].attributes[9];
    viewer.impact_resistance = (at->attributes_clamped[3] >> 4) + 50;
    viewer.status_recovery = (at->attributes_clamped[10] >> 4) + 50;
    viewer.attack_speed = (at->attributes_clamped[1] >> 3) + 100;
    viewer.magic_speed = (at->attributes_clamped[8] >> 3) + 100;
    viewer.accessory_element_bonus[0] = at->accessory_elements[0] / 5;
    viewer.accessory_element_bonus[1] = at->accessory_elements[1] / 5;
    viewer.accessory_element_bonus[2] = at->accessory_elements[2] / 5;
    viewer.accessory_element_bonus[3] = at->accessory_elements[3] / 5;
    viewer.accessory_element_bonus[4] = at->accessory_elements[4] / 5;
    viewer.accessory_element_bonus[5] = at->accessory_elements[5] / 5;
    viewer.element_defence[0] = ((at->attributes_clamped[11] >> 2) + at->armour_elements[0]) / 7;
    viewer.element_defence[1] = ((at->attributes_clamped[11] >> 2) + at->armour_elements[1]) / 7;
    viewer.element_defence[2] = ((at->attributes_clamped[12] >> 2) + at->armour_elements[2]) / 7;
    viewer.element_defence[3] = ((at->attributes_clamped[12] >> 2) + at->armour_elements[3]) / 7;
    viewer.element_defence[4] = ((at->attributes_clamped[13] >> 2) + at->armour_elements[4]) / 7;
    viewer.element_defence[5] = ((at->attributes_clamped[13] >> 2) + at->armour_elements[5]) / 7;
    for (i = 0; i < 6; i++) {
        if (at->recs[10].element == i)
            viewer.shield_element_defence = at->attributes_clamped[i / 2 + 11] + (at->recs[10].element_amount >> 1);
    }
    min = 0xffff;
    max = 0;
    if (max < viewer.weapon_attack[0])
        max = viewer.weapon_attack[0];
    if (viewer.weapon_attack[0] < min)
        min = viewer.weapon_attack[0];
    if (max < viewer.weapon_attack[1])
        max = viewer.weapon_attack[1];
    if (viewer.weapon_attack[1] < min)
        min = viewer.weapon_attack[1];
    if (max < viewer.weapon_attack[2])
        max = viewer.weapon_attack[2];
    if (viewer.weapon_attack[2] < min)
        min = viewer.weapon_attack[2];
    viewer.weapon_attack_min = min;
    viewer.weapon_attack_max = max;
    viewer.equip_weight = at->equip_weight;
    viewer.equip_weight_limit = viewer.max_hp / 12 + 120;
    if (apply)
        equip_restore_slots();
    else
        player_store_attributes();
}

/*
 * Fades the screen to black over sixteen frames: each frame subtracts
 * 16 from the display through a translucent quad without clearing.
 */
ADDRESS(0x8005aa80, 0x280)
void fade_screen_to_black(void)
{
    POLY_F4 poly;
    DR_MODE mode;
    RECT clear = {0, 0, 320, 480};
    u_long ot[1];
    int n;

    n = 16;
    while (n--) {
        world_state.draw[0].isbg = 0;
        world_state.draw[1].isbg = 0;
        world_state.draw[1].dfe = 1;
        world_state.draw[0].dfe = 1;
        world_state.draw[1].isbg = 0;
        world_state.draw[0].isbg = 0;
        PutDrawEnv(&world_state.draw[world_state.frame]);
        PutDrawEnv(&world_state.draw[1 - world_state.frame]);
        ClearOTagR(ot, 1);
        setcode(&poly, 0x28);
        setSemiTrans(&poly, 1);
        /* The quad is linked while it is filled in: its tag word first, the
         * OT entry between the last corner's x and y (retail's store and
         * load order pins both there). */
        poly.tag = (ot[0] & 0xffffff) | (5 << 24);
        poly.x0 = 0;
        poly.y0 = 0;
        poly.x1 = world_state.draw[1 - world_state.frame].clip.w;
        poly.y1 = 0;
        poly.x2 = 0;
        poly.y2 = world_state.draw[1 - world_state.frame].clip.h;
        poly.x3 = world_state.draw[1 - world_state.frame].clip.w;
        mesh_set_ot(ot, &poly);
        poly.y3 = world_state.draw[1 - world_state.frame].clip.h;
        poly.r0 = poly.g0 = poly.b0 = 16;
        SetDrawMode(&mode, 0, 0, GetTPage(0, 2, 640, 0), NULL);
        addPrim(ot, &mode);
        VSync(0);
        DrawOTag(ot);
    }
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}

/*
 * The equip info page: sorts the table C ids held into one list per kind,
 * draws the slot pictures, then lets the player pick a slot (unk2DC 1)
 * and an item for it from its list (unk2C7: the list kind; 10 is the
 * magic grid), recomputing the figures as the choice changes.
 *
 * Shapes retail's code fixes: the archive is reached through one pointer
 * (`arc`), the file cursor advances by (*p + 4), `base` and `offset` are
 * ints (the zero in `base` is what reload copies for every zero argument
 * of the first block), the kind sort reads the kind from the table each
 * time and its last arm goes through a list and a count pointer, the
 * held id is read by pointer arithmetic on viewer.equipment, each list case
 * sets the kind after its call and the two file numbers in its own
 * order, and the navigation cases follow the cursor's path. Each loop
 * family has its own counter (i, k, the preview's j), and the picked
 * index its own int (sel, at). The grid's down step tests the row
 * against 1, as retail does.
 */
ADDRESS(0x8005ad00, 0x1afc)
void menu_equip_info_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR picture = {640, 0};
    RECT unused[4];
    DVECTOR image = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR unused2;
    CdlLOC loc;
    u_long *p;
    StArchive *arc;
    int base;
    int offset;
    int file;
    int debounce;
    int prev;
    int row;
    u16 cur;
    int n;
    int i;
    int k;
    int held;
    int sel;

    base = 0;
    offset = 0;
    arc = &cd_state.stat_archive;
    cd_loc_add_sectors(&arc->file.pos, arc->sectors[1], &loc);
    CdControl(CdlSetloc, (u_char *)&loc, NULL);
    ResetGraph(1);
    fade_screen_to_black();
    sp->item_messages = mem_alloc(21504);
    sp->ring_magic_bits = mem_alloc(144);
    p = CD_BUFFER;
    cd_read_member(3, 1, p);
    copy_words((u32 *)&menu_messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (*p + 4));
    tim_load_images_at(p + 1, &sp->image_vram[0], &sp->image_vram[1], 1);
    p = (u_long *)((u8 *)p + (*p + 4));
    copy_words(sp->ring_magic_bits, (u32 *)(p + 1), 36);
    p = CD_BUFFER;
    cd_read_member(3, 4, p);
    copy_words((u32 *)sp->item_messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (*p + 4));
    tim_load_images_at(p + 1, &sp->image_vram[2], &sp->image_vram[3], 1);
    cd_read_member(3, 5, CD_BUFFER);
    tim_load_images_at(CD_BUFFER, &sp->image_vram[4], NULL, 1);
    DrawSync(0);
    cd_read_member(3, 7, CD_BUFFER);
    tim_load_images_at(CD_BUFFER, &image, &clut, 1);
    DrawSync(0);
    sp->state.vram[0].vx = 640;
    sp->state.vram[0].vy = 256;
    sp->state.vram[1].vx = 560;
    sp->state.vram[1].vy = 498;
    sp->category_lists[0] = mem_alloc(40);
    sp->category_lists[1] = mem_alloc(60);
    sp->category_lists[2] = mem_alloc(40);
    sp->category_lists[3] = mem_alloc(40);
    sp->category_lists[4] = mem_alloc(70);
    sp->category_lists[5] = mem_alloc(30);
    sp->category_lists[6] = mem_alloc(140);
    sp->category_lists[7] = mem_alloc(70);
    sp->category_lists[8] = mem_alloc(40);
    sp->category_lists[9] = mem_alloc(92);
    sp->choices = mem_alloc(140);
    sp->category_counts[0] = sp->category_counts[1] = sp->category_counts[2] = sp->category_counts[3] = sp->category_counts[4]
        = sp->category_counts[5] = sp->category_counts[6] = sp->category_counts[7] = sp->category_counts[8] = sp->category_counts[9] = 0;
    for (i = 0; i < 314; i++) {
        if (inventory.counts[i].count == 0 || inventory.counts[i].in_bag)
            continue;
        if (vis_table_c.types[i].category == 0xff)
            continue;
        if (vis_table_c.types[i].category == 9) {
            sp->category_lists[6][sp->category_counts[6]++] = i;
        } else if (vis_table_c.types[i].category == 10) {
            sp->category_lists[1][sp->category_counts[1]++] = i;
        } else if (vis_table_c.types[i].category == 20 || vis_table_c.types[i].category == 30) {
            if (i == 266 || i == 310 || i == 311 || i == 312 || i == 313)
                continue;
            sp->category_lists[9][sp->category_counts[9]++] = i;
        } else {
            {
            s16 *list = sp->category_lists[vis_table_c.types[i].category];
            s16 *len = &sp->category_counts[vis_table_c.types[i].category];
            list[(*len)++] = i;
            }
        }
    }
    clear_vram();
    DrawSync(0);
    gfx_setup_640x240();
    sp->bar_scale = -1;
    for (i = 0; i < 14; i++) {
        if (sp->bar_scale < viewer.attributes[i])
            sp->bar_scale = viewer.attributes[i];
    }
    prev = 0;
    sp->mode = 1;
    sp->show_item_values = 0;
    sp->cursor = 0;
    debounce = 0;
    sp->hp_before = viewer.hp;
    sp->mp_before = viewer.mp;
    player_compute_derived_stats(0);
    pad_clear_input();
    menu_second_page = 0;
    {
        RECT clear = {640, 0, 384, 256};
        int slots[20] = {3, 4, 8, 9, 2, 10, 16, 17, 0, 13, 1, 14, 7, 5, 11, 18, 19, 20, 6, 12};
        int j;

        ClearImage(&clear, 0, 0, 0);
        DrawSync(0);
        DAT_80088192 = 0;
        menu_equip_info_draw();
        menu_equip_info_draw();
        equip_load_slot_picture(slots[0]);
        menu_equip_info_draw();
        for (j = 1; j < 20; j++) {
            if ((viewer.equipment[3] == 0xffff || vis_table_c.types[viewer.equipment[3]].category != 9
                 || slots[j] != 16)
                && (viewer.equipment[4] == 0xffff || vis_table_c.types[viewer.equipment[4]].category != 9
                    || slots[j] != 17)
                && (viewer.equipment[9] == 0xffff || vis_table_c.types[viewer.equipment[9]].category != 10
                    || (slots[j] != 2 && slots[j] != 10 && slots[j] != 15))) {
                equip_load_slot_picture(slots[j]);
                menu_equip_info_draw();
                play_cursor_sound();
            }
        }
    }
    menu_equip_info_draw();
    play_cursor_sound();
    pad_clear_input();
    for (;;) {
        if (debounce != 0) {
            pad_state.pressed_repeat = 0;
            debounce--;
        }
        if (sp->mode == 1) {
            if (pad_state.pressed & 0x800)
                menu_second_page = 1;
            else if (pad_state.pressed & 0x400)
                menu_second_page = 0;
            if (pad_state.held & 0x1000) {
                if (sp->show_item_values == 0)
                    play_confirm_sound();
                sp->show_item_values = 1;
            } else {
                sp->show_item_values = 0;
            }
            if (pad_state.pressed & 0x2000) {
                play_cancel_sound();
                pad_state.pressed = 0;
                break;
            }
            if (pad_state.pressed & 0x4000) {
                pad_state.pressed = 0;
                sp->mode = 2;
                held = sp->cursor;
                if (held == 16)
                    cur = *(viewer.equipment + 2);
                else if (held < 10)
                    cur = *(viewer.equipment + held);
                else
                    cur = *(viewer.equipment + held - 1);
                sp->choice_count = 0;
                switch (sp->cursor) {
                case 3:
                case 4:
                    n = sp->category_counts[6];
                    for (k = 0; k < n; k++) {
                        if ((sp->cursor == 3 && sp->category_lists[6][k] != viewer.equipment[4])
                            || (sp->cursor == 4 && sp->category_lists[6][k] != viewer.equipment[3]))
                            sp->choices[sp->choice_count++] = sp->category_lists[6][k];
                    }
                    play_confirm_sound();
                    sp->list_kind = 6;
                    base = 0;
                    offset = 1;
                    break;
                case 17:
                case 18:
                    n = sp->category_counts[7];
                    for (k = 0; k < n; k++) {
                        if ((sp->cursor == 17 && sp->category_lists[7][k] != viewer.equipment[17])
                            || (sp->cursor == 18 && sp->category_lists[7][k] != viewer.equipment[16]))
                            sp->choices[sp->choice_count++] = sp->category_lists[7][k];
                    }
                    play_confirm_sound();
                    sp->list_kind = 7;
                    base = 166;
                    offset = 6;
                    break;
                case 8:
                    n = sp->category_counts[0];
                    for (k = 0; k < n; k++)
                        sp->choices[sp->choice_count++] = sp->category_lists[0][k];
                    play_confirm_sound();
                    sp->list_kind = 0;
                    base = 72;
                    offset = 2;
                    break;
                case 9:
                case 10:
                    n = sp->category_counts[1];
                    for (k = 0; k < n; k++)
                        sp->choices[sp->choice_count++] = sp->category_lists[1][k];
                    play_confirm_sound();
                    sp->list_kind = 1;
                    offset = 3;
                    base = 93;
                    break;
                case 2:
                case 16:
                    n = sp->category_counts[2];
                    for (k = 0; k < n; k++)
                        sp->choices[sp->choice_count++] = sp->category_lists[2][k];
                    play_confirm_sound();
                    sp->list_kind = 2;
                    offset = 4;
                    base = 124;
                    break;
                case 11:
                    n = sp->category_counts[3];
                    for (k = 0; k < n; k++)
                        sp->choices[sp->choice_count++] = sp->category_lists[3][k];
                    play_confirm_sound();
                    sp->list_kind = 3;
                    offset = 5;
                    base = 145;
                    break;
                case 0:
                case 14:
                    n = sp->category_counts[4];
                    for (k = 0; k < n; k++) {
                        if ((sp->cursor == 0 && sp->category_lists[4][k] != viewer.equipment[13])
                            || (sp->cursor == 14 && sp->category_lists[4][k] != viewer.equipment[0]))
                            sp->choices[sp->choice_count++] = sp->category_lists[4][k];
                    }
                    play_confirm_sound();
                    sp->list_kind = 4;
                    offset = 7;
                    base = 202;
                    break;
                case 1:
                case 15:
                    n = sp->category_counts[8];
                    for (k = 0; k < n; k++) {
                        if ((sp->cursor == 1 && sp->category_lists[8][k] != viewer.equipment[14])
                            || (sp->cursor == 15 && sp->category_lists[8][k] != viewer.equipment[1]))
                            sp->choices[sp->choice_count++] = sp->category_lists[8][k];
                    }
                    play_confirm_sound();
                    sp->list_kind = 8;
                    offset = 8;
                    base = 238;
                    break;
                case 7:
                    n = sp->category_counts[5];
                    for (k = 0; k < n; k++)
                        sp->choices[sp->choice_count++] = sp->category_lists[5][k];
                    play_confirm_sound();
                    sp->list_kind = 5;
                    offset = 9;
                    base = 259;
                    break;
                case 5:
                case 12:
                case 19:
                case 20:
                case 21:
                    n = sp->category_counts[9];
                    for (k = 0; k < n; k++) {
                        if ((sp->category_lists[9][k] != viewer.equipment[5] && sp->category_lists[9][k] != viewer.equipment[19]
                             && sp->category_lists[9][k] != viewer.equipment[11]
                             && sp->category_lists[9][k] != viewer.equipment[20]
                             && sp->category_lists[9][k] != viewer.equipment[18])
                            || (sp->cursor == 5 && sp->category_lists[9][k] == viewer.equipment[5])
                            || (sp->cursor == 20 && sp->category_lists[9][k] == viewer.equipment[19])
                            || (sp->cursor == 12 && sp->category_lists[9][k] == viewer.equipment[11])
                            || (sp->cursor == 21 && sp->category_lists[9][k] == viewer.equipment[20])
                            || (sp->cursor == 19 && sp->category_lists[9][k] == viewer.equipment[18]))
                            sp->choices[sp->choice_count++] = sp->category_lists[9][k];
                    }
                    play_confirm_sound();
                    sp->list_kind = 9;
                    offset = 10;
                    base = 275;
                    break;
                case 6:
                case 13:
                    if ((sp->cursor == 6 && viewer.equipment[0] != 0xffff)
                        || (sp->cursor == 13 && viewer.equipment[13] != 0xffff)) {
                        play_confirm_sound();
                    } else {
                        play_refuse_sound();
                        sp->mode = 1;
                        goto next;
                    }
                    sp->list_kind = 10;
                    if (cur == 0xffff)
                        sp->choice_index = 0;
                    else
                        sp->choice_index = cur;
                    offset = 977;
                    base = 976;
                    sp->choice_id = sp->choice_index;
                    player_compute_derived_stats(1);
                    break;
                default:
                    offset = 0;
                    base = 0;
                    break;
                }
                if (sp->list_kind == 10) {
                    sp->tex_pos.vx = 640;
                    sp->tex_pos.vy = 0;
                    if (sp->cursor == 6)
                        sp->ring_magic = sp->ring_magic_bits + (viewer.equipment[0] - 196);
                    else
                        sp->ring_magic = sp->ring_magic_bits + (viewer.equipment[13] - 196);
                    cd_read_member(7, 975, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &sp->tex_pos, NULL, 1);
                    cd_read_member(7, sp->choice_index + 995, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &sp->state.vram[0], &sp->state.vram[1], 1);
                } else {
                    if (cur == 0xffff) {
                        sp->choice_index = -1;
                    } else {
                        for (i = 0; i < sp->choice_count; i++) {
                            if (sp->choices[i] == cur) {
                                sp->choice_index = i;
                                break;
                            }
                        }
                    }
                    sel = sp->choice_index;
                    if (sel == -1) {
                        sp->choice_id = sel;
                        file = base;
                    } else {
                        sp->choice_id = sp->choices[sel];
                        file = sp->choice_id + offset;
                    }
                    cd_read_member(7, file * 3, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &sp->state.vram[0], &sp->state.vram[1], 1);
                    DrawSync(0);
                    cd_read_member(7, file * 3 + 1, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &picture, NULL, 1);
                    player_compute_derived_stats(0);
                }
                pad_state.pressed = 0;
                goto moved;
            }
            if (pad_state.pressed_repeat & 0x20) {
                debounce = 1;
                switch (sp->cursor) {
                case 0:
                    sp->cursor = 6;
                    break;
                case 6:
                    sp->cursor = 7;
                    break;
                case 7:
                    sp->cursor = 13;
                    break;
                case 13:
                    sp->cursor = 14;
                    break;
                case 5:
                    sp->cursor = 20;
                    break;
                case 20:
                    sp->cursor = 12;
                    break;
                case 12:
                    sp->cursor = 21;
                    break;
                case 21:
                    sp->cursor = 19;
                    break;
                default:
                    if (sp->cursor < 14)
                        sp->cursor += 7;
                    break;
                }
            } else if (pad_state.pressed_repeat & 0x80) {
                debounce = 1;
                switch (sp->cursor) {
                case 14:
                    sp->cursor = 13;
                    break;
                case 13:
                    sp->cursor = 7;
                    break;
                case 7:
                    sp->cursor = 6;
                    break;
                case 6:
                    sp->cursor = 0;
                    break;
                case 19:
                    sp->cursor = 21;
                    break;
                case 21:
                    sp->cursor = 12;
                    break;
                case 12:
                    sp->cursor = 20;
                    break;
                case 20:
                    sp->cursor = 5;
                    break;
                default:
                    if (sp->cursor > 5)
                        sp->cursor -= 7;
                    break;
                }
            } else if (pad_state.pressed_repeat & 0x40) {
                debounce = 1;
                switch (sp->cursor) {
                case 9:
                    sp->cursor = 11;
                    break;
                case 6:
                    sp->cursor = 1;
                    break;
                case 13:
                    sp->cursor = 15;
                    break;
                default:
                    if (sp->cursor != 5 && sp->cursor != 20 && sp->cursor != 12 && sp->cursor != 21
                        && sp->cursor != 19)
                        sp->cursor++;
                    break;
                }
            } else if (pad_state.pressed_repeat & 0x10) {
                debounce = 1;
                switch (sp->cursor) {
                case 10:
                    sp->cursor = 8;
                    break;
                case 20:
                case 21:
                    sp->cursor = 11;
                    break;
                default:
                    if (sp->cursor != 0 && sp->cursor != 6 && sp->cursor != 7 && sp->cursor != 13
                        && sp->cursor != 14)
                        sp->cursor--;
                    break;
                }
            } else {
                goto idle;
            }
            pad_state.pressed_repeat = 0;
            sp->blink_phase = 0;
    idle:
            pad_state.pressed = 0;
    moved:
            if (sp->cursor != prev) {
                play_cursor_sound();
                prev = sp->cursor;
            }
        } else {
            if (sp->list_kind != 10) {
                if (pad_state.pressed & 0x4000) {
                    sel = sp->choice_id;
                    if (sel != -1 && vis_table_c.types[sel].max_durability != 0
                        && vis_table_c.types[sel].durability == 0
                        && vis_table_c.types[sel].category != 4) {
                        play_refuse_sound();
                        pad_state.pressed = 0;
                    } else {
                        play_confirm_sound();
                        equip_apply_slot_change(1);
                        player_compute_derived_stats(0);
                        if (viewer.max_hp < sp->hp_before)
                            viewer.hp = viewer.max_hp;
                        if (viewer.max_mp < sp->mp_before)
                            viewer.mp = viewer.max_mp;
                        sp->mode = 1;
                        pad_state.pressed = 0;
                    }
                } else if (pad_state.pressed_repeat & 0x20) {
                    play_cursor_sound();
                    if (sp->choice_index < sp->choice_count - 1) {
                        sp->choice_index++;
                        sp->choice_id = sp->choices[sp->choice_index];
                    } else {
                        sp->choice_index = -1;
                        sp->choice_id = -1;
                    }
                } else if (pad_state.pressed_repeat & 0x80) {
                    play_cursor_sound();
                    if (sp->choice_index >= 0)
                        sp->choice_index--;
                    else
                        sp->choice_index = sp->choice_count - 1;
                    {
                        int at = sp->choice_index;

                        if (at == -1)
                            sp->choice_id = at;
                        else
                            sp->choice_id = sp->choices[at];
                    }
                }
                if (pad_state.pressed_repeat & 0xa0) {
                    debounce = 3;
                    if (sp->choice_index == -1)
                        file = base;
                    else
                        file = sp->choice_id + offset;
                    cd_read_member(7, file * 3, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &sp->state.vram[0], &sp->state.vram[1], 1);
                    cd_read_member(7, file * 3 + 1, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &picture, NULL, 1);
                    player_compute_derived_stats(1);
                    pad_state.pressed_repeat = 0;
                }
            } else {
                row = sp->choice_index;
                if (pad_state.pressed_repeat & 0x20) {
                    debounce = 1;
                    play_cursor_sound();
                    pad_state.pressed_repeat = 0;
                    if (sp->choice_index < 15)
                        sp->choice_index += 3;
                } else if (pad_state.pressed_repeat & 0x80) {
                    debounce = 1;
                    play_cursor_sound();
                    pad_state.pressed_repeat = 0;
                    if (sp->choice_index > 1)
                        sp->choice_index -= 3;
                } else if (pad_state.pressed_repeat & 0x10) {
                    debounce = 1;
                    play_cursor_sound();
                    pad_state.pressed_repeat = 0;
                    if (sp->choice_index % 3 != 0)
                        sp->choice_index--;
                } else if (pad_state.pressed_repeat & 0x40) {
                    debounce = 1;
                    play_cursor_sound();
                    pad_state.pressed_repeat = 0;
                    if (sp->choice_index % 3 != 1)
                        sp->choice_index++;
                } else if (pad_state.pressed & 0x4000) {
                    pad_state.pressed = 0;
                    if (*sp->ring_magic & (1 << sp->choice_index)) {
                        play_confirm_sound();
                        sp->rect.x = (s16)(sp->choice_index / 3) * 48 + 640;
                        sp->rect.y = (s16)(sp->choice_index % 3) * 24;
                        sp->rect.w = 48;
                        sp->rect.h = 24;
                        if (sp->cursor == 6) {
                            viewer.equipment[6] = sp->choice_index;
                            MoveImage(&sp->rect, 740, 124);
                        } else {
                            viewer.equipment[12] = sp->choice_index;
                            MoveImage(&sp->rect, 788, 124);
                        }
                        sp->mode = 1;
                        player_compute_derived_stats(0);
                    } else {
                        play_refuse_sound();
                    }
                }
                if (row != sp->choice_index) {
                    cd_read_member(7, sp->choice_index + 995, CD_BUFFER);
                    tim_load_images_at(CD_BUFFER, &sp->state.vram[0], &sp->state.vram[1], 1);
                    sp->choice_id = sp->choice_index;
                    player_compute_derived_stats(1);
                }
            }
            if (pad_state.pressed & 0x2000) {
                play_cancel_sound();
                pad_state.pressed = 0;
                player_compute_derived_stats(0);
                sp->mode = 1;
            } else if (sp->list_kind != 10) {
                if (pad_state.held & 0x1000) {
                    if (sp->show_item_values == 0)
                        play_confirm_sound();
                    sp->show_item_values = 1;
                } else {
                    sp->show_item_values = 0;
                }
            }
        }
    next:
        DAT_80088192 = 1;
        frame_rate_update();
        menu_equip_info_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    mem_free(sp->category_lists[0]);
    mem_free(sp->category_lists[1]);
    mem_free(sp->category_lists[2]);
    mem_free(sp->category_lists[3]);
    mem_free(sp->category_lists[4]);
    mem_free(sp->category_lists[5]);
    mem_free(sp->category_lists[6]);
    mem_free(sp->category_lists[7]);
    mem_free(sp->category_lists[8]);
    mem_free(sp->category_lists[9]);
    mem_free(sp->item_messages);
    mem_free(sp->ring_magic_bits);
    mem_free(sp->choices);
    menu_reload_assets();
    pad_clear_input();
}

/*
 * The creature book: lists the ids set in DAT_801cceb0 five to a row,
 * three rows shown with their pictures, and shows the detail page of the
 * confirmed one (menu_creature_book_draw draws the page).
 *
 * The detail load is skipped for the id in `shown`, which starts as -1
 * and is never updated here (shop_run's shop screens keep theirs
 * current): a call-crossing variable with no saved register left, so
 * retail rematerialises the -1 in the reload register at the test.
 */
ADDRESS(0x8005c7fc, 0xa60)
void menu_creature_book_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR image = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR detail = {960, 446};
    DVECTOR detail_clut = {560, 498};
    u8 unused[64];
    u32 *bits;
    u32 bit;
    u_long *p;
    int col;
    int row;
    int first;
    int shown = -1;
    int used;

    int n;
    int i;

    menu_show_loading_screen();
    sp->state.book.names = mem_alloc(21504);
    sp->state.book.entries = mem_alloc(6644);
    sp->state.book.cell_tex[0].vx = 640;
    sp->state.book.cell_tex[0].vy = 144;
    sp->state.book.cell_tex[1].vx = 640;
    sp->state.book.cell_tex[1].vy = 192;
    sp->state.book.cell_tex[2].vx = 736;
    sp->state.book.cell_tex[2].vy = 144;
    sp->state.book.cell_tex[3].vx = 736;
    sp->state.book.cell_tex[3].vy = 192;
    sp->state.book.cell_tex[4].vx = 832;
    sp->state.book.cell_tex[4].vy = 144;
    sp->state.book.cell_tex[5].vx = 928;
    sp->state.book.cell_tex[5].vy = 0;
    sp->state.book.cell_tex[6].vx = 928;
    sp->state.book.cell_tex[6].vy = 48;
    sp->state.book.cell_tex[7].vx = 928;
    sp->state.book.cell_tex[7].vy = 96;
    sp->state.book.cell_tex[8].vx = 928;
    sp->state.book.cell_tex[8].vy = 144;
    sp->state.book.cell_tex[9].vx = 928;
    sp->state.book.cell_tex[9].vy = 192;
    sp->state.book.cell_tex[10].vx = 640;
    sp->state.book.cell_tex[10].vy = 256;
    sp->state.book.cell_tex[11].vx = 640;
    sp->state.book.cell_tex[11].vy = 304;
    sp->state.book.cell_tex[12].vx = 640;
    sp->state.book.cell_tex[12].vy = 352;
    sp->state.book.cell_tex[13].vx = 640;
    sp->state.book.cell_tex[13].vy = 400;
    sp->state.book.cell_tex[14].vx = 640;
    sp->state.book.cell_tex[14].vy = 448;
    sp->state.book.image.vx = 640;
    sp->state.book.image.vy = 0;
    sp->state.book.kinds = mem_alloc(151);
    sp->state.book.count = 0;
    n = 0;
    bits = DAT_801cceb0[0];
    bit = 1;
    for (i = 0; i < 151; i++) {
        if (n == 32) {
            n = 0;
            bit = 1;
            bits++;
        }
        if (*bits & bit)
            sp->state.book.kinds[sp->state.book.count++] = i;
        bit <<= 1;
        n++;
    }
    sp->state.book.rows = sp->state.book.count / 5;
    sp->state.book.last = sp->state.book.count % 5;
    sp->scroll = sp->state.book.rows - 2;
    if (sp->scroll < 0)
        sp->scroll = 0;
    p = CD_BUFFER;
    cd_read_member(3, 1, p);
    copy_words((u32 *)&menu_messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (*p + 4));
    tim_load_images_at(p + 1, &sp->image_vram[0], &sp->image_vram[1], 1);
    p = CD_BUFFER;
    cd_read_member(3, 2, p);
    copy_words((u32 *)sp->state.book.names, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (*p + 4));
    tim_load_images_at(p + 1, &sp->image_vram[2], &sp->image_vram[3], 1);
    cd_read_member(3, 5, CD_BUFFER);
    tim_load_images_at(CD_BUFFER, &sp->image_vram[4], NULL, 1);
    DrawSync(0);
    cd_read_member(3, 7, CD_BUFFER);
    tim_load_images_at(CD_BUFFER, &image, &clut, 1);
    DrawSync(0);
    cd_read_member(4, 400, CD_BUFFER);
    copy_words((u32 *)sp->state.book.entries, (u32 *)CD_BUFFER, 1661);
    cd_state.load_cursor = CD_BUFFER;
    n = 15;
    if (sp->state.book.count < 16)
        n = sp->state.book.count;
    used = 0;
    for (i = 0; i < n; i++) {
        int k = sp->scroll * 5 + i;

        if (k < sp->state.book.count) {
            if (used + 0x2800 > 0x12000) {
                DrawSync(0);
                cd_state.load_cursor = CD_BUFFER;
                used = 0;
            }
            {
                int size = cd_read_member(4, sp->state.book.kinds[k], cd_state.load_cursor) << 11;

                tim_load_images_at(cd_state.load_cursor, &sp->state.book.cell_tex[k % 15], NULL, 1);
                used += size;
                cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
            }
        }
    }
    clear_vram();
    DrawSync(0);
    gfx_setup_640x240();
    sp->mode = 1;
    sp->cursor = 0;
    pad_clear_input();
    for (;;) {
        first = sp->scroll;
        if (sp->mode == 1) {
            col = (s16)(sp->cursor % 5);
            row = (s16)(sp->cursor / 5);
            if (pad_state.pressed & 0x2000) {
                play_cancel_sound();
                pad_state.pressed = 0;
                break;
            }
            if (pad_state.pressed_repeat & 0x20) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (col < 4)
                    sp->cursor++;
            }
            if (pad_state.pressed_repeat & 0x80) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (col != 0)
                    sp->cursor--;
            }
            if (pad_state.pressed_repeat & 0x10) {
                play_cursor_sound();
                if (pad_state.held & 0x8000) {
                    sp->scroll -= 3;
                    if (sp->scroll < 0)
                        sp->scroll = 0;
                } else if (row > 0) {
                    sp->cursor -= 5;
                } else if (sp->scroll > 0) {
                    sp->scroll--;
                }
                pad_state.pressed_repeat = 0;
            }
            if (pad_state.pressed_repeat & 0x40) {
                play_cursor_sound();
                if (pad_state.held & 0x8000) {
                    if (sp->state.book.rows > 2) {
                        sp->scroll += 3;
                        if (sp->scroll > sp->state.book.rows - 2)
                            sp->scroll = sp->state.book.rows - 2;
                    }
                } else if (sp->scroll + row < sp->state.book.rows) {
                    if (row < 2)
                        sp->cursor += 5;
                    else if (sp->scroll <= sp->state.book.rows - 3)
                        sp->scroll++;
                }
                pad_state.pressed_repeat = 0;
            }
            if (pad_state.pressed_repeat & 0x800) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->scroll -= 3;
                if (sp->scroll < 0)
                    sp->scroll = 0;
            }
            if (pad_state.pressed_repeat & 0x200) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->state.book.rows > 2) {
                    sp->scroll += 3;
                    if (sp->scroll > sp->state.book.rows - 2)
                        sp->scroll = sp->state.book.rows - 2;
                }
            }
            if (pad_state.pressed & 0x5000) {
                int k;

                play_confirm_sound();
                pad_state.pressed = 0;
                k = sp->scroll * 5 + sp->cursor;
                if (k < sp->state.book.count) {
                    if (k != shown) {
                        u_long *p = CD_BUFFER;


                        cd_read_member(4, sp->state.book.kinds[k] + 200, p);
                        tim_load_images_at(p + 1, &sp->state.book.image, NULL, 1);
                        p = (u_long *)((u8 *)p + (*p + 4));
                        tim_load_images_at(p + 1, &detail, &detail_clut, 1);
                    }
                    sp->mode = 2;
                }
            }
            if (sp->scroll != first) {
                int count;
                int k;

                if (first < sp->scroll) {
                    count = (sp->scroll - first) * 5;
                    k = (first + 3) * 5;
                } else {
                    count = (first - sp->scroll) * 5;
                    k = sp->scroll * 5;
                }
                cd_state.load_cursor = CD_BUFFER;
                used = 0;
                for (i = 0; i < count; i++, k++) {
                    if (k < sp->state.book.count) {
                        if (used + 0x2800 > 0x12000) {
                            DrawSync(0);
                            cd_state.load_cursor = CD_BUFFER;
                            used = 0;
                        }
                        {
                            int size =
                                cd_read_member(4, sp->state.book.kinds[k], cd_state.load_cursor) << 11;

                            tim_load_images_at(cd_state.load_cursor, &sp->state.book.cell_tex[k % 15], NULL, 1);
                            used += size;
                            cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
                        }
                    }
                }
            }
        } else if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            sp->mode = 1;
        }
        frame_rate_update();
        menu_creature_book_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    mem_free(sp->state.book.names);
    mem_free(sp->state.book.kinds);
    mem_free(sp->state.book.entries);
    menu_load_main_messages();
    menu_reload_assets();
    pad_clear_input();
}

/* The storage page's cursor: eight columns (four per grid) by five rows. */
#define CURSOR_COL ((s16)(sp->cursor % 8))
#define CURSOR_ROW (sp->cursor / 8)

/*
 * The arrange items icon view: the items carried (ids0) and stored (ids1, the
 * count flag bit) as two four-column grids of five rows; confirming a cell
 * twice moves its item to the first free entry of the other list. Leaving
 * writes the flag bits back.
 */
ADDRESS(0x8005d25c, 0x16c8)
void menu_arrange_icons_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT clear;
    DVECTOR picture = {640, 0};
    char name0[] = "\\st\\itemcg\\icon1\\0000.tim";
    char name1[] = "\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR image = {896, 448};
    DVECTOR clut = {544, 502};
    RECT move;
    s16 r0;
    s16 r1;
    s16 shown;
    s16 n;
    s16 id;
    s16 idx;
    s16 k;
    u_long *p;
    int used;
    int total;
    int i;
    int hole;
    int empty;

    menu_show_loading_screen();
    sp->state.grid.messages = mem_alloc(21504);
    p = cd_file_buffer;
    cd_read_member(3, 1, p);
    copy_words((u32 *)&menu_messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (*p + 4));
    tim_load_images_at(p + 1, &sp->image_vram[0], &sp->image_vram[1], 1);
    p = cd_file_buffer;
    cd_read_member(3, 4, p);
    copy_words((u32 *)sp->state.grid.messages, (u32 *)(p + 1), 5376);
    p = (u_long *)((u8 *)p + (*p + 4));
    tim_load_images_at(p + 1, &sp->image_vram[2], &sp->image_vram[3], 1);
    cd_read_member(3, 5, cd_file_buffer);
    tim_load_images_at(cd_file_buffer, &sp->image_vram[4], NULL, 1);
    DrawSync(0);
    cd_read_member(3, 7, cd_file_buffer);
    tim_load_images_at(cd_file_buffer, &image, &clut, 1);
    DrawSync(0);
    sp->state.grid.hand_count = 0;
    sp->state.grid.bag_count = 0;
    sp->state.grid.hand_ids = mem_alloc(628);
    sp->state.grid.bag_ids = mem_alloc(628);
    if (sp->state.grid.hand_ids == NULL || sp->state.grid.bag_ids == NULL)
        exit(1);
    fill_words((u32 *)sp->state.grid.hand_ids, -1, 157);
    fill_words((u32 *)sp->state.grid.bag_ids, -1, 157);
    sp->state.grid.hand_count = 0;
    sp->state.grid.bag_count = 0;
    for (i = 0; i < 314; i++) {
        if (vis_table_c.types[i].unk5[0] != 0xff && i < 314 && i != 266
            && inventory.counts[i].count != 0 && i != viewer.equipment[0] && i != viewer.equipment[1]
            && i != viewer.equipment[2] && i != viewer.equipment[3] && i != viewer.equipment[4]
            && i != viewer.equipment[5] && i != viewer.equipment[7] && i != viewer.equipment[8]
            && i != viewer.equipment[9] && i != viewer.equipment[10] && i != viewer.equipment[11]
            && i != viewer.equipment[13] && i != viewer.equipment[14] && i != viewer.equipment[15]
            && i != viewer.equipment[16] && i != viewer.equipment[17] && i != viewer.equipment[18]
            && i != viewer.equipment[19] && i != viewer.equipment[20]) {
            if (!inventory.counts[i].in_bag)
                sp->state.grid.hand_ids[sp->state.grid.hand_count++] = i;
            else
                sp->state.grid.bag_ids[sp->state.grid.bag_count++] = i;
        }
    }
    sp->state.grid.cell_tex = mem_alloc(160);
    if (sp->state.grid.cell_tex == NULL)
        exit(1);
    for (i = 0; i < 20; i++) {
        sp->state.grid.cell_tex[i].vx = i / 4 * 56 + 640;
        sp->state.grid.cell_tex[i].vy = i % 4 * 28 + 124;
    }
    for (i = 20; i < 29; i++) {
        sp->state.grid.cell_tex[i].vx = 960;
        sp->state.grid.cell_tex[i].vy = (i - 20) * 28;
    }
    for (i = 29; i < 40; i++) {
        sp->state.grid.cell_tex[i].vx = (i - 29) / 9 * 56 + 640;
        sp->state.grid.cell_tex[i].vy = (i - 29) % 9 * 28 + 256;
    }
    i = 0;
    total = 0;
    clear.w = 56;
    clear.h = 28;
    cd_state.load_cursor = CD_BUFFER;
    for (n = 40; n--; i++) {
        int size;

        if (total + 0x1000 > 0x12000) {
            DrawSync(0);
            cd_state.load_cursor = CD_BUFFER;
            total = 0;
        }
        if (i < 20 && i < sp->state.grid.hand_count) {
            size = cd_read_member(6, vis_table_c.types[sp->state.grid.hand_ids[i]].unk5[0],
                                 cd_state.load_cursor) << 11;

            tim_load_images_at(cd_state.load_cursor, &sp->state.grid.cell_tex[i], NULL, 1);
            total += size;
            cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
        } else if (i >= 20 && i - 20 < sp->state.grid.bag_count) {
            size = cd_read_member(6, vis_table_c.types[sp->state.grid.bag_ids[i - 20]].unk5[0],
                                 cd_state.load_cursor) << 11;

            tim_load_images_at(cd_state.load_cursor, &sp->state.grid.cell_tex[i], NULL, 1);
            total += size;
            cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
        } else {
            clear.x = sp->state.grid.cell_tex[i].vx;
            clear.y = sp->state.grid.cell_tex[i].vy;
            ClearImage(&clear, 0, 0, 0);
        }
    }
    shown = -1;
    sp->cursor = 0;
    sp->state.grid.hand_row = 0;
    sp->state.grid.bag_row = 0;
    sp->mode = 1;
    clear_vram();
    DrawSync(0);
    gfx_setup_640x240();
    pad_clear_input();
    for (;;) {
        r0 = sp->state.grid.hand_row;
        r1 = sp->state.grid.bag_row;
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            if (sp->mode & 0x8000) {
                sp->mode &= 0x7fff;
            } else if (sp->mode == 2) {
                sp->mode = 1;
            } else {
                break;
            }
        }
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            if (sp->mode & 0x8000) {
                if (CURSOR_COL < 4) {
                    idx = CURSOR_ROW * 4 + CURSOR_COL + sp->state.grid.hand_row * 4;
                    k = sp->state.grid.bag_count;
                    for (hole = 0; hole < sp->state.grid.bag_count; hole++) {
                        if (sp->state.grid.bag_ids[hole] == -1) {
                            k = hole;
                            break;
                        }
                    }
                    sp->state.grid.bag_ids[k] = sp->state.grid.hand_ids[idx];
                    sp->state.grid.hand_ids[idx] = -1;
                    idx %= 20;
                    move.x = sp->state.grid.cell_tex[idx].vx;
                    move.y = sp->state.grid.cell_tex[idx].vy;
                    move.w = 56;
                    move.h = 28;
                    if (k >= sp->state.grid.bag_row * 4 && k < sp->state.grid.bag_row * 4 + 20) {
                        k %= 20;
                        MoveImage(&move, sp->state.grid.cell_tex[k + 20].vx,
                                  sp->state.grid.cell_tex[k + 20].vy);
                    }
                    ClearImage(&move, 0, 0, 0);
                    sp->state.grid.bag_count++;
                    sp->state.grid.hand_count--;
                } else {
                    idx = CURSOR_ROW * 4 + CURSOR_COL + (sp->state.grid.bag_row * 4 - 4);
                    k = sp->state.grid.hand_count;
                    for (hole = 0; hole < sp->state.grid.hand_count; hole++) {
                        if (sp->state.grid.hand_ids[hole] == -1) {
                            k = hole;
                            break;
                        }
                    }
                    sp->state.grid.hand_ids[k] = sp->state.grid.bag_ids[idx];
                    sp->state.grid.bag_ids[idx] = -1;
                    idx %= 20;
                    move.x = sp->state.grid.cell_tex[idx + 20].vx;
                    move.y = sp->state.grid.cell_tex[idx + 20].vy;
                    move.w = 56;
                    move.h = 28;
                    if (k >= sp->state.grid.hand_row * 4 && k < sp->state.grid.hand_row * 4 + 20) {
                        k %= 20;
                        MoveImage(&move, sp->state.grid.cell_tex[k].vx, sp->state.grid.cell_tex[k].vy);
                    }
                    ClearImage(&move, 0, 0, 0);
                    sp->state.grid.hand_count++;
                    sp->state.grid.bag_count--;
                }
                sp->mode &= 0x7fff;
            } else {
                empty = 0;
                if (CURSOR_COL < 4) {
                    int cell = (sp->state.grid.hand_row + CURSOR_ROW) * 4;

                    if (sp->state.grid.hand_ids[cell + CURSOR_COL] == -1)
                        empty = 1;
                } else {
                    int cell = (sp->state.grid.bag_row + CURSOR_ROW) * 4 - 4;

                    if (sp->state.grid.bag_ids[cell + CURSOR_COL] == -1)
                        empty = 1;
                }
                if (!empty)
                    sp->mode |= 0x8000;
            }
        }
        if (!(sp->mode & 0x8000)) {
            if (pad_state.pressed & 0x1000) {
                play_confirm_sound();
                pad_state.pressed = 0;
                if (sp->mode == 1)
                    sp->mode = 2;
                else if (sp->mode == 2)
                    sp->mode = 1;
            }
            if (pad_state.pressed_repeat & 0x20) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor % 8 != 7)
                    sp->cursor++;
            }
            if (pad_state.pressed_repeat & 0x80) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor & 7)
                    sp->cursor--;
            }
            if (pad_state.pressed_repeat & 0x10) {
                play_cursor_sound();
                if (pad_state.held & 0x8000) {
                    if (CURSOR_COL < 4) {
                        sp->state.grid.hand_row -= 5;
                        if (sp->state.grid.hand_row < 0)
                            sp->state.grid.hand_row = 0;
                    } else {
                        sp->state.grid.bag_row -= 5;
                        if (sp->state.grid.bag_row < 0)
                            sp->state.grid.bag_row = 0;
                    }
                } else if (sp->cursor >= 8) {
                    sp->cursor -= 8;
                } else if (CURSOR_COL < 4) {
                    if (sp->state.grid.hand_row > 0)
                        sp->state.grid.hand_row--;
                } else if (sp->state.grid.bag_row > 0) {
                    sp->state.grid.bag_row--;
                }
                pad_state.pressed_repeat = 0;
            }
            if (pad_state.pressed_repeat & 0x40) {
                play_cursor_sound();
                if (pad_state.held & 0x8000) {
                    if (CURSOR_COL < 4) {
                        sp->state.grid.hand_row += 5;
                        if (sp->state.grid.hand_row > 73)
                            sp->state.grid.hand_row = 73;
                    } else {
                        sp->state.grid.bag_row += 5;
                        if (sp->state.grid.bag_row > 73)
                            sp->state.grid.bag_row = 73;
                    }
                } else if (sp->cursor < 32) {
                    sp->cursor += 8;
                } else if (CURSOR_COL < 4) {
                    if (sp->state.grid.hand_row < 73)
                        sp->state.grid.hand_row++;
                } else if (sp->state.grid.bag_row < 73) {
                    sp->state.grid.bag_row++;
                }
                pad_state.pressed_repeat = 0;
            }
            if (pad_state.pressed_repeat & 0x400) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->state.grid.hand_row -= 5;
                if (sp->state.grid.hand_row < 0)
                    sp->state.grid.hand_row = 0;
            }
            if (pad_state.pressed_repeat & 0x100) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->state.grid.hand_row += 5;
                if (sp->state.grid.hand_row > 73)
                    sp->state.grid.hand_row = 73;
            }
            if (pad_state.pressed_repeat & 0x800) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->state.grid.bag_row -= 5;
                if (sp->state.grid.bag_row < 0)
                    sp->state.grid.bag_row = 0;
            }
            if (pad_state.pressed_repeat & 0x200) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->state.grid.bag_row += 5;
                if (sp->state.grid.bag_row > 73)
                    sp->state.grid.bag_row = 73;
            }
            if (sp->mode == 2) {
                if (CURSOR_COL < 4)
                    id = sp->state.grid.hand_ids[sp->state.grid.hand_row * 4 + (CURSOR_ROW * 4 + CURSOR_COL)];
                else
                    id = sp->state.grid.bag_ids[sp->state.grid.bag_row * 4 + (CURSOR_ROW * 4 + CURSOR_COL) - 4];
                if (shown != id && id != -1) {
                    cd_read_member(6, vis_table_c.types[id].unk5[0] + 200, cd_file_buffer);
                    tim_load_images_at(cd_file_buffer, &picture, NULL, 1);
                    shown = id;
                }
            }
            if (sp->state.grid.hand_row != r0) {
                if (r0 < sp->state.grid.hand_row) {
                    n = (sp->state.grid.hand_row - r0) * 4;
                    i = sp->state.grid.hand_row * 4 + 20 - n;
                } else {
                    n = (r0 - sp->state.grid.hand_row) * 4;
                    i = sp->state.grid.hand_row * 4;
                }
                used = 0;
                clear.w = 56;
                clear.h = 28;
                cd_state.load_cursor = CD_BUFFER;
                while (n--) {
                    if (used + 0x1000 > 0x12000) {
                        DrawSync(0);
                        cd_state.load_cursor = CD_BUFFER;
                        used = 0;
                    }
                    if (sp->state.grid.hand_ids[i] != -1) {
                        int size = cd_read_member(
                            6, vis_table_c.types[sp->state.grid.hand_ids[i]].unk5[0], cd_state.load_cursor) << 11;

                        tim_load_images_at(cd_state.load_cursor, &sp->state.grid.cell_tex[i % 20], NULL, 1);
                        used += size;
                        cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
                    } else {
                        clear.x = sp->state.grid.cell_tex[i % 20].vx;
                        clear.y = sp->state.grid.cell_tex[i % 20].vy;
                        ClearImage(&clear, 0, 0, 0);
                    }
                    i++;
                }
            }
            if (sp->state.grid.bag_row != r1) {
                if (r1 < sp->state.grid.bag_row) {
                    n = (sp->state.grid.bag_row - r1) * 4;
                    i = sp->state.grid.bag_row * 4 + 20 - n;
                } else {
                    n = (r1 - sp->state.grid.bag_row) * 4;
                    i = sp->state.grid.bag_row * 4;
                }
                used = 0;
                clear.w = 56;
                clear.h = 28;
                cd_state.load_cursor = CD_BUFFER;
                while (n--) {
                    if (used + 0x1000 > 0x12000) {
                        DrawSync(0);
                        cd_state.load_cursor = CD_BUFFER;
                        used = 0;
                    }
                    if (sp->state.grid.bag_ids[i] != -1) {
                        int size = cd_read_member(
                            6, vis_table_c.types[sp->state.grid.bag_ids[i]].unk5[0], cd_state.load_cursor) << 11;

                        tim_load_images_at(cd_state.load_cursor, &sp->state.grid.cell_tex[i % 20 + 20], NULL, 1);
                        used += size;
                        cd_state.load_cursor = (u_long *)((u8 *)cd_state.load_cursor + size);
                    } else {
                        clear.x = sp->state.grid.cell_tex[i % 20 + 20].vx;
                        clear.y = sp->state.grid.cell_tex[i % 20 + 20].vy;
                        ClearImage(&clear, 0, 0, 0);
                    }
                    i++;
                }
            }
        }
        frame_rate_update();
        menu_arrange_icons_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    for (i = 0, n = sp->state.grid.hand_count; n;) {
        if (sp->state.grid.hand_ids[i] != -1) {
            inventory.counts[sp->state.grid.hand_ids[i]].in_bag = 0;
            n--;
        }
        i++;
    }
    for (i = 0, n = sp->state.grid.bag_count; n;) {
        if (sp->state.grid.bag_ids[i] != -1) {
            inventory.counts[sp->state.grid.bag_ids[i]].in_bag = 1;
            n--;
        }
        i++;
    }
    menu_load_main_messages();
    mem_free(sp->state.grid.hand_ids);
    mem_free(sp->state.grid.bag_ids);
    mem_free(sp->state.grid.cell_tex);
    mem_free(sp->state.grid.messages);
    menu_reload_assets();
    pad_clear_input();
}

#undef CURSOR_COL
#undef CURSOR_ROW

/*
 * Lets the player spend points on the fourteen attributes (two columns of
 * seven, none above 999), then adds them to viewer.base_attributes and the total in
 * viewer.sp_total. Returns 0 when cancelled, else 1.
 */
ADDRESS(0x8005e924, 0x2ec)
int menu_attribute_points_run(int points)
{
    MenuScratch *sp = MENU_SCRATCH;
    int result;
    int i;

    sp->state.stats.value = points;
    sp->state.stats.selected = 0;
    for (i = 0; i < 14; i++) {
        sp->state.stats.base[0][i] = viewer.base_attributes[i];
        sp->state.stats.bonus[0][i] = 0;
    }
    pad_clear_input();
    do {
        if (pad_state.pressed & 0x2000) {
            pad_state.pressed = 0;
            play_cancel_sound();
            result = 0;
            goto done;
        }
        if (pad_state.pressed_repeat & 0x10) {
            pad_state.pressed_repeat = 0;
            play_cursor_sound();
            if (sp->state.stats.selected % 7 == 0)
                sp->state.stats.selected += 6;
            else
                sp->state.stats.selected--;
        }
        if (pad_state.pressed_repeat & 0x40) {
            pad_state.pressed_repeat = 0;
            play_cursor_sound();
            if (sp->state.stats.selected % 7 == 6)
                sp->state.stats.selected -= 6;
            else
                sp->state.stats.selected++;
        }
        if (pad_state.pressed_repeat & 0xa0) {
            pad_state.pressed_repeat = 0;
            play_cursor_sound();
            if (sp->state.stats.selected < 7)
                sp->state.stats.selected += 7;
            else
                sp->state.stats.selected -= 7;
        }
        if (pad_state.pressed_repeat & 0x4000) {
            pad_state.pressed_repeat = 0;
            if (sp->state.stats.value > 0) {
                if (sp->state.stats.base[0][sp->state.stats.selected]
                        + sp->state.stats.bonus[0][sp->state.stats.selected]
                    < 999) {
                    if (sp->state.stats.value >= 2)
                        play_confirm_sound();
                    sp->state.stats.bonus[0][sp->state.stats.selected]++;
                    sp->state.stats.value--;
                } else {
                    play_refuse_sound();
                }
            }
        }
        frame_rate_update();
        menu_attribute_points_draw();
    } while (sp->state.stats.value > 0);
    menu_attribute_points_draw();
    for (i = 0; i < 14; i++) {
        viewer.base_attributes[i] += sp->state.stats.bonus[0][i];
        viewer.sp_total += sp->state.stats.bonus[0][i];
    }
    result = 1;
done:
    pad_clear_input();
    return result;
}

/*
 * The item use page: lists the usable items held (table C ids 268..313
 * of kind 20 or 30), five rows with their icons, and uses the confirmed
 * one (item_use). Returns the use result (game_progress.item_use_result), 0 when
 * cancelled; items from 310 and item 288 recompute the viewer's figures.
 */
ADDRESS(0x8005ec10, 0x77c)
int menu_use_item_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int result = 0;
    u8 icons[48] = {
        0x00, 0xff, 0x01, 0x02, 0x03, 0xff, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0a, 0x0b, 0x0c, 0xff, 0xff, 0xff, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0xff,
        0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
        0x1e, 0x1f, 0x20, 0x21, 0xff, 0xff, 0xff, 0xff, 0x22, 0x22, 0x22, 0x22,
    };
    RECT rect;
    RECT row;
    u8 *image;
    u_long *p;
    int reload;
    int first;
    int from;
    int to;
    int n;
    int i;
    int k;
    s16 id;

    sp->state.page_list.icons[0].vx = 896;
    sp->state.page_list.icons[0].vy = 440;
    sp->state.page_list.icons[1].vx = 920;
    sp->state.page_list.icons[1].vy = 440;
    sp->state.page_list.icons[2].vx = 896;
    sp->state.page_list.icons[2].vy = 464;
    sp->state.page_list.icons[3].vx = 920;
    sp->state.page_list.icons[3].vy = 464;
    sp->state.page_list.icons[4].vx = 896;
    sp->state.page_list.icons[4].vy = 488;
    sp->state.block = mem_alloc(21504);
    image = (u8 *)CD_BUFFER;
    cd_read_member(3, 3, (u_long *)image);
    copy_words(sp->state.block, (u32 *)(image + 4), 5376);
    image += *(u32 *)image + 4;
    tim_load_images_at(image + 4, &sp->image_vram[2], &sp->image_vram[3], 1);
    cd_state.load_cursor = CD_BUFFER;
    cd_read_member(6, 800, cd_state.load_cursor);
    sp->state.page_list.entries = mem_alloc(92);
    sp->state.page_list.count = 0;
    for (i = 268; i < 314; i++) {
        if ((vis_table_c.types[i].category == 20 || vis_table_c.types[i].category == 30)
            && inventory.counts[i].count != 0 && inventory.counts[i].in_bag == 0)
            sp->state.page_list.entries[sp->state.page_list.count++] = i;
    }
    n = 5;
    if (sp->state.page_list.count < 5)
        n = sp->state.page_list.count;
    for (i = 0; i < n; i++) {
        k = icons[sp->state.page_list.entries[i] - 266];
        p = CD_BUFFER;
        while (k--)
            p = (u_long *)((u8 *)p + (*p + 4));
        rect.x = sp->state.page_list.icons[i].vx;
        rect.y = sp->state.page_list.icons[i].vy;
        rect.w = rect.h = 24;
        LoadImage(&rect, p + 6);
    }
    sp->scroll = 0;
    sp->cursor = 0;
    pad_clear_input();
    reload = 0;
    for (;;) {
        first = sp->scroll;
        if (pad_state.pressed & 0x4000) {
            pad_state.pressed = 0;
            if (sp->scroll + sp->cursor < sp->state.page_list.count)
                item_use(sp->state.page_list.entries[sp->scroll + sp->cursor]);
            else
                game_progress.item_use_result = 2;
            if (game_progress.item_use_result == 1)
                play_confirm_sound();
            else if (game_progress.item_use_result == 2)
                play_refuse_sound();
            else
                play_cancel_sound();
            if (game_progress.item_use_result != 2) {
                result = game_progress.item_use_result;
                if (game_progress.item_use_result == 1) {
                    id = sp->state.page_list.entries[sp->scroll + sp->cursor];
                    if (id >= 310 || id == 288)
                        reload = 1;
                }
                break;
            }
        }
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            break;
        }
        if (pad_state.pressed_repeat & 0x10) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            if (sp->cursor == 0) {
                if (sp->scroll > 0)
                    sp->scroll--;
            } else {
                sp->cursor--;
            }
            sp->blink_phase = 0;
        }
        if (pad_state.pressed_repeat & 0x40) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            if (sp->cursor == 4) {
                if (sp->scroll < sp->state.page_list.count - 5)
                    sp->scroll++;
            } else {
                sp->cursor++;
            }
            sp->blink_phase = 0;
        }
        if (pad_state.pressed_repeat & 0x20) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->scroll += 5;
            if (sp->scroll > sp->state.page_list.count - 5) {
                if (sp->state.page_list.count < 5)
                    sp->scroll = 0;
                else
                    sp->scroll = sp->state.page_list.count - 5;
            }
            sp->blink_phase = 0;
        }
        if (pad_state.pressed_repeat & 0x80) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->scroll -= 5;
            if (sp->scroll < 0)
                sp->scroll = 0;
            sp->blink_phase = 0;
        }
        if (sp->scroll != first) {
            if (first < sp->scroll) {
                from = first + 5;
                to = sp->scroll + 5;
            } else {
                from = sp->scroll;
                to = first;
            }
            for (i = from; i < to; i++) {
                k = icons[sp->state.page_list.entries[i] - 266];
                p = CD_BUFFER;
                while (k--)
                    p = (u_long *)((u8 *)p + (*p + 4));
                row.x = sp->state.page_list.icons[i % 5].vx;
                row.y = sp->state.page_list.icons[i % 5].vy;
                row.w = row.h = 24;
                LoadImage(&row, p + 6);
            }
        }
        frame_rate_update();
        menu_use_item_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
    mem_free(sp->state.block);
    mem_free(sp->state.page_list.entries);
    if (reload)
        player_compute_derived_stats(0);
    {
        u8 *restore = (u8 *)CD_BUFFER;
        cd_read_member(3, 9, (u_long *)restore);
        tim_load_images_at(restore + 4, NULL, NULL, 0);
        restore += *(u32 *)restore + 4;
        tim_load_images_at(restore + 4, NULL, NULL, 0);
    }
    return result;
}

/*
 * Copies the texture page at 896,256 up to 896,0, then loads the menu
 * messages (CD archive 3 member 0) and their font image, and the message
 * copy and image of member 3.
 */
ADDRESS(0x8005f38c, 0x170)
void shop_open(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT page = {896, 256, 64, 256};
    u8 *p;

    sp->image_vram[0].vx = 960;
    sp->image_vram[0].vy = 256;
    sp->image_vram[1].vx = 560;
    sp->image_vram[1].vy = 495;
    sp->image_vram[2].vx = 896;
    sp->image_vram[2].vy = 256;
    sp->image_vram[3].vx = 560;
    sp->image_vram[3].vy = 497;
    sp->state.block = mem_alloc(21504);
    MoveImage(&page, 896, 0);
    cd_wait_idle();
    p = (u8 *)cd_file_buffer;
    cd_read_member(3, 0, (u_long *)p);
    copy_words((u32 *)&menu_messages, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    tim_load_images_at(p + 4, &sp->image_vram[0], &sp->image_vram[1], 1);
    p = (u8 *)cd_file_buffer;
    cd_read_member(3, 3, (u_long *)p);
    copy_words(sp->state.block, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    tim_load_images_at(p + 4, &sp->image_vram[2], &sp->image_vram[3], 1);
    menu_store_screen();
    pad_clear_input();
}

/* Copies the saved page back to 896,256. */
ADDRESS(0x8005f4fc, 0x50)
void shop_restore_texture_page(void)
{
    RECT saved = {896, 0, 64, 256};

    MoveImage(&saved, 896, 256);
}

/*
 * The shop screens over the game (shop_open opens them): unk2DC bit 0
 * lists the shop's stock (inventory.shop_ids ids with stock left and
 * item_is_uncollected set), bits 1/2 the items held but not equipped to trade
 * for items 284/285, otherwise the damaged items held to repair for
 * viewer.hp. Bit 15 is the confirmation step.
 *
 * The trade test is written as two bit tests (fold merges them into one
 * word-wide read of unk2DC, which is retail's `lw`). The three removals
 * each close their own list gap and take the same exit, the buy only
 * when the stock runs out; jump2 merges the tails. The item's record is
 * indexed through vis_table_c each time, never through a pointer.
 */
ADDRESS(0x8005f54c, 0xacc)
void shop_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};
    int shown;
    int done;
    int cost;
    int kind;
    int i;

    shop_open();
    sp->scroll = 0;
    sp->cursor = 0;
    if (sp->mode & 1) {
        sp->state.list.ids = mem_alloc(28);
        sp->state.list.count = 0;
        for (i = 0; i < 14; i++) {
            if (inventory.shop_stock[i] != 0 && item_is_uncollected(inventory.shop_ids[i]) != 0)
                sp->state.list.ids[sp->state.list.count++] = i;
        }
    } else if ((sp->mode & 2) || (sp->mode & 4)) {
        sp->state.list.ids = mem_alloc(530);
        sp->state.list.count = 0;
        for (i = 0; i < 266; i++) {
            if (inventory.counts[i].count != 0 && i != viewer.equipment[0] && i != viewer.equipment[1]
                && i != viewer.equipment[2] && i != viewer.equipment[3] && i != viewer.equipment[4]
                && i != viewer.equipment[5] && i != viewer.equipment[7] && i != viewer.equipment[8]
                && i != viewer.equipment[9] && i != viewer.equipment[10] && i != viewer.equipment[11]
                && i != viewer.equipment[13] && i != viewer.equipment[14] && i != viewer.equipment[15]
                && i != viewer.equipment[16] && i != viewer.equipment[17] && i != viewer.equipment[18]
                && i != viewer.equipment[19] && i != viewer.equipment[20])
                sp->state.list.ids[sp->state.list.count++] = i;
        }
    } else {
        sp->state.list.ids = mem_alloc(530);
        sp->state.list.count = 0;
        for (i = 0; i < 266; i++) {
            if (inventory.counts[i].count != 0
                && vis_table_c.types[i].max_durability != vis_table_c.types[i].durability)
                sp->state.list.ids[sp->state.list.count++] = i;
        }
    }
    shown = -1;
    for (;;) {
        if (sp->mode & 0x8000) {
            if (pad_state.pressed & 0x2000) {
                play_cancel_sound();
                pad_state.pressed = 0;
                sp->mode &= ~0x8000;
            }
            if (pad_state.pressed & 0x4000) {
                pad_state.pressed = 0;
                if ((s16)sp->state.list.confirm_choice == 1) {
                    done = 1;
                    if (sp->mode & 1) {
                        int id = inventory.shop_ids[sp->state.list.ids[sp->scroll + sp->cursor]];

                        if (inventory.counts[266].count < vis_table_c.types[id].price) {
                            done = 0;
                            play_refuse_sound();
                        } else {
                            inventory.counts[266].count -= vis_table_c.types[id].price;
                            inventory.counts[id].count++;
                            inventory.shop_stock[sp->state.list.ids[sp->scroll + sp->cursor]]--;
                            if (inventory.shop_stock[sp->state.list.ids[sp->scroll + sp->cursor]] == 0) {
                                for (i = sp->scroll + sp->cursor; i < sp->state.list.count - 1; i++)
                                    sp->state.list.ids[i] = sp->state.list.ids[i + 1];
                                sp->state.list.count--;
                            }
                            play_confirm_sound();
                            sp->mode &= ~0x8000;
                        }
                    } else if ((sp->mode & 2) || (sp->mode & 4)) {
                        int exchange = 285;
                        int j;

                        if (sp->mode & 2)
                            exchange = 284;
                        inventory.counts[sp->state.list.ids[sp->scroll + sp->cursor]].count--;
                        inventory.counts[exchange].count++;
                        for (j = sp->scroll + sp->cursor; j < sp->state.list.count - 1; j++)
                            sp->state.list.ids[j] = sp->state.list.ids[j + 1];
                        sp->state.list.count--;
                        play_confirm_sound();
                        sp->mode &= ~0x8000;
                    } else {
                        int id = sp->state.list.ids[sp->scroll + sp->cursor];
                        int j;

                        cost = vis_table_c.types[id].repair_price * 2 / 3
                            * (vis_table_c.types[id].max_durability - vis_table_c.types[id].durability);
                        if (vis_table_c.types[id].durability == 0)
                            cost *= 2;
                        if (cost < viewer.hp) {
                            viewer.hp -= cost;
                            vis_table_c.types[id].durability = vis_table_c.types[id].max_durability;
                            for (j = sp->scroll + sp->cursor; j < sp->state.list.count - 1; j++)
                                sp->state.list.ids[j] = sp->state.list.ids[j + 1];
                            sp->state.list.count--;
                            play_confirm_sound();
                            sp->mode &= ~0x8000;
                        } else {
                            done = 0;
                            play_refuse_sound();
                        }
                    }
                    if (done && sp->scroll + 4 >= sp->state.list.count) {
                        if (sp->scroll > 0)
                            sp->scroll--;
                        else if (sp->cursor > 0)
                            sp->cursor--;
                    }
                } else {
                    play_cancel_sound();
                    pad_state.pressed = 0;
                    sp->mode &= ~0x8000;
                }
            }
            if (pad_state.pressed_repeat & 0x80) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->blink_phase = 0;
                sp->state.list.confirm_choice = 1;
            }
            if (pad_state.pressed_repeat & 0x20) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                sp->blink_phase = 0;
                sp->state.list.confirm_choice = 0;
            }
        } else {
            if (pad_state.pressed & 0x2000) {
                play_cancel_sound();
                pad_state.pressed = 0;
                break;
            }
            if (pad_state.pressed_repeat & 0x10) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor == 0) {
                    if (sp->scroll > 0)
                        sp->scroll--;
                } else {
                    sp->cursor--;
                }
            }
            if (pad_state.pressed_repeat & 0x40) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor == 4) {
                    if (sp->scroll < sp->state.list.count - 5)
                        sp->scroll++;
                } else {
                    sp->cursor++;
                }
            }
            if (pad_state.pressed & 0x4000) {
                pad_state.pressed = 0;
                if (sp->scroll + sp->cursor >= sp->state.list.count) {
                    play_refuse_sound();
                } else {
                    if (sp->mode & 1)
                        kind = vis_table_c.types[inventory.shop_ids[sp->state.list.ids[sp->scroll + sp->cursor]]].unk5[0];
                    else
                        kind = vis_table_c.types[sp->state.list.ids[sp->scroll + sp->cursor]].unk5[0];
                    play_confirm_sound();
                    sp->state.list.confirm_choice = 1;
                    sp->mode |= 0x8000;
                    if (kind != shown) {
                        cd_read_member(6, kind + 600, cd_file_buffer);
                        tim_load_images_at(cd_file_buffer, &pos, NULL, 1);
                        shown = kind;
                    }
                }
            }
        }
        frame_rate_update();
        shop_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    shop_restore_texture_page();
    pad_clear_input();
    mem_free(sp->state.list.ids);
    mem_free(sp->state.block);
    menu_restore_screen();
}

/*
 * The area list page: marks the areas not yet visited (89) and the groups
 * with none visited (99), shows ten rows from the first row and scrolls
 * them while the list is shown (area_info_list_shown).
 */
ADDRESS(0x80060018, 0x31c)
void menu_area_info_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s32 areas[35] = {
        90, 4,  5,  1,  6,  91, 11, 27, 12, 28, 15, 33, 92, 3,  20, 24, 18, 93,
        10, 21, 32, 37, 94, 9,  8,  13, 38, 95, 2,  17, 96, 25, 26, 14, 23,
    };
    s32 *head;
    int seen;
    int moved;
    int i;

    head = NULL;
    pad_clear_input();
    seen = 0;
    sp->state.map.scroll = 0;
    sp->state.map.current = -1;
    for (i = 0; i < 35; i++) {
        if (areas[i] >= 90) {
            if (head != NULL && !seen)
                *head = 99;
            head = &areas[i];
            seen = 0;
            continue;
        }
        if (game_progress.areas_visited[areas[i]] & 1)
            seen = 1;
        else
            areas[i] = 89;
        if (sp->state.map.current == -1 && areas[i] == area_state.area)
            sp->state.map.current = *head;
    }
    if (!seen)
        *head = 99;
    for (i = 0; i < 10; i++)
        sp->state.map.ids[i] = areas[i];
    for (;;) {
        moved = 0;
        if (pad_state.pressed & 0x2000) {
            area_info_list_shown = 0;
            pad_state.pressed = 0;
            play_cancel_sound();
            break;
        }
        if (area_info_list_shown == 1) {
            if (pad_state.pressed_repeat & 0x10) {
                pad_state.pressed_repeat = 0;
                if (sp->state.map.scroll > 0) {
                    play_cursor_sound();
                    moved = 1;
                    sp->state.map.scroll--;
                }
            }
            if (pad_state.pressed_repeat & 0x40) {
                pad_state.pressed_repeat = 0;
                if (sp->state.map.scroll < 25) {
                    play_cursor_sound();
                    moved = 1;
                    sp->state.map.scroll++;
                }
            }
            DAT_8008818c = sp->state.map.scroll;
        } else {
            DAT_8008818c = 0;
        }
        if (moved) {
            for (i = 0; i < 10; i++)
                sp->state.map.ids[i] = areas[sp->state.map.scroll + i];
        }
        if (pad_state.pressed_repeat & 0x400) {
            pad_state.pressed_repeat = 0;
            area_info_list_shown = 0;
        }
        if (pad_state.pressed_repeat & 0x800) {
            pad_state.pressed_repeat = 0;
            area_info_list_shown = 1;
        }
        frame_rate_update();
        menu_area_info_draw();
    }
    pad_clear_input();
}

/*
 * Page 1 (unk2DC 1) edits the option bytes game_settings.sfx_volume (a 0..63
 * slider) and unk2..unk5 (on/off), row 5 opens page 2. Page 2 lists the
 * twelve button masks (pad_state.buttons) as button numbers
 * (MenuState.levels); a row waits for the next button (unk2E2), row 12
 * restores the defaults, row 13 returns, and buttons used twice mark
 * levels.flags.
 */
ADDRESS(0x80060334, 0x878)
void menu_options_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;


    u16 *key;

    sp->cursor = 0;
    sp->mode = 1;
    pad_clear_input();
    while (1) {
        if (sp->mode == 1) {
            if (pad_state.pressed & 0x2000) {
                play_cancel_sound();
                pad_state.pressed = 0;
                break;
            }
            if (pad_state.pressed_repeat & 0x10) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor == 0)
                    sp->cursor = 5;
                else
                    sp->cursor--;
            }
            if (pad_state.pressed_repeat & 0x40) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor == 5)
                    sp->cursor = 0;
                else
                    sp->cursor++;
            }
            if (pad_state.pressed & 0x4000) {
                pad_state.pressed = 0;
                if (sp->cursor == 5) {
                    const u16 *decode = pad_state.buttons;
                    int i;

                    play_confirm_sound();
                    for (i = 0; i < 12; i++, decode++) {
                        if (*decode & 0x1000)
                            sp->state.levels.level[i] = 0;
                        else if (*decode & 0x2000)
                            sp->state.levels.level[i] = 1;
                        else if (*decode & 0x8000)
                            sp->state.levels.level[i] = 2;
                        else if (*decode & 0x4000)
                            sp->state.levels.level[i] = 3;
                        else if (*decode & 0x80)
                            sp->state.levels.level[i] = 4;
                        else if (*decode & 0x10)
                            sp->state.levels.level[i] = 5;
                        else if (*decode & 0x20)
                            sp->state.levels.level[i] = 6;
                        else if (*decode & 0x40)
                            sp->state.levels.level[i] = 7;
                        else if (*decode & 0x400)
                            sp->state.levels.level[i] = 8;
                        else if (*decode & 0x100)
                            sp->state.levels.level[i] = 9;
                        else if (*decode & 0x800)
                            sp->state.levels.level[i] = 10;
                        else if (*decode & 0x200)
                            sp->state.levels.level[i] = 11;
                    }
                    sp->cursor = 0;
                    sp->mode = 2;
                    sp->scroll = 0;
                    sp->state.levels.flags = 0;
                }
            }
            if (pad_state.pressed_repeat & 0x20) {
                u8 *option = &game_settings.sfx_volume;

                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor <= 0) {
                    if (option[sp->cursor] < 63)
                        option[sp->cursor]++;
                } else if (sp->cursor < 5) {
                    option[sp->cursor] = 0;
                }
            }
            if (pad_state.pressed_repeat & 0x80) {
                u8 *option = &game_settings.sfx_volume;

                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor <= 0) {
                    if (option[sp->cursor] != 0)
                        option[sp->cursor]--;
                } else if (sp->cursor < 5) {
                    option[sp->cursor] = 1;
                }
            }
        } else if (sp->scroll == 0) {
            if (pad_state.pressed & 0x2000) {
                if (sp->state.levels.flags == 0) {
                    pad_update_action_mask();
                    play_confirm_sound();
                    pad_state.pressed = 0;
                    sp->mode = 1;
                    sp->cursor = 5;
                } else {
                    play_refuse_sound();
                    pad_state.pressed = 0;
                }
            }
            if (pad_state.pressed & 0x4000) {
                pad_state.pressed = 0;
                if (sp->cursor < 12) {
                    play_confirm_sound();
                    sp->blink_phase = 0;
                    sp->scroll = 1;
                } else if (sp->cursor == 12) {
                    play_confirm_sound();
                    pad_set_default_buttons();
                    pad_update_action_mask();
                    sp->state.levels.level[0] = 5;
                    sp->state.levels.level[1] = 7;
                    sp->state.levels.level[2] = 4;
                    sp->state.levels.level[3] = 6;
                    sp->state.levels.level[4] = 8;
                    sp->state.levels.level[5] = 10;
                    sp->state.levels.level[6] = 9;
                    sp->state.levels.level[7] = 11;
                    sp->state.levels.level[9] = 2;
                    sp->state.levels.flags = 0;
                    sp->state.levels.level[8] = 0;
                    sp->state.levels.level[11] = 1;
                    sp->state.levels.level[10] = 3;
                } else if (sp->state.levels.flags == 0) {
                    pad_update_action_mask();
                    play_confirm_sound();
                    pad_state.pressed = 0;
                    sp->mode = 1;
                    sp->cursor = 5;
                } else {
                    play_refuse_sound();
                    pad_state.pressed = 0;
                }
            }
            if (pad_state.pressed & 8) {
                play_cursor_sound();
                pad_state.pressed = 0;
                sp->cursor = 13;
            }
            if (pad_state.pressed_repeat & 0x10) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor == 0)
                    sp->cursor = 12;
                else if (sp->cursor == 6)
                    sp->cursor = 13;
                else if (sp->cursor == 12)
                    sp->cursor = 5;
                else if (sp->cursor == 13)
                    sp->cursor = 11;
                else
                    sp->cursor--;
            }
            if (pad_state.pressed_repeat & 0x40) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor == 12)
                    sp->cursor = 0;
                else if (sp->cursor == 5)
                    sp->cursor = 12;
                else if (sp->cursor == 11)
                    sp->cursor = 13;
                else if (sp->cursor == 13)
                    sp->cursor = 6;
                else
                    sp->cursor++;
            }
            if (pad_state.pressed_repeat & 0x20) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor < 6)
                    sp->cursor += 6;
                else if (sp->cursor == 12)
                    sp->cursor = 13;
                else if (sp->cursor == 13)
                    sp->cursor = 12;
                else
                    sp->cursor -= 6;
            }
            if (pad_state.pressed_repeat & 0x80) {
                play_cursor_sound();
                pad_state.pressed_repeat = 0;
                if (sp->cursor < 7)
                    sp->cursor += 6;
                else if (sp->cursor == 12)
                    sp->cursor = 13;
                else if (sp->cursor == 13)
                    sp->cursor = 12;
                else
                    sp->cursor -= 6;
            }
        } else if (pad_state.pressed) {
            key = &pad_state.buttons[sp->cursor];
            if (pad_state.pressed & 0x1000) {
                sp->state.levels.level[sp->cursor] = 0;
                *key = 0x1000;
            } else if (pad_state.pressed & 0x2000) {
                sp->state.levels.level[sp->cursor] = 1;
                *key = 0x2000;
            } else if (pad_state.pressed & 0x8000) {
                sp->state.levels.level[sp->cursor] = 2;
                *key = 0x8000;
            } else if (pad_state.pressed & 0x4000) {
                sp->state.levels.level[sp->cursor] = 3;
                *key = 0x4000;
            } else if (pad_state.pressed & 0x80) {
                sp->state.levels.level[sp->cursor] = 4;
                *key = 0x80;
            } else if (pad_state.pressed & 0x10) {
                sp->state.levels.level[sp->cursor] = 5;
                *key = 0x10;
            } else if (pad_state.pressed & 0x20) {
                sp->state.levels.level[sp->cursor] = 6;
                *key = 0x20;
            } else if (pad_state.pressed & 0x40) {
                sp->state.levels.level[sp->cursor] = 7;
                *key = 0x40;
            } else if (pad_state.pressed & 0x400) {
                sp->state.levels.level[sp->cursor] = 8;
                *key = 0x400;
            } else if (pad_state.pressed & 0x100) {
                sp->state.levels.level[sp->cursor] = 9;
                *key = 0x100;
            } else if (pad_state.pressed & 0x800) {
                sp->state.levels.level[sp->cursor] = 10;
                *key = 0x800;
            } else if (pad_state.pressed & 0x200) {
                sp->state.levels.level[sp->cursor] = 11;
                *key = 0x200;
            }
            {
                const u16 *scan = pad_state.buttons;
                int used = 15;
                int i;

                sp->state.levels.flags = 0;
                for (i = 0; i < 12; i++, scan++) {
                    if (*scan & used)
                        sp->state.levels.flags |= *scan;
                    else
                        used |= *scan;
                }
            }
            pad_state.pressed = 0;
            pad_state.pressed_repeat = 0;
            sp->scroll = 0;
        }
        frame_rate_update();
        if (sp->mode == 2)
            menu_button_config_draw();
        else
            menu_options_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
}

/* Clears any pending SwCARD event. */
ADDRESS(0x80060bac, 0x54)
void card_clear_sw_events(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;

    TestEvent(sp->sw_events[0]);
    TestEvent(sp->sw_events[1]);
    TestEvent(sp->sw_events[2]);
    TestEvent(sp->sw_events[3]);
}

/* Clears any pending HwCARD event. */
ADDRESS(0x80060c00, 0x54)
void card_clear_hw_events(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;

    TestEvent(sp->hw_events[0]);
    TestEvent(sp->hw_events[1]);
    TestEvent(sp->hw_events[2]);
    TestEvent(sp->hw_events[3]);
}

ADDRESS(0x80060c54, 0x190)
void card_open_events(void)
{
    MenuCardScratch *sp;

    EnterCriticalSection();
    MENU_CARD_SCRATCH->sw_events[0] = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->sw_events[1] = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->sw_events[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->sw_events[3] = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[0] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[1] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[2] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    MENU_CARD_SCRATCH->hw_events[3] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    sp = MENU_CARD_SCRATCH;
    EnableEvent(sp->sw_events[0]);
    EnableEvent(sp->sw_events[1]);
    EnableEvent(sp->sw_events[2]);
    EnableEvent(sp->sw_events[3]);
    EnableEvent(sp->hw_events[0]);
    EnableEvent(sp->hw_events[1]);
    EnableEvent(sp->hw_events[2]);
    EnableEvent(sp->hw_events[3]);
    ExitCriticalSection();
}

ADDRESS(0x80060de4, 0xf0)
void card_close_events(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;

    EnterCriticalSection();
    DisableEvent(sp->sw_events[0]);
    DisableEvent(sp->sw_events[1]);
    DisableEvent(sp->sw_events[2]);
    DisableEvent(sp->sw_events[3]);
    DisableEvent(sp->hw_events[0]);
    DisableEvent(sp->hw_events[1]);
    DisableEvent(sp->hw_events[2]);
    DisableEvent(sp->hw_events[3]);
    CloseEvent(sp->sw_events[0]);
    CloseEvent(sp->sw_events[1]);
    CloseEvent(sp->sw_events[2]);
    CloseEvent(sp->sw_events[3]);
    CloseEvent(sp->hw_events[0]);
    CloseEvent(sp->hw_events[1]);
    CloseEvent(sp->hw_events[2]);
    CloseEvent(sp->hw_events[3]);
    ExitCriticalSection();
}

/* Waits for a SwCARD event: 0 done, 1 error, 2 timeout, 3 new card. */
ADDRESS(0x80060ed4, 0xb4)
int card_wait_sw_event(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;
    long ioe, error, timeout, new;

    while (1) {
        ioe = TestEvent(sp->sw_events[0]);
        error = TestEvent(sp->sw_events[1]);
        timeout = TestEvent(sp->sw_events[2]);
        new = TestEvent(sp->sw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/* The same wait on the HwCARD events. */
ADDRESS(0x80060f88, 0xb4)
int card_wait_hw_event(void)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;
    long ioe, error, timeout, new;

    while (1) {
        ioe = TestEvent(sp->hw_events[0]);
        error = TestEvent(sp->hw_events[1]);
        timeout = TestEvent(sp->hw_events[2]);
        new = TestEvent(sp->hw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/*
 * Reads port's directory into its CardDirectory: when unk2DC's high byte is set
 * it first measures the space all files use, then lists this game's files
 * and records the lowest unused file number in unk204.
 */
ADDRESS(0x8006103c, 0x3ac)
void card_read_directory(int port)
{
    MenuScratch *sp = MENU_SCRATCH;
    MenuCardScratch *cs = (MenuCardScratch *)sp;
    struct DIRENTRY dir[15];
    char path[32];
    int used[8];
    int total, i, n;

    sp->card = &cs->cards[port];
    if (sp->mode & 0xff00) {
        strcpy(path, "bu");
        strcat(path, itoa(port));
        strcat(path, "0:*");
        sp->card->count = 0;
        if (firstfile(path, &dir[sp->card->count])) {
            do {
                sp->card->count++;
            } while (nextfile(&dir[sp->card->count]));
        }
        total = 0;
        for (i = 0; i < sp->card->count; i++)
            total += dir[i].size;
        if (total > 0x1a000)
            sp->card->has_room = 0;
        else
            sp->card->has_room = 1;
    }
    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, "BASLUS-00863FS_ST");
    strcat(path, "*");
    sp->card->count = 0;
    if (firstfile(path, &dir[sp->card->count])) {
        do {
            sp->card->count++;
        } while (nextfile(&dir[sp->card->count]));
    }
    if (sp->card->count == 0) {
        sp->free_file_number = 1;
        if (sp->mode & 0xff00) {
            if (sp->card->has_room == 0)
                sp->card->status = 5;
            else
                sp->card->status = 0;
        } else {
            sp->card->status = 4;
        }
        return;
    }
    for (i = 7; i >= 0; i--)
        used[i] = 0;
    for (i = 0; i < sp->card->count; i++) {
        strcpy(sp->card->files[i].name, dir[i].name);
        n = (dir[i].name[17] - '0') * 10 + (dir[i].name[18] - '0');
        used[n] = -1;
        sp->card->files[i].number = n;
    }
    for (i = 1; i < 8; i++) {
        if (used[i] != -1)
            break;
    }
    sp->free_file_number = i;
    sp->card->status = 0;
}

/*
 * Probes port with _card_info (and, for mode 1, clears and loads it with
 * _card_clear/_card_load), retrying up to ten times, and leaves the result
 * in the port's CardDirectory status.
 */
ADDRESS(0x800613e8, 0x190)
void card_probe(int port, int mode)
{
    MenuCardScratch *sp = MENU_CARD_SCRATCH;
    int retry;
    int result;

    card_open_events();
    retry = 9;
    do {
        card_clear_sw_events();
        _card_info(port << 4);
        result = card_wait_sw_event();
        if (result == 0) {
            sp->cards[port].status = 0;
        } else if (result == 1) {
            if (retry == 0) {
                sp->cards[port].status = 2;
                break;
            }
            continue;
        } else if (result == 2) {
            if (retry == 0) {
                sp->cards[port].status = 2;
                break;
            }
            continue;
        } else if (result == 3) {
            sp->cards[port].status = 1;
        }
        if (mode == 1) {
            card_clear_hw_events();
            _card_clear(port << 4);
            if (card_wait_hw_event() != 0) {
                sp->cards[port].status = 2;
                continue;
            }
            card_clear_sw_events();
            _card_load(port << 4);
            result = card_wait_sw_event();
            if (result == 1 || result == 2) {
                if (retry == 0) {
                    sp->cards[port].status = 2;
                    break;
                }
                continue;
            }
            if (result == 0) {
                sp->cards[port].status = 0;
            } else if (result == 3) {
                sp->cards[port].status = 3;
            }
            break;
        }
    } while (retry--);
    card_close_events();
}

/* Probes port; 0 with a message id in unk0 when it has no usable card. */
ADDRESS(0x80061578, 0x60)
int card_check_inserted(int port)
{
    MenuScratch *sp = MENU_SCRATCH;

    card_probe(port, 0);
    if (sp->card->status == 1) {
        sp->state.half[0] = 351;
    } else if (sp->card->status == 2) {
        sp->state.half[0] = 350;
    } else {
        return 1;
    }
    return 0;
}

/*
 * A yes/no dialog over the menu loop: returns the choice in unk202, or -1
 * when the card in the selected port goes away (probed every third frame).
 */
ADDRESS(0x800615d8, 0x120)
int card_ask_yes_no(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n = 0;

    sp->blink_phase = 0;
    sp->dialog_choice = 0;
    pad_clear_input();
    while (1) {
        if (n == 0) {
            if (card_check_inserted(sp->port) == 0)
                return -1;
            n = 3;
        }
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            break;
        }
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            sp->dialog_choice = 0;
            break;
        }
        if (pad_state.pressed & 0x80) {
            play_cursor_sound();
            pad_state.pressed = 0;
            sp->dialog_choice = 1;
        }
        if (pad_state.pressed & 0x20) {
            play_cursor_sound();
            pad_state.pressed = 0;
            sp->dialog_choice = 0;
        }
        n--;
        card_screen_draw();
    }
    pad_clear_input();
    return sp->dialog_choice;
}

/* The port choice: 1 with unk1F4 set to the chosen port, 0 when cancelled. */
ADDRESS(0x800616f8, 0x154)
int card_choose_port(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    sp->port = 0;
    sp->blink_phase = 0;
    pad_clear_input();
    while (1) {
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            sp->card = &((MenuCardScratch *)sp)->cards[sp->port];
            break;
        }
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            sp->port = -1;
            break;
        }
        if (pad_state.pressed_repeat & 0x10) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->port = 0;
            sp->blink_phase = 0;
        }
        if (pad_state.pressed_repeat & 0x40) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->port = 1;
            sp->blink_phase = 0;
        }
        card_screen_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
    return sp->port != -1;
}

/*
 * Reads each listed file's header bytes into its CardFileEntry.
 */
ADDRESS(0x8006184c, 0x1bc)
int card_read_file_headers(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int retry;
    char path[32];
    CardHeader header;
    int i, fd;

    retry = 3;
    i = sp->card->count;
    while (i--) {
        if (card_check_inserted(sp->port) == 0)
            return -1;
        strcpy(path, "bu");
        strcat(path, itoa(sp->port));
        strcat(path, "0:");
        strcat(path, sp->card->files[i].name);
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            sp->state.half[0] = 354;
            return 0;
        }
    read:
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &header, sizeof(header)) == -1) {
            if (retry == 0) {
                close(fd);
                return 0;
            }
            retry--;
            lseek(fd, 0, SEEK_SET);
            goto read;
        }
        (i + sp->card->files)->area = header.title.area;
        (i + sp->card->files)->sp_total = header.title.sp_total;
        (i + sp->card->files)->items_found = header.title.items_found;
        (i + sp->card->files)->data_sum = header.title.data_sum;
        close(fd);
    }
    return 1;
}

/*
 * The file list: moves the cursor over the port's files (plus a "new file"
 * row when there is room) and asks for confirmation. Returns 1 with the
 * chosen row in unk1FE, 0 when cancelled, -1 when the card goes away.
 */
ADDRESS(0x80061a08, 0x32c)
int card_choose_file(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n = 0;

    if ((sp->mode & 0xff00) && sp->card->has_room) {
        sp->row_count = sp->card->count + 1;
        sp->has_new_row = 1;
    } else {
        sp->row_count = sp->card->count;
        sp->has_new_row = 0;
    }
    sp->state.half[1] = 0;
    sp->file_row = 0;
    sp->first_row = 0;
    sp->blink_phase = 0;
    pad_clear_input();
    while (1) {
        if (n == 0) {
            if (card_check_inserted(sp->port) == 0)
                return -1;
            n = 20;
        }
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            sp->state.half[1] = 1;
            sp->blink_phase = 32;
            sp->dialog_choice = 1;
            while (1) {
                if (pad_state.pressed & 0x4000) {
                    play_confirm_sound();
                    pad_state.pressed = 0;
                    break;
                }
                if (pad_state.pressed & 0x2000) {
                    play_cancel_sound();
                    pad_state.pressed = 0;
                    sp->dialog_choice = 0;
                    break;
                }
                if (pad_state.pressed & 0x80) {
                    play_cursor_sound();
                    pad_state.pressed = 0;
                    sp->dialog_choice = 1;
                }
                if (pad_state.pressed & 0x20) {
                    play_cursor_sound();
                    pad_state.pressed = 0;
                    sp->dialog_choice = 0;
                }
                card_screen_draw();
            }
            if (sp->dialog_choice)
                break;
            sp->state.half[1] = 0;
            continue;
        }
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            sp->file_row = -1;
            break;
        }
        if (pad_state.pressed_repeat & 0x10) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            if (sp->file_row == 0) {
                if (sp->first_row > 0)
                    sp->first_row--;
            } else {
                sp->file_row--;
            }
            sp->blink_phase = 0;
        }
        if (pad_state.pressed_repeat & 0x40) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            if (sp->file_row == 2) {
                if (sp->first_row < sp->row_count - 3)
                    sp->first_row++;
            } else if (sp->file_row + sp->first_row < sp->row_count - 1) {
                sp->file_row++;
            }
            sp->blink_phase = 0;
        }
        card_screen_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
        n--;
    }
    pad_clear_input();
    if (sp->file_row == -1)
        return 0;
    sp->file_row += sp->first_row;
    return 1;
}

/*
 * Writes size bytes from data to the open file fd at offset, 127 bytes per
 * 128-byte frame with an inverted byte sum in the last byte, reading each
 * frame back to verify it (three tries). Returns the sum of the frame sums,
 * or -1.
 */
ADDRESS(0x80061d34, 0x1b4)
int card_write_blocks(int fd, int offset, u8 *data, int size)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 frame[128];
    u8 check[128];
    u8 *src = data;
    int retry = 0;
    int n = 0;
    u8 total = 0;
    u8 sum;
    int i;

    if (size > 0) {
        int block = 0;

        do {
            frame[127] = i = 0;
            for (; i < 127; i++) {
                frame[i] = *src;
                frame[127] += *src++;
            }
            frame[127] = ~frame[127];
        again:
            if (n == 0) {
                if (card_check_inserted(sp->port) == 0)
                    return -1;
                n = 3;
            }
            if (retry >= 3) {
                sp->state.half[0] = 355;
                return -1;
            }
            retry++;
            lseek(fd, offset + block * 128, SEEK_SET);
            if (write(fd, frame, 128) == -1)
                goto again;
            lseek(fd, offset + block * 128, SEEK_SET);
            if (read(fd, check, 128) == -1)
                goto again;
            sum = i = 0;
            for (; i < 127; i++)
                sum += check[i];
            sum = ~sum;
            if (sum != frame[127])
                goto again;
            size -= 127;
            block++;
            retry = 0;
            total += sum;
            n--;
        } while (size > 0);
    }
    return total;
}

/* Reads size bytes back the same way, verifying each frame's sum. */
ADDRESS(0x80061ee8, 0x174)
int card_read_blocks(int fd, int offset, u8 *data, int size)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 frame[128];
    u8 *dst = data;
    int retry = 0;
    int n = 0;
    u8 total = 0;
    u8 sum;
    int i;

    if (size > 0) {
        int block = 0;

        do {
        again:
            if (n == 0) {
                if (card_check_inserted(sp->port) == 0)
                    return -1;
                n = 3;
            }
            if (retry >= 3)
                return -1;
            retry++;
            lseek(fd, offset + block * 128, SEEK_SET);
            if (read(fd, frame, 128) == -1) {
                sp->state.half[0] = 354;
                goto again;
            }
            for (i = 0, sum = 0; i < 127; i++)
                sum += frame[i];
            sum = ~sum;
            if (sum != frame[127]) {
                sp->state.half[0] = 349;
                goto again;
            }
            for (i = 0; i < 127; i++)
                *dst++ = frame[i];
            size -= 127;
            block++;
            retry = 0;
            n--;
            total += frame[127];
        } while (size > 0);
    }
    return total;
}

/*
 * Loads the selected save file (row unk1FE) into data: opens it, skips the
 * header and reads 7560 bytes, checking the sum against the record's.
 */
ADDRESS(0x8006205c, 0x1cc)
int card_load_game(u8 *data)
{
    MenuScratch *sp = MENU_SCRATCH;
    char path[32];
    CardHeader header;
    int retry = 3;
    int fd, sum;

open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->port));
    strcat(path, "0:");
    strcat(path, sp->card->files[sp->file_row].name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (retry) {
            retry--;
            goto open;
        }
        sp->state.half[0] = 354;
        return 0;
    }
read:
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &header, sizeof(header)) == -1) {
        if (retry) {
            retry--;
            lseek(fd, 0, SEEK_SET);
            goto read;
        }
        sp->state.half[0] = 354;
        close(fd);
        return 0;
    }
    sum = card_read_blocks(fd, sizeof(header), data, 7560);
    if (sum == -1 || sum != sp->card->files[sp->file_row].data_sum) {
        if (sum != sp->card->files[sp->file_row].data_sum)
            sp->state.half[0] = 349;
        if (retry == 0) {
            pad_clear_input();
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto read;
    }
    close(fd);
    return 1;
}

/*
 * Saves to the selected file, or to a new file numbered unk204: builds the
 * header (the title with the file number, the area name and the play
 * time, an icon by file number), writes the 7560 bytes of data after it,
 * then the header again with the data's sum, and records the position the
 * game resumes from.
 *
 * The header's block count is a local set at the top and used once. Its
 * `li` is not in the header block when sched1 orders it (local-alloc moves
 * a once-used constant's initialisation down to the use afterwards), which
 * is what leaves the unk1FE load ahead of the icon store.
 */
ADDRESS(0x80062228, 0x998)
int card_save_game(u8 *data)
{
    MenuScratch *sp = MENU_SCRATCH;
    int retry = 3;
    int blocks = 2;
    char num[8];
    char path[32];
    CardHeader header;
    u16 cluts[7][16] = {
    {
        0x7fff, 0x03bf, 0x033f, 0x05dd, 0x027d, 0x0275, 0x060e, 0x060c,
        0x05e9, 0x09a6, 0x1560, 0x0ce0, 0x0000, 0x0000, 0x0000, 0x0000,
    },
    {
        0x7fff, 0x019b, 0x6739, 0x1018, 0x0535, 0x00f2, 0x1991, 0x0c10,
        0x112d, 0x00ac, 0x008a, 0x09a6, 0x1560, 0x0ce0, 0x0000, 0x249c,
    },
    {
        0x7fff, 0x6739, 0x5aff, 0x00f2, 0x019b, 0x0ce0, 0x1560, 0x2200,
        0x5a23, 0x28c0, 0x5180, 0x6f9f, 0x1018, 0x0000, 0x0d20, 0x2a40,
    },
    {
        0x7fff, 0x2d6b, 0x4210, 0x4631, 0x4e73, 0x5ad6, 0x6739, 0x739c,
        0x6e80, 0x76c9, 0x5180, 0x0000, 0x0421, 0x0000, 0x0000, 0x0000,
    },
    {
        0x7fff, 0x39ce, 0x5294, 0x0421, 0x6739, 0x0ce7, 0x1fbd, 0x02d6,
        0x0e52, 0x0dac, 0x0e69, 0x0d85, 0x0d24, 0x08a2, 0x0ce0, 0x0000,
    },
    {
        0x7fff, 0x00f2, 0x10eb, 0x019b, 0x2e15, 0x25d3, 0x3678, 0x25d2,
        0x2e36, 0x1d90, 0x1db2, 0x112d, 0x08a7, 0x0000, 0x0000, 0x0000,
    },
    {
        0x4500, 0x2108, 0x318c, 0x4e73, 0x739c, 0x263d, 0x0d98, 0x019b,
        0x04ca, 0x473f, 0x010d, 0x5962, 0x2ca0, 0x2418, 0x49bc, 0x0000,
    },
    };
    u8 icons[7][0x100] = {
    {
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x6c, 0x65, 0x69, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0x5c, 0x57, 0x59, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x95, 0x96, 0x66, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xc7, 0x6a, 0x9a, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0x9a, 0x99, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0x65, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xac, 0x99, 0xa9, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0x55, 0x65, 0x98, 0xcc, 0xcc,
        0xcc, 0xac, 0xaa, 0x9a, 0x56, 0x59, 0xc9, 0xcc, 0xcc, 0x56, 0x55, 0x55, 0x59, 0x59, 0x69, 0xcc,
        0xac, 0x39, 0x33, 0x33, 0x59, 0x69, 0x59, 0xc9, 0x56, 0x55, 0x55, 0x65, 0x6a, 0xca, 0x6a, 0xca,
        0x89, 0xcc, 0xcc, 0x89, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0x56, 0x96, 0xc6, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x7c, 0x95, 0x95, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0x56, 0x69, 0x59, 0xc6, 0xcc, 0xcc, 0xcc, 0xcc, 0x7c, 0xac, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0x9a, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0x9a, 0xc9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0xc6, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xac, 0x99, 0xa9, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x56, 0x55, 0x65, 0x98, 0xcc, 0xcc,
        0xcc, 0xac, 0x44, 0x94, 0x56, 0x59, 0xc9, 0xcc, 0xcc, 0x46, 0x2a, 0x42, 0x59, 0x59, 0x69, 0xcc,
        0xac, 0x39, 0x1b, 0x31, 0x59, 0x69, 0x59, 0xc9, 0x56, 0x55, 0x55, 0x65, 0x6a, 0xca, 0x6a, 0xca,
        0x89, 0xcc, 0xcc, 0x89, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    },
    {
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xbe, 0xee, 0xed, 0xed, 0xbe, 0xee, 0xee, 0xee, 0xee, 0xdc, 0x14, 0xd4, 0xec, 0xeb, 0xee,
        0xee, 0xee, 0x5b, 0x11, 0x51, 0xeb, 0xec, 0xee, 0xee, 0xee, 0x4e, 0x11, 0x41, 0xee, 0xee, 0xee,
        0xee, 0xee, 0x14, 0xc0, 0x10, 0xe4, 0xee, 0xee, 0xee, 0xee, 0x14, 0x11, 0x11, 0xe4, 0xee, 0xee,
        0xee, 0xee, 0x45, 0x44, 0x44, 0xe5, 0xee, 0xee, 0xee, 0xae, 0x99, 0x99, 0x99, 0xa9, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x68, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0x8e, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x9e, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0xef, 0xef, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xf3, 0xe3, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xce, 0x37, 0xc7, 0xee, 0xee, 0xee, 0xee, 0xce, 0xbe, 0xbe, 0xbe, 0xce, 0xee, 0xee,
        0xee, 0xeb, 0xcd, 0xbd, 0xcd, 0xed, 0xeb, 0xee, 0xee, 0xee, 0xde, 0x14, 0xd4, 0xee, 0xee, 0xee,
        0xee, 0xee, 0x5e, 0x11, 0x51, 0xee, 0xee, 0xee, 0xee, 0xee, 0x4e, 0x02, 0x42, 0xee, 0xee, 0xee,
        0xee, 0xee, 0x24, 0xc0, 0x20, 0xe4, 0xee, 0xee, 0xee, 0xee, 0x14, 0x02, 0x12, 0xe4, 0xee, 0xee,
        0xee, 0xee, 0x45, 0x44, 0x44, 0xe5, 0xee, 0xee, 0xee, 0xae, 0x99, 0x99, 0x99, 0xa9, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x68, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0x8e, 0xee, 0xee, 0xee, 0xee,
        0xee, 0xee, 0xee, 0x9e, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
    },
    {
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xd7, 0xdd, 0xdd, 0xdd,
        0xdd, 0xfe, 0x67, 0x6d, 0x7d, 0xdf, 0xdd, 0xdd, 0x5d, 0x76, 0x56, 0xd6, 0x67, 0xfe, 0xdd, 0xdd,
        0x80, 0x5b, 0x65, 0x75, 0x56, 0x6d, 0xdd, 0xdd, 0x98, 0xea, 0xf7, 0xe6, 0xd5, 0x5d, 0xdd, 0xdd,
        0xab, 0x62, 0x7f, 0x5e, 0xdd, 0x6d, 0xdd, 0xdd, 0x65, 0xe5, 0xe6, 0xe5, 0xdd, 0xe5, 0xdd, 0xdd,
        0x7d, 0x56, 0x55, 0xd6, 0xdd, 0xd6, 0xdd, 0xdd, 0xdd, 0x7f, 0xe6, 0xdd, 0x5d, 0xd7, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0x5d, 0x6e, 0x64, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0x11, 0x11, 0xdd, 0x6d, 0xdd, 0xdd, 0xdd, 0x0d, 0xb8, 0x00, 0xd2, 0x4d, 0xd7, 0xdd, 0xdd,
        0x80, 0xa9, 0xc0, 0xc0, 0x4d, 0xdf, 0xdd, 0xdd, 0xb0, 0x2a, 0x00, 0x1c, 0x3d, 0xd6, 0xdd, 0xdd,
        0x00, 0x00, 0x20, 0x20, 0x6d, 0xd5, 0xdd, 0xdd, 0x01, 0xc0, 0x00, 0x2c, 0x56, 0xdd, 0xdd, 0xdd,
        0x1d, 0x2c, 0x1c, 0x52, 0xdd, 0x76, 0xdd, 0xdd, 0xdd, 0x11, 0xc2, 0xe5, 0x56, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0x76, 0xdf, 0xdd, 0xdd, 0xdd, 0x6d, 0x44, 0x53, 0x5d, 0x76, 0xd6, 0xdd, 0xdd,
        0xdd, 0x6f, 0xd5, 0xdd, 0x65, 0x6f, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x57, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xd6, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x5d, 0xde, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0x5d, 0x6e, 0x54, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    },
    {
        0xbb, 0xbb, 0xbb, 0xbb, 0x5b, 0x76, 0xb0, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xb5, 0xbb, 0x0b, 0xbb,
        0xbb, 0xb7, 0xbb, 0x5b, 0xb7, 0xbb, 0xbb, 0xb0, 0x6b, 0xb0, 0xbb, 0x3b, 0x60, 0x0b, 0xbb, 0xb7,
        0x7b, 0x60, 0x77, 0x67, 0x70, 0xbb, 0xb7, 0xb7, 0x05, 0x00, 0x00, 0x00, 0x00, 0xb5, 0xb6, 0xb6,
        0xc6, 0x00, 0x03, 0x00, 0x0c, 0x36, 0xbb, 0xb6, 0x01, 0x50, 0x37, 0x00, 0x00, 0xb1, 0xbb, 0xb5,
        0x05, 0x00, 0x00, 0x00, 0x00, 0xb5, 0x5b, 0xbb, 0x6b, 0x00, 0x00, 0x00, 0x70, 0xbb, 0x3b, 0xbb,
        0xbb, 0x76, 0x77, 0x77, 0x67, 0xb3, 0xb3, 0xbb, 0xbb, 0xbb, 0x53, 0x76, 0x00, 0x27, 0xbb, 0xbb,
        0xbb, 0xbb, 0x64, 0x07, 0x70, 0xb5, 0xbb, 0xbb, 0xbb, 0xbb, 0x5b, 0x76, 0x56, 0xbb, 0xbb, 0xbb,
        0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
        0xbb, 0xbb, 0xbb, 0xbb, 0x5b, 0x76, 0xb0, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xb6, 0xbb, 0x0b, 0xbb,
        0xbb, 0xbb, 0xbb, 0x6b, 0xbb, 0xbb, 0xbb, 0xb0, 0xbb, 0xbb, 0xbb, 0x5b, 0xbb, 0x7b, 0xbb, 0xb7,
        0xbb, 0xbb, 0xbb, 0x4b, 0xb2, 0xbb, 0xb6, 0xb7, 0x67, 0xb0, 0xbb, 0xbb, 0x60, 0xb7, 0xb5, 0xb6,
        0x7b, 0x60, 0x77, 0x67, 0x70, 0x42, 0xbb, 0xb6, 0x05, 0x00, 0x00, 0x00, 0x00, 0xbb, 0xbb, 0xb5,
        0xc6, 0x0c, 0x03, 0xc0, 0x0c, 0xb6, 0x4b, 0xbb, 0x01, 0x50, 0x37, 0x00, 0x00, 0xb1, 0x3b, 0xbb,
        0x05, 0x00, 0x90, 0x00, 0x00, 0xb5, 0xb2, 0xbb, 0x6b, 0x00, 0x80, 0x00, 0x70, 0x26, 0xbb, 0xbb,
        0xbb, 0x76, 0xa7, 0x77, 0x67, 0xb5, 0xbb, 0xbb, 0xbb, 0xbb, 0x4b, 0x65, 0x56, 0xbb, 0xbb, 0xbb,
        0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
    },
    {
        0xff, 0xff, 0xdf, 0xab, 0xdb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbd, 0xaa, 0xba, 0xfd, 0xff, 0xff,
        0x20, 0xff, 0xac, 0xac, 0xac, 0xfc, 0x2f, 0xf0, 0x02, 0xd4, 0x3c, 0xb3, 0x3c, 0xdc, 0x04, 0xf2,
        0x1f, 0xcf, 0x63, 0x87, 0x9d, 0xc7, 0x1f, 0xff, 0xff, 0xbf, 0x33, 0x33, 0x73, 0xb5, 0xff, 0xff,
        0xff, 0xbf, 0x33, 0x33, 0x33, 0xb3, 0xff, 0xff, 0xff, 0xcf, 0x79, 0x33, 0x67, 0xb5, 0xff, 0xff,
        0xff, 0xcf, 0x57, 0x3c, 0xe8, 0xc8, 0xff, 0xff, 0xff, 0xcf, 0xc3, 0xcc, 0x33, 0xcc, 0xff, 0xff,
        0xff, 0xdf, 0xac, 0xab, 0xaa, 0xdc, 0xff, 0xff, 0xff, 0xff, 0xbc, 0xaa, 0xba, 0xfc, 0xff, 0xff,
        0xff, 0xff, 0xdf, 0xcc, 0xdc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xab, 0xdb, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xcd, 0xaa, 0xba, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xac, 0xab, 0xac, 0xfc, 0xff, 0xff,
        0x1f, 0xd4, 0x3c, 0xc3, 0x3c, 0xdb, 0x14, 0xff, 0x04, 0xcf, 0x63, 0x7e, 0x9d, 0xc7, 0x0f, 0xf4,
        0x10, 0xbf, 0x93, 0x97, 0x73, 0xc5, 0x1f, 0xf0, 0xff, 0xbf, 0x33, 0x33, 0x33, 0xb3, 0xff, 0xff,
        0xff, 0xbf, 0x79, 0x33, 0x33, 0xb3, 0xff, 0xff, 0xff, 0xcf, 0x57, 0x3c, 0x78, 0xc8, 0xff, 0xff,
        0xff, 0xcf, 0xc3, 0xcb, 0x33, 0xcc, 0xff, 0xff, 0xff, 0xdf, 0xab, 0xac, 0xaa, 0xdb, 0xff, 0xff,
        0xff, 0xff, 0xbc, 0xaa, 0xca, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xcc, 0xdc, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    {
        0xdd, 0xdd, 0x2b, 0x2d, 0xdb, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0x5b, 0x6c, 0xb5, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xc2, 0x68, 0xb6, 0xdc, 0xdd, 0xdd, 0xdd, 0xdd, 0x87, 0x66, 0x86, 0xb5, 0xdd, 0xdd,
        0xdd, 0xbd, 0x68, 0x6c, 0x66, 0x58, 0xdc, 0xdd, 0xdd, 0x82, 0x66, 0x66, 0x66, 0x86, 0xdb, 0xdd,
        0xdd, 0x7c, 0x66, 0x66, 0x67, 0x66, 0xc5, 0xdd, 0xdd, 0x2d, 0x9b, 0x77, 0x66, 0x66, 0x28, 0xdd,
        0xdd, 0xdd, 0x5c, 0x68, 0x66, 0x66, 0x28, 0xdd, 0xdd, 0xdd, 0x82, 0xb6, 0x66, 0x66, 0xb8, 0xdd,
        0xd0, 0xd0, 0x82, 0x6c, 0x66, 0x66, 0xb8, 0xdd, 0xd0, 0xd0, 0x8b, 0x66, 0x66, 0x66, 0x58, 0xdb,
        0xdd, 0xdd, 0x52, 0x85, 0x88, 0x58, 0xb5, 0xdd, 0xdd, 0xdd, 0xcd, 0x2c, 0x22, 0xc2, 0xdc, 0xdd,
        0x1d, 0xd3, 0x2c, 0xcd, 0xcd, 0xcd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xcd, 0xb2, 0x2b, 0xdc, 0xdd, 0xdd,
        0xdd, 0x22, 0x4c, 0x44, 0x24, 0xcb, 0xdc, 0xdd, 0xbd, 0xc8, 0x22, 0x22, 0x42, 0x42, 0xc5, 0xdd,
        0xad, 0x66, 0x48, 0x44, 0x68, 0x66, 0x24, 0xdd, 0xdd, 0x6a, 0x66, 0x66, 0x66, 0x66, 0x24, 0xdd,
        0xdd, 0xca, 0x6c, 0xc6, 0x6c, 0x66, 0xb4, 0xdd, 0xdd, 0x6a, 0x66, 0x66, 0x66, 0x68, 0xb8, 0xdd,
        0xdd, 0xad, 0x66, 0x66, 0x86, 0x65, 0xb8, 0xdd, 0xdd, 0x8a, 0xb7, 0x9b, 0x57, 0x66, 0x28, 0xdd,
        0xdd, 0x6b, 0x28, 0xa2, 0x68, 0x66, 0xc4, 0xdd, 0xdd, 0x5a, 0x6b, 0xb6, 0xa5, 0x66, 0x54, 0xdb,
        0xdd, 0x7b, 0x84, 0x48, 0x48, 0x48, 0xb7, 0xdd, 0xdd, 0xcd, 0x22, 0xbb, 0xbb, 0x22, 0xdc, 0xdd,
        0xdd, 0xdc, 0xdc, 0xdc, 0xcd, 0xcd, 0xcd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
    },
    {
        0xff, 0xff, 0x9f, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0x85, 0x51, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x9f, 0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x61, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xb2, 0x95, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x0b, 0xc6, 0xf0, 0xff, 0xff,
        0xff, 0xff, 0x2f, 0xcf, 0xf0, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x8c, 0x0c, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x7f, 0x7f, 0x75, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xf5, 0xf9, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf3, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf2, 0x0c, 0xfb, 0xff, 0xff, 0xff,
        0xff, 0x3f, 0xf1, 0xcf, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x42, 0xff, 0xfb, 0x43, 0xff, 0xff,
        0xff, 0xf4, 0x31, 0xc4, 0x40, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x9f, 0xff, 0xff, 0xf9, 0xef, 0xff, 0xff, 0xff, 0xff, 0x85, 0x51, 0xff, 0xef, 0xfd,
        0xff, 0xff, 0xff, 0x9f, 0xf5, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6f, 0xff, 0xf7, 0x3f, 0xf4,
        0xff, 0xff, 0x71, 0xb3, 0x15, 0x2b, 0x11, 0xf2, 0xff, 0x21, 0xcf, 0x0b, 0x06, 0xfc, 0x2f, 0xf1,
        0x1f, 0xff, 0xff, 0xcc, 0xc0, 0xff, 0x3f, 0xf1, 0xff, 0xff, 0xff, 0x8f, 0xfc, 0xff, 0x3f, 0xf1,
        0xff, 0xff, 0xff, 0x7f, 0xf5, 0xff, 0x4f, 0xff, 0xff, 0xff, 0xff, 0xf5, 0xf9, 0xff, 0x4f, 0xff,
        0xff, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0x42, 0xff, 0xff, 0xff, 0xff, 0x0c, 0xfb, 0xff, 0xf3, 0xff,
        0xff, 0xff, 0xff, 0xcf, 0xfb, 0xff, 0xf4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x4f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xcf, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    },
    };
    char title[64] = "\x82\x72\x82\x73\x81\x40\x82\x4f\x82\x4f\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x81\x40\x82\x72\x82\x6f\x81\x40\x81\x40\x81\x40\x82\x4f\0\0\0\0\0\0\0\0\0\0";
    char zero[2] = "\x82\x4f";
    u16 areas[42][14] = {
    {
        0x7282, 0x6782, 0x6082, 0x6382, 0x6e82, 0x7682, 0x4081, 0x7382,
        0x6e82, 0x7682, 0x6482, 0x7182, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6c82, 0x6e82, 0x6d82, 0x7282, 0x7382, 0x6482, 0x7182, 0x4081,
        0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6782, 0x7482, 0x6c82, 0x6082, 0x6d82, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6c82, 0x6e82, 0x6d82, 0x7282, 0x7382, 0x6482, 0x7182, 0x4081,
        0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6582, 0x6882, 0x7182, 0x6482, 0x4081, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6382, 0x6482, 0x6082, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7582, 0x6e82, 0x6882, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6482, 0x6082, 0x7182, 0x7382, 0x6782, 0x4081, 0x7682, 0x6e82,
        0x7182, 0x6b82, 0x6382, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7682, 0x6082, 0x7382, 0x6482, 0x7182, 0x7682, 0x6e82, 0x7182,
        0x6b82, 0x6382, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x6882, 0x6b82, 0x6b82, 0x7482, 0x7282, 0x6882, 0x6e82, 0x6d82,
        0x4081, 0x7682, 0x6e82, 0x7182, 0x6b82, 0x6382,
    },
    {
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
        0x4081, 0x4081, 0x4081, 0x4081, 0x4081, 0x4081,
    },
    {
        0x7282, 0x6782, 0x6082, 0x6382, 0x6e82, 0x7682, 0x4081, 0x7382,
        0x6e82, 0x7682, 0x6482, 0x7182, 0x4081, 0x4081,
    },
    {
        0x7282, 0x6782, 0x6082, 0x6382, 0x6e82, 0x7682, 0x4081, 0x7382,
        0x6e82, 0x7682, 0x6482, 0x7182, 0x4081, 0x4081,
    },
    };
    int new_file, n, fd, sum;

    new_file = sp->file_row == sp->card->count;
    strcpy(header.magic, "SC");
    header.icon = 0x12;
    header.blocks = blocks;
    memset(header.unk44, 0, sizeof(header.unk44));
    if (new_file)
        n = sp->free_file_number;
    else
        n = sp->card->files[sp->file_row].number;
    memcpy(header.clut, cluts[n % 7], sizeof(header.clut));
    memcpy(header.icons, icons[n % 7], sizeof(header.icons));
    title[6] = zero[0];
    title[7] = zero[1] + n / 10;
    title[8] = zero[0];
    title[9] = zero[1] + n % 10;
    copy_halfwords((u16 *)&title[12], areas[area_state.area], 14);
    if (viewer.sp_total >= 1000) {
        title[46] = zero[0];
        title[47] = zero[1] + viewer.sp_total / 1000;
    }
    if (viewer.sp_total >= 100) {
        title[48] = zero[0];
        title[49] = zero[1] + viewer.sp_total / 100 % 10;
    }
    if (viewer.sp_total >= 10) {
        title[50] = zero[0];
        title[51] = zero[1] + viewer.sp_total / 10 % 10;
    }
    title[52] = zero[0];
    title[53] = zero[1] + viewer.sp_total % 10;
    ((CardTitle *)title)->area = area_state.area;
    ((CardTitle *)title)->sp_total = viewer.sp_total;
    ((CardTitle *)title)->items_found = viewer.items_found;
    memcpy(&header.title, title, sizeof(title));
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->port));
    strcat(path, "0:");
    if (new_file) {
        strcat(path, "BASLUS-00863FS_ST");
        num[0] = sp->free_file_number / 10 + '0';
        num[1] = sp->free_file_number % 10 + '0';
        num[2] = '\0';
        strcat(path, num);
        fd = open(path, O_CREAT | (2 << 16));
        if (fd == -1) {
            if (retry == 0) {
                sp->state.half[0] = 355;
                return 0;
            }
            retry--;
            goto open;
        }
        close(fd);
    } else {
        strcat(path, sp->card->files[sp->file_row].name);
    }
    fd = open(path, O_RDWR);
    if (fd == -1) {
        if (retry == 0) {
            sp->state.half[0] = 355;
            return 0;
        }
        retry--;
        goto open;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &header, sizeof(header)) == -1) {
        if (retry == 0) {
            sp->state.half[0] = 355;
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = card_write_blocks(fd, sizeof(header), data, 7560);
    if (sum == -1) {
        if (retry == 0) {
            pad_clear_input();
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header.title.data_sum = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &header, sizeof(header)) == -1) {
        if (retry == 0) {
            sp->state.half[0] = 355;
            close(fd);
            return 0;
        }
        retry--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    viewer.save_area = area_state.loaded_area;
    viewer.save_x = viewer.position.vx >> 12;
    viewer.save_y = viewer.position.vy >> 12;
    viewer.save_z = viewer.position.vz >> 12;
    viewer.save_dx = viewer.position.vx - (viewer.save_x << 12);
    viewer.save_dy = viewer.position.vy - (viewer.save_y << 12);
    viewer.save_dz = viewer.position.vz - (viewer.save_z << 12);
    viewer.save_yaw = viewer.view_rot.vy + 2048;
    close(fd);
    pad_clear_input();
    return 1;
}

/* Formats the memory card in port. */
ADDRESS(0x80062bc0, 0x68)
long card_format(int port)
{
    char path[32];

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    return format(path);
}

/* The pressed-button word as the waits read it: one volatile halfword at
 * its own address (the counter interrupt updates it). */
#define PRESSED_NOW (*(volatile u16 *)&pad_state.pressed)

/*
 * The memory card save/load screen (unk2DC's high byte set: save). Steps
 * through port choice, probe, format offer, directory, file headers, file
 * choice and the transfer, shifting unk2DC's step bit as it goes; returns
 * 0 when the port choice is cancelled, else 1 after the result message.
 */
ADDRESS(0x80062c28, 0x450)
int card_screen_run(int arg)
{
    MenuScratch *sp = MENU_SCRATCH;
    int result;
    u8 *buf;

    sp->state.half[0] = 0;
    sp->state.half[1] = 0;
    sp->show_backdrop = arg;
    pad_clear_input();
    card_screen_draw();
    card_screen_draw();
    sp->mode <<= 1;
select:
    if (card_choose_port() == 0)
        return 0;
    sp->mode <<= 1;
    card_screen_draw();
    card_screen_draw();
    card_probe(sp->port, 1);
    if ((sp->mode & 0xff00) && sp->card->status == 3) {
        sp->mode <<= 1;
        sp->state.half[1] = 0;
        result = card_ask_yes_no();
        if (result == 0) {
            sp->mode >>= 2;
            goto select;
        }
        if (result == -1) {
            sp->mode <<= 4;
            goto done;
        }
        sp->state.half[1] = 1;
        card_screen_draw();
        card_screen_draw();
        if (card_format(sp->port) == 0) {
            sp->state.half[0] = 356;
            sp->mode <<= 4;
            goto done;
        }
        sp->mode <<= 1;
    } else {
        if (sp->card->status == 2) {
            sp->state.half[0] = 350;
            sp->mode <<= 5;
            goto done;
        }
        sp->mode <<= 2;
    }
    card_read_directory(sp->port);
    if (sp->card->status != 0) {
        if (sp->card->status == 4)
            sp->state.half[0] = 352;
        else if (sp->card->status == 5)
            sp->state.half[0] = 353;
        sp->mode <<= 3;
        goto done;
    }
    result = card_read_file_headers();
    if (result == 0) {
        sp->state.half[0] = 352;
        sp->mode <<= 3;
        goto done;
    }
    if (result == -1) {
        sp->mode <<= 3;
        goto done;
    }
    result = card_choose_file();
    if (result == 0) {
        sp->mode >>= 3;
        goto select;
    }
    if (result == -1) {
        sp->mode <<= 3;
        goto done;
    }
    sp->state.half[0] = 0;
    sp->mode <<= 1;
    card_screen_draw();
    card_screen_draw();
    card_probe(sp->port, 0);
    if (sp->card->status == 1) {
        sp->state.half[0] = 351;
        sp->mode <<= 2;
        goto done;
    }
    if (sp->card->status == 2) {
        sp->state.half[0] = 350;
        sp->mode <<= 2;
        goto done;
    }
    buf = mem_alloc(7686);
    if (buf == NULL) {
        sp->mode <<= 2;
        goto done;
    }
    if (sp->mode & 0xff00) {
        save_pack_data((StSaveData *)buf, sp->save_fresh);
        result = card_save_game(buf);
    } else {
        result = card_load_game(buf);
        if (result)
            save_unpack_data((StSaveData *)buf);
    }
    mem_free(buf);
    if (result == 0) {
        if (sp->state.half[0] == 0)
            if (sp->mode & 0xff00)
                sp->state.half[0] = 355;
            else
                sp->state.half[0] = 354;
        sp->mode <<= 2;
        goto done;
    }
    sp->mode <<= 1;
done:
    card_screen_draw();
    card_screen_draw();
    if (sp->mode & 0x80) {
        while (PRESSED_NOW == 0)
            ;
        play_cancel_sound();
        pad_state.pressed = 0;
        sp->mode = 2;
        goto select;
    }
    if (sp->mode & 0x8000) {
        while (PRESSED_NOW == 0)
            ;
        play_cancel_sound();
        pad_state.pressed = 0;
        sp->mode = 0x200;
        goto select;
    }
    if (sp->mode & 0xff00) {
        while (PRESSED_NOW == 0)
            ;
        play_cancel_sound();
        pad_state.pressed = 0;
    }
    viewer.card_screen_done = 1;
    pad_clear_input();
    return 1;
}

/*
 * The quit dialog: returns 1 after setting game_exit_request to 2 (leave for
 * TITLE.EXE) when the first choice is confirmed, else 0.
 */
ADDRESS(0x80063078, 0x134)
int menu_end_game_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    sp->cursor = 1;
    pad_clear_input();
    while (1) {
        if (pad_state.pressed & 0x2000) {
            play_cancel_sound();
            pad_state.pressed = 0;
            break;
        }
        if (pad_state.pressed_repeat & 0x80) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->cursor = 0;
        }
        if (pad_state.pressed_repeat & 0x20) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->cursor = 1;
        }
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            if (sp->cursor != 0)
                break;
            game_exit_request = 2;
            return 1;
        }
        frame_rate_update();
        menu_end_game_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
    return 0;
}

/*
 * The pause screen: runs pause_screen_draw until button 8, keeping the play
 * clock (viewer.play_frames/unk34) still.
 * The controller owner becomes live after the draw setup. The explicit
 * top-tested wait keeps the button read before each pair of frame calls.
 */
ADDRESS(0x800631ac, 0xb4)
void pause_screen_run(void)
{
    s32 seconds = viewer.play_seconds;
    u8 frames = viewer.play_frames;
    PadState *pad;

    cd_wait_idle();
    menu_store_screen();
    pad_clear_input();
    DrawSync(0);
    pad = &pad_state;
pause_wait:
    if (pad->pressed & 8)
        goto pause_done;
    frame_rate_update();
    pause_screen_draw();
    goto pause_wait;
pause_done:
    pad_clear_input();
    menu_restore_screen();
    viewer.play_seconds = seconds;
    viewer.play_frames = frames;
}

/*
 * A two-choice page over the menu font (CD archive 3 member 0): returns
 * the choice.
 */
ADDRESS(0x80063260, 0x174)
int menu_game_over_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 *p;

    sp->image_vram[0].vx = 960;
    sp->image_vram[0].vy = 256;
    sp->image_vram[1].vx = 560;
    sp->image_vram[1].vy = 495;
    cd_wait_idle();
    p = (u8 *)cd_file_buffer;
    cd_read_member(3, 0, (u_long *)p);
    copy_words((u32 *)&menu_messages, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    tim_load_images_at(p + 4, &sp->image_vram[0], &sp->image_vram[1], 1);
    sp->state.word = 0;
    sp->blink_phase = 0;
    pad_clear_input();
    while (1) {
        if (pad_state.pressed & 0x4000) {
            play_confirm_sound();
            pad_state.pressed = 0;
            break;
        }
        if (pad_state.pressed_repeat & 0x40) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->state.word = 1;
        }
        if (pad_state.pressed_repeat & 0x10) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->state.word = 0;
        }
        sp->font_mode = 0;
        frame_rate_update();
        menu_game_over_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
    return sp->state.word;
}

/*
 * The memory card choice screen: loads the menu messages and image, then
 * runs the two-row choice until a row is confirmed; row 0 runs the save
 * screen (card_screen_run) and returns once it reports 1.
 */
ADDRESS(0x800633d4, 0x264)
void menu_save_prompt_run(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT clear = {0, 0, 320, 480};
    u_long *image;
    int result;

    sp->image_vram[0].vx = 960;
    sp->image_vram[0].vy = 256;
    sp->image_vram[1].vx = 560;
    sp->image_vram[1].vy = 495;
    cd_wait_idle();
    image = cd_file_buffer;
    cd_read_member(3, 0, image);
    copy_words((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (cd_file_buffer[0] + 4));
    tim_load_images_at(image + 1, &sp->image_vram[0], &sp->image_vram[1], 1);
    ClearImage(&clear, 0, 0, 0);
    sp->main_row = 0;
    sp->blink_phase = 0;
    pad_clear_input();
    sp->rect.x = 0;
    sp->rect.y = 0;
    sp->rect.w = 320;
    sp->rect.h = 240;
    for (;;) {
        if (pad_state.pressed & 0x4000) {
            pad_state.pressed = 0;
            play_confirm_sound();
            if (sp->main_row != 0)
                break;
            ClearImage(&clear, 0, 0, 0);
            cd_wait_idle();
            area_save_snapshot(area_state.loaded_area);
            anim_free_all_for_menu();
            pad_clear_input();
            pad_vibrate(0);
            card_screen_open(1, 0);
            sp->save_fresh = 1;
            result = card_screen_run(0);
            pad_clear_input();
            card_screen_close();
            if (result == 1)
                break;
        }
        if (pad_state.pressed_repeat & 0x80) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->main_row = 0;
        }
        if (pad_state.pressed_repeat & 0x20) {
            play_cursor_sound();
            pad_state.pressed_repeat = 0;
            sp->main_row = 1;
        }
        sp->font_mode = 0;
        frame_rate_update();
        menu_save_prompt_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
    ClearImage(&clear, 0, 0, 0);
}
