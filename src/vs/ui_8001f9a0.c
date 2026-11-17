/*
 * VS.EXE: a menu page with a caption, a highlighted choice and four
 * picture tiles (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>

/*
 * Draws message 370 + unk1BC, the highlight bar over the choice unk1CC
 * selects, clears the two seven-entry tables and draws four tiles of the
 * picture at (896, 512).
 */
ADDRESS(0x8001f9a0, 0x1f8)
void func_8001f9a0(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;

    func_8001d8b8(VS_MESSAGE(370 + sp->unk1BC), 50, 60, &sp->style[0]);
    sp->unk218.x = 112;
    sp->unk218.w = 100;
    sp->unk218.h = 18;
    if (sp->unk1CC == 2)
        sp->unk218.y = 140;
    else
        sp->unk218.y = 164;
    func_8001de8c(&sp->unk218, &sp->unk214, 257, 1);
    func_8001e890(1, 0, 0);
    for (i = 0; i < 7; i++) {
        DAT_801c4a38[i] = 0;
        DAT_80117598[i] = 0;
    }
    sp->unk240.vx = 896;
    sp->unk240.vy = 512;
    sp->unk244.vx = 544;
    sp->unk244.vy = 511;
    sp->unk224.x = 166;
    sp->unk224.y = 142;
    sp->unk224.w = 40;
    sp->unk224.h = 16;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk224.y = 166;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk240.vx = 906;
    sp->unk224.x = 118;
    sp->unk224.y = 141;
    sp->unk224.w = 40;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk240.vx = 934;
    sp->unk224.x = 118;
    sp->unk224.y = 165;
    sp->unk224.w = 40;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
}
