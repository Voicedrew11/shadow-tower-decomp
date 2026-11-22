/*
 * VS.EXE: a screen frame with the emblem panel (WIP module), shaped as
 * ST.EXE's func_80030ab8 (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>
#include <vs/funcs.h>

/* The two unused initializers: 8 and 4 bytes, so .sdata under -G8. */
SDATA(0x800632b8, 0xc)

ADDRESS(0x800228d8, 0x128)
void func_800228d8(int page)
{
    RECT unused = {319, 110, 24, 16};
    CVECTOR unused2 = {16, 48, 16};
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
    if (sp->unk1CC & 0x8000)
        func_80022794();
    func_80021cfc();
    func_800222d8(page);
    func_8001f828();
    gfx_end_frame();
}
