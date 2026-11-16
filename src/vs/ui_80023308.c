/*
 * VS.EXE: a screen frame dispatching on its page number (WIP module),
 * shaped as ST.EXE's func_80030ab8 (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>
#include <vs/funcs.h>

ADDRESS(0x80023308, 0x154)
void func_80023308(int page)
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
    switch (page) {
    case 1:
    case 5:
    case 6:
        func_80022f14(page);
        break;
    case 2:
    case 3:
    case 4:
    case 9:
    case 10:
    case 11:
        func_80023114(page);
        break;
    case 99:
        func_8002326c();
        break;
    }
    func_80022a00();
    func_8001f23c(24, 16, 592, 208);
    func_8001f828();
    func_8001d374();
}
