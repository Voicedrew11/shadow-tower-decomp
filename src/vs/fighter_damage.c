/*
 * VS.EXE: damage to a fighter and whether a fighter stands outside the
 * arena (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>
#include <vs/buffer.h>

/*
 * Damages the opponent of fighter `id` by `hit`: its three base amounts
 * plus `amount` of kind `kind`, each reduced by the opponent's resistance
 * to that kind. State 3 leaves the remaining hit points, state 4 ends the
 * fight.
 */
ADDRESS(0x8002e114, 0x200)
void fighter_apply_hit(int id, VsHit *hit)
{
    u16 damage[9];
    VsFighter *base = shared_buffer.fighter;
    VsFighter *f = base + (1 - id);
    u16 *resist = f->data.resistances;
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
    if (total < f->hp) {
        f->hp_target = f->hp - total;
        f->state = FIGHTER_STATE_HURT;
    } else {
        f->hp_target = 0;
        f->state = FIGHTER_STATE_DOWN;
    }
    f->anim_step = 0;
    f->anim_frame = 0;
}

/* Damages the opponent of fighter `id` with that fighter's attack
 * `attack`, as fighter_apply_hit. */
ADDRESS(0x8002e314, 0x1fc)
void fighter_apply_attack_hit(u8 id, u8 attack)
{
    VsFighter *self = &shared_buffer.fighter[id];
    u16 *damage = self->data.attacks[attack].damage;
    VsFighter *f = &shared_buffer.fighter[1 - id];
    u16 *resist = f->data.resistances;
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
    if (total < f->hp) {
        f->hp_target = f->hp - total;
        f->state = FIGHTER_STATE_HURT;
    } else {
        f->hp_target = 0;
        f->state = FIGHTER_STATE_DOWN;
    }
    f->anim_step = 0;
    f->anim_frame = 0;
}

/* 1 when fighter `id` is outside the central 20000-unit square and its
 * distance measure passes 15500 squared. */
ADDRESS(0x8002e510, 0xd4)
int fighter_is_out_of_bounds(int id)
{
    VsFighter *f = &shared_buffer.fighter[id];

    if (f->position.vx < -10000 || f->position.vx > 10000 || f->position.vz < -10000 ||
        f->position.vz > 10000) {
        if (f->position.vx * f->position.vx + f->position.vz * f->position.vz + f->data.radius >
            15500 * 15500)
            return 1;
    }
    return 0;
}
