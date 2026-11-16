/*
 * ST.EXE: effect movement helpers of the effect script interpreter
 * (0x800385b0, not reconstructed; WIP module split from src/st/effect.c
 * by it and by the spawn function 0x80036484): steering and moving the
 * current effect, firing from it, and two VRAM copy helpers.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/effect.h>

/*
 * Adds `accel` (scaled by func_8004a0f8) to the current effect's velocity,
 * limits its length to `limit` and moves the effect by it; then, unless
 * `height` is -1, requests an attack (effect_attack). Returns -1 when the
 * attack was made, else 0.
 */
ADDRESS(0x80037e10, 0x1e8)
int effect_accelerate(int scale, int limit, int attack, int height, SVECTOR *accel)
{
    StEffect *e = effects.cur;
    int length;

    func_8004a0f8(scale, accel);
    e->unk40.vx += accel->vx;
    e->unk40.vy += accel->vy;
    e->unk40.vz += accel->vz;
    length = func_8004a340(e->unk40.vx, e->unk40.vy, e->unk40.vz);
    if (limit < length) {
        e->unk40.vx = e->unk40.vx * limit / length;
        e->unk40.vy = e->unk40.vy * limit / length;
        e->unk40.vz = e->unk40.vz * limit / length;
    }
    e->unk20.vx += e->unk40.vx;
    e->unk20.vy += e->unk40.vy;
    e->unk20.vz += e->unk40.vz;
    if (height != -1 && effect_attack(attack, height, 0))
        return -1;
    return 0;
}

/*
 * Turns the current effect toward its target (the entry func_8004b658
 * finds for an effect with flag 0x40, else the viewer when func_8003eb34
 * allows) by at most `turn`, and accelerates it along its heading; with
 * `scale_near` instead of `scale` while the target is within `near`.
 */
ADDRESS(0x80037ff8, 0x1b0)
int effect_home(int limit, int scale, int turn, int attack, int height, int near,
                int scale_near, int range)
{
    StEffect *e = effects.cur;
    VECTOR target;
    SVECTOR dir;
    SVECTOR v;
    int dist;
    Vis212 *t;

    if (e->unk0 & 0x40) {
        t = func_8004b658(&e->unk20, e->unk30.vy, e->unk30.vx, 24000, range, range, &dist, 0);
        if (t == NULL)
            goto move;
        target.vx = t->unk74.vx;
        target.vy = t->unk74.vy - (t->unk30 >> 1);
        target.vz = t->unk74.vz;
    aim:
        func_8004a390(target.vx - e->unk20.vx, target.vy - e->unk20.vy,
                      target.vz - e->unk20.vz, &dir);
        e->unk30.vx = func_800496c8(e->unk30.vx, dir.vx, turn);
        e->unk30.vy = func_800496c8(e->unk30.vy, dir.vy, turn);
    } else {
        dist = func_8003eb34(&e->unk20, e->unk30.vy, 24000, 4096);
        if (dist == -1)
            goto move;
        target.vx = viewer.x;
        target.vy = viewer.y;
        target.vz = viewer.z;
        goto aim;
    }
move:
    func_80049f08(&e->unk30, &v);
    if (dist >= 0 && near >= dist)
        scale = scale_near;
    return effect_accelerate(scale, limit, attack, height, &v);
}

/*
 * Fires a kind-128 projectile request (func_8003beb4) from the current
 * effect along its velocity once its countdown has run out, then moves
 * it and turns its roll by `spin`. Returns 128 when the request was
 * taken, else 0.
 */
ADDRESS(0x800381a8, 0x158)
int effect_fire(int arg0, int height, int spin)
{
    StEffect *e = effects.cur;
    VisLists *req;
    int fired = 0;

    if (e->unk8 == 0) {
        req = &vis_lists;
        req->unk4 = e->unk20;
        req->unk1C = arg0;
        req->unk1E = height;
        req->unk14 = e->unk40;
        req->unk22 = SquareRoot0(e->unk40.vx * e->unk40.vx + e->unk40.vz * e->unk40.vz);
        req->unk20 = ratan2(e->unk40.vz, e->unk40.vx);
        vis_lists.unk0 = 128;
        fired = (func_8003beb4(req) != -1) << 7;
        e->unk20.vx += e->unk40.vx;
        e->unk20.vy += e->unk40.vy;
        e->unk20.vz += e->unk40.vz;
        e->unk30.vz = (e->unk30.vz - spin) & 0xfff;
    } else {
        e->unk8--;
    }
    return fired;
}

/* effect_attack, then moves the current effect and turns its roll by
 * `spin`; returns the attack's result. */
ADDRESS(0x80038300, 0x7c)
int effect_attack_move(int arg0, u_long height, int spin)
{
    StEffect *e = effects.cur;
    int result = effect_attack(arg0, height, 0);

    e->unk20.vx += e->unk40.vx;
    e->unk20.vy += e->unk40.vy;
    e->unk20.vz += e->unk40.vz;
    e->unk30.vz = (e->unk30.vz - spin) & 0xfff;
    return result;
}

/* Moves the current effect back by its velocity (no direct callers). */
ADDRESS(0x8003837c, 0x40)
void effect_step_back(void)
{
    StEffect *e = effects.cur;

    e->unk20.vx -= e->unk40.vx;
    e->unk20.vy -= e->unk40.vy;
    e->unk20.vz -= e->unk40.vz;
}

/* Moves the current effect back by half its velocity (no direct
 * callers). */
ADDRESS(0x800383bc, 0x7c)
void effect_step_back_half(void)
{
    StEffect *e = effects.cur;

    e->unk20.vx -= e->unk40.vx / 2;
    e->unk20.vy -= e->unk40.vy / 2;
    e->unk20.vz -= e->unk40.vz / 2;
}

#include "../lib/vram.inc"
