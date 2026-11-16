/*
 * ST.EXE: opening the item/status menu over the game (WIP module; one
 * translation unit with the screens around it, see src/st/menu_vram.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/menu.h>
#include <st/memory.h>
#include <st/cd.h>
#include <st/funcs.h>
#include <st/card.h>

SDATA(0x800880bc, 0x10)

/*
 * Copies the texture page at 896,256 up to 896,0, then loads the menu
 * messages (CD archive 3 member 0) and their font image, and the message
 * copy and image of member 3.
 */
ADDRESS(0x8005f38c, 0x170)
void func_8005f38c(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    RECT page = {896, 256, 64, 256};
    u8 *p;

    sp->unk2E6.vx = 960;
    sp->unk2E6.vy = 256;
    sp->unk2EA.vx = 560;
    sp->unk2EA.vy = 495;
    sp->unk2EE.vx = 896;
    sp->unk2EE.vy = 256;
    sp->unk2F2.vx = 560;
    sp->unk2F2.vy = 497;
    sp->unk0.block = func_800152d0(21504);
    MoveImage(&page, 896, 0);
    func_80015ad4();
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 0, (u_long *)p);
    func_80014cfc((u32 *)&menu_messages, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    func_8003ec5c(p + 4, &sp->unk2E6, &sp->unk2EA, 1);
    p = (u8 *)DAT_80088c88;
    func_80016030(3, 3, (u_long *)p);
    func_80014cfc(sp->unk0.block, (u32 *)(p + 4), 5376);
    p += *(u32 *)p + 4;
    func_8003ec5c(p + 4, &sp->unk2EE, &sp->unk2F2, 1);
    func_80058c04();
    func_80057a3c();
}

/* Copies the saved page back to 896,256. */
ADDRESS(0x8005f4fc, 0x50)
void func_8005f4fc(void)
{
    RECT saved = {896, 0, 64, 256};

    MoveImage(&saved, 896, 256);
}
