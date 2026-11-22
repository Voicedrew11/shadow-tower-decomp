/*
 * VS.EXE: three menu captions and the menu page frame (WIP module). VS's
 * copies of ST.EXE's card_draw_transfer, card_draw_checking, card_draw_message and
 * card_screen_draw (src/st/render.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>
#include <vs/funcs.h>

ADDRESS(0x80020384, 0x9c)
void card_draw_transfer(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    if (sp->unk1CC & 0xff)
        ui_draw_text(VS_MESSAGE(330), 32, 74, &sp->style[0]);
    else
        ui_draw_text(VS_MESSAGE(331), 32, 74, &sp->style[0]);
    ui_draw_text(VS_MESSAGE(332), 40, 100, &sp->style[0]);
}

ADDRESS(0x80020420, 0x40)
void card_draw_finished(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    ui_draw_text(VS_MESSAGE(340), 32, 74, &sp->style[0]);
}

ADDRESS(0x80020460, 0x50)
void card_draw_message(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    ui_draw_text(VS_MESSAGE(sp->unk0.menu.half[0]), 60, 114, &sp->style[0]);
}

/* The menu page frame: title tabs, the page body by page bit, the frame.
 * Unlike ST.EXE's copy it skips the tabs on the first page and always
 * draws the trailing panel. */
ADDRESS(0x800204b0, 0x324)
void card_screen_draw(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    gfx_begin_frame();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[1].r = 98;
    sp->style[1].g = 66;
    sp->style[1].b = 48;
    sp->style[2].r = 96;
    sp->style[2].b = sp->style[2].g = 32;
    sp->style[0].cd = sp->style[1].cd = sp->style[2].cd = 57;
    if (sp->unk1D4 < 16)
        sp->unk248 = 8 - sp->unk1D4;
    else
        sp->unk248 = sp->unk1D4 - 24;
    sp->unk214.b = 0;
    sp->unk214.r = (u8)sp->unk248 * 4 + 96;
    sp->unk214.g = (u8)sp->unk248 * 2 + 48;
    if (sp->unk1CC != 0x2 && sp->unk1CC != 0x200) {
        sp->unk240.vx = 896;
        sp->unk244.vx = 544;
        sp->unk244.vy = 511;
        sp->unk224.x = 80;
        sp->unk224.y = 45;
        sp->unk224.w = 40;
        sp->unk224.h = 16;
        sp->unk240.vy = 512;
        ui_draw_image(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
        sp->unk224.y = 44;
        if (sp->unk1CC & 0xff) {
            sp->unk240.vx = 906;
            sp->unk224.x = 32;
        } else {
            sp->unk240.vx = 934;
            sp->unk224.x = 32;
        }
        sp->unk224.w = 40;
        ui_draw_image(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    }
    switch (sp->unk1CC) {
    case 0x2:
    case 0x200:
        card_draw_port_choice();
        break;
    case 0x4:
    case 0x400:
        card_draw_checking();
        break;
    case 0x8:
    case 0x800:
        card_draw_format_prompt();
        break;
    case 0x10:
    case 0x1000:
        card_draw_file_list();
        break;
    case 0x20:
    case 0x2000:
        card_draw_transfer();
        break;
    case 0x40:
    case 0x4000:
        card_draw_finished();
        break;
    case 0x80:
    case 0x8000:
        card_draw_message();
        break;
    }
    ui_draw_panel(16, 36, 288, 192);
    ui_draw_window(16, 36, 288, 192);
    menu_draw_background_lowres();
    gfx_flip();
    gfx_draw_ot();
}
