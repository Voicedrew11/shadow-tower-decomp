/*
 * ST.EXE: the id grid screen frame, a VRAM clear and a framed panel (WIP
 * module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/menu.h>

SDATA(0x80088000, 0x14)

/*
 * Draws the id grid: both column blocks while the 0x1 page bit is set,
 * otherwise the block under the cursor with its detail panel.
 */
ADDRESS(0x80031aec, 0x210)
void func_80031aec(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT arrow = {319, 110, 24, 16};
    CVECTOR color = {16, 48, 16, 0};

    func_800174c4();
    sp->style[0].r = sp->style[4].r = 96;
    sp->style[0].g = sp->style[4].g = 96;
    sp->style[0].b = sp->style[4].b = 96;
    sp->style[0].cd = 73;
    sp->style[4].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    if (sp->unk2DC & 0x8000)
        func_800319a8();
    DAT_800881ae = 1;
    if (sp->unk2DC & 0x1) {
        func_80030be4();
        func_80030fa0();
    } else {
        if ((short)(sp->unk2E0 % 8) < 4)
            func_80030be4();
        else
            func_80030fa0();
        func_80031674();
    }
    DAT_800881ae = 0;
    if ((short)(sp->unk2E0 % 8) < 4)
        func_8002916c(&arrow, &color, 0);
    else
        func_8002916c(&arrow, &color, 2048);
    func_800286a8(1, 0, 0);
    func_80031378();
    func_8002c9e8();
    func_80017680();
}

/* Clears the 640x480 VRAM area from the origin to black. */
ADDRESS(0x80031cfc, 0x54)
void func_80031cfc(void)
{
    RECT r = {0, 0, 640, 480};

    ClearImage(&r, 0, 0, 0);
}

ADDRESS(0x80031d50, 0x94)
void func_80031d50(void)
{
    func_800174c4();
    func_800175a4();
    func_8002948c();
    func_80021518(120, 120, 200, 136, 0, 144, 80, 16, 255, 25, 32738, 96, 96, 96, 0);
    func_80017634();
}
