/*
 * ST.EXE player death and target queries, 0x8003bd14..0x8003c000.
 * The final target query has its own extent before the status unit.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
#include <kf/lib/null.h>
#include <RAND.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/anim.h>
#include <st/data.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/hud.h>
#include <st/light.h>
#include <st/menu.h>
#include <st/player.h>
#include <st/scratch.h>
#include <st/sound.h>
#include <st/vis.h>
#include <st/card.h>
#include <st/item.h>
#include <st/menu_main.h>

/* Kills the player (state 4). */
ADDRESS(0x8003bd14, 0x9c)
void player_kill(int unused)
{
    if (viewer.unk218 != 4) {
        viewer.unk218 = 4;
        viewer.unk2 = 0;
        func_80057c0c();
        world_state.unk1EE15 = 1;
        viewer.unk286 = 0;
        viewer.unk17C = 170;
        viewer.unk17A = (-1024 - (viewer.unk268 & 0x7ff)) / 24;
    }
}

ADDRESS(0x8003bdb0, 0x60)
void func_8003bdb0(void)
{
    if (viewer.unk2D4 >= 1152 && vis_lists.unk30 >= 1600) {
        player_kill(0);
        viewer.unk238 = 1;
    }
}

ADDRESS(0x8003be10, 0xa4)
void func_8003be10(void)
{
    if (vis_lists.unk4C != 0x80000) {
        if (vis_lists.unk4C < viewer.position.vy) {
            viewer.unk2EC = 1;
        } else {
            viewer.unk2EC = 0;
        }
    }
    if (vis_lists.unk58 != 0x80000) {
        if (vis_lists.unk58 < viewer.position.vy) {
            viewer.unk2F0 = 1;
        } else {
            viewer.unk2F0 = 0;
        }
    }
    if (area_state.unk4 == 10 && viewer.unk178 == 327) {
        viewer.unk2F0 = 0;
    }
}

/* The viewer's distance to `t` when it is within t's reach, else -1. */
ADDRESS(0x8003beb4, 0x14c)
int func_8003beb4(void *request)
{
    PlayerTarget *t = request;
    int dx;
    int dz;
    int dist;

    dx = viewer.position.vx - t->pos.vx;
    if (dx >= -10240 && dx <= 10240) {
        dz = viewer.position.vz - t->pos.vz;
        if (dz >= -10240 && dz <= 10240) {
            dist = SquareRoot0(dx * dx + dz * dz);
            if (dist - t->unk22 * rcos(t->unk20 - (short)ratan2(dz, dx)) / 4096 < t->unk1C + 256) {
                if (viewer.position.vy + 1600 < t->pos.vy) {
                    if (viewer.position.vy + 1600 < t->pos.vy - t->unk1E) {
                        return -1;
                    }
                } else if (t->pos.vy < viewer.position.vy) {
                    return -1;
                }
                return dist;
            }
        }
    }
    return -1;
}

