/*
 * VS.EXE: the versus setup page (WIP module): its seven option lines with
 * the cursor bar, then both players' requested items in framed boxes.
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
#include <vs/main.h>
#include <vs/funcs.h>

/* menu_vs_setup_draw_options's bar and shade initializers. */
RODATA(0x80011260, 0x30)

/* The dimmed text colour for options that do not apply. */
SDATA(0x800632c4, 0x4)
CVECTOR DAT_800632c4 = {48, 48, 48, 73};

void ui_draw_gouraud_quad(DVECTOR *xy, CVECTOR *rgb, int semi);

/* Whether each player has a fighter for the options (bss, unowned). */

ADDRESS(0x80022a00, 0x514)
void menu_vs_setup_draw_options(void)
{
    DVECTOR bar[4] = {{96, 60}, {234, 60}, {64, 70}, {246, 70}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    DVECTOR tex;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;

    tex.vx = 640;
    tex.vy = 124;
    sp->unk240.vx = 896;
    sp->unk240.vy = 496;
    sp->unk244.vx = 544;
    sp->unk244.vy = 505;
    sp->unk224.x = 48;
    sp->unk224.y = 20;
    sp->unk224.w = 255;
    sp->unk224.h = 16;
    ui_draw_image(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    for (i = 0; i < 7; i++) {
        switch (i) {
        case 3:
            if (DAT_80063380)
                ui_draw_text(VS_MESSAGE(i), 59, i * 20 + 56, &sp->style[0]);
            else
                ui_draw_text(VS_MESSAGE(i), 59, i * 20 + 56, &DAT_800632c4);
            break;
        case 4:
            if (DAT_80063381)
                ui_draw_text(VS_MESSAGE(i), 59, i * 20 + 56, &sp->style[0]);
            else
                ui_draw_text(VS_MESSAGE(i), 59, i * 20 + 56, &DAT_800632c4);
            break;
        case 5:
        case 6:
            if (DAT_80063380 && DAT_80063381)
                ui_draw_text(VS_MESSAGE(i), 84, i * 20 + 56, &sp->style[0]);
            else
                ui_draw_text(VS_MESSAGE(i), 84, i * 20 + 56, &DAT_800632c4);
            break;
        default:
            ui_draw_text(VS_MESSAGE(i), 94, i * 20 + 56, &sp->style[0]);
            break;
        }
        if (i == sp->unk1CE)
            ui_draw_gouraud_quad(bar, lit, 1);
        else
            ui_draw_gouraud_quad(bar, dim, 1);
        ui_add_draw_mode(1, 0, 0);
        bar[0].vy += 20;
        bar[1].vy += 20;
        bar[2].vy += 20;
        bar[3].vy += 20;
    }
    ui_draw_text(VS_MESSAGE(8), 296, 64, &sp->style[0]);
    sp->unk238.x = 396;
    sp->unk238.y = 84;
    sp->unk238.w = 56;
    sp->unk238.h = 28;
    if (DAT_80063330[0] != -1) {
        sp->style[0].cd |= 0x80;
        ui_draw_text(VS_ITEM_NAME(DAT_80063330[0]), 294, 116, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        ui_draw_image(&sp->unk238, &tex, NULL, NULL, 16);
    }
    sp->unk224.x = 280;
    sp->unk224.y = 60;
    sp->unk224.w = 292;
    sp->unk224.h = 74;
    sp->unk220.r = 48;
    sp->unk220.g = 48;
    sp->unk220.b = 48;
    ui_draw_rect_outline(&sp->unk224, &sp->unk220, 513, 1);
    ui_add_draw_mode(1, 0, 0);
    ui_draw_text(VS_MESSAGE(9), 296, 144, &sp->style[0]);
    sp->unk238.y = 164;
    tex.vy += 28;
    if (DAT_80063334 != -1) {
        sp->style[0].cd |= 0x80;
        ui_draw_text(VS_ITEM_NAME(DAT_80063334), 294, 196, &sp->style[0]);
        sp->style[0].cd &= 0x7f;
        ui_draw_image(&sp->unk238, &tex, NULL, NULL, 16);
    }
    sp->unk224.y = 140;
    ui_draw_rect_outline(&sp->unk224, &sp->unk220, 513, 1);
    ui_add_draw_mode(1, 0, 0);
}
