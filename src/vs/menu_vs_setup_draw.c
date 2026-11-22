/*
 * VS.EXE: three dialog bodies and a screen frame dispatching on its page
 * number (WIP module); the frame is shaped as ST.EXE's menu_creature_book_draw
 * (src/st/render.c).
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

/* menu_vs_setup_draw_notice's box widths. */
RODATA(0x80011290, 0x18)
/* The red and light text colours: 4-byte initializers, so .sdata under
 * -G8 (the light one is merged across the three dialogs). */
SDATA(0x800632c8, 0x8)

/* A two-choice dialog: its caption and the two choices, the chosen one red
 * (page 5, or page 1 when round_count is 1). */
ADDRESS(0x80022f14, 0x200)
void menu_vs_setup_draw_choice(int page)
{
    CVECTOR red = {200, 50, 50, 73};
    CVECTOR light = {96, 96, 96, 73};
    int msg[3];
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    switch (page) {
    case 1:
        msg[0] = 20;
        msg[1] = 21;
        msg[2] = 22;
        break;
    case 5:
    case 6:
        msg[0] = 10;
        msg[1] = 11;
        msg[2] = 12;
        break;
    }
    sp->style[0].r = sp->style[3].r = 60;
    sp->style[0].g = sp->style[3].g = 60;
    sp->style[0].b = sp->style[3].b = 60;
    ui_draw_text(VS_MESSAGE(msg[0]), 128, 104, &light);
    if (page == 5 || (page == 1 && round_count == page)) {
        ui_draw_text(VS_MESSAGE(msg[1]), 200, 122, &red);
        ui_draw_text(VS_MESSAGE(msg[2]), 320, 122, &light);
    } else {
        ui_draw_text(VS_MESSAGE(msg[1]), 200, 122, &light);
        ui_draw_text(VS_MESSAGE(msg[2]), 320, 122, &red);
    }
    ui_draw_panel(120, 100, 346, 40);
    ui_add_draw_mode(1, 0, 0);
}

/* A one-line dialog: message 21 + page in a box sized for it. */
ADDRESS(0x80023114, 0x158)
void menu_vs_setup_draw_notice(int page)
{
    CVECTOR light = {96, 96, 96, 73};
    int h = 0;
    s16 widths[12] = {292, 292, 292, 238, 238, 222, 492, 492, 0, 368, 323, 266};
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->style[0].r = sp->style[3].r = 60;
    sp->style[0].g = sp->style[3].g = 60;
    sp->style[0].b = sp->style[3].b = 60;
    if (page == 10)
        h = 15;
    ui_draw_text(VS_MESSAGE(21 + page), 128, 115, &light);
    if (page == 10)
        h = 20;
    ui_draw_panel(120, 110, widths[page], h + 20);
    ui_add_draw_mode(1, 0, 0);
}

/* A one-line dialog: message 26. */
ADDRESS(0x8002326c, 0x9c)
void menu_vs_setup_draw_no_item_notice(void)
{
    CVECTOR light = {96, 96, 96, 73};
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->style[0].r = sp->style[3].r = 60;
    sp->style[0].g = sp->style[3].g = 60;
    sp->style[0].b = sp->style[3].b = 60;
    ui_draw_text(VS_MESSAGE(26), 108, 115, &light);
    ui_draw_panel(100, 110, 214, 20);
    ui_add_draw_mode(1, 0, 0);
}

/* The page frame: dispatches the dialog bodies by page number. */
ADDRESS(0x80023308, 0x154)
void menu_vs_setup_draw(int page)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    gfx_begin_frame();
    sp->style[0].r = sp->style[3].r = 96;
    sp->style[0].g = sp->style[3].g = 96;
    sp->style[0].b = sp->style[3].b = 96;
    sp->style[0].cd = 73;
    sp->style[3].cd = 49;
    if (sp->blink_phase < 16)
        sp->blink_level = 8 - sp->blink_phase;
    else
        sp->blink_level = sp->blink_phase - 24;
    sp->cursor_color.b = 0;
    sp->cursor_color.r = (u8)sp->blink_level * 4 + 96;
    sp->cursor_color.g = (u8)sp->blink_level * 2 + 48;
    switch (page) {
    case 1:
    case 5:
    case 6:
        menu_vs_setup_draw_choice(page);
        break;
    case 2:
    case 3:
    case 4:
    case 9:
    case 10:
    case 11:
        menu_vs_setup_draw_notice(page);
        break;
    case 99:
        menu_vs_setup_draw_no_item_notice();
        break;
    }
    menu_vs_setup_draw_options();
    ui_draw_panel(24, 16, 592, 208);
    menu_draw_background();
    gfx_end_frame();
}
