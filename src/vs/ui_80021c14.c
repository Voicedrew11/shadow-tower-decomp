/*
 * VS.EXE: a two-page screen frame (WIP module), shaped as ST.EXE's
 * menu_creature_book_draw (src/st/ui.c).
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
    if (sp->unk1D4 < 16)
        sp->unk248 = 8 - sp->unk1D4;
    else
        sp->unk248 = sp->unk1D4 - 24;
    sp->unk214.b = 0;
    sp->unk214.r = (u8)sp->unk248 * 4 + 96;
    sp->unk214.g = (u8)sp->unk248 * 2 + 48;
    if (sp->unk1CC == 1)
        menu_creature_select_draw_grid(page);
    else
        menu_creature_book_draw_entry(page);
    menu_draw_background();
    gfx_end_frame();
}
