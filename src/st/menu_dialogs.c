/*
 * ST.EXE: the remaining menu page bodies, the page frame that dispatches
 * them by page bit, and three two-choice dialogs (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/menu.h>

ADDRESS(0x8002c8bc, 0x9c)
void func_8002c8bc(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    if (sp->unk2DC & 0xff)
        func_8002732c(MESSAGE(330), 32, 74, &sp->style[0]);
    else
        func_8002732c(MESSAGE(331), 32, 74, &sp->style[0]);
    func_8002732c(MESSAGE(332), 40, 100, &sp->style[0]);
}

ADDRESS(0x8002c958, 0x40)
void func_8002c958(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(340), 32, 74, &sp->style[0]);
}

ADDRESS(0x8002c998, 0x50)
void func_8002c998(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(sp->unk0.half[0]), 60, 114, &sp->style[0]);
}

ADDRESS(0x8002c9e8, 0x9c)
void func_8002c9e8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int n;

    sp->unk350.w = 128;
    sp->unk350.h = 240;
    sp->unk358.vx = 768;
    sp->unk350.y = 0;
    sp->unk350.x = 0;
    sp->unk358.vy = 256;
    n = 5;
    while (n--) {
        func_80028364(&sp->unk350, &sp->unk358, 0, 0, 16);
        sp->unk350.x += 128;
    }
}

ADDRESS(0x8002ca84, 0x314)
void func_8002ca84(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 96;
    sp->style[3].b = sp->style[3].g = 32;
    sp->style[0].cd = sp->style[2].cd = sp->style[3].cd = 57;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk358.vx = 576;
    sp->unk358.vy = 448;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 81;
    sp->unk33C.y = 45;
    sp->unk33C.w = 40;
    sp->unk33C.h = 16;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, 0, 3);
    sp->unk33C.y = 44;
    if (sp->unk2DC & 0xff)
        sp->unk358.vx = 586;
    else
        sp->unk358.vx = 613;
    sp->unk33C.x = 32;
    sp->unk33C.w = 40;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, 0, 3);
    switch (sp->unk2DC) {
    case 0x2:
    case 0x200:
        func_8002c090();
        break;
    case 0x4:
    case 0x400:
        func_8002c050();
        break;
    case 0x8:
    case 0x800:
        func_8002c1a4();
        break;
    case 0x10:
    case 0x1000:
        func_8002c2b8();
        break;
    case 0x20:
    case 0x2000:
        func_8002c8bc();
        break;
    case 0x40:
    case 0x4000:
        func_8002c958();
        break;
    case 0x80:
    case 0x8000:
        func_8002c998();
        break;
    }
    func_80029548(16, 36, 288, 192);
    func_800175a4();
    if (sp->unk206 == 1)
        func_8002948c();
    func_80017634();
}

ADDRESS(0x8002cd98, 0xec)
void func_8002cd98(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_8002732c(MESSAGE(400), 76, 92, &sp->style[0]);
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 24;
    if (sp->unk2E0 == 0) {
        func_8002732c(MESSAGE(401), 121, 136, &sp->style[2]);
        func_8002732c(MESSAGE(402), 189, 136, &sp->style[0]);
    } else {
        func_8002732c(MESSAGE(401), 121, 136, &sp->style[0]);
        func_8002732c(MESSAGE(402), 189, 136, &sp->style[2]);
    }
    func_80029618(40, 80, 240, 80);
}

ADDRESS(0x8002ce84, 0x74)
void func_8002ce84(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[0].cd = sp->style[2].cd = 57;
    func_8002cd98();
    func_800175a4();
    func_8002948c();
    func_80017634();
}

ADDRESS(0x8002cef8, 0x1b8)
void func_8002cef8(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    short x[2] = {16, 10};

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[0].cd = 49;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    sp->unk32C.b = 0;
    sp->unk330.x = 93;
    sp->unk330.y = sp->unk0.word * 24 + 118;
    sp->unk330.w = 134;
    sp->unk330.h = 16;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    func_8002732c(MESSAGE(800), x[0] + 97, 120, &sp->style[0]);
    func_8002732c(MESSAGE(801), x[1] + 97, 144, &sp->style[0]);
    func_80029618(72, 100, 176, 80);
    func_800175a4();
    ClearImage(&world_state.draw[world_state.id].clip, 0, 0, 0);
    func_80017634();
}

ADDRESS(0x8002d0b0, 0x12c)
void func_8002d0b0(void)
{
    MenuScratch *sp = MENU_SCRATCH;

    func_800174c4();
    sp->style[0].b = sp->style[0].g = sp->style[0].r = 96;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[0].cd = sp->style[2].cd = 57;
    func_8002732c(MESSAGE(850), 92, 120, &sp->style[0]);
    if (sp->unk2DE == 0) {
        func_8002732c(MESSAGE(851), 127, 152, &sp->style[2]);
        func_8002732c(MESSAGE(852), 183, 152, &sp->style[0]);
    } else {
        func_8002732c(MESSAGE(851), 127, 152, &sp->style[0]);
        func_8002732c(MESSAGE(852), 183, 152, &sp->style[2]);
    }
    func_80029618(72, 100, 176, 80);
    func_800175a4();
    func_80017634();
}
