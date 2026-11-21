/*
 * VS.EXE: the fighters' gauges (WIP module). -G8: the blink toggle is a
 * $gp-relative .sdata static.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/buffer.h>

/* Flips every frame; a low hit-point bar blinks with it. */
DATA(0x800632e4, 0x1, ".sdata")
static u8 DAT_800632e4 = 0;

/*
 * Draws each fighter's hit-point bar (growing outwards from the screen
 * centre, white while below 31% on alternate frames) and, when it has
 * one, its second gauge under it; `level` (at most 32) scales the colours.
 */
ADDRESS(0x80029b10, 0x418)
void func_80029b10(int level)
{
    int i = 0;
    int w;
    VsFighter *f;
    POLY_G4 *p;
    StPacketBuffer *packet;

    DAT_800632e4 = 1 - DAT_800632e4;
    if (level > 32)
        level = 32;
    for (; i < 2; i++) {
        f = &DAT_800633b0.fighter[i];
        packet = DAT_801624d0.packet;
        p = (POLY_G4 *)packet->cur;
        packet->cur = (u_long *)(p + 1);
        setPolyG4(p);
        setSemiTrans(p, 1);
        w = f->unk4DF08 * 100 / f->unk4DF18.unk1B8;
        if (i == 0)
            setXY4(p, 115 - w, 20, 115, 20, 120 - w, 25, 120, 25);
        else
            setXY4(p, w + 205, 20, 205, 20, w + 200, 25, 200, 25);
        setRGB0(p, 0, 0, 0);
        setRGB1(p, 0, 0, 0);
        if (w >= 31 || DAT_800632e4 != 0) {
            setRGB3(p, level * 250 / 32, 0, 0);
            setRGB2(p, 250 - w * 5 / 2, w * 5 / 2 * level / 32, 0);
        } else {
            setRGB3(p, 200, 200, 200);
            setRGB2(p, 200, 200, 200);
        }
        addPrim(&DAT_801624d0.ot_cur[1], p);
    }
    for (i = 0; i < 2; i++) {
        f = &DAT_800633b0.fighter[i];
        if (f->unk4DF18.unk1BA != 0) {
            packet = DAT_801624d0.packet;
            p = (POLY_G4 *)packet->cur;
            packet->cur = (u_long *)(p + 1);
            setPolyG4(p);
            setSemiTrans(p, 1);
            w = f->unk4DF0A * 100 / f->unk4DF18.unk1BA;
            if (i == 0)
                setXY4(p, 118 - w, 25, 118, 25, 120 - w, 28, 120, 28);
            else
                setXY4(p, w + 202, 25, 202, 25, w + 200, 28, 200, 28);
            setRGB0(p, 0, 0, 0);
            setRGB1(p, 0, 0, 0);
            setRGB3(p, level * 250 / 32, 0, 0);
            setRGB2(p, level * 250 / 32, w * 5 / 2 * level / 32, 0);
            addPrim(&DAT_801624d0.ot_cur[1], p);
        }
    }
}
