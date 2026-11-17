/*
 * ST.EXE player reset and status timers, 0x8003c000..0x8003c95c.
 * The final status update has its own extent before the damage unit.
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

ADDRESS(0x8003c000, 0x54)
int func_8003c000(int x, int y, int z, int range, int kind)
{
    return func_8004a580((VECTOR *)&viewer.position.vx, x, y, z, range + 256, 1000, kind);
}

ADDRESS(0x8003c054, 0x8c)
void func_8003c054(void)
{
    viewer.unk2CE = 0;
    viewer.unk2D0 = 0;
    viewer.unk2D2 = 0;
    viewer.unk239 = 0;
    viewer.unk2D4 = 0;
    viewer.unk218 = 0;
    viewer.unk294 = 0;
    viewer.unk292 = 0;
    viewer.unk290 = 0;
    viewer.unk28C = 0;
    viewer.unk28A = 0;
    viewer.unk288 = 0;
    viewer.unk284 = 0;
    viewer.unk280.halves[1] = 0;
    viewer.unk280.halves[0] = 0;
    *(SVECTOR *)&viewer.unk268 = viewer.unk278;
    viewer.unk155[0] = 255;
    viewer.unk155[1] = 255;
    viewer.unk2F0 = 0;
    viewer.unk2EC = 0;
    vis_lists.unk47DC = 0;
    viewer.unk2F4 = -1;
}

ADDRESS(0x8003c0e0, 0x9c)
void func_8003c0e0(void)
{
    func_8003f12c(viewer.unk38[3], 0, 0);
    func_8003f12c(viewer.unk38[16], 1, 0);
    func_8003f12c(viewer.unk38[4], 2, 0);
    func_8003f12c(viewer.unk38[17], 3, 0);
    func_8003ee38(viewer.unk38[6], 0);
    func_8003ee38(viewer.unk38[12], 1);
    func_8003c054();
    func_8003cedc();
    lighting.unk7 = 255;
}

/*
 * Steps a status timer: the level climbs by one towards its peak, then the
 * peak clears and the level falls back to 0. Returns max scaled by the level
 * over `ramp` (max once the level reaches it), or -1 when the timer is idle;
 * the new values are stored only when `apply` is set.
 */
ADDRESS(0x8003c17c, 0xd0)
short func_8003c17c(short *level, short *peak, int ramp, int max, int apply)
{
    short p;
    short l;
    short v;

    l = *level;
    p = 0;
    if (peak) {
        p = *peak;
    }
    if (l != 0 || p != 0) {
        if (l < p) {
            l++;
        } else {
            p = 0;
            l--;
        }
        if (l < ramp) {
            v = l * max / ramp;
        } else {
            v = max;
        }
        if (apply) {
            *level = l;
            if (peak) {
                *peak = p;
            }
        }
        return v;
    }
    return -1;
}

/* Steps the status timers and the damage fades once per frame. */
/* Not exact: timer-word tests retain %hi(viewer) across callbacks, where
 * retail derives the viewer base from the current timer address (first
 * divergence +0xf0). This build is 20 bytes longer than retail. */
