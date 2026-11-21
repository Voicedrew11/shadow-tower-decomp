/*
 * VS.EXE: the 640- and 320-wide display setups and the graphics reset (WIP
 * module). VS's copies of ST.EXE's func_800172bc, func_800173a8
 * (src/st/gfx.c) and func_80058cc8 (src/st/menu_main.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <vs/data.h>
#include <LIBSN.H>

/* The graphics reset's RECT initializer: 8 bytes, so .sdata under -G8. */
SDATA(0x80063234, 0x8)

/* Reads `size` bytes of host file `name` into `buf` (PC file server). */
ADDRESS(0x80016c48, 0x60)
void func_80016c48(char *name, void *buf, int size)
{
    int fd = PCopen(name, 0, 0);

    PCread(fd, buf, size);
    PCclose(fd);
}

ADDRESS(0x80016ca8, 0xec)
void func_80016ca8(void)
{
    int h = 240;

    gte_SetGeomOffset(320, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&vs_gfx_state.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&vs_gfx_state.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&vs_gfx_state.draw[1], 0, h, 640, h);
    SetDefDispEnv(&vs_gfx_state.disp[1], 0, 0, 640, h);
    vs_gfx_state.draw[0].dtd = 1;
    vs_gfx_state.draw[1].dtd = 1;
    vs_gfx_state.draw[0].isbg = 1;
    vs_gfx_state.draw[1].isbg = 1;
    setRGB0(&vs_gfx_state.draw[0], 0, 0, 0);
    setRGB0(&vs_gfx_state.draw[1], 0, 0, 0);
}

/* The 320-wide setup; unlike ST.EXE's copy it also clears the background. */
ADDRESS(0x80016d94, 0x124)
void func_80016d94(void)
{
    int h = 240;

    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&vs_gfx_state.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&vs_gfx_state.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&vs_gfx_state.draw[1], 0, h, 320, h);
    SetDefDispEnv(&vs_gfx_state.disp[1], 0, 0, 320, h);
    vs_gfx_state.draw[0].dtd = 1;
    vs_gfx_state.draw[1].dtd = 1;
    vs_gfx_state.draw[0].isbg = 1;
    vs_gfx_state.draw[1].isbg = 1;
    setRGB0(&vs_gfx_state.draw[0], 0, 0, 0);
    setRGB0(&vs_gfx_state.draw[1], 0, 0, 0);
    PutDrawEnv(&vs_gfx_state.draw[0]);
    PutDrawEnv(&vs_gfx_state.draw[1]);
    PutDispEnv(&vs_gfx_state.disp[0]);
    PutDispEnv(&vs_gfx_state.disp[1]);
}

/* ResetGraph only (ST.EXE's copy then reloads and shows the loading
 * picture); the two locals are unused, as there. */
ADDRESS(0x80016eb8, 0x58)
void func_80016eb8(void)
{
    RECT rect = {576, 384, 64, 16};
    DVECTOR size = {576, 480};

    ResetGraph(1);
}
