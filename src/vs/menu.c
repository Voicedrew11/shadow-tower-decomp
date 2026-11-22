/*
 * VS.EXE: the versus setup screens (WIP module): the setup page's loop,
 * the team screen (pictures, slots and input), the item request list,
 * the requestable-item test and the memory card screen with the match banking.
 * One translation unit: retail lays out its constants (0x80011110, the
 * request list's picture paths and the card screen's file name prefix)
 * before its one switch table (0x80011160, menu_vs_setup_run's), the order
 * cc1 writes one object's read-only data in.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>

/* Whether each player's memory card holds a usable save, read and set
 * $gp-relative here (main reads them absolutely): these tentative
 * definitions merge into vs.sbss_save_loaded's, which a first-linked
 * object allocates at the end of the first .sbss run. */
u8 save_loaded_1p;
u8 save_loaded_2p;

/* The setup, team and pick screens index both requested items. */
#define VS_PICKED_PAIR

#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>
#include <vs/sound.h>
#include <vs/funcs.h>
#include <MALLOC.H>
#include <MEMORY.H>
#include <LIBAPI.H>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <vs/buffer.h>

/* The module's constants, then its switch table. */
RODATA(0x80011110, 0x6c)

/*
 * The versus setup screen's loop: loads the menu
 * tables, fonts and pictures, then runs the seven-line setup page (team,
 * memory card and save entries per player) and its confirm and cancel
 * prompts; returns 1 to start the match, 0 to leave.
 */

int cd_read_member(int archive, int member, u_long *buf);
u32 *copy_words(u32 *dst, u32 *src, int count);
void clear_vram(void);
void gfx_setup_640x240(void);
void gfx_setup_320x240(void);
void lineup_screen_show(void);
void menu_creature_select_run(int player);
void menu_item_request_run(int player);
int card_screen_run(int mode, int port);
int card_check_saves_changed(void);
int item_request_any_available(int player);
void pad_clear_input(void);
void menu_vs_setup_draw(int page);

/* The CD state's reusable menu-file buffer. */
#define LOAD_BUFFER (vs_cd_state.load_buffer)

#define PAD(n) (vs_pad_state.pads[(n)])
#define PRESSED (PAD(0).pressed | PAD(1).pressed)
#define REPEAT (PAD(0).pressed_repeat | PAD(1).pressed_repeat)

/* Both card entries run the same page transition with a literal port. */
#define CARD_PAGE(port)                     \
    sp->mode = 2;                        \
    sp->font_mode = 0;                        \
    sp->show_backdrop = 0;                        \
    card_screen_run(0, port);                 \
    sp->font_mode = 1;                        \
    sp->mode = 0

/* Likewise the team page and the save page of each player. */
#define TEAM_PAGE(player)                       sp->font_mode = 1;                            menu_creature_select_run(player);                      sp->mode = 0

#define SAVE_PAGE(player)                                          if (saves[0].save_id == saves[1].save_id) {                sp->mode = 9;                                           } else if (item_request_any_available(player) == 0) {                           sp->mode = 99;                                          } else {                                                           sp->font_mode = 1;                                               menu_item_request_run(player);                                         sp->mode = 0;                                           }

ADDRESS(0x80016f10, 0x98c)
int menu_vs_setup_run(void)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u_long *buf;
    u32 b;

    clear_vram();
    DrawSync(0);
    gfx_setup_640x240();
    sp->message_font_vram.vx = 960;
    sp->message_font_vram.vy = 256;
    sp->message_font_clut.vx = 560;
    sp->message_font_clut.vy = 495;
    sp->item_font_vram.vx = 896;
    sp->item_font_vram.vy = 256;
    sp->item_font_clut.vx = 560;
    sp->item_font_clut.vy = 496;
    sp->picture_vram.vx = 768;
    sp->picture_vram.vy = 256;
    pos.vx = 640;
    pos.vy = 124;
    {
        u_long *message = LOAD_BUFFER;

        cd_read_member(8, 4, message);
        copy_words((u32 *)&vs_menu_messages[1], (u32 *)&message[1], 0x1500);
        message = (u_long *)((message[0] + sizeof(*message)) + (u8 *)message);
        tim_load_images_at(message + 1, &sp->item_font_vram, &sp->item_font_clut);
        DrawSync(0);
        message = LOAD_BUFFER;
        cd_read_member(8, 8, message);
        copy_words((u32 *)&vs_menu_messages[0], (u32 *)&message[1], 0x1500);
        message = (u_long *)((message[0] + sizeof(*message)) + (u8 *)message);
        tim_load_images_at(message + 1, &sp->message_font_vram, &sp->message_font_clut);
        DrawSync(0);
    }
    buf = LOAD_BUFFER;
    cd_read_member(8, 5, buf);
    tim_load_images_at(buf, &sp->picture_vram, NULL);
    DrawSync(0);
    cd_read_member(8, 7, buf);
    tim_load_images_at(buf, &tex, &clut);
    DrawSync(0);
    if (requested_items[0] != -1) {
        cd_read_member(11, vis_table_c.types[requested_items[0]].unk5[0], buf);
        tim_load_images_at(buf, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (requested_item_2p != -1) {
        cd_read_member(11, vis_table_c.types[requested_item_2p].unk5[0], buf);
        tim_load_images_at(buf, &pos, NULL);
        DrawSync(1);
    }
    sp->main_row = 0;
    sp->mode = 0;
    pad_clear_input();
    sp->font_mode = 1;
    for (;;) {
        if (sp->mode == 0) {
            if (PRESSED & PAD_BUTTON_CROSS) {
                switch (sp->main_row) {
                case 0:
                    if (requested_items[0] != -1) {
                        if (requested_item_2p != -1)
                            goto start;
                        sp->mode = 4;
                    } else {
                        if (requested_item_2p == -1)
                            goto start;
                        sp->mode = 3;
                    }
                    goto buzz;
                start:
                    play_confirm_sound();
                    sp->mode = 1;
                    break;
                case 1:
                    if (!(vs_pad_state.pads[0].pressed & PAD_BUTTON_CROSS))
                        goto buzz;
                    play_confirm_sound();
                    vs_pad_state.pads[0].pressed = 0;
                    CARD_PAGE(0);
                    break;
                case 2:
                    if (vs_pad_state.pads[1].pressed & PAD_BUTTON_CROSS) {
                        play_confirm_sound();
                        vs_pad_state.pads[1].pressed = 0;
                        CARD_PAGE(1);
                        break;
                    }
                    goto buzz;
                case 3:
                    if (!(vs_pad_state.pads[0].pressed & PAD_BUTTON_CROSS) || !save_loaded_1p)
                        goto buzz;
                    play_confirm_sound();
                    vs_pad_state.pads[0].pressed = 0;
                    TEAM_PAGE(0);
                    break;
                case 4:
                    if ((vs_pad_state.pads[1].pressed & PAD_BUTTON_CROSS) && save_loaded_2p) {
                        play_confirm_sound();
                        vs_pad_state.pads[1].pressed = 0;
                        TEAM_PAGE(1);
                        break;
                    }
                    goto buzz;
                case 5:
                    if (!(vs_pad_state.pads[0].pressed & PAD_BUTTON_CROSS) || !save_loaded_1p
                        || !save_loaded_2p)
                        goto buzz;
                    play_confirm_sound();
                    vs_pad_state.pads[0].pressed = 0;
                    SAVE_PAGE(0);
                    break;
                case 6:
                    if (!(vs_pad_state.pads[1].pressed & PAD_BUTTON_CROSS) || !save_loaded_1p
                        || !save_loaded_2p)
                        goto buzz;
                    play_confirm_sound();
                    vs_pad_state.pads[1].pressed = 0;
                    SAVE_PAGE(1);
                    break;
                buzz:
                    play_refuse_sound();
                    break;
                }
                vs_pad_state.pads[0].pressed = 0;
                vs_pad_state.pads[1].pressed = 0;
            }
            if (REPEAT & PAD_BUTTON_UP) {
                play_cursor_sound();
                vs_pad_state.pads[0].pressed_repeat = 0;
                vs_pad_state.pads[1].pressed_repeat = 0;
                sp->main_row = (sp->main_row + 6) % 7;
            }
            if (REPEAT & PAD_BUTTON_DOWN) {
                play_cursor_sound();
                vs_pad_state.pads[0].pressed_repeat = 0;
                vs_pad_state.pads[1].pressed_repeat = 0;
                sp->main_row = (sp->main_row + 1) % 7;
            }
            if (PRESSED & PAD_BUTTON_CIRCLE) {
                play_cursor_sound();
                sp->mode = 6;
            }
            if (PRESSED & PAD_BUTTON_START) {
                sp->main_row = 0;
                if (requested_items[0] != -1) {
                    if (requested_item_2p != -1)
                        goto ready;
                    sp->mode = 4;
                } else {
                    if (requested_item_2p == -1)
                        goto ready;
                    sp->mode = 3;
                }
                play_refuse_sound();
                goto ready_done;
            ready:
                play_confirm_sound();
                sp->mode = 1;
            ready_done:
                PAD(0).pressed = 0;
                PAD(1).pressed = 0;
                goto next;
            }
        } else if (sp->mode == 1) {
            if (PRESSED & PAD_BUTTON_CROSS) {
                if (round_count == 1) {
                    if (lineups[0].slot[0] == -1 || lineups[1].slot[0] == -1)
                        sp->mode = 2;
                } else if (lineups[0].slot[2] == -1 || lineups[1].slot[2] == -1) {
                    sp->mode = 2;
                }
                if (sp->mode == 1) {
                    if (requested_items[0] != -1 && requested_item_2p != -1) {
                        menu_vs_setup_draw(11);
                        menu_vs_setup_draw(11);
                        if (card_check_saves_changed()) {
                            sp->mode = 10;
                            play_refuse_sound();
                        }
                    }
                    if (sp->mode == 1) {
                        clear_vram();
                        DrawSync(0);
                        gfx_setup_320x240();
                        lineup_screen_show();
                        return 1;
                    }
                }
            }
            if (PRESSED & PAD_BUTTON_CIRCLE)
                sp->mode = 0;
            b = PRESSED;
            if (b & PAD_BUTTON_RIGHT)
                round_count = 3;
            if (b & PAD_BUTTON_LEFT)
                round_count = 1;
        } else if (sp->mode == 5 || sp->mode == 6) {
            if (PRESSED & PAD_BUTTON_CROSS) {
                if (sp->mode == 5) {
                    vs_pad_state.pads[0].pressed = 0;
                    vs_pad_state.pads[1].pressed = 0;
                    return 0;
                }
                sp->mode = 0;
            }
            if (PRESSED & PAD_BUTTON_CIRCLE)
                sp->mode = 0;
            if (PRESSED & PAD_BUTTON_RIGHT)
                sp->mode = 6;
            if (PRESSED & PAD_BUTTON_LEFT)
                sp->mode = 5;
        } else if (PRESSED & (PAD_BUTTON_CIRCLE | PAD_BUTTON_CROSS)) {
            sp->mode = 0;
        }
    next:
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[1].pressed = 0;
        vs_pad_state.pads[0].pressed_repeat = 0;
        vs_pad_state.pads[1].pressed_repeat = 0;
        menu_vs_setup_draw(sp->mode);
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
}

/*
 * The team screen's picture load: the title strips,
 * each team slot's picture and both players' fighter pictures go to VRAM
 * through the CD state's load buffer.
 */

/* The load's RECT initializer: 8 bytes, so .sdata under -G8. */
SDATA(0x8006323c, 0x8)

int cd_read_member(int archive, int member, u_long *buf);
void lineup_screen_draw(void);

ADDRESS(0x8001789c, 0x328)
void lineup_screen_show(void)
{
    RECT base = {0, 0, 640, 240};
    DVECTOR pos;
    RECT r;
    RECT r2;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 *id;
    int n;
    int x;
    int slot;
    int i;
    u_long *buf;

    sp->state.creature_select.pos[0].vx = 640;
    sp->state.creature_select.pos[0].vy = 144;
    sp->state.creature_select.pos[1].vx = 640;
    sp->state.creature_select.pos[1].vy = 192;
    sp->state.creature_select.pos[2].vx = 736;
    sp->state.creature_select.pos[2].vy = 144;
    sp->state.creature_select.pos[3].vx = 736;
    sp->state.creature_select.pos[3].vy = 192;
    sp->state.creature_select.pos[4].vx = 832;
    sp->state.creature_select.pos[4].vy = 144;
    sp->state.creature_select.pos[5].vx = 832;
    sp->state.creature_select.pos[5].vy = 192;
    sp->state.creature_select.pos[6].vx = 928;
    sp->state.creature_select.pos[6].vy = 144;
    sp->state.creature_select.pos[7].vx = 928;
    sp->state.creature_select.pos[7].vy = 172;
    sp->state.creature_select.pos[8].vx = 928;
    sp->state.creature_select.pos[8].vy = 200;
    sp->state.creature_select.pos[15].vx = 640;
    sp->state.creature_select.pos[15].vy = 0;
    vs_cd_state.load_cursor = vs_cd_state.load_buffer;
    buf = vs_cd_state.load_cursor;

    for (n = 0, x = 320; n < 5; n++, x += 64) {
        cd_read_member(5, n + 31, buf);
        pos.vx = base.x + x;
        pos.vy = base.y;
        tim_load_images_at(buf, &pos, NULL);
        DrawSync(0);
    }
    id = lineups[0].slot;
    for (i = 0; i < 6; i++) {
        slot = *id++;
        if (slot == -1) {
            r2.x = sp->state.creature_select.pos[i].vx;
            r2.y = sp->state.creature_select.pos[i].vy;
            r2.w = 96;
            r2.h = 48;
            ClearImage(&r2, 0, 0, 0);
        } else {
            int size = cd_read_member(10, slot, vs_cd_state.load_cursor) * 2048;

            tim_load_images_at(vs_cd_state.load_cursor, &sp->state.creature_select.pos[i], NULL);
            vs_cd_state.load_cursor = (u_long *)((u8 *)vs_cd_state.load_cursor + size);
        }
    }
    for (i = 0; i < 2; i++) {
        if (requested_items[i] == -1) {
            r.x = sp->state.creature_select.pos[6 + i].vx;
            r.y = sp->state.creature_select.pos[6 + i].vy;
            r.w = 56;
            r.h = 28;
            ClearImage(&r, 0, 0, 0);
        } else {
            int size = cd_read_member(11, vis_table_c.types[requested_items[i]].unk5[0],
                                     vs_cd_state.load_cursor) * 2048;

            tim_load_images_at(vs_cd_state.load_cursor, &sp->state.creature_select.pos[6 + i], NULL);
            vs_cd_state.load_cursor = (u_long *)((u8 *)vs_cd_state.load_cursor + size);
        }
    }
    r.x = sp->state.creature_select.pos[8].vx;
    r.y = sp->state.creature_select.pos[8].vy;
    r.w = 56;
    r.h = 28;
    ClearImage(&r, 10, 10, 10);
    DrawSync(0);
    lineup_screen_draw();
    lineup_screen_draw();
}

/*
 * Clears a VRAM block.
 */
/* Clears the 96 x 144 VRAM block at (832, 0) to black in three strips. */
ADDRESS(0x80017bc4, 0xc0)
void menu_creature_select_clear_parked(void)
{
    RECT r;

    setRECT(&r, 832, 0, 96, 48);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    setRECT(&r, 832, 48, 96, 48);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    setRECT(&r, 832, 96, 96, 48);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
}

/*
 * The team screen's three slot pictures: each slot's
 * 96x48 picture lives at its scratchpad VRAM position and is parked in
 * the column at x 832 while the team is edited.
 */
/* Parks the three slot pictures at x 832 and takes a copy of `player`'s
 * team into the edit record. */
ADDRESS(0x80017c84, 0x120)
void menu_creature_select_pick_up_slot(VsMenuScratch *sp, int player, VsLineupEdit *edit)
{
    RECT r;

    r.x = sp->state.creature_select.pos[12].vx;
    r.y = sp->state.creature_select.pos[12].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, 832, 0);
    DrawSync(0);
    r.x = sp->state.creature_select.pos[13].vx;
    r.y = sp->state.creature_select.pos[13].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, 832, 48);
    DrawSync(0);
    r.x = sp->state.creature_select.pos[14].vx;
    r.y = sp->state.creature_select.pos[14].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, 832, 96);
    DrawSync(0);
    edit->lineup.slot[0] = lineups[player].slot[0];
    edit->lineup.slot[1] = lineups[player].slot[1];
    edit->lineup.slot[2] = lineups[player].slot[2];
}

