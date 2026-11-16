/*
 * ST.EXE: turning and tilting the view from the held buttons, with
 * acceleration and easing back (WIP module inside the translation unit
 * that also holds 0x8003cb88..0x80042d74).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/player.h>

ADDRESS(0x80041d4c, 0x354)
void func_80041d4c(void)
{
    int limit;

    if (DAT_801cd180.unk2 & DAT_801cd180.unk26) {
        viewer.unk2CA += viewer.unk2E8 >> 2;
        if (viewer.unk2E8 < viewer.unk2CA)
            viewer.unk2CA = viewer.unk2E8;
    } else if (DAT_801cd180.unk2 & DAT_801cd180.unk28) {
        viewer.unk2CA -= viewer.unk2E8 >> 2;
        if (viewer.unk2CA < -viewer.unk2E8)
            viewer.unk2CA = -viewer.unk2E8;
    } else if (viewer.unk2CA > 0) {
        viewer.unk2CA -= viewer.unk2E8 >> 2;
        if (viewer.unk2CA < 0)
            viewer.unk2CA = 0;
    } else if (viewer.unk2CA < 0) {
        viewer.unk2CA += viewer.unk2E8 >> 2;
        if (viewer.unk2CA > 0)
            viewer.unk2CA = 0;
    }
    viewer.unk278.vy = (viewer.unk278.vy + viewer.unk2CA) & 0xfff;
    if (DAT_801cd180.unk2 & DAT_801cd180.unk2E) {
        if (DAT_801cd180.unk2 & DAT_801cd180.unk30) {
            if (viewer.unk278.vx > 2048) {
                limit = (4224 - viewer.unk278.vx) >> 2;
                viewer.unk2CC += 8;
                if (limit < viewer.unk2CC) {
                    viewer.unk2CC = limit;
                    if (limit == 0)
                        viewer.unk2CC = 4224 - viewer.unk278.vx;
                }
            } else if (viewer.unk278.vx < 2048) {
                limit = -((viewer.unk278.vx - 128) >> 2);
                viewer.unk2CC -= 8;
                if (viewer.unk2CC < limit) {
                    viewer.unk2CC = limit;
                    if (limit == 0)
                        viewer.unk2CC = -(viewer.unk278.vx - 128);
                }
            }
        } else {
            viewer.unk2CC -= 3;
            if (viewer.unk2CC < -32)
                viewer.unk2CC = -32;
        }
    } else if (DAT_801cd180.unk2 & DAT_801cd180.unk30) {
        viewer.unk2CC += 3;
        if (viewer.unk2CC > 32)
            viewer.unk2CC = 32;
    } else if (viewer.unk2CC > 0) {
        viewer.unk2CC -= 3;
        if (viewer.unk2CC < 0)
            viewer.unk2CC = 0;
    } else if (viewer.unk2CC < 0) {
        viewer.unk2CC += 3;
        if (viewer.unk2CC > 0)
            viewer.unk2CC = 0;
    }
    if (viewer.unk2CC > 0) {
        viewer.unk278.vx = (viewer.unk278.vx + viewer.unk2CC) & 0xfff;
        if (func_8004a194(viewer.unk278.vx, 700))
            viewer.unk278.vx = 700;
    } else if (viewer.unk2CC < 0) {
        viewer.unk278.vx = (viewer.unk278.vx + viewer.unk2CC) & 0xfff;
        if (func_8004a194(3396, viewer.unk278.vx))
            viewer.unk278.vx = 3396;
    }
}
