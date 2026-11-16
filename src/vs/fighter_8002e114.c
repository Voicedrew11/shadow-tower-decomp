/*
 * VS.EXE: damage to a fighter and whether a fighter stands outside the
 * arena (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>

/*
 * Damages the opponent of fighter `id` by `hit`: its three base amounts
 * plus `amount` of kind `kind`, each reduced by the opponent's resistance
 * to that kind. State 3 leaves the remaining hit points, state 4 ends the
 * fight.
 *
 * Not exact (size-exact): retail materialises DAT_800633b0 in $a2 and the
 * resistance offset in $a3 before computing 1 - id; this build uses $v1
 * and $a2.
 */
ADDRESS(0x8002e114, 0x200)
void func_8002e114(int id, VsHit *hit)
{
    u16 damage[9];
    VsFighter *f = &DAT_800633b0[1 - id];
    u16 *resist = f->unk4E0D4;
    int total = 0;
    int i;

    damage[0] = hit->damage[0];
    damage[1] = hit->damage[1];
    damage[2] = hit->damage[2];
    damage[3] = damage[4] = damage[5] = damage[6] = damage[7] = damage[8] = 0;
    damage[3 + hit->kind] = hit->amount;
    for (i = 0; i < 9; i++) {
        if (damage[i]) {
            if (*resist == 0)
                total += damage[i] * 100000 / (1000 - damage[i]);
            else
                total += damage[i] * 50000 / (*resist * (1000 - damage[i]));
        }
        resist++;
    }
    if (total < f->unk4DF08) {
        f->unk4DF0C = f->unk4DF08 - total;
        f->unk4DF0E = 3;
    } else {
        f->unk4DF0C = 0;
        f->unk4DF0E = 4;
    }
    f->unk4DEEC = 0;
    f->unk4DEEA = 0;
}

/* Damages the opponent of fighter `id` with that fighter's attack
 * `attack`, as func_8002e114. */
ADDRESS(0x8002e314, 0x1fc)
void func_8002e314(u8 id, u8 attack)
{
    VsFighter *self = &DAT_800633b0[id];
    u16 *damage = self->unk4E062[attack].damage;
    VsFighter *f = &DAT_800633b0[1 - id];
    u16 *resist = f->unk4E0D4;
    int total = 0;
    int i;

    for (i = 0; i < 9; i++) {
        if (*damage) {
            if (*resist == 0)
                total += *damage * 100000 / (1000 - *damage);
            else
                total += *damage * 50000 / (*resist * (1000 - *damage));
        }
        damage++;
        resist++;
    }
    if (total < f->unk4DF08) {
        f->unk4DF0C = f->unk4DF08 - total;
        f->unk4DF0E = 3;
    } else {
        f->unk4DF0C = 0;
        f->unk4DF0E = 4;
    }
    f->unk4DEEC = 0;
    f->unk4DEEA = 0;
}

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