/* Stores the edit record as `player`'s team with the picked slot and the
 * cursor's slot swapped, and puts the pictures back swapped too. */
ADDRESS(0x80017da4, 0x244)
void menu_creature_select_swap_slots(VsMenuScratch *sp, int player, VsLineupEdit *edit)
{
    RECT r;
    int picked = edit->held_slot;
    int cursor = ~sp->cursor;
    int dest_x, dest_y;

    lineups[player].slot[0] = edit->lineup.slot[0];
    lineups[player].slot[1] = edit->lineup.slot[1];
    lineups[player].slot[2] = edit->lineup.slot[2];
    lineups[player].slot[picked] = edit->lineup.slot[cursor];
    lineups[player].slot[cursor] = edit->lineup.slot[picked];
    r.x = 832;
    r.y = 0;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->state.creature_select.pos[12].vx, sp->state.creature_select.pos[12].vy);
    DrawSync(0);
    r.x = 832;
    r.y = 48;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->state.creature_select.pos[13].vx, sp->state.creature_select.pos[13].vy);
    DrawSync(0);
    r.x = 832;
    r.y = 96;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->state.creature_select.pos[14].vx, sp->state.creature_select.pos[14].vy);
    DrawSync(0);
    r.x = sp->state.creature_select.pos[-sp->cursor + 11].vx;
    r.y = sp->state.creature_select.pos[-sp->cursor + 11].vy;
    dest_x = sp->state.creature_select.pos[edit->held_slot + 12].vx;
    dest_y = sp->state.creature_select.pos[edit->held_slot + 12].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, dest_x, dest_y);
    DrawSync(0);
    switch (edit->held_slot) {
    case 0:
        r.x = 832;
        r.y = 0;
        break;
    case 1:
        r.x = 832;
        r.y = 48;
        break;
    case 2:
        r.x = 832;
        r.y = 96;
        break;
    }
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->state.creature_select.pos[-sp->cursor + 11].vx, sp->state.creature_select.pos[-sp->cursor + 11].vy);
    DrawSync(0);
}

/* Stores the edit record as `player`'s team without the picked slot: the
 * later slots and their pictures move up and the last picture is
 * cleared. */
ADDRESS(0x80017fe8, 0x258)
void menu_creature_select_remove_slot(VsMenuScratch *sp, int player, VsLineupEdit *edit)
{
    RECT r;

    lineups[player].slot[0] = edit->lineup.slot[0];
    lineups[player].slot[1] = edit->lineup.slot[1];
    lineups[player].slot[2] = edit->lineup.slot[2];
    lineups[player].slot[edit->held_slot] = -1;
    if (lineups[player].slot[0] == -1) {
        r.x = 832;
        r.y = 48;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->state.creature_select.pos[12].vx, sp->state.creature_select.pos[12].vy);
        DrawSync(0);
        r.x = 832;
        r.y = 96;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->state.creature_select.pos[13].vx, sp->state.creature_select.pos[13].vy);
        DrawSync(0);
        lineups[player].slot[0] = lineups[player].slot[1];
        lineups[player].slot[1] = lineups[player].slot[2];
    } else if (lineups[player].slot[1] == -1) {
        r.x = 832;
        r.y = 0;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->state.creature_select.pos[12].vx, sp->state.creature_select.pos[12].vy);
        DrawSync(0);
        r.x = 832;
        r.y = 96;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->state.creature_select.pos[13].vx, sp->state.creature_select.pos[13].vy);
        DrawSync(0);
        lineups[player].slot[0] = lineups[player].slot[0];
        lineups[player].slot[1] = lineups[player].slot[2];
    } else if (lineups[player].slot[2] == -1) {
        r.x = 832;
        r.y = 0;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->state.creature_select.pos[12].vx, sp->state.creature_select.pos[12].vy);
        DrawSync(0);
        r.x = 832;
        r.y = 48;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->state.creature_select.pos[13].vx, sp->state.creature_select.pos[13].vy);
        DrawSync(0);
        lineups[player].slot[0] = lineups[player].slot[0];
        lineups[player].slot[1] = lineups[player].slot[1];
    }
    r.x = sp->state.creature_select.pos[14].vx;
    r.y = sp->state.creature_select.pos[14].vy;
    r.w = 96;
    r.h = 48;
    ClearImage2(&r, 0, 0, 0);
    DrawSync(0);
    lineups[player].slot[2] = -1;
}

