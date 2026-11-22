/*
 * VS.EXE: a fighter's hit test (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/funcs.h>

/*
 * Fighter n's hit test against the opponent: within reach plus the
 * opponent's hit radius and inside n's facing arc of +-arc. A guarding
 * opponent (state 5) facing the attack blocks it (state copied to 0x4df14,
 * returns 0); otherwise the opponent's sparks start and it returns 1.
 */
ADDRESS(0x8002dec4, 0x250)
int fighter_hit_test(int n, s16 reach, s16 arc)
{
    VsFighter *f = &shared_buffer.fighter[n];
    VsFighter *o = &shared_buffer.fighter[1 - n];
    SVECTOR d;
    s16 angle;
    int lo;
    int hi;
    int t;
    int state;

    if (o->state == 4)
        goto miss;
    if (round_phase != 1)
        goto miss;
    d.vx = f->position.vx - o->position.vx;
    d.vz = f->position.vz - o->position.vz;
    if (d.vx * d.vx + d.vz * d.vz > (reach + o->data.radius) * (reach + o->data.radius))
        goto miss;
    angle = ratan2(d.vx, d.vz) & 0xfff;
    lo = f->facing - arc;
    if (lo < 0) {
        if (lo + 4096 > angle && f->facing + arc < angle)
            goto miss;
    } else {
        hi = f->facing + arc;
        if (hi >= 4096) {
            if (hi - 4096 < angle && angle < lo)
                goto miss;
        } else {
            if (angle < lo)
                goto miss;
            if (hi < angle)
                goto miss;
        }
    }
    state = o->state;
    if (state != 5)
        goto hit;
    t = (angle + 2046) & 0xfff;
    if (t < o->facing) {
        if (o->facing - t < 256 || o->facing - t > 3840)
            goto blocked;
        goto hit;
    }
    if (t - o->facing >= 256 && t - o->facing <= 3840)
        goto hit;
    o->guard_flash = state;
miss:
    return 0;
blocked:
    o->guard_flash = state;
    return 0;
hit:
    sparks_start_burst(1 - n);
    return 1;
}
