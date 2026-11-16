/*
 * VS.EXE: three menu captions (WIP module). VS's copies of ST.EXE's
 * func_8002c8bc, func_8002c050 and func_8002c998 (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>

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

    func_8001d8b8(VS_MESSAGE(sp->unk0.half[0]), 60, 114, &sp->style[0]);
}
