/*
 * ST.EXE: the equipment screen and an id grid screen (WIP module, one
 * translation unit): the slot list, the candidate list of one slot, the
 * magic sub-list and their panels, then two four-column grids of table C ids
 * the player moves between, its frame and its CD loads (sharing the id grid
 * layout and panels of src/st/render.c, ArrangeGrids). The screen's open and close
 * (0x800637c0, 0x80063954) sit in band P's range. Retail keeps the id grid's
 * constants (0x800138a0) between the equipment screen's (0x80013880) and the
 * equipment screen's switch tables (0x800138d8), which only one -G8 object
 * produces.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/effect.h>
#include <st/text.h>
#include <st/menu.h>
#include <st/vis.h>
#include <st/equip_menu.h>
#include <st/card.h>
#include <st/sound.h>
#include <st/funcs.h>
#include <psyq/kernel.h>
#include <LIBETC.H>
#include <LIBSN.H>
#include <psyq/libc.h>
#include <st/idgrid.h>
#include <st/pad.h>
#include <st/menu_main.h>

RODATA(0x80013880, 0xf4)
SDATA(0x8008810c, 0x7c)

/* The list's scroll top (sp->scroll) and cursor offset (sp->cursor), saved
 * while the magic sub-list is open. */
DATA(0x800881e8, 0x4, ".sbss")
static s16 equip_saved_list_cursor[2];

#define EQ equip_menu_state
#define PAD pad_state
#define SLOT(k) (&viewer.equipment[k])

/* Runs player_compute_derived_stats(0) with the list state kept, then clamps
 * the two viewer figures it may have raised. */
ADDRESS(0x80063bdc, 0xd4)
void menu_equip_recalc_stats(void)
{
    MenuMessages *messages = MENU_SCRATCH->state.list.messages;
    s16 *ids = MENU_SCRATCH->state.list.ids;
    s16 count = MENU_SCRATCH->state.list.count;
    u16 unkA = MENU_SCRATCH->state.list.confirm_choice;

    MENU_SCRATCH->hp_before = viewer.hp;
    MENU_SCRATCH->mp_before = viewer.mp;
    player_compute_derived_stats(0);
    if (viewer.max_hp < MENU_SCRATCH->hp_before)
        viewer.hp = viewer.max_hp;
    if (viewer.max_mp < MENU_SCRATCH->mp_before)
        viewer.mp = viewer.max_mp;
    MENU_SCRATCH->state.list.messages = messages;
    MENU_SCRATCH->state.list.ids = ids;
    MENU_SCRATCH->state.list.count = count;
    MENU_SCRATCH->state.list.confirm_choice = unkA;
}

/* The candidate list of one slot: pages through it, equips the shown id
 * (dropping the slots it excludes) or clears the slot, and loads the
 * shown id's image. */
ADDRESS(0x80063cb0, 0x638)
void menu_equip_pick_item(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};

    if (PAD.pressed_repeat & PAD_BUTTON_RIGHT) {
        play_cursor_sound();
        if (EQ.list_index == sp->state.list.count - 1) {
            EQ.list_index = -1;
            EQ.shown_id = -1;
        } else {
            EQ.list_index++;
            EQ.shown_id = sp->state.list.ids[EQ.list_index];
        }
        EQ.input_hold = 1;
    } else if (PAD.pressed_repeat & PAD_BUTTON_LEFT) {
        play_cursor_sound();
        if (EQ.list_index == -1)
            EQ.list_index = sp->state.list.count - 1;
        else
            EQ.list_index--;
        if (EQ.list_index == -1)
            EQ.shown_id = EQ.list_index;
        else
            EQ.shown_id = sp->state.list.ids[EQ.list_index];
        EQ.input_hold = 1;
    } else if (PAD.pressed & PAD_BUTTON_CIRCLE) {
        play_cancel_sound();
        PAD.pressed = 0;
        sp->mode &= ~EQUIP_MENU_ITEM_LIST;
        mem_free(sp->state.list.ids);
    } else if (PAD.pressed & PAD_BUTTON_CROSS) {
        if (EQ.shown_id != -1) {
            if (EQ.slot != SLOT(EQUIP_SLOT_RIGHT_RING) && EQ.slot != SLOT(EQUIP_SLOT_LEFT_RING)
                && vis_table_c.types[EQ.shown_id].max_durability &&
                !vis_table_c.types[EQ.shown_id].durability) {
                play_refuse_sound();
            } else {
                play_confirm_sound();
                PAD.pressed = 0;
                sp->mode &= ~EQUIP_MENU_ITEM_LIST;
                mem_free(sp->state.list.ids);
                if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_RING)
                    && EQ.shown_id != viewer.equipment[EQUIP_SLOT_RIGHT_RING])
                    viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC] = EQUIP_NONE;
                if (EQ.slot == SLOT(EQUIP_SLOT_LEFT_RING)
                    && EQ.shown_id != viewer.equipment[EQUIP_SLOT_LEFT_RING])
                    viewer.equipment[EQUIP_SLOT_LEFT_MAGIC] = EQUIP_NONE;
                *EQ.slot = EQ.shown_id;
                if (EQ.slot == SLOT(EQUIP_SLOT_GLOVES))
                    viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR] = EQ.shown_id;
                if (EQ.slot == SLOT(EQUIP_SLOT_ARMOR) && viewer.equipment[EQUIP_SLOT_ARMOR] != EQUIP_NONE &&
                    vis_table_c.types[viewer.equipment[EQUIP_SLOT_ARMOR]].category == 10) {
                    viewer.equipment[EQUIP_SLOT_GLOVES] = EQUIP_NONE;
                    viewer.equipment[EQUIP_SLOT_BOOTS] = EQUIP_NONE;
                }
                if ((EQ.slot == SLOT(EQUIP_SLOT_BOOTS) || EQ.slot == SLOT(EQUIP_SLOT_GLOVES))
                    && viewer.equipment[EQUIP_SLOT_ARMOR] != EQUIP_NONE &&
                    *EQ.slot != 0xffff && vis_table_c.types[viewer.equipment[EQUIP_SLOT_ARMOR]].category == 10)
                    viewer.equipment[EQUIP_SLOT_ARMOR] = EQUIP_NONE;
                if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_HAND)
                    && viewer.equipment[EQUIP_SLOT_RIGHT_HAND] != EQUIP_NONE &&
                    vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_HAND]].category == 9)
                    viewer.equipment[EQUIP_SLOT_LEFT_HAND] = EQUIP_NONE;
                if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_RESERVE)
                    && viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] != EQUIP_NONE &&
                    vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE]].category == 9)
                    viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] = EQUIP_NONE;
                if (EQ.slot == SLOT(EQUIP_SLOT_LEFT_HAND)
                    && viewer.equipment[EQUIP_SLOT_RIGHT_HAND] != EQUIP_NONE
                    && viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE &&
                    vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_HAND]].category == 9)
                    viewer.equipment[EQUIP_SLOT_RIGHT_HAND] = EQUIP_NONE;
                if (EQ.slot == SLOT(EQUIP_SLOT_LEFT_RESERVE)
                    && viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] != EQUIP_NONE
                    && viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] != EQUIP_NONE &&
                    vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE]].category == 9)
                    viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] = EQUIP_NONE;
            }
        } else {
            play_confirm_sound();
            PAD.pressed = 0;
            sp->mode &= ~EQUIP_MENU_ITEM_LIST;
            mem_free(sp->state.list.ids);
            *EQ.slot = 0xffff;
            if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_RING))
                viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC] = EQUIP_NONE;
            if (EQ.slot == SLOT(EQUIP_SLOT_LEFT_RING))
                viewer.equipment[EQUIP_SLOT_LEFT_MAGIC] = EQUIP_NONE;
        }
        menu_equip_recalc_stats();
        PAD.pressed = 0;
    }
    if (EQ.shown_id != EQ.loaded_id) {
        if (EQ.shown_id == -1)
            cd_read_member(ARCHIVE_ICON, 801, cd_file_buffer);
        else
            cd_read_member(ARCHIVE_ICON, vis_table_c.types[EQ.shown_id].unk5[0] + 600, cd_file_buffer);
        tim_load_images_at(cd_file_buffer, &pos, NULL, 1);
        EQ.loaded_id = EQ.shown_id;
    }
}

