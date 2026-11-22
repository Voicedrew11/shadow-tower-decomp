/*
 * VS.EXE: a two-page screen frame (WIP module), shaped as ST.EXE's
 * menu_creature_book_draw (src/st/render.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>
#include <vs/funcs.h>

ADDRESS(0x80021c14, 0xe8)
void menu_creature_select_draw(int page)
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
    if (sp->mode == 1)
        menu_creature_select_draw_grid(page);
    else
        menu_creature_book_draw_entry(page);
    menu_draw_background();
    gfx_end_frame();
}
