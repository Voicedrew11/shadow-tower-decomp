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
/* Not exact: from the facing-arc test on, retail copies the angle into a
 * fresh register per path and keeps the opponent base and state for the
 * block store; 0x14 bytes short. */
ADDRESS(0x8002dec4, 0x250)
int func_8002dec4(int n, s16 reach, s16 arc)
{
    VsFighter *f = &DAT_800633b0.fighter[n];
    VsFighter *o = &DAT_800633b0.fighter[1 - n];
    SVECTOR d;
    int angle;
    int lo;
    int hi;
    int t;

    if (o->unk4DF0E == 4)
        return 0;
    if (DAT_80063324 != 1)
        return 0;
    d.vx = f->unk4DEF0.vx - o->unk4DEF0.vx;
    d.vz = f->unk4DEF0.vz - o->unk4DEF0.vz;
    if (d.vx * d.vx + d.vz * d.vz > (reach + o->unk4DF18.unk1B4) * (reach + o->unk4DF18.unk1B4))
        return 0;
    angle = ratan2(d.vx, d.vz) & 0xfff;
    lo = f->unk4DF02 - arc;
    if (lo < 0) {
        if (angle < lo + 4096 && f->unk4DF02 + arc < angle)
            return 0;
    } else {
        hi = f->unk4DF02 + arc;
        if (hi >= 4096) {
            if (angle > hi - 4096 && angle < lo)
                return 0;
        } else {
            if (angle < lo)
                return 0;
            if (hi < angle)
                return 0;
        }
    }
    if (o->unk4DF0E == 5) {
        t = (angle + 2046) & 0xfff;
        if (t < o->unk4DF02) {
            if (o->unk4DF02 - t >= 256 && o->unk4DF02 - t <= 3840)
                goto hit;
        } else if (t - o->unk4DF02 >= 256 && t - o->unk4DF02 <= 3840) {
            goto hit;
        }
        o->unk4DF14 = o->unk4DF0E;
        return 0;
    }
hit:
    func_8002d738(1 - n);
    return 1;
}
