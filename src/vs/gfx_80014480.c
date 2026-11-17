/*
 * VS.EXE: the graphics start-up (WIP module), VS's version of ST.EXE's
 * func_80016fb8 (src/st/gfx.c): the 320x240 double buffer, the GTE
 * offset, screen, back and far colours, and the two packet buffers.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <vs/data.h>

#define PACKET_WORDS (0x20000 / 4)

ADDRESS(0x80014480, 0x1b4)
void func_80014480(void)
{
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&DAT_801624d0.draw[0], 0, 0, 320, 240);
    SetDefDispEnv(&DAT_801624d0.disp[0], 0, 240, 320, 240);
    SetDefDrawEnv(&DAT_801624d0.draw[1], 0, 240, 320, 240);
    SetDefDispEnv(&DAT_801624d0.disp[1], 0, 0, 320, 240);
    DAT_801624d0.draw[0].dtd = 1;
    DAT_801624d0.draw[1].dtd = 1;
    DAT_801624d0.draw[0].isbg = 1;
    DAT_801624d0.draw[1].isbg = 1;
    setRGB0(&DAT_801624d0.draw[0], 0, 0, 0);
    setRGB0(&DAT_801624d0.draw[1], 0, 0, 0);
    PutDispEnv(&DAT_801624d0.disp[0]);
    PutDrawEnv(&DAT_801624d0.draw[0]);
    gte_SetBackColor(80, 80, 80);
    gte_SetFarColor(5, 5, 5);
    DAT_801624d0.frame = 255;
    DAT_801624d0.unkE0 = DAT_8011c528;
    DAT_801624d0.packets[0].start = DAT_801624d0.unkE0;
    DAT_801624d0.packets[0].end = DAT_801624d0.packets[0].start + PACKET_WORDS;
    DAT_801624d0.packets[1].start = DAT_801624d0.packets[0].end;
    DAT_801624d0.packets[1].end = DAT_801624d0.packets[1].start + PACKET_WORDS;
    SetDispMask(1);
}
