/*
 * ST.EXE: the status screen frame (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/menu.h>
#include <st/vis.h>

SDATA(0x80087f88, 0x10)

/* Unreferenced: four zero bytes before this unit's constants. */
DATA(0x80087f84, 0x4, ".sdata")
static s32 DAT_80087f84 = 0;

/* The status screen frame: page toggles, colours, arrows and pages. */
ADDRESS(0x8002fc4c, 0x200)
void func_8002fc4c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT unused[2];
    RECT left = {310, 119, 16, 8};
    RECT right = {630, 119, 16, 8};

    func_800174c4();
    if (DAT_801cd180.unk4 & 0x800)
        DAT_800881aa = 1;
    else if (DAT_801cd180.unk4 & 0x400)
        DAT_800881aa = 0;
    DAT_801cd180.unk4 = 0;
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
    if (DAT_80088192) {
        if (DAT_800881aa == 0)
            func_8002916c(&right, &sp->unk32C, 0);
        else
            func_8002916c(&left, &sp->unk32C, 2048);
    }
    if (sp->unk2DC == 2) {
        if (sp->unk2C7 != 10)
            func_8002f498();
        else
            func_8002f97c();
    } else {
        func_8002e4d0();
    }
    if (DAT_800881aa == 0)
        func_8002ed98();
    else
        func_8002e94c();
    func_8002e490();
    func_8002c9e8();
    func_80017680();
}
