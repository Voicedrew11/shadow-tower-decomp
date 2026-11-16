/*
 * VS.EXE: a full-screen semi-transparent overlay (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>

/* func_80028fcc's switch table. */
JUMPTABLES(0x8001150c, 0x18)

/* The overlay's texture quarter, stepped 0..5 by each frame that asks. */
DATA(0x800632e0, 0x4, ".sdata")
static s32 DAT_800632e0 = 0;

/*
 * Adds a 320x240 semi-transparent textured quad at the back of the frame's
 * ordering table, its texture window chosen by DAT_800632e0; advances that
 * when `hold` is 0.
 */
ADDRESS(0x80028fcc, 0x208)
void func_80028fcc(int hold)
{
    StPacketBuffer *packet = DAT_801624d0.packet;
    POLY_FT4 *p = (POLY_FT4 *)packet->cur;

    packet->cur = (u_long *)(p + 1);
    SetPolyFT4(p);
    setXY4(p, 0, 0, 320, 0, 0, 240, 320, 240);
    setSemiTrans(p, 1);
    switch (DAT_800632e0) {
    case 0:
    case 1:
        setUV4(p, 0, 0, 127, 0, 0, 127, 127, 127);
        p->clut = 0x7f10;
        p->tpage = 142;
        break;
    case 2:
    case 3:
        setUV4(p, 0, 0, 127, 0, 0, 127, 127, 127);
        p->clut = 0x7f10;
        p->tpage = 143;
        break;
    case 4:
    case 5:
        setUV4(p, 0, 128, 127, 128, 0, 255, 127, 255);
        p->clut = 0x7f10;
        p->tpage = 142;
        break;
    default:
        setUV4(p, 128, 0, 255, 0, 128, 127, 255, 127);
        break;
    }
    setRGB0(p, 100, 100, 100);
    addPrim(&DAT_801624d0.ot_cur[VS_OT_LENGTH - 1], p);
    if (hold == 0) {
        if (++DAT_800632e0 >= 6)
            DAT_800632e0 = 0;
    }
}