/*
 * The slot list: moves the cursor; on confirm lists the held ids that fit
 * the slot (or, for the two ring slots, opens the magic sub-list) and loads
 * the equipped id's image. Returns 1 on cancel.
 *
 * The list loop reaches the item records through a pointer to the table
 * set just before it, and every case of its switch tests the record's
 * kind and adds the id itself; jump2 merges the copies. Both matter to the
 * allocation only: the pointer keeps the table's address out of loop.c's
 * hoisting budget, and the written-out adds are what rank the scratchpad
 * ahead of `kind` and `cur` for the saved registers.
 */
ADDRESS(0x800642e8, 0x8c0)
int menu_equip_pick_slot(int *sel)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR pos = {448, 256};
    int kind;
    int cur;
    int i;
    ItemTable *table;

    if (PAD.pressed & PAD_BUTTON_CIRCLE) {
        play_cancel_sound();
        PAD.pressed = 0;
        return 1;
    }
    if (PAD.pressed_repeat & PAD_BUTTON_UP) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->cursor == 0) {
            if (sp->scroll > 0)
                sp->scroll--;
        } else {
            sp->cursor--;
        }
    } else if (PAD.pressed_repeat & PAD_BUTTON_DOWN) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->cursor == 4) {
            if (sp->scroll < 10)
                sp->scroll++;
        } else {
            sp->cursor++;
        }
    } else if (PAD.pressed_repeat & PAD_BUTTON_R1) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->scroll == 0)
            sp->cursor = 0;
        else if (sp->scroll >= 5)
            sp->scroll -= 5;
        else
            sp->scroll = 0;
    } else if (PAD.pressed_repeat & PAD_BUTTON_R2) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->scroll == 10)
            sp->cursor = 4;
        else if (sp->scroll < 6)
            sp->scroll += 5;
        else
            sp->scroll = 10;
    } else if (PAD.pressed & PAD_BUTTON_CROSS) {
        PAD.pressed = 0;
        switch (sp->scroll + sp->cursor) {
        case 0:
            kind = 0;
            cur = viewer.equipment[EQUIP_SLOT_HELMET];
            EQ.slot = SLOT(EQUIP_SLOT_HELMET);
            break;
        case 1:
            kind = 1;
            cur = viewer.equipment[EQUIP_SLOT_ARMOR];
            EQ.slot = SLOT(EQUIP_SLOT_ARMOR);
            break;
        case 2:
            kind = 2;
            cur = viewer.equipment[EQUIP_SLOT_GLOVES];
            EQ.slot = SLOT(EQUIP_SLOT_GLOVES);
            break;
        case 3:
            kind = 3;
            cur = viewer.equipment[EQUIP_SLOT_BOOTS];
            EQ.slot = SLOT(EQUIP_SLOT_BOOTS);
            break;
        case 4:
            kind = 6;
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_HAND];
            EQ.slot = SLOT(EQUIP_SLOT_RIGHT_HAND);
            break;
        case 5:
            kind = 6;
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE];
            EQ.slot = SLOT(EQUIP_SLOT_RIGHT_RESERVE);
            break;
        case 6:
            kind = 7;
            cur = viewer.equipment[EQUIP_SLOT_LEFT_HAND];
            EQ.slot = SLOT(EQUIP_SLOT_LEFT_HAND);
            break;
        case 7:
            kind = 7;
            cur = viewer.equipment[EQUIP_SLOT_LEFT_RESERVE];
            EQ.slot = SLOT(EQUIP_SLOT_LEFT_RESERVE);
            break;
        case 8:
            kind = 8;
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET];
            EQ.slot = SLOT(EQUIP_SLOT_RIGHT_BRACELET);
            break;
        case 9:
            kind = 8;
            cur = viewer.equipment[EQUIP_SLOT_LEFT_BRACELET];
            EQ.slot = SLOT(EQUIP_SLOT_LEFT_BRACELET);
            break;
        case 10:
            kind = 5;
            cur = viewer.equipment[EQUIP_SLOT_AMULET];
            EQ.slot = SLOT(EQUIP_SLOT_AMULET);
            break;
        case 11:
            kind = 4;
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_RING];
            EQ.slot = SLOT(EQUIP_SLOT_RIGHT_RING);
            break;
        case 12:
            kind = -1;
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC];
            EQ.slot = SLOT(EQUIP_SLOT_RIGHT_MAGIC);
            break;
        case 13:
            kind = 4;
            cur = viewer.equipment[EQUIP_SLOT_LEFT_RING];
            EQ.slot = SLOT(EQUIP_SLOT_LEFT_RING);
            break;
        case 14:
            kind = -1;
            cur = viewer.equipment[EQUIP_SLOT_LEFT_MAGIC];
            EQ.slot = SLOT(EQUIP_SLOT_LEFT_MAGIC);
            break;
        default:
            cur = 0xffff;
            kind = -1;
            EQ.slot = NULL;
            break;
        }
        if (kind == -1) {
            if ((EQ.slot == SLOT(EQUIP_SLOT_RIGHT_MAGIC)
                && viewer.equipment[EQUIP_SLOT_RIGHT_RING] != EQUIP_NONE) ||
                (EQ.slot == SLOT(EQUIP_SLOT_LEFT_MAGIC)
                    && viewer.equipment[EQUIP_SLOT_LEFT_RING] != EQUIP_NONE)) {
                u8 entries[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16};
                u32 *bits;

                if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_MAGIC))
                    bits = &equip_magic_list.bits[viewer.equipment[EQUIP_SLOT_RIGHT_RING] - 196];
                else
                    bits = &equip_magic_list.bits[viewer.equipment[EQUIP_SLOT_LEFT_RING] - 196];
                equip_magic_list.count = 0;
                for (i = 0; i < 12; i++)
                    equip_magic_list.entries[i] = 255;
                for (i = 0; i < 12; i++) {
                    if (*bits & (1 << entries[i]))
                        equip_magic_list.entries[equip_magic_list.count++] = entries[i];
                }
                if (equip_magic_list.count) {
                    play_confirm_sound();
                    sp->mode = (sp->mode | EQUIP_MENU_MAGIC_LIST) & ~EQUIP_MENU_SLOT_LIST;
                    equip_saved_list_cursor[0] = sp->scroll;
                    equip_saved_list_cursor[1] = sp->cursor;
                    sp->scroll = 0;
                    sp->cursor = 0;
                } else {
                    play_refuse_sound();
                }
            } else {
                play_refuse_sound();
            }
            return 0;
        }
        play_confirm_sound();
        sp->state.list.ids = mem_alloc(532);
        sp->state.list.count = 0;
        table = &vis_table_c;
        for (i = 0; i < 266; i++) {
            if (!inventory.counts[i].count || inventory.counts[i].in_bag ||
                table->types[i].category == 255)
                continue;
            switch (kind) {
            case 8:
                if (i == (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_BRACELET) ? viewer.equipment[EQUIP_SLOT_LEFT_BRACELET] : viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET]))
                    continue;
                if (table->types[i].category == kind)
                    sp->state.list.ids[sp->state.list.count++] = i;
                break;
            case 4:
                if (i == (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_RING) ? viewer.equipment[EQUIP_SLOT_LEFT_RING] : viewer.equipment[EQUIP_SLOT_RIGHT_RING]))
                    continue;
                if (table->types[i].category == kind)
                    sp->state.list.ids[sp->state.list.count++] = i;
                break;
            case 6:
                if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_HAND) && i == viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE])
                    continue;
                if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_RESERVE) && i == viewer.equipment[EQUIP_SLOT_RIGHT_HAND])
                    continue;
                if (table->types[i].category == 6 || table->types[i].category == 9)
                    sp->state.list.ids[sp->state.list.count++] = i;
                break;
            case 1:
                if (table->types[i].category == 1 || table->types[i].category == 10)
                    sp->state.list.ids[sp->state.list.count++] = i;
                break;
            case 7:
                if (EQ.slot == SLOT(EQUIP_SLOT_LEFT_HAND) && i == viewer.equipment[EQUIP_SLOT_LEFT_RESERVE])
                    continue;
                if (EQ.slot == SLOT(EQUIP_SLOT_LEFT_RESERVE) && i == viewer.equipment[EQUIP_SLOT_LEFT_HAND])
                    continue;
                if (table->types[i].category == kind)
                    sp->state.list.ids[sp->state.list.count++] = i;
                break;
            default:
                if (table->types[i].category == kind)
                    sp->state.list.ids[sp->state.list.count++] = i;
                break;
            }
        }
        if (cur == 0xffff) {
            EQ.list_index = -1;
        } else {
            for (i = 0; i < sp->state.list.count; i++) {
                if (sp->state.list.ids[i] == cur)
                    break;
            }
            if (i == sp->state.list.count)
                i = -1;
            EQ.list_index = i;
        }
        if (cur != 0xffff)
            i = vis_table_c.types[cur].unk5[0];
        else
            i = -1;
        sp->mode |= EQUIP_MENU_ITEM_LIST;
        if (cur == 0xffff) {
            cd_read_member(ARCHIVE_ICON, 801, cd_file_buffer);
            EQ.shown_id = -1;
            EQ.loaded_id = -1;
        } else {
            cd_read_member(ARCHIVE_ICON, i + 600, cd_file_buffer);
            EQ.shown_id = cur;
            EQ.loaded_id = cur;
        }
        tim_load_images_at(cd_file_buffer, &pos, NULL, 1);
        *sel = cur;
    }
    return 0;
}

