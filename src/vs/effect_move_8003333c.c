/*
 * VS.EXE: moving the current effect and the VRAM block move and roll (WIP
 * module). VS's copies of ST.EXE's src/st/effect_move.c functions; the
 * VRAM pair is shared (src/lib/vram.inc).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/effect.h>
#include <vs/effect.h>

/* The effect attack, then moves the current effect and turns its roll by
 * `spin`; returns the attack's result. */
ADDRESS(0x8003333c, 0x78)
int func_8003333c(int arg0, u_long height, int spin)
{
    StEffect *e = DAT_801ce698.cur;
    int result = func_80032110(arg0, height);

    e->unk20.vx += e->unk40.vx;
    e->unk20.vy += e->unk40.vy;
    e->unk20.vz += e->unk40.vz;
    e->unk30.vz = (e->unk30.vz - spin) & 0xfff;
    return result;
}

/* Moves the current effect back by its velocity. */
ADDRESS(0x800333b4, 0x40)
void func_800333b4(void)
{
    StEffect *e = DAT_801ce698.cur;

    e->unk20.vx -= e->unk40.vx;
    e->unk20.vy -= e->unk40.vy;
    e->unk20.vz -= e->unk40.vz;
}

/* Moves the current effect back by half its velocity. */
ADDRESS(0x800333f4, 0x7c)
void func_800333f4(void)
{
    StEffect *e = DAT_801ce698.cur;

    e->unk20.vx -= e->unk40.vx / 2;
    e->unk20.vy -= e->unk40.vy / 2;
    e->unk20.vz -= e->unk40.vz / 2;
}

#include "../lib/vram.inc"
