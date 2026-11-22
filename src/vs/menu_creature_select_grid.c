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

/* Number-width flag for ui_draw_number (ST.EXE's DIGITS, src/st/render.c). */
#define DIGITS(n) ((n) << 24)

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
    c = sp->unk1D0;
    row = c >> 2;
    if (c < 0) {
        int slot = ~c;

        if (sp->unk1D6) {
            sp->unk218.x = 26;
            sp->unk218.y = slot * 56 + 44;
        } else {
            sp->unk218.x = 32;
            sp->unk218.y = slot * 56 + 48;
        }
    } else {
        s16 col = c - row * 4;

        sp->unk218.x = col * 112 + 176;
        sp->unk218.y = row * 56 + 48;
    }
    sp->unk218.w = 96;
    sp->unk218.h = 48;
    ui_draw_rect_outline(&sp->unk218, &sp->unk214, 513, 1);
    ui_add_draw_mode(1, 0, 0);
    if (sp->unk1D2 > 0)
        ui_draw_arrow(&up, &sp->unk214, 3072);
    if (sp->unk1D2 <= sp->unk0.team.rows - 3)
        ui_draw_arrow(&down, &sp->unk214, 1024);
    ui_add_draw_mode(1, 0, 0);
    c = sp->unk1D0;
    if (c < 0) {
        id = teams[player].slot[-c - 1];
        if (id != -1) {
            for (k = 0; k < sp->unk0.team.count; k++) {
                if (id == sp->unk0.team.kinds[k]) {
                    idx = k;
                    break;
                }
            }
        } else {
            idx = -1;
        }
    } else {
        idx = sp->unk1D2 * 4 + c;
        id = sp->unk0.team.kinds[idx];
        for (k = 0; k < 3; k++) {
            if (id == teams[player].slot[k])
                idx = -1;
        }
    }
    if (idx < sp->unk0.team.count && idx != -1) {
        sp->style[0].cd |= 0x80;
        ui_draw_text(VS_ITEM_NAME(id), 428, 21, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
    }
    ui_draw_panel(420, 16, 200, 22);
    sp->unk238.w = 96;
    sp->unk238.h = 48;
    sp->unk238.y = 48;
    frame.r = 50;
    frame.g = 50;
    frame.b = 50;
    idx = sp->unk1D2 * 4;
    for (i = 0; i < 3; i++) {
        if (sp->unk1D2 + i == sp->unk0.team.rows)
            n = sp->unk0.team.last;
        else
            n = 4;
        sp->unk238.x = 176;
        for (j = 0; j < n; idx++, j++) {
            if (idx < sp->unk0.team.count) {
                ui_draw_rect_outline(&sp->unk238, &frame, 513, 0);
                id = sp->unk0.team.kinds[idx];
                for (k = 0; k < 3; k++) {
                    if (teams[player].slot[k] == id) {
                        black.r = 0;
                        black.g = 0;
                        black.b = 0;
                        ui_fill_rect(&sp->unk238, &black, 0);
                        break;
                    }
                }
                if (k == 3)
                    ui_draw_image(&sp->unk238, &sp->unk0.team.pos[idx % 12], NULL, NULL, 16);
            }
            sp->unk238.x += 112;
        }
        sp->unk238.y += 56;
    }
    sp->unk238.x = 32;
    sp->unk238.y = 48;
    for (i = 0; i < 3; i++) {
        ui_draw_number((i + 1) | DIGITS(3), 110, i * 56 + 85, &sp->style[3]);
        if (teams[player].slot[i] == -1) {
            black.r = 0;
            black.g = 0;
            black.b = 0;
            ui_fill_rect(&sp->unk238, &black, 0);
        } else {
            x = sp->unk238.x;
            y = sp->unk238.y;
            if (sp->unk1D6 && ~sp->unk1D0 == i) {
                sp->unk238.x -= 6;
                sp->unk238.y -= 4;
            }
            ui_draw_image(&sp->unk238, &sp->unk0.team.pos[12 + i], NULL, NULL, 16);
            sp->unk238.x = x;
            sp->unk238.y = y;
        }
        ui_draw_rect_outline(&sp->unk238, &frame, 513, 0);
        sp->unk238.y += 56;
    }
    ui_add_draw_mode(0, 0, 0);
    ui_draw_panel(160, 40, 464, 176);
    ui_draw_panel(16, 40, 138, 176);
}
