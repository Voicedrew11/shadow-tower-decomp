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
 * Starts a frame and draws, in team mode (round_count == 3), the six
 * slot pictures in two columns of three (black for an empty slot),
 * otherwise each player's first slot; then each player's fighter
 * picture over the shared frame tile, and the two background strips.
 */
ADDRESS(0x80021984, 0x290)
void lineup_screen_draw(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 *slot = teams[0].slot;
    CVECTOR black;
    int i;

    gfx_begin_frame();
    sp->unk238.w = 48;
    sp->unk238.h = 48;
    sp->unk238.y = 48;
    sp->unk238.x = 56;
    if (round_count == 3) {
        for (i = 0; i < 6; i++) {
            if (*slot++ == -1) {
                black.r = 0;
                black.g = 0;
                black.b = 0;
                ui_fill_rect(&sp->unk238, &black, 0);
            } else {
                ui_draw_image(&sp->unk238, &sp->unk0.team.pos[i], NULL, NULL, 272);
            }
            sp->unk238.y += 48;
            if (i == 2) {
                sp->unk238.x = 216;
                sp->unk238.y = 48;
            }
        }
    } else {
        sp->unk238.x = 56;
        sp->unk238.y = 96;
        ui_draw_image(&sp->unk238, &sp->unk0.team.pos[0], NULL, NULL, 272);
        sp->unk238.x = 216;
        sp->unk238.y = 96;
        ui_draw_image(&sp->unk238, &sp->unk0.team.pos[3], NULL, NULL, 272);
    }
    sp->unk238.w = 28;
    sp->unk238.h = 28;
    sp->unk238.x = 20;
    sp->unk238.y = 164;
    for (i = 0; i < 2; i++) {
        if (requested_items[i] == -1) {
            black.r = 0;
            black.g = 0;
            black.b = 0;
            ui_fill_rect(&sp->unk238, &black, 0);
        } else {
            ui_draw_image(&sp->unk238, &sp->unk0.team.pos[6 + i], NULL, NULL, 272);
        }
        ui_draw_image(&sp->unk238, &sp->unk0.team.pos[8], NULL, NULL, 272);
        sp->unk238.x = 272;
    }
    sp->unk238.w = 160;
    sp->unk238.h = 240;
    sp->unk238.x = 0;
    sp->unk238.y = 0;
    sp->unk240.vx = 320;
    sp->unk240.vy = 0;
    ui_draw_image(&sp->unk238, &sp->unk240, NULL, NULL, 16);
    sp->unk238.x = 160;
    sp->unk240.vx = 480;
    ui_draw_image(&sp->unk238, &sp->unk240, NULL, NULL, 16);
    ui_add_draw_mode(0, 0, 0);
    gfx_end_frame();
}
