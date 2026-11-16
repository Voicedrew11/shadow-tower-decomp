/*
 * VS.EXE: whether a fighter stands outside the arena (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>

/* 1 when fighter `id` is outside the central 20000-unit square and its
 * distance measure passes 15500 squared. */
ADDRESS(0x8002e510, 0xd4)
int func_8002e510(int id)
{
    VsFighter *f = &DAT_800633b0[id];

    if (f->unk4DEF0.vx < -10000 || f->unk4DEF0.vx > 10000 || f->unk4DEF0.vz < -10000 ||
        f->unk4DEF0.vz > 10000) {
        if (f->unk4DEF0.vx * f->unk4DEF0.vx + f->unk4DEF0.vz * f->unk4DEF0.vz + f->unk4E0CC >
            15500 * 15500)
            return 1;
    }
    return 0;
}
