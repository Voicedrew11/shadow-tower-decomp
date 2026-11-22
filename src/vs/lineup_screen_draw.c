/*
 * VS.EXE: the team screen's frame (WIP module): both teams' slot
 * pictures, the two fighters' pictures and the background strips.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#define VS_PICKED_PAIR
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/*
 * Starts a frame and draws, in 3-set mode (round_count == 3), the six
 * slot pictures in two columns of three (black for an empty slot),
 * otherwise each player's first slot; then each player's fighter
 * picture over the shared frame tile, and the two background strips.
 */
ADDRESS(0x80021984, 0x290)
void lineup_screen_draw(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 *slot = lineups[0].slot;
    CVECTOR black;
    int i;

    gfx_begin_frame();
    sp->image_rect.w = 48;
    sp->image_rect.h = 48;
    sp->image_rect.y = 48;
    sp->image_rect.x = 56;
    if (round_count == 3) {
        for (i = 0; i < 6; i++) {
            if (*slot++ == -1) {
                black.r = 0;
                black.g = 0;
                black.b = 0;
                ui_fill_rect(&sp->image_rect, &black, 0);
            } else {
                ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[i], NULL, NULL, 272);
            }
            sp->image_rect.y += 48;
            if (i == 2) {
                sp->image_rect.x = 216;
                sp->image_rect.y = 48;
            }
        }
    } else {
        sp->image_rect.x = 56;
        sp->image_rect.y = 96;
        ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[0], NULL, NULL, 272);
        sp->image_rect.x = 216;
        sp->image_rect.y = 96;
        ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[3], NULL, NULL, 272);
    }
    sp->image_rect.w = 28;
    sp->image_rect.h = 28;
    sp->image_rect.x = 20;
    sp->image_rect.y = 164;
    for (i = 0; i < 2; i++) {
        if (requested_items[i] == -1) {
            black.r = 0;
            black.g = 0;
            black.b = 0;
            ui_fill_rect(&sp->image_rect, &black, 0);
        } else {
            ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[6 + i], NULL, NULL, 272);
        }
        ui_draw_image(&sp->image_rect, &sp->state.creature_select.pos[8], NULL, NULL, 272);
        sp->image_rect.x = 272;
    }
    sp->image_rect.w = 160;
    sp->image_rect.h = 240;
    sp->image_rect.x = 0;
    sp->image_rect.y = 0;
    sp->tex_pos.vx = 320;
    sp->tex_pos.vy = 0;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
    sp->image_rect.x = 160;
    sp->tex_pos.vx = 480;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
    ui_add_draw_mode(0, 0, 0);
    gfx_end_frame();
}
