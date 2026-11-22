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

    if (sp->mode & 0xff)
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

    ui_draw_text(VS_MESSAGE(sp->state.menu.half[0]), 60, 114, &sp->style[0]);
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
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    if (sp->mode != 0x2 && sp->mode != 0x200) {
        sp->tex_pos.vx = 896;
        sp->clut_pos.vx = 544;
        sp->clut_pos.vy = 511;
        sp->rect.x = 80;
        sp->rect.y = 45;
        sp->rect.w = 40;
        sp->rect.h = 16;
        sp->tex_pos.vy = 512;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
        sp->rect.y = 44;
        if (sp->mode & 0xff) {
            sp->tex_pos.vx = 906;
            sp->rect.x = 32;
        } else {
            sp->tex_pos.vx = 934;
            sp->rect.x = 32;
        }
        sp->rect.w = 40;
        ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    }
    switch (sp->mode) {
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
