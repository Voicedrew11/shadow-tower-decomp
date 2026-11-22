/*
 * VS.EXE: the team screen's item grid page (WIP module): the cursor box,
 * the scroll arrows, the name of the item under the cursor, three rows of
 * item pictures (dark when already in the team) and the team's three
 * slots.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/* The scroll arrows' initializers: 8 bytes each, so .sdata under -G8. */
SDATA(0x80063270, 0x10)

ADDRESS(0x80020aa4, 0x69c)
void menu_creature_select_draw_grid(int player)
{
    RECT up = {392, 32, 32, 12};
    RECT down = {392, 224, 32, 12};
    CVECTOR black;
    CVECTOR frame;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int c;
    s16 row;
    int id;
    int idx;
    int n;
    int i, j, k;
    int x, y;

    ui_draw_text(VS_MESSAGE(180), 44, 21, &sp->style[0]);
    ui_draw_panel(16, 16, 138, 22);
    c = sp->cursor;
    row = c >> 2;
    if (c < 0) {
        int slot = ~c;

        if (sp->holding) {
            sp->cursor_rect.x = 26;
            sp->cursor_rect.y = slot * 56 + 44;
        } else {
            sp->cursor_rect.x = 32;
            sp->cursor_rect.y = slot * 56 + 48;
        }
    } else {
        s16 col = c - row * 4;

        sp->cursor_rect.x = col * 112 + 176;
        sp->cursor_rect.y = row * 56 + 48;
    }
    sp->cursor_rect.w = 96;
    sp->cursor_rect.h = 48;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    ui_add_draw_mode(1, 0, 0);
    if (sp->scroll > 0)
        ui_draw_arrow(&up, &sp->cursor_color, 3072);
    if (sp->scroll <= sp->state.creature_select.rows - 3)
        ui_draw_arrow(&down, &sp->cursor_color, 1024);
    ui_add_draw_mode(1, 0, 0);
    c = sp->cursor;
    if (c < 0) {
        id = lineups[player].slot[-c - 1];
        if (id != -1) {
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
        idx = sp->scroll * 4 + c;
        id = sp->state.creature_select.kinds[idx];
        for (k = 0; k < 3; k++) {
            if (id == lineups[player].slot[k])
                idx = -1;
        }
    }
    if (idx < sp->state.creature_select.count && idx != -1) {
        sp->style[0].cd |= 0x80;
        ui_draw_text(VS_ITEM_NAME(id), 428, 21, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
    }
    ui_draw_panel(420, 16, 200, 22);
    sp->image_rect.w = 96;
    sp->image_rect.h = 48;
    sp->image_rect.y = 48;
    frame.r = 50;
    frame.g = 50;
    frame.b = 50;
    idx = sp->scroll * 4;
    for (i = 0; i < 3; i++) {
        if (sp->scroll + i == sp->state.creature_select.rows)
            n = sp->state.creature_select.last;
        else
            n = 4;
        sp->image_rect.x = 176;
        for (j = 0; j < n; idx++, j++) {
            if (idx < sp->state.creature_select.count) {
                ui_draw_rect_outline(&sp->image_rect, &frame, 513, 0);
                id = sp->state.creature_select.kinds[idx];
                for (k = 0; k < 3; k++) {
                    if (lineups[player].slot[k] == id) {
                        black.r = 0;
                        black.g = 0;
                        black.b = 0;
                        ui_fill_rect(&sp->image_rect, &black, 0);
                        break;
                    }
                }
                if (k == 3)
                    ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[idx % 12], NULL, NULL, 16);
            }
            sp->image_rect.x += 112;
        }
        sp->image_rect.y += 56;
    }
    sp->image_rect.x = 32;
    sp->image_rect.y = 48;
    for (i = 0; i < 3; i++) {
        ui_draw_number((i + 1) | DIGITS(3), 110, i * 56 + 85, &sp->style[3]);
        if (lineups[player].slot[i] == -1) {
            black.r = 0;
            black.g = 0;
            black.b = 0;
            ui_fill_rect(&sp->image_rect, &black, 0);
        } else {
            x = sp->image_rect.x;
            y = sp->image_rect.y;
            if (sp->holding && ~sp->cursor == i) {
                sp->image_rect.x -= 6;
                sp->image_rect.y -= 4;
            }
            ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[12 + i], NULL, NULL, 16);
            sp->image_rect.x = x;
            sp->image_rect.y = y;
        }
        ui_draw_rect_outline(&sp->image_rect, &frame, 513, 0);
        sp->image_rect.y += 56;
    }
    ui_add_draw_mode(0, 0, 0);
    ui_draw_panel(160, 40, 464, 176);
    ui_draw_panel(16, 40, 138, 176);
}
