/*
 * VS.EXE: pushing overlapping fighters apart (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>
#include <vs/buffer.h>

/* Moves the two fighters apart along the line between them until their hit
 * radii no longer overlap; returns 1 when they did. */
ADDRESS(0x8002dcd8, 0x1ec)
int fighters_push_apart(void)
{
    VsFighter *a = &shared_buffer.fighter[0];
    VsFighter *b = &shared_buffer.fighter[1];
    SVECTOR d;
    long dist;
    long reach;

    d.vx = a->unk4DEF0.vx - b->unk4DEF0.vx;
    d.vz = a->unk4DEF0.vz - b->unk4DEF0.vz;
    dist = d.vx * d.vx + d.vz * d.vz;
    reach = (a->unk4DF18.unk1B4 + b->unk4DF18.unk1B4) * (a->unk4DF18.unk1B4 + b->unk4DF18.unk1B4);
    if (dist < reach) {
        dist = SquareRoot0(dist);
        reach = SquareRoot0(reach);
        if (dist != 0) {
            d.vx = d.vx * (reach - dist) / (dist * 2);
            d.vz = d.vz * (reach - dist) / (dist * 2);
        } else {
            d.vx = reach / 2;
            d.vz = 0;
        }
        a->unk4DEF0.vx += d.vx;
        a->unk4DEF0.vz += d.vz;
        b->unk4DEF0.vx -= d.vx;
        b->unk4DEF0.vz -= d.vz;
        return 1;
    }
    return 0;
}