ADDRESS(0x8003c24c, 0x710)
void player_update_status(int apply)
{
    short level;

    if (viewer.unk2D6 != 0) {
        viewer.unk2D6 -= viewer.unk2D8;
        if (viewer.unk2D6 < 0) {
            viewer.unk2D6 = 0;
        }
        if (viewer.unk2D6 >= 4096) {
            level = 4096;
        } else {
            level = viewer.unk2D6;
        }
        func_80021994(60, 0, 0, level, 1);
    }
    if (viewer.unk2DA != 0) {
        viewer.unk2DA -= viewer.unk2DC;
        if (viewer.unk2DA < 0) {
            viewer.unk2DA = 0;
        }
        if (viewer.unk2DA >= 4096) {
            level = 4096;
        } else {
            level = viewer.unk2DA;
        }
        func_80021994(60, 60, 0, level, 1);
    }

    level = func_8003c17c(&viewer.unk190[0][0], &viewer.unk190[0][1], 20, 4096, apply);
    if (level != -1) {
        func_80021994(0, 40, 0, level, 1);
        if (*(s32 *)viewer.unk190[0] == 0) {
            viewer.unk2C &= ~7;
        }
    }
    level = func_8003c17c(&viewer.unk190[1][0], &viewer.unk190[1][1], 20, 4096, apply);
    if (level != -1) {
        if ((viewer.unk190[1][0] & 3) == 3) {
            level = 4096;
        } else {
            level -= 512;
        }
        func_80021994(40, 40, 0, level, 1);
        func_8003ead0();
        vis_table_b.unk71A4 = vis_table_b.unk71A6 = vis_table_b.unk71A8 = 0;
        vis_lists.unk47CE = vis_lists.unk47D0 = vis_lists.unk47D2 = 0;
        vis_table_a.unk6160 = vis_table_a.unk6162 = vis_table_a.unk6164 = 0;
        viewer.unk1CC = viewer.unk1CE = viewer.unk1D0 = 0;
        if (*(s32 *)viewer.unk190[1] == 0) {
            viewer.unk2C &= ~0x18;
        }
    }
    level = func_8003c17c(&viewer.unk190[2][0], &viewer.unk190[2][1], 0, 4096, apply);
    if (level != -1) {
        func_80021994(20, 20, 20, level, 2);
        if (*(s32 *)viewer.unk190[2] == 0) {
            viewer.unk2C &= ~0x60;
            lighting.unk4 |= 4;
        }
    }
    level = func_8003c17c(&viewer.unk190[3][0], &viewer.unk190[3][1], 20, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[3] == 0) {
            viewer.unk2C &= ~0x80;
            func_8005a3a8(0);
            PLAYER_CLAMP_HP();
        }
    }
    level = func_8003c17c(&viewer.unk190[4][0], &viewer.unk190[4][1], 20, 4096, apply);
    if (level != -1) {
        func_80021994(40, 0, 20, level, 1);
        if (*(s32 *)viewer.unk190[4] == 0) {
            viewer.unk2C &= ~0x100;
            func_8005a3a8(0);
            PLAYER_CLAMP_HP();
        }
    }
    if (viewer.unk2EC) {
        func_80021994(0, 0, 40, 4096, 1);
    }
    if (viewer.unk2F0) {
        func_80021994(40, 40, 0, 4096, 1);
    }
    level = func_8003c17c(&viewer.unk190[5][0], &viewer.unk190[5][1], 0, 4096, apply);
    if (level != -1) {
        lighting.unk4 |= 0x10;
        lighting.unk3 |= 2;
        if (*(s32 *)viewer.unk190[5] == 0) {
            viewer.unk2C &= ~0x20000;
            viewer.unk2C &= ~0x10000;
        }
    }
    level = func_8003c17c(&viewer.unk190[6][0], &viewer.unk190[6][1], 0, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[6] == 0) {
            viewer.unk2C &= ~0x40000;
        }
    }
    level = func_8003c17c(&viewer.unk190[7][0], &viewer.unk190[7][1], 0, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[7] == 0) {
            viewer.unk2C &= ~0x80000;
        }
    }
    level = func_8003c17c(&viewer.unk190[8][0], &viewer.unk190[8][1], 0, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[8] == 0) {
            viewer.unk2C &= ~0x100000;
        }
    }
    level = func_8003c17c(&viewer.unk190[9][0], &viewer.unk190[9][1], 0, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[9] == 0) {
            viewer.unk2C &= ~0x200000;
        }
    }
    level = func_8003c17c(&viewer.unk190[10][0], &viewer.unk190[10][1], 0, 4096, apply);
    if (level != -1) {
        lighting.unk9 = 1;
        lighting.unk4 |= 0x20;
        lighting.unk3 |= 1;
        if (*(s32 *)viewer.unk190[10] == 0) {
            viewer.unk2C &= ~0x400000;
        }
    }
    level = func_8003c17c(&viewer.unk190[11][0], &viewer.unk190[11][1], 0, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[11] == 0) {
            viewer.unk2C &= ~0x800000;
        }
    }
    level = func_8003c17c(&viewer.unk190[12][0], &viewer.unk190[12][1], 0, 4096, apply);
    if (level != -1) {
        if (*(s32 *)viewer.unk190[12] == 0) {
            viewer.unk2C &= ~0x800;
        }
    }
}