/*
 * A single TIM upload, as one pass of tim_load_images_at.
 */
/* Uploads one TIM image (and its CLUT) to the given positions. */
ADDRESS(0x80018240, 0xac)
void tim_load_image_at(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos)
{
    u_long flags;
    u_long *cursor;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    cursor = tim;
    if (cursor[0] != 0x10)
        return;
    cursor += 2;
    flags = tim[1];
    if (flags & 8) {
        clut_data = tim + 5;
        clut = cursor;
        cursor = (u_long *)((u_char *)cursor + tim[2]);
    }
    image = cursor;
    image_data = image + 3;
    cursor = (u_long *)((u_char *)cursor + image[0]);
    if (flags & 8) {
        ((RECT *)(clut + 1))->x = clut_pos->vx;
        ((RECT *)(clut + 1))->y = clut_pos->vy;
        LoadImage((RECT *)(clut + 1), clut_data);
    }
    ((RECT *)(image + 1))->x = pos->vx;
    ((RECT *)(image + 1))->y = pos->vy;
    LoadImage((RECT *)(image + 1), image_data);
}

/*
 * The team screen's input loop: moves the cursor over
 * the team slots and the item grid, picks items into the team, swaps
 * slots, scrolls the grid (reloading the pictures that scroll in) and
 * opens the detail page, until the cancel button leaves.
 */
int cd_read_member(int archive, int member, u_long *buf);
void menu_creature_select_clear_parked(void);
void menu_creature_select_pick_up_slot(VsMenuScratch *sp, int player, VsLineupEdit *edit);
void menu_creature_select_swap_slots(VsMenuScratch *sp, int player, VsLineupEdit *edit);
void menu_creature_select_remove_slot(VsMenuScratch *sp, int player, VsLineupEdit *edit);
void tim_load_image_at(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos);
void menu_creature_select_draw(int page);

/* The detail page toggle: the attribute figures when nonzero (bss, unowned). */

#define HELD (vs_pad_state.pads[0].held | vs_pad_state.pads[1].held)

/* The team being edited: the three ids of `player`. */
#define TEAM (lineups[player].slot)

