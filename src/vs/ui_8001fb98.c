/*
 * VS.EXE: a two-choice menu caption (WIP module). VS's copy of ST.EXE's
 * func_8002c1a4 (src/st/ui.c); VS highlights with style 1, not 2.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>

ADDRESS(0x8001fb98, 0x114)
void func_8001fb98(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    if (sp->unk0.half[1] == 0) {
        func_8001d8b8(VS_MESSAGE(311), 32, 74, &sp->style[0]);
        if (sp->unk1C6) {
            func_8001d8b8(VS_MESSAGE(325), 100, 160, &sp->style[1]);
            func_8001d8b8(VS_MESSAGE(326), 170, 160, &sp->style[0]);
        } else {
            func_8001d8b8(VS_MESSAGE(325), 100, 160, &sp->style[0]);
            func_8001d8b8(VS_MESSAGE(326), 170, 160, &sp->style[1]);
        }
    } else {
        func_8001d8b8(VS_MESSAGE(312), 32, 74, &sp->style[0]);
        func_8001d8b8(VS_MESSAGE(332), 40, 100, &sp->style[0]);
    }
}
