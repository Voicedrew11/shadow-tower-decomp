/*
 * VS.EXE: three menu captions and the menu page frame (WIP module). VS's
 * copies of ST.EXE's func_8002c8bc, func_8002c050, func_8002c998 and
 * func_8002ca84 (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>
#include <vs/funcs.h>

ADDRESS(0x80020384, 0x9c)
void func_80020384(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    if (sp->unk1CC & 0xff)
        func_8001d8b8(VS_MESSAGE(330), 32, 74, &sp->style[0]);
    else
        func_8001d8b8(VS_MESSAGE(331), 32, 74, &sp->style[0]);
    func_8001d8b8(VS_MESSAGE(332), 40, 100, &sp->style[0]);
}

ADDRESS(0x80020420, 0x40)
void func_80020420(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    func_8001d8b8(VS_MESSAGE(340), 32, 74, &sp->style[0]);
}

ADDRESS(0x80020460, 0x50)
void func_80020460(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    func_8001d8b8(VS_MESSAGE(sp->unk0.menu.half[0]), 60, 114, &sp->style[0]);
}

/* The menu page frame: title tabs, the page body by page bit, the frame.
 * Unlike ST.EXE's copy it skips the tabs on the first page and always
 * draws the trailing panel. */
/* Not exact: in the second tab draw retail stores x = 32 before setting up
 * the call's arguments; this build stores it after them. */
ADDRESS(0x800204b0, 0x324)
void func_800204b0(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    func_8001d1b8();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[1].r = 98;
    sp->style[1].g = 66;
    sp->style[1].b = 48;
    sp->style[2].r = 96;
    sp->style[2].b = sp->style[2].g = 32;
    sp->style[0].cd = sp->style[1].cd = sp->style[2].cd = 57;
    if (sp->unk1D4 < 16)
        sp->unk248 = 8 - sp->unk1D4;
    else
        sp->unk248 = sp->unk1D4 - 24;
    sp->unk214.b = 0;
    sp->unk214.r = (u8)sp->unk248 * 4 + 96;
    sp->unk214.g = (u8)sp->unk248 * 2 + 48;
    if (sp->unk1CC != 0x2 && sp->unk1CC != 0x200) {
        sp->unk240.vx = 896;
        sp->unk244.vx = 544;
        sp->unk244.vy = 511;
        sp->unk224.x = 80;
        sp->unk224.y = 45;
        sp->unk224.w = 40;
        sp->unk224.h = 16;
        sp->unk240.vy = 512;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
        sp->unk224.y = 44;
        if (sp->unk1CC & 0xff)
            sp->unk240.vx = 906;
        else
            sp->unk240.vx = 934;
        sp->unk224.w = 40;
        sp->unk224.x = 32;
        func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    }
    switch (sp->unk1CC) {
    case 0x2:
    case 0x200:
        func_8001f9a0();
        break;
    case 0x4:
    case 0x400:
        func_8001f960();
        break;
    case 0x8:
    case 0x800:
        func_8001fb98();
        break;
    case 0x10:
    case 0x1000:
        func_8001fcac();
        break;
    case 0x20:
    case 0x2000:
        func_80020384();
        break;
    case 0x40:
    case 0x4000:
        func_80020420();
        break;
    case 0x80:
    case 0x8000:
        func_80020460();
        break;
    }
    func_8001f23c(16, 36, 288, 192);
    func_8001f30c(16, 36, 288, 192);
    func_8001f8c4();
    func_8001d298();
    func_8001d328();
}
