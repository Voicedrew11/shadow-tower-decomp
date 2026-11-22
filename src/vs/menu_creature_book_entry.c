/*
 * VS.EXE: the creature entry page (WIP module), VS's copy of ST.EXE's
 * creature book entry page menu_creature_book_draw_entry (src/st/render.c):
 * the creature comes from the team slot or the grid under the cursor, and
 * the page toggle is DAT_8006335c (ST.EXE's menu_second_page).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/pad.h>
#include <vs/funcs.h>

/* The kind names' widths. */
RODATA(0x8001123c, 0x22)
/* The page arrows' initializers: 8 bytes each, so .sdata under -G8. */
SDATA(0x80063280, 0x10)

/* Number-width flag for ui_draw_number (ST.EXE's DIGITS, src/st/render.c). */
#define DIGITS(n) ((n) << 24)

/*
 * The cell grid (src/vs/ui_cell_grid.c) has no prototype in a header, and
 * retail's page saw it as returning int (an implicit declaration, written
 * out here): reload keeps the inches quotient's high word out of $v0
 * (`mfhi v1`), which it only does when some call in the function has a
 * value. The result is not used.
 */
int ui_draw_cell_grid();

/* The detail page toggle: the attribute figures when nonzero (bss, unowned). */

ADDRESS(0x80021140, 0x844)
void menu_creature_book_draw_entry(int player)
{
    RECT left = {319, 119, 16, 8};
    RECT right = {621, 119, 16, 8};
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u16 kind_w[17] = {113, 58, 36, 36, 58, 135, 168, 157, 69, 135, 80, 0, 58, 0, 69, 113, 0};

    CreatureBookEntry *item;
    int kind;
    int col;
    int i;
    u16 a, b;

    col = sp->cursor;
    if (col < 0)
        kind = lineups[player].slot[-col - 1];
    else
        kind = sp->state.creature_select.kinds[sp->scroll * 4 + col];
    item = &sp->state.creature_select.entries[kind];
    if (player) {
        vs_pad_state.pads[0].pressed = 0;
        vs_pad_state.pads[0].pressed_repeat = 0;
        vs_pad_state.pads[0].held = 0;
    } else {
        vs_pad_state.pads[1].pressed = 0;
        vs_pad_state.pads[1].pressed_repeat = 0;
        vs_pad_state.pads[1].held = 0;
    }
    if (DAT_8006335c == 0)
        ui_draw_arrow(&right, &sp->cursor_color, 0);
    else
        ui_draw_arrow(&left, &sp->cursor_color, 2048);
    ui_add_draw_mode(1, 0, 0);
    sp->style[0].cd |= 0x80;
    ui_draw_text(VS_ITEM_NAME(kind), 32, 16, &sp->style[0]);
    {
        DVECTOR tex = {960, 446};
        DVECTOR clut = {560, 497};
        DVECTOR pos = {32, 150};

        sp->style[0].cd &= 0x7f;
        ui_draw_cell_grid(&pos, &tex, &clut, &sp->style[0], 0xf0005);
    }
    if (DAT_8006335c) {
        for (i = 0; i < 14; i++) {
            ui_draw_text(VS_MESSAGE(100 + i), 336, i * 15 + 16, &sp->style[0]);
            ui_draw_number((u16)item->attributes[i] | DIGITS(2), 582, i * 15 + 16, &sp->style[3]);
        }
    } else {
        for (i = 0; i < 10; i++)
            ui_draw_text(VS_MESSAGE(117 + i), 336, i * 15 + 16, &sp->style[0]);
        if (item->category < 17)
            ui_draw_text(VS_MESSAGE(150 + item->category), 606 - kind_w[item->category], 16,
                          &sp->style[0]);
        ui_draw_number(item->hp | DIGITS(4), 558, 31, &sp->style[3]);
        ui_draw_number(item->sp_gain | DIGITS(2), 582, 46, &sp->style[3]);
        a = item->height * 10 / 3048;
        ui_draw_number(a | DIGITS(3), 522, 61, &sp->style[3]);
        ui_draw_glyph(55, 559, 61, &sp->style[3]);
        ui_draw_number((u16)((item->height * 10 - a * 3048) / 254) | DIGITS(2), 570, 61,
                      &sp->style[3]);
        ui_draw_glyph(54, 595, 61, &sp->style[3]);
        a = item->weight * 100 / 454;
        b = item->weight * 1000 / 454;
        ui_draw_number(a | DIGITS(4), 522, 76, &sp->style[3]);
        ui_draw_glyph(18, 574, 76, &sp->style[3]);
        ui_draw_number((u16)(b - a * 10) | DIGITS(1), 582, 76, &sp->style[3]);
        ui_draw_glyph(53, 594, 76, &sp->style[3]);
        if (item->pairs[0][1]) {
            ui_draw_number((u16)item->pairs[0][1] | DIGITS(3), 522, 91, &sp->style[3]);
            ui_draw_glyph(15, 558, 91, &sp->style[3]);
        }
        ui_draw_number((u16)item->pairs[0][0] | DIGITS(3), 570, 91, &sp->style[3]);
        if (item->pairs[1][1]) {
            ui_draw_number((u16)item->pairs[1][1] | DIGITS(3), 522, 106, &sp->style[3]);
            ui_draw_glyph(15, 558, 106, &sp->style[3]);
        }
        ui_draw_number((u16)item->pairs[1][0] | DIGITS(3), 570, 106, &sp->style[3]);
        if (item->pairs[2][1]) {
            ui_draw_number((u16)item->pairs[2][1] | DIGITS(3), 522, 121, &sp->style[3]);
            ui_draw_glyph(15, 558, 121, &sp->style[3]);
        }
        ui_draw_number((u16)item->pairs[2][0] | DIGITS(3), 570, 121, &sp->style[3]);
        if (item->pairs[3][1]) {
            ui_draw_number((u16)item->pairs[3][1] | DIGITS(3), 522, 136, &sp->style[3]);
            ui_draw_glyph(15, 558, 136, &sp->style[3]);
        }
        ui_draw_number((u16)item->pairs[3][0] | DIGITS(3), 570, 136, &sp->style[3]);
        if (item->abilities & 0xf)
            ui_draw_text(VS_MESSAGE(129 + (item->abilities & 0xf)), 336, 181, &sp->style[0]);
        if (item->abilities >> 8 & 0xf)
            ui_draw_text(VS_MESSAGE(129 + (item->abilities >> 8 & 0xf)), 336, 196, &sp->style[0]);
        if (item->abilities >> 16 & 0xf)
            ui_draw_text(VS_MESSAGE(129 + (item->abilities >> 16 & 0xf)), 336, 211, &sp->style[0]);
    }
    sp->image_rect.x = 55;
    sp->image_rect.y = 32;
    sp->image_rect.w = 230;
    sp->image_rect.h = 115;
    ui_draw_rect_outline(&sp->image_rect, &sp->style[0], 513, 1);
    ui_add_draw_mode(0, 0, 0);
    ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[15], NULL, NULL, 16);
    ui_draw_panel(24, 12, 292, 216);
    ui_draw_panel(324, 12, 292, 216);
}
