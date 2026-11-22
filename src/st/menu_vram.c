/*
 * ST.EXE: saving and restoring the VRAM the equip screen draws over (WIP
 * module). Its own translation unit: its RECT literals
 * (.sdata 0x800880e4..0x8008810c) repeat src/st/menu.c's {896, 256, 64, 256}
 * and {896, 0, 64, 256} instead of merging with them.
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
void menu_equip_save_vram(void)
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
void menu_equip_restore_vram(void)
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
 * Opens the equip screen (the pause menu's "equip" row): saves the VRAM it uses, loads the menu
 * messages (CD archive 3 member 3) into a buffer of their own, its font
 * image, and a 144-byte block (member 1), and records the equipment ids.
 */
ADDRESS(0x800637c0, 0x194)
void menu_equip_open(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT page = {896, 256, 64, 256};
    u8 *p;

    menu_equip_save_vram();
    sp->unk2DC = 1;
    sp->unk2E6[0].vx = 960;
    sp->unk2E6[0].vy = 256;
    sp->unk2E6[1].vx = 560;
    sp->unk2E6[1].vy = 495;
    sp->unk2E6[2].vx = 896;
    sp->unk2E6[2].vy = 256;
    sp->unk2E6[3].vx = 560;
    sp->unk2E6[3].vy = 497;
    sp->unk0.block = mem_alloc(21504);
    DAT_801e0d08.bits = mem_alloc(144);
    p = (u8 *)DAT_80088c88;
    cd_read_member(3, 1, (u_long *)p);
    p += *(u32 *)p + 4;
    p += *(u32 *)p + 4;
    copy_words(DAT_801e0d08.bits, (u32 *)(p + 4), 36);
    p = (u8 *)DAT_80088c88;
    cd_read_member(3, 3, (u_long *)p);
    copy_words(sp->unk0.block, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    tim_load_images_at(p + 4, &sp->unk2E6[2], &sp->unk2E6[3], 1);
    DAT_801e0d28[0] = viewer.unk38[3];
    DAT_801e0d28[1] = viewer.unk38[4];
    DAT_801e0d28[2] = viewer.unk38[16];
    DAT_801e0d28[3] = viewer.unk38[17];
    DAT_801e0d28[4] = viewer.unk38[6];
    DAT_801e0d28[5] = viewer.unk38[12];
    pad_clear_input();
}

/*
 * Closes them: restores the VRAM, frees both buffers and reloads the
 * equipment models whose ids changed.
 */
ADDRESS(0x80063954, 0x288)
void menu_equip_close(void)
{
    RECT a = {896, 0, 64, 256};
    RECT b;
    RECT c = {159, 130, 16, 8};
    RECT d = {296, 130, 16, 8};

    menu_equip_restore_vram();
    mem_free(MENU_SCRATCH->unk0.block);
    mem_free(DAT_801e0d08.bits);
    if ((DAT_801e0d28[0] != viewer.unk38[3] && viewer.unk38[3] != 0xffff)
        || (DAT_801e0d28[2] != viewer.unk38[16] && viewer.unk38[16] != 0xffff)
        || (DAT_801e0d28[1] != viewer.unk38[4] && viewer.unk38[4] != 0xffff)
        || (DAT_801e0d28[3] != viewer.unk38[17] && viewer.unk38[17] != 0xffff)
        || (DAT_801e0d28[4] != viewer.unk38[6] && viewer.unk38[6] != 0xffff)
        || (DAT_801e0d28[5] != viewer.unk38[12] && viewer.unk38[12] != 0xffff))
        loading_screen_show();
    if (DAT_801e0d28[0] != viewer.unk38[3] || DAT_801e0d28[1] != viewer.unk38[4]) {
        player_load_equipment_model(viewer.unk38[3], 0, 1);
        player_load_equipment_model(viewer.unk38[4], 2, 1);
    }
    if (DAT_801e0d28[2] != viewer.unk38[16] || DAT_801e0d28[3] != viewer.unk38[17]) {
        player_load_equipment_model(viewer.unk38[16], 1, 1);
        player_load_equipment_model(viewer.unk38[17], 3, 1);
    }
    if (DAT_801e0d28[4] != viewer.unk38[6])
        player_load_magic(viewer.unk38[6], 0);
    if (DAT_801e0d28[5] != viewer.unk38[12])
        player_load_magic(viewer.unk38[12], 1);
    loading_screen_end();
    viewer.unk38[15] = 0xffff;
}
