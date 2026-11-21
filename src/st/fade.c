/*
 * ST.EXE: the screen fade channels, drawn and cleared once per frame (WIP
 * module). Split from src/st/quad.c only because func_80021654 there is not
 * exact (4 bytes longer than retail).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/gfx.h>

/* Draws and clears fade channel 0. */
ADDRESS(0x8002181c, 0xbc)
void func_8002181c(void)
{
    if (world_state.fade.active[0]) {
        func_80021518(0, 0, 320, 240, 0, 224, 15, 15, 1,
                      getTPage(0, world_state.fade.mode[0], 896, 256), getClut(560, 496),
                      (u8)world_state.fade.r[0], (u8)world_state.fade.g[0],
                      (u8)world_state.fade.b[0], 0);
        world_state.fade.b[0] = 0;
        world_state.fade.g[0] = 0;
        world_state.fade.r[0] = 0;
        world_state.fade.active[0] = 0;
    }
}

/* Draws and clears fade channel 1. */
ADDRESS(0x800218d8, 0xbc)
void func_800218d8(void)
{
    if (world_state.fade.active[1]) {
        func_80021518(0, 0, 320, 240, 0, 224, 15, 15, 1,
                      getTPage(0, world_state.fade.mode[1], 896, 256), getClut(560, 496),
                      (u8)world_state.fade.r[1], (u8)world_state.fade.g[1],
                      (u8)world_state.fade.b[1], 0);
        world_state.fade.b[1] = 0;
        world_state.fade.g[1] = 0;
        world_state.fade.r[1] = 0;
        world_state.fade.active[1] = 0;
    }
}

/* Adds a colour scaled by scale/4096 to the fade channel of `mode`. */
ADDRESS(0x80021994, 0x130)
void func_80021994(int r, int g, int b, int scale, int mode)
{
    switch (mode) {
    case 1:
    case 3:
        world_state.fade.active[0]++;
        world_state.fade.mode[0] = mode;
        world_state.fade.r[0] += (r * scale) >> 12;
        world_state.fade.g[0] += (g * scale) >> 12;
        world_state.fade.b[0] += (b * scale) >> 12;
        break;
    case 2:
        world_state.fade.active[1]++;
        world_state.fade.mode[1] = mode;
        world_state.fade.r[1] += (r * scale) >> 12;
        world_state.fade.g[1] += (g * scale) >> 12;
        world_state.fade.b[1] += (b * scale) >> 12;
        break;
    }
}
