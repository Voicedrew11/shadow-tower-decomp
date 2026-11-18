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
/* Not exact (size-exact): retail copies the attack angle into a fresh
 * temporary on each arc path (`move a0,a2`, `move v1,a2`, `move a1,a2`: a
 * narrowing that folded to a copy, as an inlined helper with a short
 * parameter would leave) and reads the opponent's state through a1; with the
 * angle a 16-bit local this build has the first copy ($a3, reused for the
 * guard test) but compares the angle's own register on the three paths.
 * Ruled out: int or u16 angle, (s16) casts at each compare, an int copy
 * assigned on each path. */
ADDRESS(0x8002dec4, 0x250)
int func_8002dec4(int n, s16 reach, s16 arc)
{
    VsFighter *f = &DAT_800633b0.fighter[n];
    VsFighter *o = &DAT_800633b0.fighter[1 - n];
    SVECTOR d;
    s16 angle;
    int lo;
    int hi;
    int t;
    int state;

    if (o->unk4DF0E == 4)
        goto miss;
    if (DAT_80063324 != 1)
        goto miss;
    d.vx = f->unk4DEF0.vx - o->unk4DEF0.vx;
    d.vz = f->unk4DEF0.vz - o->unk4DEF0.vz;
    if (d.vx * d.vx + d.vz * d.vz > (reach + o->unk4DF18.unk1B4) * (reach + o->unk4DF18.unk1B4))
        goto miss;
    angle = ratan2(d.vx, d.vz) & 0xfff;
    lo = f->unk4DF02 - arc;
    if (lo < 0) {
        if (angle < lo + 4096 && f->unk4DF02 + arc < angle)
            goto miss;
    } else {
        hi = f->unk4DF02 + arc;
        if (hi >= 4096) {
            if (angle > hi - 4096 && angle < lo)
                goto miss;
        } else {
            if (angle < lo)
                goto miss;
            if (hi < angle)
                goto miss;
        }
    }
    state = o->unk4DF0E;
    if (state != 5)
        goto hit;
    t = (angle + 2046) & 0xfff;
    if (t < o->unk4DF02) {
        if (o->unk4DF02 - t < 256 || o->unk4DF02 - t > 3840)
            goto blocked;
        goto hit;
    }
    if (t - o->unk4DF02 >= 256 && t - o->unk4DF02 <= 3840)
        goto hit;
blocked:
    o->unk4DF14 = state;
miss:
    return 0;
hit:
    func_8002d738(1 - n);
    return 1;
}
