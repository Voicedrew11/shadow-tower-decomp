/*
 * VS.EXE: a two-page screen frame (WIP module), shaped as ST.EXE's
 * func_80030ab8 (src/st/ui.c).
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
void func_80021c14(int page)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    func_8001d1b8();
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
        func_80020aa4(page);
    else
        func_80021140(page);
    func_8001f828();
    func_8001d374();
}