ADDRESS(0x800182ec, 0x998)
void menu_creature_select_loop(VsMenuScratch *sp, int player)
{
    VsLineupEdit edit;
    int shown = -1;
    u_long *buf;
    int top;
    int col;
    int row;
    int pick;
    int idx;
    int slot;
    int id;
    int n;
    int size;
    int i;
    int j;
    int k;

    edit.holding = 0;
    for (;;) {
        if (player) {
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[0].pressed_repeat = 0;
            vs_pad_state.pads[0].held = 0;
        } else {
            vs_pad_state.pads[1].pressed = 0;
            vs_pad_state.pads[1].pressed_repeat = 0;
            vs_pad_state.pads[1].held = 0;
        }
        top = sp->scroll;
        if (sp->mode == 1) {
            int cursor = sp->cursor;

            if (cursor < 0) {
                col = -1;
                row = ~cursor;
            } else {
                col = (s16)(cursor - (cursor >> 2) * 4);
                row = cursor >> 2;
            }
            if (PRESSED & PAD_BUTTON_CIRCLE) {
                if (!edit.holding) {
                    play_cancel_sound();
                    return;
                }
                play_cancel_sound();
                menu_creature_select_remove_slot(sp, player, &edit);
                edit.holding = 0;
                sp->holding = 0;
                menu_creature_select_clear_parked();
            }
            if (!edit.holding) {
                if (REPEAT & PAD_BUTTON_RIGHT) {
                    play_cursor_sound();
                    if (col < 0 && !edit.holding)
                        sp->cursor = row << 2;
                    else if (col < 3)
                        sp->cursor++;
                }
                if (!edit.holding && (REPEAT & PAD_BUTTON_LEFT)) {
                    play_cursor_sound();
                    if (col > 0)
                        sp->cursor--;
                    else if (col == 0)
                        sp->cursor = ~row;
                }
            }
            if (REPEAT & PAD_BUTTON_UP) {
                play_cursor_sound();
                if (edit.holding) {
                    if (row > 0) {
                        sp->cursor++;
                        menu_creature_select_swap_slots(sp, player, &edit);
                    }
                } else if (HELD & PAD_BUTTON_SQUARE) {
                    sp->scroll -= 3;
                    if (sp->scroll < 0)
                        sp->scroll = 0;
                } else if (sp->cursor < 0) {
                    if (row > 0)
                        sp->cursor++;
                } else if (row > 0) {
                    sp->cursor -= 4;
                } else if (sp->scroll > 0) {
                    sp->scroll--;
                }
            } else if (REPEAT & PAD_BUTTON_DOWN) {
                play_cursor_sound();
                if (edit.holding) {
                    if (row < 2 && TEAM[row + 1] != -1) {
                        sp->cursor--;
                        menu_creature_select_swap_slots(sp, player, &edit);
                    }
                } else if (HELD & PAD_BUTTON_SQUARE) {
                    if (sp->state.creature_select.rows > 2) {
                        sp->scroll += 3;
                        if (sp->scroll > sp->state.creature_select.rows - 2)
                            sp->scroll = sp->state.creature_select.rows - 2;
                    }
                } else if (sp->cursor < 0) {
                    if (row < 2)
                        sp->cursor--;
                } else if (sp->scroll + row < sp->state.creature_select.rows) {
                    if (row < 2)
                        sp->cursor += 4;
                    else if (sp->scroll <= sp->state.creature_select.rows - 3)
                        sp->scroll++;
                }
            } else if (REPEAT & PAD_BUTTON_R1) {
                play_cursor_sound();
                sp->scroll -= 3;
                if (sp->scroll < 0)
                    sp->scroll = 0;
            } else if (REPEAT & PAD_BUTTON_R2) {
                play_cursor_sound();
                if (sp->state.creature_select.rows > 2) {
                    sp->scroll += 3;
                    if (sp->scroll > sp->state.creature_select.rows - 2)
                        sp->scroll = sp->state.creature_select.rows - 2;
                }
            } else if (PRESSED & PAD_BUTTON_CROSS) {
                if (edit.holding) {
                    play_confirm_sound();
                    edit.holding = 0;
                    sp->holding = 0;
                    menu_creature_select_clear_parked();
                } else {
                    pick = sp->scroll * 4 + sp->cursor;
                    edit.holding = 1;
                    sp->holding = 1;
                    if (sp->cursor < 0) {
                        edit.held_slot = ~sp->cursor;
                        if (TEAM[edit.held_slot] != -1) {
                            play_confirm_sound();
                            menu_creature_select_pick_up_slot(sp, player, &edit);
                            goto scrolled;
                        }
                    } else if (TEAM[2] == -1) {
                        if (sp->state.creature_select.kinds[pick] == 0xff) {
                            play_refuse_sound();
                            edit.holding = 0;
                            sp->holding = 0;
                            goto scrolled;
                        }
                        for (i = 0; i < 3; i++) {
                            if (TEAM[i] == sp->state.creature_select.kinds[pick])
                                break;
                        }
                        if (i == 3) {
                            for (i = 0; i < 3; i++) {
                                if (TEAM[i] == -1) {
                                    RECT r;

                                    TEAM[i] = sp->state.creature_select.kinds[pick];
                                    r.x = sp->state.creature_select.pos[pick % 12].vx;
                                    r.y = sp->state.creature_select.pos[pick % 12].vy;
                                    r.w = 96;
                                    r.h = 48;
                                    MoveImage2(&r, sp->state.creature_select.pos[12 + i].vx,
                                               sp->state.creature_select.pos[12 + i].vy);
                                    DrawSync(0);
                                    break;
                                }
                            }
                            if (i != 3) {
                                play_confirm_sound();
                                sp->cursor = ~i;
                                edit.held_slot = i;
                                menu_creature_select_pick_up_slot(sp, player, &edit);
                                goto scrolled;
                            }
                        }
                    }
                    play_refuse_sound();
                    edit.holding = 0;
                    sp->holding = 0;
                }
            } else if ((PRESSED & PAD_BUTTON_TRIANGLE) && !edit.holding) {
                DAT_8006335c = 0;
                play_confirm_sound();
                if (sp->cursor < 0) {
                    id = TEAM[-sp->cursor - 1];
                    if (id != -1) {
                        /* Retail only updates idx when a kind matches. */
                        for (k = 0; k < sp->state.creature_select.count; k++) {
                            if (id == sp->state.creature_select.kinds[k]) {
                                idx = k;
                                break;
                            }
                        }
                    } else {
                        idx = -1;
                    }
                } else {
                    idx = sp->scroll * 4 + sp->cursor;
                    id = sp->state.creature_select.kinds[idx];
                    for (k = 0; k < 3; k++) {
                        if (id == TEAM[k]) {
                            idx = -1;
                            break;
                        }
                    }
                }
                if (idx < sp->state.creature_select.count && idx != -1) {
                    if (idx != shown) {
                        buf = vs_cd_state.load_buffer;
                        cd_read_member(10, sp->state.creature_select.kinds[idx] + 200, buf);
                        shown = idx;
                        tim_load_images_at(&buf[1], &sp->state.creature_select.pos[15], NULL);
                        {
                            DVECTOR tex = {960, 446};
                            DVECTOR clut = {560, 497};

                            buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
                            tim_load_image_at(buf + 1, &tex, &clut);
                        }
                    }
                    sp->mode = 2;
                }
            }
        scrolled:
            if (sp->scroll != top) {
                if (top < sp->scroll) {
                    n = (sp->scroll - top) * 4;
                    slot = (top + 3) * 4;
                } else {
                    n = (top - sp->scroll) * 4;
                    slot = sp->scroll * 4;
                }
                vs_cd_state.load_cursor = vs_cd_state.load_buffer;
                for (j = 0; j < n; j++, slot++) {
                    if (slot < sp->state.creature_select.count) {
                        size = cd_read_member(10, sp->state.creature_select.kinds[slot],
                                             vs_cd_state.load_cursor) * 2048;
                        tim_load_images_at(vs_cd_state.load_cursor, &sp->state.creature_select.pos[slot % 12], NULL);
                        vs_cd_state.load_cursor = (u_long *)((u8 *)vs_cd_state.load_cursor + size);
                    }
                }
                DrawSync(0);
            }
        } else if (PRESSED & PAD_BUTTON_CIRCLE) {
            play_cancel_sound();
            sp->mode = 1;
        } else if (PRESSED & PAD_BUTTON_R1) {
            DAT_8006335c = 1;
        } else if (PRESSED & PAD_BUTTON_L1) {
            DAT_8006335c = 0;
        }
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[1].pressed = 0;
        vs_pad_state.pads[0].pressed_repeat = 0;
        vs_pad_state.pads[1].pressed_repeat = 0;
        menu_creature_select_draw(player);
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
}

/*
 * The team screen: lays out the picture positions,
 * lists the fighters `player` owns, loads the message tables, fonts and
 * pictures, runs the screen (menu_creature_select_loop) and then reloads the menu
 * tables and both players' fighter pictures.
 */

int cd_read_member(int archive, int member, u_long *buf);
u32 *copy_words(u32 *dst, u32 *src, int count);
void clear_vram(void);
void menu_creature_select_loop(VsMenuScratch *sp, int player);

ADDRESS(0x80018c84, 0x764)
void menu_creature_select_run(int player)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    u8 unused[64];
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u_long *buf;
    u_long *tim;
    u32 *owned;
    int size;
    int n;
    int i;
    u32 mask;
    u32 word;

    sp->holding = 0;
    sp->state.creature_select.pos[0].vx = 640;
    sp->state.creature_select.pos[0].vy = 144;
    sp->state.creature_select.pos[1].vx = 640;
    sp->state.creature_select.pos[1].vy = 192;
    sp->state.creature_select.pos[2].vx = 736;
    sp->state.creature_select.pos[2].vy = 144;
    sp->state.creature_select.pos[3].vx = 736;
    sp->state.creature_select.pos[3].vy = 192;
    sp->state.creature_select.pos[4].vx = 832;
    sp->state.creature_select.pos[4].vy = 144;
    sp->state.creature_select.pos[5].vx = 928;
    sp->state.creature_select.pos[5].vy = 0;
    sp->state.creature_select.pos[6].vx = 928;
    sp->state.creature_select.pos[6].vy = 48;
    sp->state.creature_select.pos[7].vx = 928;
    sp->state.creature_select.pos[7].vy = 96;
    sp->state.creature_select.pos[8].vx = 928;
    sp->state.creature_select.pos[8].vy = 144;
    sp->state.creature_select.pos[9].vx = 928;
    sp->state.creature_select.pos[9].vy = 192;
    sp->state.creature_select.pos[10].vx = 640;
    sp->state.creature_select.pos[10].vy = 256;
    sp->state.creature_select.pos[11].vx = 640;
    sp->state.creature_select.pos[11].vy = 304;
    sp->state.creature_select.pos[12].vx = 640;
    sp->state.creature_select.pos[12].vy = 352;
    sp->state.creature_select.pos[13].vx = 640;
    sp->state.creature_select.pos[13].vy = 400;
    sp->state.creature_select.pos[14].vx = 640;
    sp->state.creature_select.pos[14].vy = 448;
    sp->state.creature_select.pos[15].vx = 640;
    sp->state.creature_select.pos[15].vy = 0;
    sp->state.creature_select.kinds = mem_alloc_main_ram(151);
    sp->state.creature_select.count = 0;
    for (i = 0; i < 151; i++)
        sp->state.creature_select.kinds[i] = 0xff;
    owned = saves[player].versus.creature_killed_bits;
    for (i = 0; i < 151; i++) {
        mask = 1;
        word = owned[i / 32];
        mask <<= i % 32;
        if ((word & mask) && i != 3 &&
            i != 5 && i != 6 && i != 7 && i != 9 && i != 10 && i != 11 && i != 19 &&
            i != 25 && i != 26 && i != 33 && i != 38 && i != 42 && i != 54 && i != 55 &&
            i != 67 && i != 70 && i != 73 && i != 83 && i != 84 && i != 85 && i != 86 &&
            i != 89 && i != 90 && i != 91 && i != 92 && i != 106 && i != 108 && i != 109 &&
            i != 116 && i != 118 && i != 131 && i != 132 && i != 133 && i != 135 &&
            i != 140 && i != 142 && i != 150)
            sp->state.creature_select.kinds[sp->state.creature_select.count++] = i;
    }
    sp->state.creature_select.rows = sp->state.creature_select.count / 4;
    sp->state.creature_select.last = sp->state.creature_select.count % 4;
    if (sp->state.creature_select.last == 0) {
        sp->state.creature_select.last = 4;
        sp->state.creature_select.rows--;
    }
    sp->state.creature_select.entries = mem_alloc_main_ram(0x2000);
    cd_read_member(10, 400, (u_long *)sp->state.creature_select.entries);
    buf = vs_cd_state.load_buffer;
    cd_read_member(8, 2, buf);
    copy_words((u32 *)&vs_menu_messages[1], (u32 *)&buf[1], 0x1500);
    buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
    tim_load_images_at(buf + 1, &sp->item_font_vram, &sp->item_font_clut);
    DrawSync(0);
    buf = vs_cd_state.load_buffer;
    cd_read_member(8, 1, buf);
    copy_words((u32 *)&vs_menu_messages[0], (u32 *)&buf[1], 0x1500);
    buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
    tim_load_images_at(buf + 1, &sp->message_font_vram, &sp->message_font_clut);
    DrawSync(0);
    cd_read_member(8, 7, vs_cd_state.load_buffer);
    tim_load_images_at(vs_cd_state.load_buffer, &tex, &clut);
    DrawSync(0);
    vs_cd_state.load_cursor = vs_cd_state.load_buffer;
    n = 12;
    if (sp->state.creature_select.count < 13)
        n = sp->state.creature_select.count;
    for (i = 0; i < n; i++) {
        size = cd_read_member(10, sp->state.creature_select.kinds[i], vs_cd_state.load_cursor) * 2048;
        tim_load_images_at(vs_cd_state.load_cursor, &sp->state.creature_select.pos[i], NULL);
        vs_cd_state.load_cursor = (u_long *)((u8 *)vs_cd_state.load_cursor + size);
    }
    if (DrawSync(1) > 8)
        DrawSync(0);
    vs_cd_state.load_cursor = vs_cd_state.load_buffer;
    for (i = 0; i < 3; i++) {
        if (lineups[player].slot[i] != -1) {
            size = cd_read_member(10, lineups[player].slot[i], vs_cd_state.load_cursor) * 2048;
            tim_load_images_at(vs_cd_state.load_cursor, &sp->state.creature_select.pos[12 + i], NULL);
            vs_cd_state.load_cursor = (u_long *)((u8 *)vs_cd_state.load_cursor + size);
        }
    }
    clear_vram();
    DrawSync(0);
    sp->mode = 1;
    sp->cursor = 0;
    sp->scroll = 0;
    menu_creature_select_loop(sp, player);
    free(sp->state.creature_select.kinds);
    free(sp->state.creature_select.entries);
    pos.vx = 640;
    pos.vy = 124;
    vs_cd_state.load_cursor = buf = vs_cd_state.load_buffer;
    cd_read_member(8, 4, buf);
    copy_words((u32 *)&vs_menu_messages[1], (u32 *)&buf[1], 0x1500);
    tim = (u_long *)((u8 *)buf + vs_cd_state.load_buffer[0]);
    tim_load_images_at(tim + 2, &sp->item_font_vram, &sp->item_font_clut);
    DrawSync(0);
    cd_read_member(8, 8, buf);
    copy_words((u32 *)&vs_menu_messages[0], (u32 *)&buf[1], 0x1500);
    tim = (u_long *)((u8 *)buf + vs_cd_state.load_buffer[0]);
    tim_load_images_at(tim + 2, &sp->message_font_vram, &sp->message_font_clut);
    DrawSync(0);
    cd_read_member(8, 7, buf);
    tim_load_images_at(buf, &tex, &clut);
    DrawSync(0);
    if (requested_items[0] != -1) {
        cd_read_member(11, vis_table_c.types[requested_items[0]].unk5[0], buf);
        tim_load_images_at(buf, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (requested_item_2p != -1) {
        cd_read_member(11, vis_table_c.types[requested_item_2p].unk5[0], buf);
        tim_load_images_at(buf, &pos, NULL);
        DrawSync(1);
    }
    sp->holding = 0;
}

/*
 * An item availability test on the two save records.
 */
/*
 * Whether `player`'s save lacks an item the other save has: an item id
 * (0..265) the other record marks and this one does not, that is not
 * flagged here without its second mark, and that is none of the other
 * record's 21 listed ids.
 */
ADDRESS(0x800193e8, 0x224)
int item_request_any_available(int player)
{
    int found = 0;
    VsSaveVersus *mine = &saves[player].versus;
    VsSaveVersus *other = &saves[~player & 1].versus;
    int i;

    for (i = 0; i < 266; i++) {
        u32 mask;
        u32 word;

        if (mine->inventory_counts[i] != 0 || other->inventory_counts[i] == 0)
            continue;
        mask = 1;
        word = saves[player].collected_bits[i / 32];
        mask <<= i % 32;
        if (word & mask) {
            if (mine->items_lost[i] == 0)
                continue;
        }
        if (other->equipment[EQUIP_SLOT_RIGHT_RING] == i || other->equipment[EQUIP_SLOT_RIGHT_BRACELET] == i
            || other->equipment[EQUIP_SLOT_GLOVES] == i
            || other->equipment[EQUIP_SLOT_RIGHT_HAND] == i
            || other->equipment[EQUIP_SLOT_RIGHT_RESERVE] == i || other->equipment[EQUIP_SLOT_QUICK_1] == i
            || other->equipment[EQUIP_SLOT_RIGHT_MAGIC] == i || other->equipment[EQUIP_SLOT_AMULET] == i
            || other->equipment[EQUIP_SLOT_HELMET] == i
            || other->equipment[EQUIP_SLOT_ARMOR] == i || other->equipment[EQUIP_SLOT_BOOTS] == i
            || other->equipment[EQUIP_SLOT_QUICK_3] == i
            || other->equipment[EQUIP_SLOT_LEFT_MAGIC] == i || other->equipment[EQUIP_SLOT_LEFT_RING] == i
            || other->equipment[EQUIP_SLOT_LEFT_BRACELET] == i
            || other->equipment[EQUIP_SLOT_GLOVES_MIRROR] == i
            || other->equipment[EQUIP_SLOT_LEFT_HAND] == i || other->equipment[EQUIP_SLOT_LEFT_RESERVE] == i
            || other->equipment[EQUIP_SLOT_QUICK_5] == i || other->equipment[EQUIP_SLOT_QUICK_2] == i
            || other->equipment[EQUIP_SLOT_QUICK_4] == i)
            continue;
        found = 1;
    }
    return found;
}

/*
 * The item request list (ITEM SELECT): lists the item ids `player` may
 * request from the opponent's save, lets the player scroll it (showing the
 * icon of the item under the cursor) and stores the requested item in
 * requested_items[player].
 */

int cd_read_member(int archive, int member, u_long *buf);
void clear_vram(void);
void tim_load_member_async(int archive, int member, int x, int y);
void menu_item_request_draw(int page);
void exit(int code);

#define TOP (sp->state.menu.half[0])
#define COUNT (sp->state.menu.half[1])
#define IDS (sp->state.menu.list.ids)

ADDRESS(0x8001960c, 0xb18)
void menu_item_request_run(int player)
{
    DVECTOR unused0;
    DVECTOR unused1 = {640, 0};
    char path1[28] = "q:\\st\\itemcg\\icon1\\0000.tim";
    char path5[28] = "q:\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    VsSaveVersus *mine;
    VsSaveVersus *other;
    u_long *buf;
    s16 shown;
    s16 id;
    int i;
    int j;

    buf = mem_alloc_main_ram(0x1000);
    cd_read_member(5, 6, buf);
    tim_load_images_shifted(buf, 0, 0);
    DrawSync(0);
    free(buf);
    COUNT = 0;
    IDS = mem_alloc_main_ram(0x276);
    if (IDS == NULL)
        exit(1);
    memset((u8 *)IDS, 0xff, 0x276);
    mine = &saves[player].versus;
    other = &saves[~player & 1].versus;
    COUNT = 1;
    IDS[0] = -2;
    for (i = 0; i < 266; i++) {
        u32 mask;
        u32 word;

        if (mine->inventory_counts[i] != 0 || other->inventory_counts[i] == 0)
            continue;
        mask = 1;
        word = saves[player].collected_bits[i / 32];
        mask <<= i % 32;
        if ((word & mask) && mine->items_lost[i] == 0)
            continue;
        if (other->equipment[EQUIP_SLOT_RIGHT_RING] == i || other->equipment[EQUIP_SLOT_RIGHT_BRACELET] == i
            || other->equipment[EQUIP_SLOT_GLOVES] == i
            || other->equipment[EQUIP_SLOT_RIGHT_HAND] == i
            || other->equipment[EQUIP_SLOT_RIGHT_RESERVE] == i || other->equipment[EQUIP_SLOT_QUICK_1] == i
            || other->equipment[EQUIP_SLOT_RIGHT_MAGIC] == i || other->equipment[EQUIP_SLOT_AMULET] == i
            || other->equipment[EQUIP_SLOT_HELMET] == i
            || other->equipment[EQUIP_SLOT_ARMOR] == i || other->equipment[EQUIP_SLOT_BOOTS] == i
            || other->equipment[EQUIP_SLOT_QUICK_3] == i
            || other->equipment[EQUIP_SLOT_LEFT_MAGIC] == i || other->equipment[EQUIP_SLOT_LEFT_RING] == i
            || other->equipment[EQUIP_SLOT_LEFT_BRACELET] == i
            || other->equipment[EQUIP_SLOT_GLOVES_MIRROR] == i
            || other->equipment[EQUIP_SLOT_LEFT_HAND] == i || other->equipment[EQUIP_SLOT_LEFT_RESERVE] == i
            || other->equipment[EQUIP_SLOT_QUICK_5] == i || other->equipment[EQUIP_SLOT_QUICK_2] == i
            || other->equipment[EQUIP_SLOT_QUICK_4] == i)
            continue;
        IDS[COUNT++] = i;
    }
    sp->mode = 2;
    sp->cursor = 0;
    TOP = 0;
    if (requested_items[player] != -1) {
        /* The count is never stepped: the search ends only by its match. */
        for (j = 0; COUNT != 0; j++) {
            if (requested_items[player] == IDS[j])
                break;
        }
        if (COUNT < 12 || j < 6) {
            sp->cursor = j;
        } else if (j > COUNT - 5) {
            TOP = COUNT - 11;
            sp->cursor = j - TOP;
        } else {
            TOP = j - 5;
            sp->cursor = 5;
        }
    }
    shown = -1;
    clear_vram();
    DrawSync(0);
    for (;;) {
        if (player) {
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[0].pressed_repeat = 0;
            vs_pad_state.pads[0].held = 0;
        } else {
            vs_pad_state.pads[1].pressed = 0;
            vs_pad_state.pads[1].pressed_repeat = 0;
            vs_pad_state.pads[1].held = 0;
        }
        if (PRESSED & PAD_BUTTON_CIRCLE) {
            play_cancel_sound();
            if (sp->mode & 0x8000) {
                sp->mode &= 0x7fff;
            } else {
                while ((s16)cd_queue_busy() != 0)
                    ;
                break;
            }
        }
        if (PRESSED & PAD_BUTTON_CROSS) {
            play_confirm_sound();
            if (sp->mode & 0x8000) {
                s16 chosen = IDS[TOP + sp->cursor];

                if (chosen == -2)
                    chosen = -1;
                requested_items[player] = chosen;
                while ((s16)cd_queue_busy() != 0)
                    ;
                break;
            }
            if (IDS[(s16)(TOP + sp->cursor)] != -1)
                sp->mode |= 0x8000;
        }
        if (!(sp->mode & 0x8000)) {
            if (REPEAT & PAD_BUTTON_UP) {
                play_cursor_sound();
                if (vs_pad_state.pads[0].held & PAD_BUTTON_SQUARE) {
                    if (COUNT < 11) {
                        sp->cursor = 0;
                    } else {
                        TOP -= 11;
                        if (TOP < 0) {
                            if (TOP + 10 < 0)
                                sp->cursor = 0;
                            TOP = 0;
                        }
                    }
                } else if (sp->cursor > 0) {
                    sp->cursor--;
                } else if (TOP > 0) {
                    TOP--;
                } else {
                    TOP = 0;
                    sp->cursor = 0;
                }
                vs_pad_state.pads[0].pressed_repeat = 0;
            }
            if (REPEAT & PAD_BUTTON_DOWN) {
                play_cursor_sound();
                if ((vs_pad_state.pads[0].held | vs_pad_state.pads[1].held) & PAD_BUTTON_SQUARE) {
                    if (COUNT >= 11) {
                        TOP += 11;
                        if (TOP > COUNT - 11) {
                            if (TOP >= COUNT)
                                sp->cursor = 10;
                            TOP = COUNT - 11;
                        }
                    } else {
                        sp->cursor = COUNT - 1;
                    }
                } else if (COUNT - 1 > sp->cursor + TOP) {
                    if (sp->cursor < 10) {
                        sp->cursor++;
                    } else if (TOP < COUNT - 11) {
                        TOP++;
                    } else {
                        TOP = COUNT - 11;
                        sp->cursor = COUNT - TOP - 1;
                    }
                }
            }
            if (REPEAT & PAD_BUTTON_R1) {
                play_cursor_sound();
                if (COUNT >= 11) {
                    TOP -= 11;
                    if (TOP < 0) {
                        if (TOP + 10 < 0)
                            sp->cursor = 0;
                        TOP = 0;
                    }
                } else {
                    sp->cursor = 0;
                    if (REPEAT & PAD_BUTTON_R2)
                        play_cursor_sound();
                }
                if (COUNT >= 11) {
                    TOP += 11;
                    if (TOP > COUNT - 11) {
                        if (TOP >= COUNT)
                            sp->cursor = 10;
                        TOP = COUNT - 11;
                    }
                } else {
                    sp->cursor = COUNT - 1;
                }
            }
            if (sp->mode == 2 && (s16)cd_queue_busy() == 0) {
                id = IDS[TOP + sp->cursor];
                if (shown != id && id != -1 && id != -2) {
                    tim_load_member_async(11, vis_table_c.types[id].unk5[0] + 200, 640, 0);
                    shown = id;
                }
            }
        }
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[1].pressed = 0;
        vs_pad_state.pads[0].pressed_repeat = 0;
        vs_pad_state.pads[1].pressed_repeat = 0;
        menu_item_request_draw(player);
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    free(IDS);
    pos.vx = 640;
    pos.vy = 124;
    if (requested_items[0] != -1) {
        cd_read_member(11, vis_table_c.types[requested_items[0]].unk5[0], vs_cd_state.load_buffer);
        tim_load_images_at(vs_cd_state.load_buffer, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (requested_item_2p != -1) {
        cd_read_member(11, vis_table_c.types[requested_item_2p].unk5[0], vs_cd_state.load_buffer);
        tim_load_images_at(vs_cd_state.load_buffer, &pos, NULL);
        DrawSync(1);
    }
}

/*
 * Memory card event handling, the card screen and the match
 * banking. ST.EXE links the same routines (unreconstructed
 * there). match_award_requested_item uses this module's merged "O" literal.
 */

int cd_read_member();

/* The card path pieces ("bu", "0:", "0:*", "*") and a two-byte SJIS digit:
 * short literals, so .sdata under -G8. */
SDATA(0x80063244, 0x13)

/* Clears the four software card events. */
ADDRESS(0x8001a124, 0x54)
void card_clear_sw_events(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    TestEvent(sp->state.card.sw_events[0]);
    TestEvent(sp->state.card.sw_events[1]);
    TestEvent(sp->state.card.sw_events[2]);
    TestEvent(sp->state.card.sw_events[3]);
}

/* Clears the four hardware card events. */
ADDRESS(0x8001a178, 0x54)
void card_clear_hw_events(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    TestEvent(sp->state.card.hw_events[0]);
    TestEvent(sp->state.card.hw_events[1]);
    TestEvent(sp->state.card.hw_events[2]);
    TestEvent(sp->state.card.hw_events[3]);
}

/* Opens and enables the eight card events. */
ADDRESS(0x8001a1cc, 0x190)
void card_open_events(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    EnterCriticalSection();
    sp->state.card.sw_events[0] = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    sp->state.card.sw_events[1] = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    sp->state.card.sw_events[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    sp->state.card.sw_events[3] = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    sp->state.card.hw_events[0] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    sp->state.card.hw_events[1] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    sp->state.card.hw_events[2] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    sp->state.card.hw_events[3] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    EnableEvent(sp->state.card.sw_events[0]);
    EnableEvent(sp->state.card.sw_events[1]);
    EnableEvent(sp->state.card.sw_events[2]);
    EnableEvent(sp->state.card.sw_events[3]);
    EnableEvent(sp->state.card.hw_events[0]);
    EnableEvent(sp->state.card.hw_events[1]);
    EnableEvent(sp->state.card.hw_events[2]);
    EnableEvent(sp->state.card.hw_events[3]);
    ExitCriticalSection();
}

/* Disables and closes the eight card events. */
ADDRESS(0x8001a35c, 0xf0)
void card_close_events(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    EnterCriticalSection();
    DisableEvent(sp->state.card.sw_events[0]);
    DisableEvent(sp->state.card.sw_events[1]);
    DisableEvent(sp->state.card.sw_events[2]);
    DisableEvent(sp->state.card.sw_events[3]);
    DisableEvent(sp->state.card.hw_events[0]);
    DisableEvent(sp->state.card.hw_events[1]);
    DisableEvent(sp->state.card.hw_events[2]);
    DisableEvent(sp->state.card.hw_events[3]);
    CloseEvent(sp->state.card.sw_events[0]);
    CloseEvent(sp->state.card.sw_events[1]);
    CloseEvent(sp->state.card.sw_events[2]);
    CloseEvent(sp->state.card.sw_events[3]);
    CloseEvent(sp->state.card.hw_events[0]);
    CloseEvent(sp->state.card.hw_events[1]);
    CloseEvent(sp->state.card.hw_events[2]);
    CloseEvent(sp->state.card.hw_events[3]);
    ExitCriticalSection();
}

/* Waits for a software card event: 0 done, 1 error, 2 timeout, 3 new card. */
ADDRESS(0x8001a44c, 0xb4)
int card_wait_sw_event(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    long ioe, error, timeout, new;

    for (;;) {
        ioe = TestEvent(sp->state.card.sw_events[0]);
        error = TestEvent(sp->state.card.sw_events[1]);
        timeout = TestEvent(sp->state.card.sw_events[2]);
        new = TestEvent(sp->state.card.sw_events[3]);
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

/* The same wait on the hardware card events. */
ADDRESS(0x8001a500, 0xb4)
int card_wait_hw_event(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    long ioe, error, timeout, new;

    for (;;) {
        ioe = TestEvent(sp->state.card.hw_events[0]);
        error = TestEvent(sp->state.card.hw_events[1]);
        timeout = TestEvent(sp->state.card.hw_events[2]);
        new = TestEvent(sp->state.card.hw_events[3]);
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

/* Checks that the card file of player `port` holds that player's save. */
ADDRESS(0x8001a5b4, 0x128)
void card_check_player_save(int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char path[32];
    long buf[32];
    int fd;

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, save_file_names[port]);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        sp->card->status = 4;
        return;
    }
    lseek(fd, 512, SEEK_SET);
    read(fd, buf, 128);
    close(fd);
    if (saves[port].save_id != buf[0])
        sp->card->status = 4;
    else
        sp->card->status = 0;
}

/* Scans card `port`: free space when unformatted space is asked for, then
 * the game's save files and the first unused save number. */
ADDRESS(0x8001a6dc, 0x3a8)
void card_read_directory(int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    struct DIRENTRY dir[15];
    char path[32];
    long used[8];
    int i, n, total;

    sp->card = &sp->state.card.ports[port];
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
    for (i = 0; i < 8; i++)
        used[i] = 0;
    for (i = 0; i < sp->card->count; i++) {
        strcpy(sp->card->files[i].name, dir[i].name);
        n = (((u_char *)dir[i].name)[19] - '0') * 10 + (((u_char *)dir[i].name)[20] - '0');
        used[n] = -1;
    }
    for (i = 1; i < 8; i++) {
        if (used[i] != -1)
            break;
    }
    sp->free_file_number = i;
    sp->card->status = 0;
}

/* Reads card `port`'s state through the BIOS card events, three tries. */
ADDRESS(0x8001aa84, 0x178)
void card_probe(int port, int new_ok)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int tries;
    int result;

    card_open_events();
    tries = 2;
    do {
        card_clear_sw_events();
        _card_info(port << 4);
        result = card_wait_sw_event();
        if (result == 0) {
            sp->state.card.ports[port].status = 0;
        } else if (result == 1) {
            if (tries)
                continue;
            sp->state.card.ports[port].status = 2;
            break;
        } else if (result == 2) {
            if (tries)
                continue;
            sp->state.card.ports[port].status = result;
            break;
        } else if (result == 3) {
            sp->state.card.ports[port].status = 1;
            if (!new_ok)
                break;
        }
        card_clear_hw_events();
        _card_clear(port << 4);
        if (card_wait_hw_event()) {
            sp->state.card.ports[port].status = 2;
            continue;
        }
        card_clear_sw_events();
        _card_load(port << 4);
        result = card_wait_sw_event();
        if (result == 1 || result == 2) {
            if (tries)
                continue;
            sp->state.card.ports[port].status = 2;
            break;
        }
        if (result == 0)
            sp->state.card.ports[port].status = 0;
        else if (result == 3)
            sp->state.card.ports[port].status = result;
        break;
    } while (tries--);
    card_close_events();
}

/* A yes/no prompt: returns the choice (0 or 1), 0 on cancel. */
ADDRESS(0x8001abfc, 0x11c)
s16 card_ask_yes_no(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u32 buttons;

    sp->blink_phase = 0;
    sp->dialog_choice = 0;
    for (;;) {
        if (first) {
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[0].pressed_repeat = 0;
            vs_pad_state.pads[0].held = 0;
        } else {
            vs_pad_state.pads[1].pressed = 0;
            vs_pad_state.pads[1].pressed_repeat = 0;
            vs_pad_state.pads[1].held = 0;
        }
        buttons = vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed;
        if (buttons & PAD_BUTTON_CROSS) {
            play_confirm_sound();
            break;
        }
        if (buttons & PAD_BUTTON_CIRCLE) {
            play_cancel_sound();
            sp->dialog_choice = 0;
            break;
        }
        if (buttons & PAD_BUTTON_LEFT) {
            play_cursor_sound();
            sp->dialog_choice = 1;
        }
        if ((vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed) & PAD_BUTTON_RIGHT) {
            play_cursor_sound();
            sp->dialog_choice = 0;
        }
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[1].pressed = 0;
        card_screen_draw();
    }
    return sp->dialog_choice;
}

/* Picks a card slot: returns nonzero when one was chosen. */
ADDRESS(0x8001ad18, 0x160)
int card_choose_port(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u32 buttons;

    sp->blink_phase = 0;
    for (;;) {
        if (first) {
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[0].pressed_repeat = 0;
            vs_pad_state.pads[0].held = 0;
        } else {
            vs_pad_state.pads[1].pressed = 0;
            vs_pad_state.pads[1].pressed_repeat = 0;
            vs_pad_state.pads[1].held = 0;
        }
        buttons = vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed;
        if (buttons & PAD_BUTTON_CROSS) {
            play_confirm_sound();
            sp->mode <<= 1;
            break;
        }
        if (buttons & PAD_BUTTON_CIRCLE) {
            play_cancel_sound();
            sp->mode = 0;
            break;
        }
        if ((vs_pad_state.pads[0].pressed_repeat | vs_pad_state.pads[1].pressed_repeat) & PAD_BUTTON_UP) {
            play_cursor_sound();
            sp->mode = 2;
        }
        if ((vs_pad_state.pads[0].pressed_repeat | vs_pad_state.pads[1].pressed_repeat) & PAD_BUTTON_DOWN) {
            play_cursor_sound();
            sp->mode = 512;
        }
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[1].pressed = 0;
        vs_pad_state.pads[0].pressed_repeat = 0;
        vs_pad_state.pads[1].pressed_repeat = 0;
        card_screen_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
    }
    return sp->mode != 0;
}

/* Reads the three header values of each listed save file; -1 on a read
 * error. */
ADDRESS(0x8001ae78, 0x154)
int card_read_file_headers(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char path[32];
    VsCardHeader header;
    int i, fd;

    i = sp->card->count;
    while (--i != -1) {
        strcpy(path, "bu");
        strcat(path, itoa(sp->port));
        strcat(path, "0:");
        strcat(path, sp->card->files[i].name);
        fd = open(path, O_RDONLY);
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &header, 512) == -1)
            return -1;
        (i + sp->card->files)->area = header.area;
        (i + sp->card->files)->sp_total = header.sp_total;
        (i + sp->card->files)->items_found = header.items_found;
        close(fd);
    }
    return 1;
}

/* The save file list: returns 1 with the chosen file in unk1C2, 0 on
 * cancel. */
ADDRESS(0x8001afcc, 0x348)
int card_choose_file(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->row_count = sp->card->count;
    sp->has_new_row = 0;
    sp->state.menu.half[1] = 0;
    sp->file_row = 0;
    sp->first_row = 0;
    sp->blink_phase = 0;
    for (;;) {
        if (first) {
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[0].pressed_repeat = 0;
            vs_pad_state.pads[0].held = 0;
        } else {
            vs_pad_state.pads[1].pressed = 0;
            vs_pad_state.pads[1].pressed_repeat = 0;
            vs_pad_state.pads[1].held = 0;
        }
        if ((sp->mode & 0xff00)
            || ((vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed) & PAD_BUTTON_CROSS)) {
            play_confirm_sound();
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[1].pressed = 0;
            sp->state.menu.half[1] = 1;
            sp->blink_phase = 32;
            sp->dialog_choice = 1;
            for (;;) {
                u32 buttons;

                if (first) {
                    vs_pad_state.pads[0].pressed = 0;
                    vs_pad_state.pads[0].pressed_repeat = 0;
                    vs_pad_state.pads[0].held = 0;
                } else {
                    vs_pad_state.pads[1].pressed = 0;
                    vs_pad_state.pads[1].pressed_repeat = 0;
                    vs_pad_state.pads[1].held = 0;
                }
                buttons = vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed;
                if (buttons & PAD_BUTTON_CROSS) {
                    play_confirm_sound();
                    break;
                }
                if (buttons & PAD_BUTTON_CIRCLE) {
                    play_cancel_sound();
                    sp->dialog_choice = 0;
                    break;
                }
                if (buttons & PAD_BUTTON_LEFT) {
                    play_cursor_sound();
                    sp->dialog_choice = 1;
                }
                if ((vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed) & PAD_BUTTON_RIGHT) {
                    play_cursor_sound();
                    sp->dialog_choice = 0;
                }
                vs_pad_state.pads[0].pressed = 0;
                vs_pad_state.pads[1].pressed = 0;
                card_screen_draw();
            }
            vs_pad_state.pads[0].pressed = 0;
            vs_pad_state.pads[1].pressed = 0;
            if (sp->dialog_choice)
                goto done;
            if (sp->mode & 0xff00) {
                sp->file_row = -1;
                goto done;
            }
            sp->state.menu.half[1] = 0;
            continue;
        }
        if ((vs_pad_state.pads[0].pressed | vs_pad_state.pads[1].pressed) & PAD_BUTTON_CIRCLE) {
            play_cancel_sound();
            sp->file_row = -1;
            goto done;
        }
        if ((vs_pad_state.pads[0].pressed_repeat | vs_pad_state.pads[1].pressed_repeat) & PAD_BUTTON_UP) {
            play_cursor_sound();
            if (sp->file_row == 0) {
                if (sp->first_row > 0)
                    sp->first_row--;
            } else {
                sp->file_row--;
            }
        }
        if ((vs_pad_state.pads[0].pressed_repeat | vs_pad_state.pads[1].pressed_repeat) & PAD_BUTTON_DOWN) {
            play_cursor_sound();
            if (sp->file_row == 2) {
                if (sp->first_row < sp->row_count - 3)
                    sp->first_row++;
            } else if (sp->file_row + sp->first_row < sp->row_count - 1) {
                sp->file_row++;
            }
        }
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[1].pressed = 0;
        vs_pad_state.pads[0].pressed_repeat = 0;
        vs_pad_state.pads[1].pressed_repeat = 0;
        card_screen_draw();
        sp->blink_phase = (sp->blink_phase + 1) % 32;
        }
done:
    if (sp->file_row == -1)
        return 0;
    sp->file_row += sp->first_row;
    return 1;
}

/* Writes `size` bytes of `data` at `offset` in 128-byte blocks of 127 data
 * bytes and an inverted sum byte; returns the sum of those bytes, or -1. */
ADDRESS(0x8001b314, 0xe4)
int card_write_blocks(int fd, int offset, u8 *buf, int size)
{
    u8 *data = buf;
    u8 block[128];
    int sum = 0;
    int n, i;

    for (n = 0; size > 0; n++) {
        for (block[127] = i = 0; i < 127; i++) {
            block[i] = *data;
            block[127] += *data;
            data++;
        }
        block[127] = ~block[127];
        lseek(fd, offset + n * 128, SEEK_SET);
        if (write(fd, block, 128) != 128)
            return -1;
        size -= 127;
        sum += block[127];
    }
    return sum & 0xff;
}

/* Reads blocks written by card_write_blocks, rereading a bad block up to three
 * times; returns the sum of the check bytes, or -1. */
ADDRESS(0x8001b3f8, 0x10c)
int card_read_blocks(int fd, int offset, u8 *buf, int size)
{
    u8 *data = buf;
    u8 block[128];
    int retries = 0;
    u8 sum = 0;
    int n = 0;
    u8 check;
    int i;

    while (size > 0) {
        lseek(fd, offset + n * 128, SEEK_SET);
        if (read(fd, block, 128) != 128)
            return -1;
        for (check = i = 0; i < 127; i++) {
            check += block[i];
            *data++ = block[i];
        }
        check = ~check;
        if (check != block[127]) {
            data -= 127;
            if (retries >= 3)
                return -1;
            retries++;
        } else {
            size -= 127;
            n++;
            sum += check;
        }
    }
    return sum;
}

/* Loads the chosen save file into `save`: 1 on success, 0 on failure. */
ADDRESS(0x8001b504, 0x34c)
int card_load_game(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    VsSave *loaded;
    char path[32];
    VsCardHeader *dst;
    VsCardHeader *header;
    int tries = 3;
    int fd, sum, i;
    u16 count;

    if (save == &saves[0])
        dst = &save_headers[0];
    else
        dst = &save_headers[1];
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->port));
    strcat(path, "0:");
    strcat(path, sp->card->files[sp->file_row].name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (!tries)
            return 0;
        tries--;
        goto open;
    }
    header = mem_alloc_main_ram(512);
read:
    lseek(fd, 0, SEEK_SET);
    if (read(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            free(header);
            return 0;
        }
        tries--;
        goto read;
    }
    loaded = &shared_buffer.save;
    sum = card_read_blocks(fd, 512, (u8 *)loaded, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        goto read;
    }
    if (sum != header->data_sum) {
        if (!tries) {
            close(fd);
            sp->state.menu.half[0] = 349;
            return 0;
        }
        tries--;
        goto read;
    }
    close(fd);
    *dst = *header;
    *save = *loaded;
    count = 0;
    {
        char zero[2] = "\x82\x4f";

        for (i = 0; i < 266; i++) {
            if (save->collected_bits[i / 32] & (1 << (i % 32)))
                count++;
        }
    }
    dst->items_found = count;
    free(header);
    return 1;
}

/* Writes `save` to its card file, creating it first when asked: 1 on
 * success, 0 on failure. */
ADDRESS(0x8001b850, 0x290)
int card_save_game(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char number[3];
    char path[32];
    VsCardHeader *header;
    int tries = 3;
    int blocks = 2;
    int create;
    int fd, sum;

    if (save == &saves[0])
        header = &save_headers[0];
    else
        header = &save_headers[1];
    create = 0;
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->port));
    strcat(path, "0:");
    if (create) {
        strcat(path, "BASLUS-00863FS_ST");
        number[0] = sp->free_file_number / 10 + '0';
        number[1] = sp->free_file_number % 10 + '0';
        number[2] = 0;
        strcat(path, number);
        fd = open(path, (blocks << 16) | O_CREAT);
        if (fd == -1)
            goto retry;
        close(fd);
    } else {
        strcat(path, sp->card->files[sp->file_row].name);
    }
    fd = open(path, O_WRONLY);
    if (fd == -1) {
retry:
        if (tries) {
            tries--;
            goto open;
        }
        return 0;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = card_write_blocks(fd, 512, (u8 *)save, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header->data_sum = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    close(fd);
    return 1;
}

/* Formats the card in `port`. */
ADDRESS(0x8001bae0, 0x68)
void card_format(int port)
{
    char path[32];

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    format(path);
}

/* Waits for a new press on `pad`: the controller interrupt sets its
 * pressed bits behind the compiler's back, so the poll reads through a
 * volatile view (retail rereads the halfword after its own store). */
#define WAIT_PRESS(pad)                                         \
    do {                                                        \
        (pad).pressed = 0;                                         \
        while (!*(volatile u16 *)&(pad).pressed)                   \
            ;                                                   \
        (pad).pressed = 0;                                         \
    } while (0)

/* Messages card_screen_run shows. */
#define MSG_CARD_ERROR 350
#define MSG_NEW_CARD 351
#define MSG_NO_FILE 352
#define MSG_CARD_FULL 353
#define MSG_LOAD_FAILED 354
#define MSG_SAVE_FAILED 355
#define MSG_NO_SAVE 357
#define MSG_NOT_LOADED 358

int card_overwrite_player_save(VsSave *save);

/*
 * The card screen of player `port`: loads its pictures, then steps
 * through the card checks (sp->mode holds the step as a moving bit),
 * saving (step bits 0xff00) or loading that player's record, showing a
 * message and waiting for a press when a step fails; 0x80 and 0x8000
 * restart. Reloads the menu pictures and both players' fighter faces
 * before returning 1 when the card was used, else 0.
 */
ADDRESS(0x8001bb48, 0x830)
int card_screen_run(int mode, int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    DVECTOR pos = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR unused_pos = {576, 384};
    DVECTOR unused_clut = {544, 502};
    DVECTOR face;
    u_long *buf;
    int started = 0;
    int result;
    int status;

    clear_vram();
    DrawSync(0);
    gfx_setup_320x240();
    buf = vs_cd_state.load_buffer;
    cd_read_member(8, 0, buf);
    copy_words((u32 *)vs_menu_messages[0].offsets, (u32 *)(buf + 1), 0x1500);
    buf = (u_long *)((u8 *)buf + (buf[0] + 4));
    tim_load_images_at(buf + 1, &sp->message_font_vram, &sp->message_font_clut);
    DrawSync(0);
    cd_read_member(8, 6, vs_cd_state.load_buffer);
    tim_load_images_at(vs_cd_state.load_buffer, &pos, &clut);
    DrawSync(0);
    sp->state.menu.half[0] = 0;
    sp->state.menu.half[1] = 0;
    sp->show_backdrop = mode;
    vs_pad_state.pads[0].pressed = 0;
    vs_pad_state.pads[1].pressed = 0;
    vs_pad_state.pads[0].pressed_repeat = 0;
    vs_pad_state.pads[1].pressed_repeat = 0;
again:
    sp->port = port;
    sp->card = &sp->state.card.ports[sp->port];
    if (!card_choose_port(port)) {
        result = 0;
        goto done;
    }
    result = 0;
    if (sp->mode & 0xff00) {
        if (port == 0) {
            if (!save_loaded_1p) {
                sp->state.menu.half[0] = MSG_NOT_LOADED;
                sp->mode <<= 5;
                goto wait;
            }
        } else {
            if (!save_loaded_2p) {
                sp->state.menu.half[0] = MSG_NOT_LOADED;
                sp->mode <<= 5;
                goto wait;
            }
        }
    }
    card_screen_draw();
    card_screen_draw();
    if (!started) {
        StartCARD();
        started = 1;
        _bu_init();
    }
    card_probe(sp->port, 1);
    if (sp->card->status == 2) {
        sp->state.menu.half[0] = MSG_CARD_ERROR;
        sp->mode <<= 5;
        goto wait;
    }
    sp->mode <<= 2;
    vs_pad_state.pads[0].pressed = 0;
    vs_pad_state.pads[1].pressed = 0;
    vs_pad_state.pads[0].pressed_repeat = 0;
    vs_pad_state.pads[1].pressed_repeat = 0;
    if (sp->mode & 0xff00) {
        card_check_player_save(sp->port);
        if (sp->card->status == 0) {
            strcpy(sp->card->files[0].name, save_file_names[port]);
            sp->card->count = 1;
            sp->card->has_room = 0;
        } else {
            sp->state.menu.half[0] = MSG_NO_SAVE;
            sp->mode <<= 3;
            goto wait;
        }
    } else {
        card_read_directory(sp->port);
        status = sp->card->status;
        if (status != 0) {
            if (status == 4)
                sp->state.menu.half[0] = MSG_NO_FILE;
            else if (status == 5)
                sp->state.menu.half[0] = MSG_CARD_FULL;
            sp->mode <<= 3;
            goto wait;
        }
    }
    if (card_read_file_headers() == -1) {
        sp->state.menu.half[0] = MSG_CARD_ERROR;
        sp->mode <<= 3;
        goto wait;
    }
    if (!card_choose_file(port)) {
        sp->mode >>= 3;
        goto again;
    }
    sp->mode <<= 1;
    card_screen_draw();
    card_screen_draw();
    card_probe(sp->port, 0);
    status = sp->card->status;
    if (status == 1) {
        sp->state.menu.half[0] = MSG_NEW_CARD;
        sp->mode <<= 2;
        goto wait;
    }
    if (status == 2) {
        sp->state.menu.half[0] = MSG_CARD_ERROR;
        sp->mode <<= 2;
        goto wait;
    }
    if (sp->mode & 0xff00) {
        saves[port].versus.lineup[0] = lineups[port].slot[0];
        saves[port].versus.lineup[1] = lineups[port].slot[1];
        saves[port].versus.lineup[2] = lineups[port].slot[2];
        result = card_overwrite_player_save(&saves[port]);
    } else {
        result = card_load_game(&saves[port]);
    }
    if (!result) {
        if (sp->state.menu.half[0] == 0) {
            if (sp->mode & 0xff00)
                sp->state.menu.half[0] = MSG_SAVE_FAILED;
            else
                sp->state.menu.half[0] = MSG_LOAD_FAILED;
        }
        sp->mode <<= 2;
        goto wait;
    }
    if (!(sp->mode & 0xff00)) {
        lineups[port].slot[0] = saves[port].versus.lineup[0];
        lineups[port].slot[1] = saves[port].versus.lineup[1];
        lineups[port].slot[2] = saves[port].versus.lineup[2];
        requested_item_2p = -1;
        requested_items[0] = -1;
        strcpy(save_file_names[port], sp->card->files[sp->file_row].name);
        DAT_80063328[port] = sp->file_row;
        if (port == 0)
            save_loaded_1p = 1;
        else
            save_loaded_2p = 1;
    }
    sp->mode <<= 1;
wait:
    card_screen_draw();
    card_screen_draw();
    if (port)
        WAIT_PRESS(vs_pad_state.pads[1]);
    else
        WAIT_PRESS(vs_pad_state.pads[0]);
    play_cancel_sound();
    if (sp->mode & 0x80) {
        sp->mode = 2;
        goto again;
    }
    if (sp->mode & 0x8000) {
        sp->mode = 512;
        goto again;
    }
    result = 1;
done:
    {
        u_long *menu_buf;
        u_long *tim;

        clear_vram();
        DrawSync(0);
        gfx_setup_640x240();
        face.vx = 640;
        face.vy = 124;
        vs_cd_state.load_cursor = menu_buf = vs_cd_state.load_buffer;
        cd_read_member(8, 4, menu_buf);
        copy_words((u32 *)vs_menu_messages[1].offsets, (u32 *)&menu_buf[1], 0x1500);
        tim = (u_long *)((u8 *)menu_buf + vs_cd_state.load_buffer[0]);
        tim_load_images_at(tim + 2, &sp->item_font_vram, &sp->item_font_clut);
        DrawSync(0);
        cd_read_member(8, 8, menu_buf);
        copy_words((u32 *)vs_menu_messages[0].offsets, (u32 *)&menu_buf[1], 0x1500);
        tim = (u_long *)((u8 *)menu_buf + vs_cd_state.load_buffer[0]);
        tim_load_images_at(tim + 2, &sp->message_font_vram, &sp->message_font_clut);
        DrawSync(0);
        cd_read_member(8, 7, menu_buf);
        tim_load_images_at(menu_buf, &pos, &clut);
        DrawSync(0);
        if (requested_items[0] != -1) {
            cd_read_member(11, vis_table_c.types[requested_items[0]].unk5[0], menu_buf);
            tim_load_images_at(menu_buf, &face, NULL);
            DrawSync(0);
        }
        face.vy += 28;
        if (requested_item_2p != -1) {
            cd_read_member(11, vis_table_c.types[requested_item_2p].unk5[0], menu_buf);
            tim_load_images_at(menu_buf, &face, NULL);
            DrawSync(1);
        }
        return result;
    }
}

/* Writes `save` over its player's card file (named in save_file_names): 1 on
 * success, 0 on failure.
 *
 * card_save_game without its create branch: `number` (the new file's digits
 * there) is still declared ahead of `path` and never used.
 */
ADDRESS(0x8001c378, 0x1b8)
int card_overwrite_player_save(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char number[3];
    char path[32];
    VsCardHeader *header;
    char *name;
    int tries = 3;
    int fd, sum;

    if (save == &saves[0]) {
        header = &save_headers[0];
        name = save_file_names[0];
    } else {
        header = &save_headers[1];
        name = save_file_names[1];
    }
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->port));
    strcat(path, "0:");
    strcat(path, name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (!tries)
            return 0;
        tries--;
        goto open;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = card_write_blocks(fd, 512, (u8 *)save, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header->data_sum = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    close(fd);
    return 1;
}

/* Starts the card system and checks both players' card files: 1 when
 * either fails the check. */
ADDRESS(0x8001c530, 0xb0)
int card_check_saves_changed(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;

    sp->state.menu.half[0] = 0;
    StartCARD();
    _bu_init();
    for (i = 0; i < 2; i++) {
        sp->port = i;
        sp->card = &sp->state.card.ports[sp->port];
        card_check_player_save(sp->port);
        if (sp->card->status)
            return 1;
    }
    return 0;
}

/*
 * Banks a won match: unless `again`, gives the winner (side `side`) the
 * item it requested (requested_items[side]): marks it held in its save
 * record (and collected, bumping the header's three-digit count the
 * first time), moves the loser's durability of it over and takes it
 * from the loser; then writes both records back. Returns a bit per
 * player whose write failed.
 *
 * Not exact (frame size only, every instruction agrees): retail's frame is
 * 56 bytes with eight untouched bytes at sp+24, right after `zero`; this
 * build's is 48. The allocation matches, HI is not spilled and nothing is
 * caller-saved, so the slot is a declared local of up to 8 bytes that the
 * code never uses; no twin names it (an unused `char number[3];` as in
 * card_overwrite_player_save would make the function exact).
 */
ADDRESS(0x8001c5e0, 0x2d8)
int match_award_requested_item(int side, int again)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int failed = 0;
    u16 id = requested_items[side];
    char zero[2] = "\x82\x4f";
    VsSaveVersus *win = &saves[side].versus;
    VsSaveVersus *lose = &saves[1 - side].versus;
    int started = failed;
    u16 count;
    int i;

    if (!again) {
        if (!(saves[side].collected_bits[id / 32] & (1 << (id % 32)))) {
            /* The count is reached through its own address (header + 0x40),
             * not as a member of the header element. */
            count = *(u16 *)&save_headers[side].items_found + 1;
            saves[side].items_found = count;
            if (count >= 100) {
                save_headers[side].items_found_text[0] = zero[0];
                save_headers[side].items_found_text[1] = zero[1] + count / 100 % 10;
            }
            if (count >= 10) {
                save_headers[side].items_found_text[2] = zero[0];
                save_headers[side].items_found_text[3] = zero[1] + count / 10 % 10;
            }
            save_headers[side].items_found_text[4] = zero[0];
            save_headers[side].items_found_text[5] = zero[1] + count % 10;
            *(u16 *)&save_headers[side].items_found = count;
            saves[side].collected_bits[id / 32] |= 1 << (id % 32);
        }
        win->inventory_counts[id] = 1;
        win->item_durability[id] = lose->item_durability[id];
        win->items_lost[id] = 0;
        {
            /* Retail loads this 1 on its own: an int-typed value, not the
             * byte constant stored above. */
            int on = 1;

            lose->items_lost[id] = on;
        }
        lose->inventory_counts[id] = 0;
        lose->item_durability[id] = 0;
    }
    if (!started) {
        StartCARD();
        _bu_init();
    }
    for (i = 0; i < 2; i++) {
        sp->port = i;
        sp->card = &sp->state.card.ports[sp->port];
        if (!card_overwrite_player_save(&saves[i]))
            failed |= 1 << i;
    }
    return failed;
}
