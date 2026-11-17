/*
 * The first ten of the 21 functions every FDAT overlay starts with (WIP).
 * One source for all 36 overlays: FDAT_002.BIN binds by address, the others
 * by name. A unit of its own only so that fdat_801eba4c, which is not yet
 * exact, does not move fdat_801ebec4's switch table in the overlay's unit;
 * the next two functions are src/lib/fdat_common.inc, the last nine
 * src/lib/fdat_common_b.c.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>

/* Clears the entry's unkBC. */
ADDRESS(0x801eb7ac, 0x8)
void fdat_801eb7ac(Vis212 *e)
{
    e->unkBC = 0;
}

/* Starts script `id` of the entry's record. */
ADDRESS(0x801eb7b4, 0x60)
void fdat_801eb7b4(Vis212 *e, int id)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    e->unk18 = id;
    e->unkA8 = r->unk60[id];
    e->unk0F = e->unkA8->unk00;
    e->unk08 = e->unkA8->unk01;
    e->unkB0 = e->unkA8->unk0C;
    e->unk1A = 0;
    e->unk2A = 0;
}

/* Turns the current entry towards `angle` by at most `step`. */
ADDRESS(0x801eb814, 0x6c)
void fdat_801eb814(short angle, int step)
{
    Vis212 *e = vis_table_a.unk6138;
    int d = (angle - e->unk8E) & 0xfff;
    int a;

    if (d > 2048)
        d -= 4096;
    a = abs(d);
    if (a < step)
        step = a;
    if (d != 0) {
        if (d > 0)
            e->unk8E += step;
        else
            e->unk8E -= step;
    }
}

/* Sets the current entry's horizontal velocity: `speed` along `angle`. */
ADDRESS(0x801eb880, 0x74)
void fdat_801eb880(short angle, int speed)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk9C.vx = speed * rcos(angle) >> 12;
    e->unk9C.vz = speed * rsin(angle) >> 12;
}

/* Steers the current entry's vertical speed towards height `y`. */
ADDRESS(0x801eb8f4, 0x4c)
void fdat_801eb8f4(int y, int step)
{
    Vis212 *e = vis_table_a.unk6138;

    y -= e->unk74.vy;
    if (y > 0)
        e->unk16 += step;
    else if (y < 0)
        e->unk16 -= step;
}

/* A random note offset, -2..2. */
ADDRESS(0x801eb940, 0x2c)
int fdat_801eb940(Vis212 *e)
{
    return (rand() * 5 >> 15) - 2;
}

/* Plays the entry's script sound. */
ADDRESS(0x801eb96c, 0xe0)
void fdat_801eb96c(Vis212 *e)
{
    int sound = e->unkA8->unk05;

    e->unk3A = e->unk0B * 10 + 48 + sound;
    if (e->unk70 & 0x800000)
        e->unk22 = func_80058254(e->unk0B * 10 + 48 + sound, &e->unk74, 127, 0x6000, 0x7800,
                                 fdat_801eb940(e));
    else
        e->unk22 = func_80058220(e->unk0B * 10 + 48 + sound, &e->unk74, 127, fdat_801eb940(e));
}

/*
 * Plays the script sound when its trigger fires this frame.
 * Not exact: retail keeps the 0x1000/0x4000 frame in a0 and copies it to s0
 * only for the 0x8000 (random) case; this build keeps it in s0 throughout,
 * and loads the trigger before the sound byte.
 */
ADDRESS(0x801eba4c, 0x17c)
void fdat_801eba4c(Vis212 *e)
{
    VisScript *s = e->unkA8;
    u16 frame = e->unk2A;
    u8 sound = s->unk05;
    u16 trigger = s->unk0E;
    u16 step = s->unk0C;
    int at;
    int chance;

    if (sound == 255)
        return;
    at = trigger & 0xfff;
    switch (trigger & 0xf000) {
    case 0x1000:
        if (at >= (short)frame - (short)step && at < (short)frame)
            fdat_801eb96c(e);
        break;
    case 0x2000:
        at = trigger & 0x7ff;
        if (at >= (frame & 0x7ff) - (short)step && at < (frame & 0x7ff))
            fdat_801eb96c(e);
        break;
    case 0x8000:
        chance = at;
        if (chance >= (short)frame - (short)step && chance < (short)frame
            && rand() >> 3 < chance)
            fdat_801eb96c(e);
        break;
    case 0x4000:
        if (cd_state.unk8 / 20 % at == 0)
            fdat_801eb96c(e);
        break;
    }
}

/* Hits the player with the script's damage. */
ADDRESS(0x801ebbc8, 0xc0)
void fdat_801ebbc8(Vis212 *e, VisScript *s)
{
    u16 damage[9];
    u16 effect;
    int i;

    effect = 0;
    damage[0] = s->arg.u[5];
    damage[1] = s->arg.u[6];
    damage[2] = s->arg.u[7];
    for (i = 5; i != -1; i--) {
        if ((s->arg.u[8] >> i) & 1)
            damage[3 + i] = s->arg.u[9];
        else
            damage[3 + i] = 0;
    }
    for (i = 0; i < 8; i++) {
        if ((s->arg.u[8] >> (i + 8)) & 1)
            effect |= 1 << i;
    }
    player_hit(damage, 4096, effect, (long *)&e->unk74);
}

/* Spawns the hits of attack script `s` whose frames the animation passes. */
ADDRESS(0x801ebc88, 0x23c)
void fdat_801ebc88(Vis212 *e, VisScript *s)
{
    VECTOR pos;
    SVECTOR v;
    VECTOR node;
    SVECTOR off;
    MATRIX m;
    SVECTOR rot;
    StAnimFile *file;
    int id = e->unk0B + 128;
    int i;
    int plain;

    file = world_state.unk834C[id];
    if (file == NULL)
        return;
    plain = file->unk6 == 0;
    for (i = 2; i != -1; i--) {
        if (s->arg.hit.frame[i] == 0xffff)
            continue;
        if (!((short)e->unk2A - e->unkB0 < s->arg.hit.frame[i]))
            continue;
        if ((short)e->unk2A < s->arg.hit.frame[i])
            continue;
        pos = e->unk74;
        rot.vx = 0;
        rot.vy = -e->unk8E;
        rot.vz = 0;
        RotMatrix(&rot, &m);
        if (s->arg.hit.vertex[i] != 0xffff) {
            if (plain) {
                anim_get_vertex(id, e->unk08, (short)e->unk2A, s->arg.hit.vertex[i], &v);
            } else {
                anim_set_node_vertex((StAnimSet *)e->unkA4, id, s->arg.hit.node[i],
                                     s->arg.hit.vertex[i], &node);
                v.vx = node.vx;
                v.vy = node.vy;
                v.vz = node.vz;
            }
            ApplyMatrixSV(&m, &v, &v);
            pos.vx += v.vx;
            pos.vy += v.vy;
            pos.vz += v.vz;
        }
        off.vx = s->arg.hit.offset[0];
        off.vy = s->arg.hit.offset[1];
        off.vz = s->arg.hit.offset[2];
        ApplyMatrixSV(&m, &off, &off);
        pos.vx += off.vx;
        pos.vy += off.vy;
        pos.vz += off.vz;
        func_8004cee8(e, s->arg.hit.unk1A, &pos);
        e->unk62 = 0;
    }
}