/* The magic sub-list: moves the cursor in its entries and equips one. */
ADDRESS(0x80064ba8, 0x33c)
void menu_equip_pick_magic(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 entries[] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16};
    EquipMagicList *list;
    u32 *bits;

    if (PAD.pressed & PAD_BUTTON_CIRCLE) {
        play_cancel_sound();
        PAD.pressed = 0;
        sp->mode = (sp->mode & ~EQUIP_MENU_MAGIC_LIST) | EQUIP_MENU_SLOT_LIST;
        sp->scroll = equip_saved_list_cursor[0];
        sp->cursor = equip_saved_list_cursor[1];
    } else if (PAD.pressed_repeat & PAD_BUTTON_UP) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->cursor == 0) {
            if (sp->scroll > 0)
                sp->scroll--;
        } else {
            sp->cursor--;
        }
    } else if (PAD.pressed_repeat & PAD_BUTTON_DOWN) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->cursor == 4) {
            if (sp->scroll < equip_magic_list.count - 5)
                sp->scroll++;
        } else if (sp->scroll + sp->cursor < equip_magic_list.count - 1) {
            sp->cursor++;
        }
    } else if (PAD.pressed_repeat & PAD_BUTTON_R1) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->scroll == 0)
            sp->cursor = 0;
        else if (sp->scroll >= 5)
            sp->scroll -= 5;
        else
            sp->scroll = 0;
    } else if (PAD.pressed_repeat & PAD_BUTTON_R2) {
        play_cursor_sound();
        PAD.pressed_repeat = 0;
        if (sp->scroll + 5 >= equip_magic_list.count)
            sp->cursor = equip_magic_list.count - sp->scroll - 1;
        else if (sp->scroll + 5 < equip_magic_list.count - 5)
            sp->scroll += 5;
        else
            sp->scroll = equip_magic_list.count - 5;
    } else if (PAD.pressed & PAD_BUTTON_CROSS) {
        PAD.pressed = 0;
        if (EQ.slot == SLOT(EQUIP_SLOT_RIGHT_MAGIC))
            bits = &equip_magic_list.bits[viewer.equipment[EQUIP_SLOT_RIGHT_RING] - 196];
        else
            bits = &equip_magic_list.bits[viewer.equipment[EQUIP_SLOT_LEFT_RING] - 196];
        list = &equip_magic_list;
        if (!(*bits & (1 << list->entries[sp->scroll + sp->cursor]))) {
            play_refuse_sound();
        } else {
            play_confirm_sound();
            *EQ.slot = list->entries[sp->scroll + sp->cursor];
            sp->mode = (sp->mode & ~EQUIP_MENU_MAGIC_LIST) | EQUIP_MENU_SLOT_LIST;
            sp->scroll = equip_saved_list_cursor[0];
            sp->cursor = equip_saved_list_cursor[1];
            menu_equip_recalc_stats();
        }
    }
}

/* The screen loop: the active list until the slot list cancels. */
ADDRESS(0x80064ee4, 0x10c)
void menu_equip_loop(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int sel;

    sp->scroll = 0;
    sp->cursor = 0;
    sel = -1;
    pad_clear_input();
    EQ.input_hold = 0;
    for (;;) {
        if (EQ.input_hold) {
            PAD.pressed_repeat = 0;
            EQ.input_hold--;
        }
        if (sp->mode & EQUIP_MENU_ITEM_LIST) {
            menu_equip_pick_item();
        } else if (sp->mode & EQUIP_MENU_SLOT_LIST) {
            if (menu_equip_pick_slot(&sel))
                break;
        } else if (sp->mode & EQUIP_MENU_MAGIC_LIST) {
            menu_equip_pick_magic();
        }
        frame_rate_update();
        menu_equip_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    pad_clear_input();
}

ADDRESS(0x80064ff0, 0x30)
void menu_equip_run(void)
{
    menu_equip_open();
    menu_equip_loop();
    menu_equip_close();
}

/* Unreferenced: copies the packet area to the current frame's clip and
 * dims the screen. */
ADDRESS(0x80065020, 0xbc)
void func_80065020(void)
{
    RECT r = {0, 0, 320, 240};
    CVECTOR c = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip, world_state.packets[1].end);
    ui_fill_rect(&r, &c, 1);
    ui_add_draw_mode(2, 0, 0);
}

/* The title image and frame; the screen is dimmed while unk2DC bit 0x1
 * is set (the slot list, and the item list over it). */
ADDRESS(0x800650dc, 0x114)
void menu_equip_draw_frame(void)
{
    MENU_SCRATCH->rect.x = 32;
    MENU_SCRATCH->rect.y = 24;
    MENU_SCRATCH->rect.h = 16;
    MENU_SCRATCH->clut_pos.vx = 544;
    MENU_SCRATCH->clut_pos.vy = 511;
    MENU_SCRATCH->rect.w = 50;
    MENU_SCRATCH->tex_pos.vx = 576;
    MENU_SCRATCH->tex_pos.vy = 464;
    ui_draw_image(&MENU_SCRATCH->rect, &MENU_SCRATCH->tex_pos, &MENU_SCRATCH->clut_pos, NULL, 3);
    ui_draw_window(16, 16, 288, 208);
    if (MENU_SCRATCH->mode & EQUIP_MENU_SLOT_LIST) {
        RECT r = {0, 0, 320, 240};
        CVECTOR c = {32, 32, 32, 0};

        ui_fill_rect(&r, &c, 1);
        ui_add_draw_mode(2, 0, 0);
    }
}

/* The slot list's cursor bar. */
ADDRESS(0x800651f0, 0x80)
void menu_equip_draw_cursor(void)
{
    MENU_SCRATCH->cursor_rect.w = 272;
    MENU_SCRATCH->cursor_rect.x = 24;
    MENU_SCRATCH->cursor_rect.h = 34;
    MENU_SCRATCH->cursor_rect.y = MENU_SCRATCH->cursor * 36 + 44;
    ui_draw_rect_outline(&MENU_SCRATCH->cursor_rect, &MENU_SCRATCH->cursor_color, 257, 1);
    ui_add_draw_mode(1, 0, 0);
}

