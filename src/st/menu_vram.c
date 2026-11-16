/*
 * ST.EXE: saving and restoring the VRAM the memory card and option screens
 * draw over (WIP module; one translation unit with src/st/card.c and the
 * screens before it: GCC merged their small RECT constants).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/gfx.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/equip_menu.h>
#include <st/memory.h>
#include <st/cd.h>
#include <st/funcs.h>
#include <st/card.h>

SDATA(0x800880e4, 0x28)

/*
 * Shrinks the packet buffers and stores the two VRAM areas the screens
 * overwrite (the texture page at 896,256 and the area at 448,256) above
 * the second one.
 */
ADDRESS(0x80063638, 0xc0)
void func_80063638(void)
{
    RECT page = {896, 256, 64, 256};
    RECT area = {448, 256, 128, 128};

    DrawSync(0);
    world_state.packets[1].start = world_state.packets[0].end = world_state.packets[0].start + 0x1500;
    world_state.packets[1].end = world_state.packets[1].start + 0x1500;
    StoreImage(&page, world_state.packets[1].end);
    StoreImage(&area, world_state.packets[1].end + 0x2000);
}

/* Loads the two areas back and restores the packet buffers. */
ADDRESS(0x800636f8, 0xc8)
void func_800636f8(void)
{
    RECT page = {896, 256, 64, 256};
    RECT area = {448, 256, 128, 128};

    LoadImage(&page, world_state.packets[1].end);
    LoadImage(&area, world_state.packets[1].end + 0x2000);
    DrawSync(0);
    world_state.packets[1].start = world_state.packets[0].end = world_state.packets[0].start + 0x3500;
    world_state.packets[1].end = world_state.packets[1].start + 0x3500;
}

/*
 * Opens the option screens: saves the VRAM they use, loads the menu
 * messages (CD archive 3 member 3) into a buffer of their own, its font
 * image, and a 144-byte block (member 1), and records the equipment ids.
 */
ADDRESS(0x800637c0, 0x194)
void func_800637c0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT page = {896, 256, 64, 256};
    u8 *p;

    func_80063638();
    sp->unk2DC = 1;
    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk0.block = func_800152d0(21504);
    DAT_801e0d08.bits = func_800152d0(144);
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 1, (u_long *)p);
    p += *(u32 *)p + 4;
    p += *(u32 *)p + 4;
    func_80014cfc(DAT_801e0d08.bits, (u32 *)(p + 4), 36);
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 3, (u_long *)p);
    func_80014cfc(sp->unk0.block, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    func_8003ec5c(p + 4, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    DAT_801e0d28[0] = viewer.unk38[3];
    DAT_801e0d28[1] = viewer.unk38[4];
    DAT_801e0d28[2] = viewer.unk38[16];
    DAT_801e0d28[3] = viewer.unk38[17];
    DAT_801e0d28[4] = viewer.unk38[6];
    DAT_801e0d28[5] = viewer.unk38[12];
    func_80057a3c();
}

/*
 * Closes them: restores the VRAM, frees both buffers and reloads the
 * equipment models whose ids changed.
 */
ADDRESS(0x80063954, 0x288)
void func_80063954(void)
{
    RECT a = {896, 0, 64, 256};
    RECT b;
    RECT c = {159, 130, 16, 8};
    RECT d = {296, 130, 16, 8};

    func_800636f8();
    func_800152f8(MENU_SCRATCH->unk0.block);
    func_800152f8(DAT_801e0d08.bits);
    if ((DAT_801e0d28[0] != viewer.unk38[3] && viewer.unk38[3] != 0xffff)
        || (DAT_801e0d28[2] != viewer.unk38[16] && viewer.unk38[16] != 0xffff)
        || (DAT_801e0d28[1] != viewer.unk38[4] && viewer.unk38[4] != 0xffff)
        || (DAT_801e0d28[3] != viewer.unk38[17] && viewer.unk38[17] != 0xffff)
        || (DAT_801e0d28[4] != viewer.unk38[6] && viewer.unk38[6] != 0xffff)
        || (DAT_801e0d28[5] != viewer.unk38[12] && viewer.unk38[12] != 0xffff))
        func_80021158();
    if (DAT_801e0d28[0] != viewer.unk38[3] || DAT_801e0d28[1] != viewer.unk38[4]) {
        func_8003f12c(viewer.unk38[3], 0, 1);
        func_8003f12c(viewer.unk38[4], 2, 1);
    }
    if (DAT_801e0d28[2] != viewer.unk38[16] || DAT_801e0d28[3] != viewer.unk38[17]) {
        func_8003f12c(viewer.unk38[16], 1, 1);
        func_8003f12c(viewer.unk38[17], 3, 1);
    }
    if (DAT_801e0d28[4] != viewer.unk38[6])
        func_8003ee38(viewer.unk38[6], 0);
    if (DAT_801e0d28[5] != viewer.unk38[12])
        func_8003ee38(viewer.unk38[12], 1);
    func_800214a4();
    viewer.unk38[15] = 0xffff;
}
