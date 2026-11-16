/*
 * VS.EXE: the menu panel (WIP module). VS's copy of ST.EXE's func_80029548
 * (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>

/* Draws a menu panel: a dark fill over a lighter border, both blended. */
ADDRESS(0x8001f23c, 0xd0)
void func_8001f23c(int x, int y, int w, int h)
{
    VS_MENU_SCRATCH->unk220.g = 80;
    VS_MENU_SCRATCH->unk220.r = 80;
    VS_MENU_SCRATCH->unk220.b = 96;
    VS_MENU_SCRATCH->unk22C.g = 40;
    VS_MENU_SCRATCH->unk22C.r = 40;
    VS_MENU_SCRATCH->unk224.x = x;
    VS_MENU_SCRATCH->unk224.y = y;
    VS_MENU_SCRATCH->unk224.w = w;
    VS_MENU_SCRATCH->unk224.h = h;
    VS_MENU_SCRATCH->unk22C.b = 48;
    VS_MENU_SCRATCH->unk230.x = x + 2;
    VS_MENU_SCRATCH->unk230.y = y + 1;
    VS_MENU_SCRATCH->unk230.w = w - 4;
    VS_MENU_SCRATCH->unk230.h = h - 2;
    func_8001e3d8(&VS_MENU_SCRATCH->unk230, &VS_MENU_SCRATCH->unk22C, 1);
    func_8001e890(2, 0, 0);
    func_8001e3d8(&VS_MENU_SCRATCH->unk224, &VS_MENU_SCRATCH->unk220, 1);
    func_8001e890(0, 0, 0);
}