/* The candidate list's panel: the arrows, the shown id's name, figures and
 * counts, and its image. */
ADDRESS(0x80065270, 0x500)
void menu_equip_draw_item_panel(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    s16 id;
    CVECTOR *label, *value;
    int y;
    int i;

    if (sp->state.list.count > 0) {
        RECT left = {24, 100, 16, 8};
        RECT right = {160, 100, 16, 8};

        ui_draw_arrow(&right, &sp->cursor_color, 0);
        ui_draw_arrow(&left, &sp->cursor_color, 2048);
        ui_add_draw_mode(1, 0, 0);
    }
    id = EQ.shown_id;
    if (id != -1) {
        sp->style[0].cd |= 0x80;
        ui_draw_text(MESSAGE_IN(sp->state.list.messages, id), 24, 176, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        if (vis_table_c.types[id].max_durability) {
            if (vis_table_c.types[id].durability < 4) {
                label = &sp->style[2];
                value = &sp->style[6];
            } else {
                label = &sp->style[0];
                value = &sp->style[4];
            }
            ui_draw_text(MESSAGE(MSG_DURABILITY), 48, 190, label);
            ui_draw_number(vis_table_c.types[id].durability | DIGITS(3), 104, 190, value);
            ui_draw_glyph(74, 136, 190, label);
            ui_draw_number(vis_table_c.types[id].max_durability | DIGITS(3), 148, 190, value);
        }
    }
    ui_draw_text(MESSAGE(MSG_ATTRIBUTE_COLUMNS), 165, 46, &sp->style[0]);
    y = 46;
    ui_draw_text(MESSAGE(MSG_ATTRIBUTE_COLUMNS), 165, y, &sp->style[0]);
    for (i = 0; i < 7; i++) {
        if (id == -1) {
            ui_draw_number_fixed(NUMBER_PLUS | DIGITS(3), 207, y, &sp->style[4]);
            ui_draw_number_fixed(NUMBER_PLUS | DIGITS(3), 273, y, &sp->style[4]);
        } else {
            ui_draw_number_fixed(vis_table_c.types[id].attributes[i] | NUMBER_PLUS | DIGITS(3), 207, y,
                &sp->style[4]);
            ui_draw_number_fixed(vis_table_c.types[id].attributes[i + 7] | NUMBER_PLUS | DIGITS(3), 273, y,
                &sp->style[4]);
        }
        y += 14;
    }
    if (id != -1) {
        if (vis_table_c.types[id].weight) {
            menu_figure_whole = vis_table_c.types[id].weight * 100 / 454;
            DAT_80088188 = vis_table_c.types[id].weight * 1000 / 454;
            ui_draw_text(MESSAGE(MSG_WEIGHT), 221, 156, &sp->style[0]);
            ui_draw_number_fixed(menu_figure_whole | DIGITS(3), 249, 156, &sp->style[4]);
            ui_draw_glyph(78, 269, 156, &sp->style[0]);
            ui_draw_number_fixed((u16)(DAT_80088188 - menu_figure_whole * 10) | DIGITS(1), 275, 156,
                          &sp->style[4]);
            ui_draw_glyph(108, 281, 156, &sp->style[0]);
        }
        sp->image_rect.x = 20;
        sp->image_rect.y = 40;
        sp->image_rect.w = 128;
        sp->image_rect.h = 128;
        sp->tex_pos.vx = 448;
        sp->tex_pos.vy = 256;
        ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
    } else {
        sp->image_rect.x = 28;
        sp->image_rect.y = 40;
        sp->image_rect.w = 128;
        sp->image_rect.h = 128;
        sp->tex_pos.vx = 448;
        sp->tex_pos.vy = 256;
        ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
    }
}

/* The slot list: the two load figures, then five slots with what each
 * holds and its figures. */
ADDRESS(0x80065770, 0x670)
void menu_equip_draw_slots(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    CVECTOR *label, *value;
    CVECTOR *label2, *value2;
    u16 cur;
    int row;
    int i, y, y2;

    if (viewer.equip_weight > viewer.equip_weight_limit) {
        label = &sp->style[2];
        value = &sp->style[6];
    } else {
        label = &sp->style[0];
        value = &sp->style[4];
    }
    menu_figure_whole = viewer.equip_weight * 100 / 454;
    DAT_80088188 = viewer.equip_weight * 1000 / 454;
    ui_draw_text(MESSAGE(MSG_EQUIP_WEIGHT), 113, 26, label);
    ui_draw_number(menu_figure_whole | DIGITS(3), 181, 26, value);
    ui_draw_glyph(78, 212, 26, label);
    ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | DIGITS(1), 217, 26, value);
    ui_draw_glyph(74, 230, 26, label);
    menu_figure_whole = viewer.equip_weight_limit * 100 / 454;
    DAT_80088188 = viewer.equip_weight_limit * 1000 / 454;
    ui_draw_number(menu_figure_whole | DIGITS(3), 243, 26, value);
    ui_draw_glyph(78, 274, 26, label);
    ui_draw_number((u16)(DAT_80088188 - menu_figure_whole * 10) | DIGITS(1), 279, 26, value);
    for (i = 0; i < 5; i++) {
        y = 48 + i * 36;
        y2 = 64 + i * 36;
        ui_draw_text(MESSAGE(sp->scroll + i + MSG_EQUIP_SLOT_NAMES), 28, y, &sp->style[0]);
        switch (sp->scroll + i) {
        case 0:
            cur = viewer.equipment[EQUIP_SLOT_HELMET];
            break;
        case 1:
            cur = viewer.equipment[EQUIP_SLOT_ARMOR];
            break;
        case 2:
            cur = viewer.equipment[EQUIP_SLOT_GLOVES];
            break;
        case 3:
            cur = viewer.equipment[EQUIP_SLOT_BOOTS];
            break;
        case 4:
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_HAND];
            break;
        case 5:
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE];
            break;
        case 6:
            cur = viewer.equipment[EQUIP_SLOT_LEFT_HAND];
            break;
        case 7:
            cur = viewer.equipment[EQUIP_SLOT_LEFT_RESERVE];
            break;
        case 8:
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET];
            break;
        case 9:
            cur = viewer.equipment[EQUIP_SLOT_LEFT_BRACELET];
            break;
        case 10:
            cur = viewer.equipment[EQUIP_SLOT_AMULET];
            break;
        case 11:
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_RING];
            break;
        case 12:
            cur = viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC];
            break;
        case 13:
            cur = viewer.equipment[EQUIP_SLOT_LEFT_RING];
            break;
        case 14:
            cur = viewer.equipment[EQUIP_SLOT_LEFT_MAGIC];
            break;
        default:
            cur = 0xffff;
            break;
        }
        row = sp->scroll + i;
        if (row == 12 || row == 14) {
            if (cur != 0xffff) {
                u8 entries[] = {0, 1, 255, 2, 3, 255, 4, 5, 255, 6, 7, 255, 8, 9, 255, 10, 11, 255};

                ui_draw_text(MESSAGE(MSG_MAGIC_NAMES + entries[cur]), 92, y, &sp->style[0]);
            } else {
                ui_draw_text(MESSAGE(MSG_NO_SELECT), 92, y, &sp->style[0]);
            }
        } else if (cur != 0xffff) {
            sp->style[0].cd |= 0x80;
            sp->style[3].cd |= 0x80;
            ui_draw_text(MESSAGE_IN(sp->state.list.messages, cur), 92, y, &sp->style[0]);
            sp->style[0].cd &= 0x7f;
            sp->style[3].cd &= 0x7f;
        } else {
            ui_draw_text(MESSAGE(MSG_NO_EQUIP), 92, y, &sp->style[0]);
        }
        row = sp->scroll + i;
        if (row != 12 && row != 14 && cur != 0xffff && vis_table_c.types[cur].max_durability) {
            if (vis_table_c.types[cur].durability < 4) {
                label2 = &sp->style[2];
                value2 = &sp->style[6];
            } else {
                label2 = &sp->style[0];
                value2 = &sp->style[4];
            }
            ui_draw_text(MESSAGE(MSG_DURABILITY), 92, y2, label2);
            ui_draw_number(vis_table_c.types[cur].durability | DIGITS(3), 172, y2, value2);
            ui_draw_glyph(74, 204, y2, label2);
            ui_draw_number(vis_table_c.types[cur].max_durability | DIGITS(3), 220, y2, value2);
        }
    }
}

