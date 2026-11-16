/*
 * VS.EXE: two background strip draws and a menu caption (WIP module). VS's
 * copies of ST.EXE's func_8002c9e8 and func_8002c050 (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>

ADDRESS(0x8001f828, 0x9c)
void func_8001f828(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int n;

    sp->unk238.w = 128;
    sp->unk238.h = 240;
    sp->unk240.vx = 768;
    sp->unk238.y = 0;
    sp->unk238.x = 0;
    sp->unk240.vy = 256;
    n = 5;
    while (n--) {
        func_8001e530(&sp->unk238, &sp->unk240, 0, 0, 16);
        sp->unk238.x += 128;
    }
}

/* As func_8001f828, three strips wide. */
ADDRESS(0x8001f8c4, 0x9c)
void func_8001f8c4(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int n;

    sp->unk238.w = 128;
    sp->unk238.h = 240;
    sp->unk240.vx = 768;
    sp->unk238.y = 0;
    sp->unk238.x = 0;
    sp->unk240.vy = 256;
    n = 3;
    while (n--) {
        func_8001e530(&sp->unk238, &sp->unk240, 0, 0, 16);
        sp->unk238.x += 128;
    }
}

ADDRESS(0x8001f960, 0x40)
void func_8001f960(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    func_8001d8b8(VS_MESSAGE(310), 32, 74, &sp->style[0]);
}
