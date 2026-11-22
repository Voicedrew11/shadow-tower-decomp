/*
 * VS.EXE: an empty function, like ST.EXE's func_800443ec (src/st/area_load.c),
 * and the effect hit test (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/effect.h>
#include <vs/data.h>
#include <vs/effect.h>
#include <vs/buffer.h>

ADDRESS(0x80032108, 0x8)
void func_80032108(void)
{
}

/*
 * Whether the current effect reaches the opponent of its owner: the
 * horizontal distance from the opponent within `reach` plus its hit
 * radius, unless the opponent is in state 3 or 4. ST.EXE's effect_attack
 * requests the hit instead.
 */
ADDRESS(0x80032110, 0xf0)
int effect_reaches_opponent(int reach, u_long height)
{
    StEffect *e = vs_effects.cur;
    VsFighter *f = &shared_buffer.fighter[1 - e->owner.vs.fighter];
    SVECTOR d;
    int r;

    if ((u16)(f->state - 3) < 2)
        return 0;
    d.vx = f->position.vx - e->position.vx;
    d.vz = f->position.vz - e->position.vz;
    r = reach + f->data.radius;
    return d.vx * d.vx + d.vz * d.vz <= r * r;
}