/* The magic sub-list: five entries with their names and effect figures. */
/* The text base is added as an integer after the offset, and the name table
 * pointer is assigned inside the loop: retail adds in that operand order
 * and keeps the table address in the loop. */
ADDRESS(0x80065de0, 0x160)
void menu_equip_draw_magic_list(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int i = 0;
    EquipMagicList *list = &equip_magic_list;
    CVECTOR *label = &sp->style[0];
    u16 *offsets = menu_messages.offsets;
    u32 text = (u32)menu_messages.text;
    int y, y2;
    u8 *names = NULL;

    y2 = 64;
    y = 48;
    for (; i < 5; i++) {
        if (*(list->entries + (sp->scroll + i)) != 255) {
            names = magic_name_messages;
            ui_draw_text(
                (u16 *)(offsets[MSG_MAGIC_NAMES + names[*(list->entries + (sp->scroll + i))]] + text), 28, y,
                label);
            ui_draw_text((u16 *)(offsets[MSG_MP_USED] + text), 140, y2, label);
            ui_draw_number(effects.defs[*(list->entries + (sp->scroll + i))].mp_cost | DIGITS(3),
                          204, y2, &sp->style[4]);
            y2 += 36;
            y += 36;
        }
    }
}

/* As func_80065020, from 64 KiB into the packet area; menu_equip_draw calls it. */
ADDRESS(0x80065f40, 0xc4)
void menu_equip_draw_backdrop(void)
{
    RECT r = {0, 0, 320, 240};
    CVECTOR c = {32, 32, 32};

    LoadImage(&world_state.draw[world_state.frame].clip,
              (u_long *)((u8 *)world_state.packets[1].end + 0x10000));
    ui_fill_rect(&r, &c, 1);
    ui_add_draw_mode(2, 0, 0);
}

/* Draws the screen: the text colours, the cursor blink and the active
 * list's panels. */
ADDRESS(0x80066004, 0x168)
void menu_equip_draw(void)
{
    MenuScratch *sp;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[3].r = sp->style[7].r = 96;
    sp->style[3].g = sp->style[7].g = sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 41;
    if (MENU_SCRATCH->blink_phase < 16)
        sp->blink_level = 8 - MENU_SCRATCH->blink_phase;
    else
        sp->blink_level = MENU_SCRATCH->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (!(sp->mode & EQUIP_MENU_ITEM_LIST))
        menu_equip_draw_cursor();
    if (sp->mode & EQUIP_MENU_ITEM_LIST)
        menu_equip_draw_item_panel();
    else if (sp->mode & EQUIP_MENU_SLOT_LIST)
        menu_equip_draw_slots();
    else
        menu_equip_draw_magic_list();
    menu_equip_draw_frame();
    gfx_flip();
    menu_equip_draw_backdrop();
    gfx_draw_ot();
}

/* Nonzero while the CD request ring still holds a request. */
ADDRESS(0x8006616c, 0x5c)
s16 cd_queue_busy(void)
{
    s16 busy;

    EnterCriticalSection();
    busy = 1;
    if (cd_state.current == cd_state.next_free)
        busy = cd_state.current->kind != 0;
    ExitCriticalSection();
    return busy;
}

/* The load callback: copies the loaded image to VRAM (640, 0). */
ADDRESS(0x800661c8, 0x44)
void menu_arrange_list_upload_image(void)
{
    DVECTOR pos = {640, 0};

    tim_load_images_at(cd_file_buffer, &pos, NULL, 1);
    DrawSync(0);
}

ADDRESS(0x8006620c, 0x34)
void menu_arrange_list_queue_image(u16 archive, u16 member)
{
    DVECTOR unused;

    cd_queue_read_member(archive, member, (u8 *)cd_file_buffer, menu_arrange_list_upload_image);
}

/*
 * Opens the screen: loads its message tables and images, then lists every
 * held table C id (except 266 and the selected viewer.equipment slots) into
 * the grid of its flag bit. Slots 6 and 12 are not filtered. Message files
 * use a length-prefixed cursor; raw TIM files use the load buffer directly.
 */
ADDRESS(0x80066240, 0x470)
void menu_arrange_list_open(void)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    RECT unused;
    char name[] = "\\st\\itemcg\\icon1\\0000.tim";
    u_long *tim;
    MenuScratch *sp;
    int i;

    arrange_list_state.grid = 104;
    arrange_list_state.range_held = 0;
    arrange_list_state.range_anchor = arrange_list_state.range_end = arrange_list_state.range_start = -1;
    menu_show_loading_screen();
    MENU_SCRATCH->state.grid.messages = mem_alloc(21504);
    tim = cd_file_buffer;
    cd_read_member(ARCHIVE_STAT, STAT_SCREEN_MESSAGES, tim);
    copy_words((u32 *)&menu_messages, (u32 *)(tim + 1), 5376);
    tim = (u_long *)((u8 *)tim + (*tim + sizeof(*tim)));
    tim_load_images_at(tim + 1, &MENU_SCRATCH->image_vram[0], &MENU_SCRATCH->image_vram[1], 1);
    tim = cd_file_buffer;
    cd_read_member(ARCHIVE_STAT, STAT_ITEM_NAMES_HIGH_RES, tim);
    sp = MENU_SCRATCH;
    copy_words((u32 *)sp->state.grid.messages, (u32 *)(tim + 1), 5376);
    tim = (u_long *)((u8 *)tim + (*tim + sizeof(*tim)));
    tim_load_images_at(tim + 1, &sp->image_vram[2], &sp->image_vram[3], 1);
    cd_read_member(ARCHIVE_STAT, STAT_BACKGROUND, cd_file_buffer);
    tim_load_images_at(cd_file_buffer, &sp->image_vram[4], NULL, 1);
    DrawSync(0);
    cd_read_member(ARCHIVE_STAT, STAT_HEADER_IMAGES, cd_file_buffer);
    tim_load_images_at(cd_file_buffer, &tex, &clut, 1);
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
        if (vis_table_c.types[i].unk5[0] != 255 && i < 314 && i != 266 &&
            inventory.counts[i].count && i != viewer.equipment[EQUIP_SLOT_RIGHT_RING]
                && i != viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET] &&
            i != viewer.equipment[EQUIP_SLOT_GLOVES] && i != viewer.equipment[EQUIP_SLOT_RIGHT_HAND]
                && i != viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] &&
            i != viewer.equipment[EQUIP_SLOT_QUICK_1] && i != viewer.equipment[EQUIP_SLOT_AMULET]
                && i != viewer.equipment[EQUIP_SLOT_HELMET] &&
            i != viewer.equipment[EQUIP_SLOT_ARMOR] && i != viewer.equipment[EQUIP_SLOT_BOOTS]
                && i != viewer.equipment[EQUIP_SLOT_QUICK_3] &&
            i != viewer.equipment[EQUIP_SLOT_LEFT_RING] && i != viewer.equipment[EQUIP_SLOT_LEFT_BRACELET] &&
            i != viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR] && i != viewer.equipment[EQUIP_SLOT_LEFT_HAND]
                && i != viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] &&
            i != viewer.equipment[EQUIP_SLOT_QUICK_5] && i != viewer.equipment[EQUIP_SLOT_QUICK_2]
                && i != viewer.equipment[EQUIP_SLOT_QUICK_4]) {
            if (!inventory.counts[i].in_bag)
                sp->state.grid.hand_ids[sp->state.grid.hand_count++] = i;
            else
                sp->state.grid.bag_ids[sp->state.grid.bag_count++] = i;
        }
    }
    sp->mode = 1;
    sp->cursor = 0;
    sp->state.grid.hand_row = 0;
    sp->state.grid.bag_row = 0;
    arrange_list_state.shown_id = -1;
    clear_vram();
    DrawSync(0);
    gfx_setup_640x240();
}

