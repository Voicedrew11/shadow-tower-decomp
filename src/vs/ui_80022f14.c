/*
 * VS.EXE: three dialog bodies and a screen frame dispatching on its page
 * number (WIP module); the frame is shaped as ST.EXE's func_80030ab8
 * (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/* func_80023114's box widths. */
RODATA(0x80011290, 0x18)
/* The dim and light text colours: 4-byte initializers, so .sdata under
 * -G8 (the light one is merged across the three dialogs). */
SDATA(0x800632c8, 0x8)

/* A two-choice dialog: its caption and the two choices, the chosen one lit
 * (page 5, or page 1 when DAT_80063228 is 1). */
ADDRESS(0x80022f14, 0x200)
void func_80022f14(int page)
{
    CVECTOR dim = {48, 48, 48, 73};
    CVECTOR light = {96, 96, 96, 73};
    int msg[3];
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    switch (page) {
    case 1:
        msg[0] = 20;
        msg[1] = 21;
        msg[2] = 22;
        break;
    case 5:
    case 6:
        msg[0] = 10;
        msg[1] = 11;
        msg[2] = 12;
        break;
    }
    sp->style[0].r = sp->style[3].r = 60;
    sp->style[0].g = sp->style[3].g = 60;
    sp->style[0].b = sp->style[3].b = 60;
    func_8001d8b8(VS_MESSAGE(msg[0]), 128, 104, &light);
    if (page == 5 || (page == 1 && DAT_80063228 == page)) {
        func_8001d8b8(VS_MESSAGE(msg[1]), 200, 122, &dim);
        func_8001d8b8(VS_MESSAGE(msg[2]), 320, 122, &light);
    } else {
        func_8001d8b8(VS_MESSAGE(msg[1]), 200, 122, &light);
        func_8001d8b8(VS_MESSAGE(msg[2]), 320, 122, &dim);
    }
    func_8001f23c(120, 100, 346, 40);
    func_8001e890(1, 0, 0);
}

/* A one-line dialog: message 21 + page in a box sized for it. */
ADDRESS(0x80023114, 0x158)
void func_80023114(int page)
{
    CVECTOR light = {96, 96, 96, 73};
    int h = 0;
    s16 widths[12] = {292, 292, 292, 238, 238, 222, 492, 492, 0, 368, 323, 266};
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->style[0].r = sp->style[3].r = 60;
    sp->style[0].g = sp->style[3].g = 60;
    sp->style[0].b = sp->style[3].b = 60;
    if (page == 10)
        h = 15;
    func_8001d8b8(VS_MESSAGE(21 + page), 128, 115, &light);
    if (page == 10)
        h = 20;
    func_8001f23c(120, 110, widths[page], h + 20);
    func_8001e890(1, 0, 0);
}

/* A one-line dialog: message 26. */
ADDRESS(0x8002326c, 0x9c)
void func_8002326c(void)
{
    CVECTOR light = {96, 96, 96, 73};
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->style[0].r = sp->style[3].r = 60;
    sp->style[0].g = sp->style[3].g = 60;
    sp->style[0].b = sp->style[3].b = 60;
    func_8001d8b8(VS_MESSAGE(26), 108, 115, &light);
    func_8001f23c(100, 110, 214, 20);
    func_8001e890(1, 0, 0);
}

/* The page frame: dispatches the dialog bodies by page number. */
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
