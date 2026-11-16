/*
 * ST.EXE: the pause menu's entry, exit and page loop (WIP module,
 * 0x80058c04..0x80059204): the menu image and message loads, the screen
 * reset, the equipment models reload, and the loop that runs the eight
 * pages. -G8: func_80058cc8's RECT initializer is a .sdata literal.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/scratch.h>
#include <st/menu.h>
#include <st/hud.h>
#include <st/area.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/sound.h>
#include <st/menu_main.h>

SDATA(0x800880a4, 0x8)

ADDRESS(0x80058c04, 0x20)
void func_80058c04(void)
{
    func_80026e04();
}

ADDRESS(0x80058c24, 0x28)
void func_80058c24(void)
{
    func_800595d4();
    func_80026e90();
}

/* Loads member 0 of archive 3: the menu messages, then the menu image. */
ADDRESS(0x80058c4c, 0x7c)
void func_80058c4c(void)
{
    u_long *image;

    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &MENU_SCRATCH->unk2E6[0], &MENU_SCRATCH->unk2E6[1], 1);
}

ADDRESS(0x80058cc8, 0x68)
void func_80058cc8(void)
{
    RECT rect = {576, 384, 64, 16};
    DVECTOR size = {576, 480};

    ResetGraph(1);
    func_80021158();
    func_800214a4();
}

/* Reloads the menu and the viewer's equipment models after the menu. */
ADDRESS(0x80058d30, 0x94)
void func_80058d30(void)
{
    func_80021158();
    func_80058c4c();
    func_80044404();
    func_8003f12c(viewer.unk38[3], 0, 1);
    func_8003f12c(viewer.unk38[16], 1, 1);
    func_8003f12c(viewer.unk38[4], 2, 1);
    func_8003f12c(viewer.unk38[17], 3, 1);
    func_8003ee38(viewer.unk38[6], 0);
    func_8003ee38(viewer.unk38[12], 1);
    func_800214a4();
}

/* Opens a menu screen: the VRAM positions, the menu loads and the HUD. */
ADDRESS(0x80058dc4, 0x110)
void func_80058dc4(int arg0, int arg1)
{
    MenuScratch *sp = MENU_SCRATCH;
    u_long *image;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk2E6[4].vx = 768;
    sp->unk2E6[4].vy = 256;
    func_80015ad4();
    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    func_80058c04();
    sp->unk2E5 = 0;
    SCRATCHPAD->unk208 = 0;
    if (arg0)
        sp->unk2DC = 256;
    else
        sp->unk2DC = 1;
    DrawSync(0);
}

ADDRESS(0x80058ed4, 0x20)
void func_80058ed4(void)
{
    func_80058c24();
}

/* The pause menu: runs the selected page until one closes the menu. */
ADDRESS(0x80058ef4, 0x310)
void func_80058ef4(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u_long *image;

    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk2E6[4].vx = 768;
    sp->unk2E6[4].vy = 256;
    func_80015ad4();
    image = DAT_80088c88;
    func_80016030(3, 0, image);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(image + 1), 5376);
    image = (u_long *)((u8 *)image + (DAT_80088c88[0] + 4));
    func_8003ec5c(image + 1, &sp->unk2E6[0], &sp->unk2E6[1], 1);
    func_80058c04();
    sp->unk2DE = 0;
    func_80057a3c();
    DrawSync(0);
    for (;;) {
        if (DAT_801cd180.unk4 & 0x2000) {
            func_80057cec();
            DAT_801cd180.unk4 = 0;
            break;
        }
        if (DAT_801cd180.unk4 & 0x1000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            sp->unk2E5 = 0;
            func_80060018();
        }
        if (DAT_801cd180.unk4 & 0x4000) {
            func_80057cc8();
            DAT_801cd180.unk4 = 0;
            if ((s16)sp->unk2DE == 0) {
                sp->unk2E5 = 0;
                if (func_8005ec10() == 1)
                    break;
            } else if ((s16)sp->unk2DE == 1) {
                sp->unk2E5 = 0;
                func_80064ff0();
            } else if ((s16)sp->unk2DE == 2) {
                viewer.unk21D = 1;
                sp->unk2E5 = 1;
                if (DAT_801cd180.unk2 & 0x800)
                    func_8005d25c();
                else
                    func_800675f4();
            } else if ((s16)sp->unk2DE == 3) {
                viewer.unk21D = 1;
                sp->unk2E5 = 1;
                func_8005c7fc();
            } else if ((s16)sp->unk2DE == 4) {
                viewer.unk21D = 1;
                sp->unk2E5 = 1;
                func_8005ad00();
            } else if ((s16)sp->unk2DE == 5) {
                sp->unk2E5 = 0;
                func_80060334();
            } else if ((s16)sp->unk2DE == 6) {
                sp->unk2DC = 1;
                sp->unk2E5 = 0;
                sp->unk206 = 1;
                if (func_80062c28(1) == 1)
                    break;
            } else if ((s16)sp->unk2DE == 7) {
                sp->unk2E5 = 0;
                if (func_80063078() == 1)
                    break;
            }
        }
        if (DAT_801cd180.unkC & 0x40) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2DE++;
            if ((s16)sp->unk2DE >= 8)
                sp->unk2DE = 0;
        }
        if (DAT_801cd180.unkC & 0x10) {
            func_80057d10();
            DAT_801cd180.unkC = 0;
            sp->unk2DE--;
            if ((s16)sp->unk2DE < 0)
                sp->unk2DE = 7;
        }
        sp->unk2E5 = 0;
        func_800164b4();
        func_80029b74();
    }
    func_80057a3c();
    func_80058c24();
}