#define GRID (&sp->state.grid)
#define ST arrange_list_state
#define PAD pad_state

/* Resets the held range. */
#define RANGE_CLEAR() \
    ST.range_held = 0;      \
    ST.range_start = -1;     \
    ST.range_end = -1;     \
    ST.range_anchor = -1

/*
 * The screen loop: moves ids between the grids, compacts and scrolls them
 * and draws the frame, until cancel; then writes each listed id's grid back
 * to its flag bit.
 */
ADDRESS(0x800666b0, 0xed0)
void menu_arrange_list_loop(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    DVECTOR unused0;
    DVECTOR pos = {640, 0};
    char name1[] = "\\st\\itemcg\\icon1\\0000.tim";
    char name5[] = "\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR unused1;
    s16 id;
    s16 n;
    s16 k;
    s16 idx;
    int i, j;
    int range;
    int hole;
    int found;
    int cur;
    int anchor;

    pad_clear_input();
    for (;;) {
        if (PAD.pressed & PAD_BUTTON_CIRCLE) {
            play_cancel_sound();
            PAD.pressed = 0;
            if (sp->mode & 0x8000)
                sp->mode &= 0x7fff;
            else if (sp->mode == 2)
                sp->mode = 1;
            else
                break;
        }
        if (PAD.pressed & PAD_BUTTON_CROSS) {
            play_confirm_sound();
            PAD.pressed = 0;
            if (sp->mode & 0x8000) {
                if (ST.grid == 104) {
                    if (!ST.range_held)
                        ST.range_start = ST.range_end = GRID->hand_row + sp->cursor;
                    for (range = ST.range_start; range <= ST.range_end; range++) {
                        idx = range;
                        if (GRID->hand_ids[idx] != -1) {
                            k = GRID->bag_count;
                            for (hole = 0; hole < GRID->bag_count; hole++) {
                                if (GRID->bag_ids[hole] == -1) {
                                    k = hole;
                                    break;
                                }
                            }
                            GRID->bag_ids[k] = GRID->hand_ids[idx];
                            GRID->hand_ids[idx] = -1;
                            GRID->bag_count++;
                            GRID->hand_count--;
                        }
                    }
                } else {
                    if (!ST.range_held)
                        ST.range_start = ST.range_end = GRID->bag_row + sp->cursor;
                    for (range = ST.range_start; range <= ST.range_end; range++) {
                        idx = range;
                        if (GRID->bag_ids[idx] != -1) {
                            k = GRID->hand_count;
                            for (hole = 0; hole < GRID->hand_count; hole++) {
                                if (GRID->hand_ids[hole] == -1) {
                                    k = hole;
                                    break;
                                }
                            }
                            GRID->hand_ids[k] = GRID->bag_ids[idx];
                            GRID->bag_ids[idx] = -1;
                            GRID->hand_count++;
                            GRID->bag_count--;
                        }
                    }
                }
                RANGE_CLEAR();
                sp->mode &= 0x7fff;
            } else {
                found = 0;
                if (!ST.range_held) {
                    if (ST.grid == 104) {
                        cur = GRID->hand_row;
                        found = GRID->hand_ids[cur + sp->cursor] == -1;
                    } else {
                        cur = GRID->bag_row;
                        if (GRID->bag_ids[cur + sp->cursor] == -1)
                            found = 1;
                    }
                } else {
                    found = 1;
                    if (ST.grid == 104) {
                        for (i = ST.range_start; i <= ST.range_end; i++) {
                            if (GRID->hand_ids[i] != -1) {
                                found = 0;
                                break;
                            }
                        }
                    } else {
                        for (i = ST.range_start; i <= ST.range_end; i++) {
                            if (GRID->bag_ids[i] != -1) {
                                found = 0;
                                break;
                            }
                        }
                    }
                }
                if (!found)
                    sp->mode |= 0x8000;
            }
        }
        if (!(sp->mode & 0x8000)) {
            if (PAD.pressed & PAD_BUTTON_SELECT) {
                play_confirm_sound();
                PAD.pressed = 0;
                if (ST.grid == 104) {
                    for (i = 0; i < 314; i++) {
                        if (GRID->hand_ids[i] == -1) {
                            for (j = i; j < 314; j++) {
                                if (GRID->hand_ids[j] != -1) {
                                    GRID->hand_ids[i] = GRID->hand_ids[j];
                                    GRID->hand_ids[j] = -1;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    for (i = 0; i < 314; i++) {
                        if (GRID->bag_ids[i] == -1) {
                            for (j = i; j < 314; j++) {
                                if (GRID->bag_ids[j] != -1) {
                                    GRID->bag_ids[i] = GRID->bag_ids[j];
                                    GRID->bag_ids[j] = -1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (PAD.pressed & PAD_BUTTON_SQUARE) {
                play_confirm_sound();
                PAD.pressed = 0;
                if (ST.range_held) {
                    RANGE_CLEAR();
                } else {
                    ST.range_held = 1;
                    if (ST.grid == 104)
                        cur = GRID->hand_row + sp->cursor;
                    else
                        cur = GRID->bag_row + sp->cursor;
                    ST.range_anchor = ST.range_start = ST.range_end = cur;
                }
            }
            if (PAD.pressed & PAD_BUTTON_TRIANGLE) {
                play_confirm_sound();
                PAD.pressed = 0;
                if (sp->mode == 1)
                    sp->mode = 2;
                else if (sp->mode == 2)
                    sp->mode = 1;
            }
            if (PAD.pressed_repeat & PAD_BUTTON_RIGHT) {
                play_cursor_sound();
                PAD.pressed_repeat = 0;
                if (ST.grid == 104) {
                    ST.grid = 98;
                    RANGE_CLEAR();
                }
            }
            if (PAD.pressed_repeat & PAD_BUTTON_LEFT) {
                play_cursor_sound();
                PAD.pressed_repeat = 0;
                if (ST.grid == 98) {
                    ST.grid = 104;
                    RANGE_CLEAR();
                }
            }
            if (PAD.pressed_repeat & PAD_BUTTON_UP) {
                play_cursor_sound();
                if (PAD.held & PAD_BUTTON_SQUARE) {
                    if (ST.grid == 104) {
                        if (GRID->hand_row == 0)
                            sp->cursor = 0;
                        GRID->hand_row -= 11;
                        if (GRID->hand_row < 0)
                            GRID->hand_row = 0;
                    } else {
                        if (GRID->bag_row == 0)
                            sp->cursor = 0;
                        GRID->bag_row -= 11;
                        if (GRID->bag_row < 0)
                            GRID->bag_row = 0;
                    }
                } else if (sp->cursor > 0) {
                    sp->cursor--;
                } else if (ST.grid == 104) {
                    if (GRID->hand_row > 0)
                        GRID->hand_row--;
                } else {
                    if (GRID->bag_row > 0)
                        GRID->bag_row--;
                }
                PAD.pressed_repeat = 0;
            }
            if (PAD.pressed_repeat & PAD_BUTTON_DOWN) {
                play_cursor_sound();
                if (PAD.held & PAD_BUTTON_SQUARE) {
                    if (ST.grid == 104) {
                        if (GRID->hand_row == 303)
                            sp->cursor = 10;
                        GRID->hand_row += 11;
                        if (GRID->hand_row >= 304)
                            GRID->hand_row = 303;
                    } else {
                        if (GRID->bag_row == 303)
                            sp->cursor = 10;
                        GRID->bag_row += 11;
                        if (GRID->bag_row >= 304)
                            GRID->bag_row = 303;
                    }
                } else if (sp->cursor < 10) {
                    sp->cursor++;
                } else if (ST.grid == 104) {
                    if (GRID->hand_row < 303)
                        GRID->hand_row++;
                } else {
                    if (GRID->bag_row < 303)
                        GRID->bag_row++;
                }
                PAD.pressed_repeat = 0;
            }
            if ((sp->mode == 2 && ST.grid == 104) || sp->mode != 2) {
                if (PAD.pressed_repeat & PAD_BUTTON_L1) {
                    play_cursor_sound();
                    PAD.pressed_repeat = 0;
                    if (ST.grid == 104 && GRID->hand_row == 0)
                        sp->cursor = 0;
                    GRID->hand_row -= 11;
                    if (GRID->hand_row < 0)
                        GRID->hand_row = 0;
                }
                if (PAD.pressed_repeat & PAD_BUTTON_L2) {
                    play_cursor_sound();
                    PAD.pressed_repeat = 0;
                    if (ST.grid == 104 && GRID->hand_row == 303)
                        sp->cursor = 10;
                    GRID->hand_row += 11;
                    if (GRID->hand_row >= 304)
                        GRID->hand_row = 303;
                }
            }
            if ((sp->mode == 2 && ST.grid == 98) || sp->mode != 2) {
                if (PAD.pressed_repeat & PAD_BUTTON_R1) {
                    play_cursor_sound();
                    PAD.pressed_repeat = 0;
                    if (ST.grid == 98 && GRID->bag_row == 0)
                        sp->cursor = 0;
                    GRID->bag_row -= 11;
                    if (GRID->bag_row < 0)
                        GRID->bag_row = 0;
                }
                if (PAD.pressed_repeat & PAD_BUTTON_R2) {
                    play_cursor_sound();
                    PAD.pressed_repeat = 0;
                    if (ST.grid == 98 && GRID->bag_row == 303)
                        sp->cursor = 10;
                    GRID->bag_row += 11;
                    if (GRID->bag_row >= 304)
                        GRID->bag_row = 303;
                }
            }
            if (ST.range_held) {
                if (ST.grid == 104)
                    cur = GRID->hand_row + sp->cursor;
                else
                    cur = GRID->bag_row + sp->cursor;
                anchor = ST.range_anchor;
                if (anchor < cur) {
                    ST.range_start = anchor;
                    ST.range_end = cur;
                } else {
                    if (cur < anchor)
                        ST.range_start = cur;
                    else
                        ST.range_start = anchor;
                    ST.range_end = anchor;
                }
            }
            if (sp->mode == 2 && !cd_queue_busy()) {
                if (ST.grid == 104)
                    id = GRID->hand_ids[GRID->hand_row + sp->cursor];
                else
                    id = GRID->bag_ids[GRID->bag_row + sp->cursor];
                if (ST.shown_id != id && id != -1) {
                    menu_arrange_list_queue_image(ARCHIVE_ICON, vis_table_c.types[id].unk5[0] + 200);
                    ST.shown_id = id;
                }
            }
        }
        frame_rate_update();
        menu_arrange_list_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    for (i = 0, n = GRID->hand_count; n;) {
        if (GRID->hand_ids[i] != -1) {
            inventory.counts[GRID->hand_ids[i]].in_bag = 0;
            n--;
        }
        i++;
    }
    for (i = 0, n = GRID->bag_count; n;) {
        if (GRID->bag_ids[i] != -1) {
            inventory.counts[GRID->bag_ids[i]].in_bag = 1;
            n--;
        }
        i++;
    }
}

/* Closes the screen once the CD ring is idle. */
ADDRESS(0x80067580, 0x74)
void menu_arrange_list_close(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    while (cd_queue_busy())
        ;
    menu_load_main_messages();
    mem_free(sp->state.grid.hand_ids);
    mem_free(sp->state.grid.bag_ids);
    mem_free(sp->state.grid.messages);
    menu_reload_assets();
    pad_clear_input();
}

ADDRESS(0x800675f4, 0x30)
void menu_arrange_list_run(void)
{
    menu_arrange_list_open();
    menu_arrange_list_loop();
    menu_arrange_list_close();
}

/*
 * The detail panel of the id under the cursor: its image (when the CD is
 * idle), its fourteen table C counts and its two figures.
 */
ADDRESS(0x80067624, 0x5c8)
void menu_arrange_list_draw_details(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int text_x, text_y;
    s16 id;
    int i;
    s16 x, y;
    s16 selected;
    short none;
    int n, k;
    int j;
    u8 *count;
    CVECTOR *label, *value;
    u8 whole, tenths;

    if (ST.grid == 104) {
        x = 334;
        y = 9;
        text_x = 342;
        text_y = 135;
        sp->image_rect.w = 192;
        sp->image_rect.x = x;
        sp->image_rect.h = 124;
        sp->image_rect.y = y;
    } else {
        x = 10;
        y = 9;
        text_x = 18;
        text_y = 135;
        sp->image_rect.w = 192;
        sp->image_rect.x = x;
        sp->image_rect.h = 124;
        sp->image_rect.y = y;
    }
    none = 0;
    if (ST.grid == 104) {
        selected = sp->state.grid.hand_row + sp->cursor;
        none = sp->state.grid.hand_ids[selected] == -1;
        id = sp->state.grid.hand_ids[selected];
    } else {
        selected = sp->state.grid.bag_row + sp->cursor;
        id = sp->state.grid.bag_ids[selected];
        if (sp->state.grid.bag_ids[selected] == -1)
            none = 1;
    }
    if (!none) {
        if (!cd_queue_busy()) {
            sp->tex_pos.vx = 640;
            sp->tex_pos.vy = 0;
            ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
            sp->image_rect.w = 100;
            sp->tex_pos.vx = 832;
            sp->image_rect.x += 192;
            ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
        }
        n = 0;
        k = 2;
        count = vis_table_c.types[id].attributes;
        number_narrow = 1;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    ui_draw_text(MESSAGE(MSG_HI_HAND + k), text_x - 4 + i * 72, text_y + j * 15,
                        &sp->style[0]);
                    ui_draw_number(*count | NUMBER_PLUS | DIGITS(3), text_x + i * 72 + 40, text_y + j * 15,
                                  &sp->style[4]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
        }
    done:
        number_narrow = 0;
        text_y += 60;
        if (vis_table_c.types[id].max_durability) {
            if (vis_table_c.types[id].durability < 4) {
                label = &sp->style[2];
                value = &sp->style[6];
            } else {
                label = &sp->style[0];
                value = &sp->style[4];
            }
            ui_draw_text(MESSAGE(MSG_HI_DURABILITY), text_x, text_y, label);
            ui_draw_number(vis_table_c.types[id].durability | DIGITS(3), text_x + 70, text_y, value);
            ui_draw_glyph(14, text_x + 106, text_y, label);
            ui_draw_number(vis_table_c.types[id].max_durability | DIGITS(3), text_x + 122, text_y, value);
        }
        text_y += 15;
        if (vis_table_c.types[id].weight) {
            whole = vis_table_c.types[id].weight * 100 / 454;
            tenths = vis_table_c.types[id].weight * 1000 / 454 - whole * 10;
            ui_draw_text(MESSAGE(MSG_HI_WEIGHT), text_x, text_y, &sp->style[0]);
            ui_draw_number(whole | DIGITS(3), text_x + 70, text_y, &sp->style[4]);
            ui_draw_glyph(18, text_x + 109, text_y, &sp->style[0]);
            ui_draw_number(tenths | DIGITS(1), text_x + 116, text_y, &sp->style[4]);
            ui_draw_glyph(53, text_x + 132, text_y, &sp->style[0]);
        }
    }
    ui_draw_panel(x, y, 296, 222);
}

/*
 * Draws one grid (104 or 98): the cursor bar, the scroll arrows and bar, the
 * header image and eleven rows of names, the held range highlighted.
 */
ADDRESS(0x80067bec, 0x66c)
void menu_arrange_list_draw_list(u8 grid)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused5;
    CVECTOR unused0 = {96, 96, 96};
    RECT bar;
    RECT unused1;
    RECT unused2;
    RECT unused3 = {314, 119, 16, 8};
    RECT unused4 = {625, 119, 16, 8};
    RECT up0 = {26, 28, 24, 8};
    RECT down0 = {26, 216, 24, 8};
    RECT knob0 = {16, 35, 10, 3};
    RECT up1 = {612, 28, 24, 8};
    RECT down1 = {612, 216, 24, 8};
    RECT knob1 = {611, 35, 10, 3};
    CVECTOR knob = {40, 40, 48};
    int i;
    int idx;
    s16 id;
    CVECTOR *color;
    int x, y;
    int row;

    if (ST.grid == 104) {
        sp->cursor_rect.x = 26;
        sp->cursor_rect.y = sp->cursor * 16 + 33;
    } else {
        sp->cursor_rect.x = 340;
        sp->cursor_rect.y = sp->cursor * 16 + 33;
    }
    sp->cursor_rect.w = 270;
    sp->cursor_rect.h = 15;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    if (grid == 104) {
        if (sp->state.grid.hand_row > 0)
            ui_draw_arrow(&up0, &sp->cursor_color, 3072);
        if (sp->state.grid.hand_row < 303)
            ui_draw_arrow(&down0, &sp->cursor_color, 1024);
    } else {
        if (sp->state.grid.bag_row > 0)
            ui_draw_arrow(&up1, &sp->cursor_color, 3072);
        if (sp->state.grid.bag_row < 303)
            ui_draw_arrow(&down1, &sp->cursor_color, 1024);
    }
    if (grid == 104) {
        knob0.y = sp->state.grid.hand_row * 173 / 303 + 35;
        ui_fill_rect(&knob0, &knob, 1);
        ui_add_draw_mode(1, 0, 0);
        bar.x = knob0.x - 2;
        bar.y = knob0.y - 1;
        bar.w = knob0.w;
        bar.h = knob0.h;
    } else {
        knob1.y = sp->state.grid.bag_row * 173 / 303 + 35;
        ui_fill_rect(&knob1, &knob, 1);
        ui_add_draw_mode(1, 0, 0);
        bar.x = knob1.x - 2;
        bar.y = knob1.y - 1;
        bar.w = knob1.w;
        bar.h = knob1.h;
    }
    ui_fill_rect(&bar, &knob, 1);
    ui_add_draw_mode(2, 0, 0);
    if (grid == 104) {
        sp->rect.x = 60;
        sp->rect.y = 12;
        sp->rect.w = 196;
        sp->rect.h = 16;
        sp->tex_pos.vx = 896;
        sp->tex_pos.vy = 448;
        sp->clut_pos.vx = 544;
        sp->clut_pos.vy = 502;
    } else {
        sp->rect.x = 384;
        sp->rect.y = 12;
        sp->rect.w = 196;
        sp->rect.h = 16;
        sp->tex_pos.vx = 896;
        sp->tex_pos.vy = 464;
        sp->clut_pos.vx = 544;
        sp->clut_pos.vy = 502;
    }
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    {
        CVECTOR held = {0, 128, 128, 201};
        CVECTOR empty = {0, 96, 96, 201};
        RECT box;

        if (grid == 104) {
            x = 28;
            y = 35;
        } else {
            x = 342;
            y = 35;
        }
        sp->style[0].cd |= 0x80;
        for (i = 0; i < 11; i++) {
            if (grid == 104) {
                row = sp->state.grid.hand_row + i;
                id = sp->state.grid.hand_ids[row];
                idx = row;
            } else {
                row = sp->state.grid.bag_row + i;
                id = sp->state.grid.bag_ids[row];
                idx = row;
            }
            color = &sp->style[0];
            if (grid == ST.grid && ST.range_held && idx >= ST.range_start && idx <= ST.range_end) {
                color = &held;
                if (id == -1) {
                    box.x = x;
                    box.y = y + i * 16;
                    box.w = 266;
                    box.h = 12;
                    ui_fill_rect(&box, &empty, 0);
                }
            } else {
                color = &sp->style[0];
            }
            if (id != -1)
                ui_draw_text(MESSAGE_IN(sp->state.grid.messages, id), x, y + i * 16, color);
        }
        sp->style[0].cd &= 0x7f;
    }
    if (grid == 104)
        ui_draw_panel(10, 9, 296, 222);
    else
        ui_draw_panel(334, 9, 296, 222);
}

/* Draws the screen: the text colours, the cursor blink, one grid with its
 * detail panel or both grids, and the arrow toward the current grid. */
ADDRESS(0x80068258, 0x1e8)
void menu_arrange_list_draw(void)
{
    RECT arrow = {319, 110, 24, 16};
    CVECTOR color = {16, 48, 16, 0};
    MenuScratch *sp;

    gfx_begin_frame();
    sp = MENU_SCRATCH;
    sp->style[3].r = sp->style[7].r = 96;
    sp->style[3].g = sp->style[7].g = sp->style[3].b = sp->style[7].b = 48;
    sp->style[0].b = sp->style[4].b = sp->style[0].g = sp->style[4].g = sp->style[0].r =
        sp->style[4].r = 96;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 73;
    sp->style[4].cd = sp->style[6].cd = sp->style[7].cd = 49;
    if (MENU_SCRATCH->blink_phase < 16)
        sp->blink_level = 8 - MENU_SCRATCH->blink_phase;
    else
        sp->blink_level = MENU_SCRATCH->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (sp->mode & 0x8000)
        menu_draw_button_prompt();
    if (sp->mode & 0x1) {
        menu_arrange_list_draw_list(104);
        menu_arrange_list_draw_list(98);
    } else {
        menu_arrange_list_draw_list(ST.grid == 104 ? 104 : 98);
        menu_arrange_list_draw_details();
    }
    if (ST.grid == 104)
        ui_draw_arrow(&arrow, &color, 0);
    else
        ui_draw_arrow(&arrow, &color, 2048);
    ui_add_draw_mode(1, 0, 0);
    menu_draw_background();
    gfx_end_frame();
}

/* The pad's button word (active low) read as one volatile halfword at its
 * own address; retail forms that address directly in each load. */
#define RAW_BUTTONS (*(volatile s16 *)&pad_buffers.buttons)

/*
 * Unreferenced debug tool: shows the other frame buffer and moves the
 * display area with the face buttons (triangle, cross, square, circle as
 * up, down, left, right; raw pad bits, active low) until START is pressed,
 * polling the host debugger each frame; then restores it.
 */
ADDRESS(0x80068440, 0x214)
void debug_pan_display(void)
{
    RECT saved;
    DISPENV *disp;
    DRAWENV *draw;
    int bits;

    world_state.draw[0].isbg = 0;
    world_state.draw[1].isbg = 0;
    world_state.frame = world_state.frame == 0;
    DrawSync(0);
    VSync(0);
    PutDispEnv(&world_state.disp[world_state.frame]);
    PutDrawEnv(&world_state.draw[world_state.frame]);
    disp = &world_state.disp[world_state.frame];
    draw = &world_state.draw[world_state.frame];
    copy_words((u32 *)&saved, (u32 *)&disp->disp, 2);
    while (!(RAW_BUTTONS & 0x8))
        ;
    for (;;) {
        bits = ~RAW_BUTTONS;
        if (bits & 0x2000)
            disp->disp.x += 4;
        if (bits & 0x8000)
            disp->disp.x -= 4;
        if (bits & 0x1000)
            disp->disp.y -= 4;
        if (bits & 0x4000)
            disp->disp.y += 4;
        if (bits & 0x8)
            break;
        disp->disp.x &= 0x3ff;
        disp->disp.y &= 0x1ff;
        VSync(0);
        PutDispEnv(disp);
        PutDrawEnv(draw);
        pollhost();
    }
    while (!(RAW_BUTTONS & 0x8))
        ;
    copy_words((u32 *)&disp->disp, (u32 *)&saved, 2);
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
}
