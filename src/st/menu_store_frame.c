/*
 * ST.EXE: the stored-item screen frame (WIP module): colours, cursor blink,
 * then the grid page or the page func_8003025c draws.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/menu.h>

SDATA(0x80087fb8, 0x10)

ADDRESS(0x80030ab8, 0x12c)
void func_80030ab8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused;
    RECT left = {314, 119, 16, 8};
    RECT right = {625, 119, 16, 8};

    func_800174c4();
    sp->style[0].r = sp->style[4].r = 80;
    sp->style[0].g = sp->style[4].g = 80;
    sp->style[0].b = sp->style[4].b = 80;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (sp->unk2DC == 2)
        func_8003025c();
    else
        func_8002fe4c();
    func_8002c9e8();
    func_80017680();
}
