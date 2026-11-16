/*
 * ST.EXE: the first menu page bodies (WIP module); func_8002ca84 calls one
 * per page bit.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/menu.h>

ADDRESS(0x8002c050, 0x40)
void func_8002c050(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(310), 32, 74, &sp->style[0]);
}

ADDRESS(0x8002c090, 0x114)
void func_8002c090(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short i;

    sp->unk330.x = 94;
    sp->unk330.w = 138;
    sp->unk330.h = 17;
    if (sp->unk1F8 == 0)
        sp->unk330.y = 108;
    else
        sp->unk330.y = 132;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    for (i = 0; i < 7; i++) {
        DAT_8017beb0[i] = 0;
        DAT_800881f0[i] = 0;
    }
    func_8002732c(MESSAGE(320), 102, 110, &sp->style[0]);
    func_8002732c(MESSAGE(321), 102, 134, &sp->style[0]);
}

ADDRESS(0x8002c1a4, 0x114)
void func_8002c1a4(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (sp->unk0.half[1] == 0) {
        func_8002732c(MESSAGE(311), 32, 74, &sp->style[0]);
        if (sp->unk202) {
            func_8002732c(MESSAGE(325), 121, 160, &sp->style[2]);
            func_8002732c(MESSAGE(326), 191, 160, &sp->style[0]);
        } else {
            func_8002732c(MESSAGE(325), 121, 160, &sp->style[0]);
            func_8002732c(MESSAGE(326), 191, 160, &sp->style[2]);
        }
    } else {
        func_8002732c(MESSAGE(312), 32, 74, &sp->style[0]);
        func_8002732c(MESSAGE(332), 40, 100, &sp->style[0]);
    }
}
