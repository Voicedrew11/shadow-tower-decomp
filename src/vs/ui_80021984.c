/*
 * VS.EXE: the team screen's frame (WIP module): both teams' slot
 * pictures, the two fighters' pictures and the background strips.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#define VS_PICKED_PAIR
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/*
 * Starts a frame and draws, in team mode (DAT_80063228 == 3), the six
 * slot pictures in two columns of three (black for an empty slot),
 * otherwise each player's first slot; then each player's fighter
 * picture over the shared frame tile, and the two background strips.
 */
ADDRESS(0x80021984, 0x290)
void func_80021984(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 *slot = DAT_801624b8[0].slot;
    CVECTOR black;
    int i;

    func_8001d1b8();
    sp->unk238.w = 48;
    sp->unk238.h = 48;
    sp->unk238.y = 48;
    sp->unk238.x = 56;
    if (DAT_80063228 == 3) {
        for (i = 0; i < 6; i++) {
            if (*slot++ == -1) {
                black.r = 0;
                black.g = 0;
                black.b = 0;
                func_8001e3d8(&sp->unk238, &black, 0);
            } else {
                func_8001e530(&sp->unk238, &sp->unk0.team.pos[i], NULL, NULL, 272);
            }
            sp->unk238.y += 48;
            if (i == 2) {
                sp->unk238.x = 216;
                sp->unk238.y = 48;
            }
        }
    } else {
        sp->unk238.x = 56;
        sp->unk238.y = 96;
        func_8001e530(&sp->unk238, &sp->unk0.team.pos[0], NULL, NULL, 272);
        sp->unk238.x = 216;
        sp->unk238.y = 96;
        func_8001e530(&sp->unk238, &sp->unk0.team.pos[3], NULL, NULL, 272);
    }
    sp->unk238.w = 28;
    sp->unk238.h = 28;
    sp->unk238.x = 20;
    sp->unk238.y = 164;
    for (i = 0; i < 2; i++) {
        if (DAT_80063330[i] == -1) {
            black.r = 0;
            black.g = 0;
            black.b = 0;
            func_8001e3d8(&sp->unk238, &black, 0);
        } else {
            func_8001e530(&sp->unk238, &sp->unk0.team.pos[6 + i], NULL, NULL, 272);
        }
        func_8001e530(&sp->unk238, &sp->unk0.team.pos[8], NULL, NULL, 272);
        sp->unk238.x = 272;
    }
    sp->unk238.w = 160;
    sp->unk238.h = 240;
    sp->unk238.x = 0;
    sp->unk238.y = 0;
    sp->unk240.vx = 320;
    sp->unk240.vy = 0;
    func_8001e530(&sp->unk238, &sp->unk240, NULL, NULL, 16);
    sp->unk238.x = 160;
    sp->unk240.vx = 480;
    func_8001e530(&sp->unk238, &sp->unk240, NULL, NULL, 16);
    func_8001e890(0, 0, 0);
    func_8001d374();
}
