/*
 * ST.EXE: placing a table A entry (WIP module; its callers 0x8004ae38 and
 * 0x8004af24 follow in the next band and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/anim.h>

/* Moves the entry to the cell of record unk28, offset by unk6A..unk6E. */
ADDRESS(0x8004ac38, 0x64)
void creature_return_home(Vis212 *e)
{
    VisRecord *r;
    u16 i = e->unk28;

    e->unk26 = i;
    r = &vis_records[i];
    e->unk74.vx = (r->unk08 << 12) + e->unk6A;
    e->unk74.vy = (r->unk09 << 12) + e->unk6C;
    e->unk74.vz = (r->unk0a << 12) + e->unk6E;
}

/* Clears the rotation and sets the facing to unk68, or a random one when
 * unk68 is 0xffff. */
ADDRESS(0x8004ac9c, 0x50)
void creature_reset_facing(Vis212 *e)
{
    e->unk90 = 0;
    e->unk8C = 0;
    if (e->unk68 == 0xffff)
        e->unk8E = rand() >> 3;
    else
        e->unk8E = e->unk68;
}

/* Copies the entry's settings from its record and resets its scale. */
ADDRESS(0x8004acec, 0x84)
void creature_copy_type_stats(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    e->unk01 = r->unk10;
    e->unk2C = r->unk32;
    e->unk2E = r->unk16;
    e->unk30 = r->unk18;
    e->unk32 = r->unk1A;
    e->unk70 = r->unk20;
    e->unk34 = -1;
    e->unk36 = -1;
    e->unk94.vx = e->unk94.vy = e->unk94.vz = 4096;
}

/* Resets the entry's state for a fresh start. */
ADDRESS(0x8004ad70, 0xc8)
void creature_reset_for_spawn(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    e->unk05 = 1;
    e->unk08 = 0;
    e->unk2A = 0;
    e->unk1B = 0;
    e->unk0E = r->unk05;
    e->unk0F = 2;
    e->unk1A = 255;
    e->unk22 = -1;
    e->unk36 = 4095;
    e->unk1C.bytes[2] = 13;
    e->unkA8 = NULL;
    e->unk23 = 1;
    if (e->unk68 == 0xffff)
        e->unk8E = rand() >> 3;
    e->unk20 = 55;
    e->unk9C.vx = e->unk9C.vy = e->unk9C.vz = 0;
    e->unkB2 = 0;
    e->unk24 = 2048;
}

/* Retires the entry: counts a kill unless it is state 5, then frees its
 * share of the totals and puts it back in its cell. */
ADDRESS(0x8004ae38, 0xec)
void creature_despawn(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    if (e->unk05 == 5) {
        vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] =
            vis_table_a.unk6158[3] = 255;
    } else if (e < &vis_table_a.entries[48]) {
        game_progress.unk16E4[area_state.area]++;
        viewer.unk30++;
    }
    e->unk05 = 3;
    vis_table_a.unk612C -= r->unk12;
    vis_table_a.unk6128 -= e->unk0D;
    creature_return_home(e);
}

ADDRESS(0x8004af24, 0x58)
void creature_spawn(Vis212 *e, Vis192 *r)
{
    creature_return_home(e);
    creature_reset_facing(e);
    creature_reset_for_spawn(e);
    if (r->unk00 == 2)
        e->unk03 = 2;
}

ADDRESS(0x8004af7c, 0x60)
void creature_spawn_by_index(u16 i)
{
    Vis212 *e = &vis_table_a.entries[i];

    creature_spawn(e, &vis_table_a.unk3500[e->unk02]);
}

/* Frees every entry and clears the totals. */
ADDRESS(0x8004afdc, 0x7c)
void creatures_clear(void)
{
    Vis212 *e;
    u16 i;

    vis_table_a.unk6150 = 0;
    e = vis_table_a.entries;
    for (i = 0; i < 64; i++, e++) {
        e->unk00 = 255;
        e->unk05 = 0;
        e->unkA4 = 0;
    }
    vis_table_a.unk6168 = 254;
    vis_table_a.unk6101 = 0;
    vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] =
        vis_table_a.unk6158[3] = 255;
    vis_table_a.unk6103 = 255;
    vis_table_a.unk6130 = 0;
    vis_table_a.unk612C = 0;
    vis_table_a.unk6128 = 0;
}

ADDRESS(0x8004b058, 0x38)
void creature_set_script(Vis212 *e, u8 *p)
{
    e->unk19 = e->unk0F;
    if (p != NULL) {
        e->unkA8 = (VisScript *)p;
        e->unk0F = *p;
        e->unk1A = 0;
    } else {
        e->unkA8 = NULL;
        e->unk0F = 255;
        e->unk1A = 255;
    }
}

ADDRESS(0x8004b090, 0x8)
int creature_score_script(u8 *p, int arg)
{
    return 0;
}

/* Picks the best-scoring entry of the current record's list for the current
 * entry. */
ADDRESS(0x8004b098, 0x104)
void creature_pick_script(int arg)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    u8 **list;
    u8 *p;
    u8 *best;
    int score;
    int max;
    int n;

    if (e->unk1A == 240 || e->unk1A == 0)
        return;
    list = (u8 **)r->unk60;
    max = -2;
    best = NULL;
    n = 24;
    while (n--) {
        p = *list++;
        if (p == NULL)
            break;
        score = creature_score_script(p, arg);
        if (max < score) {
            max = score;
            best = p;
        }
    }
    if (best != NULL && (best != (u8 *)e->unkA8 || e->unk1A == 255))
        creature_set_script(e, best);
}

ADDRESS(0x8004b19c, 0x4c)
void creature_pick_script_by_distance(void)
{
    creature_pick_script(vector_length_xz(vis_table_a.unk6138->unk74.vx - viewer.position.vx,
                                vis_table_a.unk6138->unk74.vz - viewer.position.vz));
}

/* Finds the entry of r's list whose first byte is id. */
ADDRESS(0x8004b1e8, 0x48)
u8 *creature_type_find_script(Vis192 *r, u8 id)
{
    u8 **list = (u8 **)r->unk60;
    u8 *p;
    int n = 24;

    while (n--) {
        p = *list++;
        if (p == NULL)
            break;
        if (*p == id)
            return p;
    }
    return NULL;
}

ADDRESS(0x8004b230, 0x40)
void creature_set_script_by_id(Vis212 *e, u8 id)
{
    creature_set_script(e, creature_type_find_script(vis_table_a.unk3500, id));
}

ADDRESS(0x8004b270, 0x44)
void creature_end_script(void)
{
    vis_table_a.unk6138->unk1A = 255;
    area_state.hooks->unk40();
}

ADDRESS(0x8004b2b4, 0x28)
void creature_set_anim(u8 v)
{
    Vis212 *e;

    if (v != 255) {
        e = vis_table_a.unk6138;
        e->unk08 = v;
        e->unk2A = 0;
    }
}

/* Steps the current entry's state: 0 waits to spawn, 1 and 2 despawn. */
ADDRESS(0x8004b2dc, 0x2fc)
void creature_update_spawn(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    u8 cur;

    switch (e->unk05) {
    case 0:
        if (e->unk07 != 255 || e->unk40 >= 28672) {
            cur = vis_table_a.unk6168;
            if (cur == vis_table_a.unk6158[0] && cur == vis_table_a.unk6158[1] &&
                cur == vis_table_a.unk6158[2] && cur == vis_table_a.unk6158[3])
                return;
            if (vis_table_a.unk6168 == 255 || vis_table_a.unk6168 != e->unk07)
                return;
        }
        if (vis_table_a.unk6128 + e->unk0D > 100 && vis_table_a.unk6144 < 32)
            return;
        if (e->unk40 < 4096 && vis_table_a.unk6144 < 56)
            return;
        if (e->unk00 != 0 && e->unk00 >= rand() / 327) {
            vis_table_a.unk612C += r->unk12;
            vis_table_a.unk6128 += e->unk0D;
            creature_spawn(vis_table_a.unk6138, vis_table_a.unk6134);
            e->unk1B = 255;
            e->unk54 = e->unk60 = 0xffff;
            e->unk44 = 0;
            if (r->unk1C & 0x80)
                e->unk44 = 128;
            area_state.hooks->unk40();
        } else if (e->unk00 != 0 && vis_table_a.unk6144 < 32) {
            e->unk07 = 253;
        }
        return;
    case 1:
        cur = vis_table_a.unk6168;
        if (cur == 255 || cur == e->unk07)
            return;
        if (e->unk07 == 255 && e->unk40 < 28672)
            return;
        vis_table_a.unk612C -= r->unk12;
        vis_table_a.unk6128 -= e->unk0D;
        e->unk05 = 0;
        creature_return_home(e);
        return;
    case 2:
        if (e->unk40 < 28672) {
            cur = vis_table_a.unk6168;
            if (cur == 255 || cur == e->unk07)
                return;
        }
        vis_table_a.unk612C -= r->unk12;
        vis_table_a.unk6128 -= e->unk0D;
        e->unk05 = 0;
        creature_return_home(e);
        return;
    }
}

ADDRESS(0x8004b5d8, 0x80)
int func_8004b5d8(int a, int b, int c)
{
    int d;

    b <<= 4;
    c <<= 4;
    a <<= 4;
    if (b == 0)
        return 0;
    b += a;
    d = b - c;
    if (d < 0)
        d = 0;
    if (c == 0)
        c = 16;
    return d + (b * b) / (c * 2);
}

/* Finds the live entry (other than the current one) nearest pos's line of
 * sight: within range, inside the yaw/pitch cones, scored by angle and
 * distance, with a random term when arg7 > 0. */
ADDRESS(0x8004b658, 0x294)
void *creature_find_in_cone(VECTOR *pos, short yaw, short pitch, int range, int arg4, int arg5, int *dist,
                    int arg7)
{
    Vis212 *e;
    Vis212 *best;
    VECTOR v;
    SVECTOR dir;
    int r;
    int d;
    int score;
    int min;
    int min_d;
    u16 n;

    best = NULL;
    min = 30000;
    min_d = -1;
    e = vis_table_a.entries;
    n = 64;
    while (n--) {
        if (e->unk05 == 1 && e != vis_table_a.unk6138) {
            r = range;
            if (e->unk70 & 0x20000)
                r <<= 1;
            v.vx = e->unk74.vx;
            v.vy = e->unk74.vy - e->unk30;
            v.vz = e->unk74.vz;
            d = vector_distance_xz_within(&e->unk74, pos->vx, 0xffff, pos->vz, r, 0, 0);
            if (d != -1) {
                vector_to_angles(e->unk74.vx - pos->vx, e->unk74.vy - pos->vy, e->unk74.vz - pos->vz,
                              &dir);
                dir.vy = (dir.vy - yaw) & 0xfff;
                if (dir.vy >= 2048)
                    dir.vy = 4096 - dir.vy;
                dir.vx = (dir.vx - pitch) & 0xfff;
                if (dir.vx >= 2048)
                    dir.vx = 4096 - dir.vx;
                if ((e->unk70 & 0x20000) && arg7 >= 0) {
                    d >>= 2;
                    dir.vx -= 512;
                    dir.vy -= 512;
                } else if (dir.vy > arg4 || dir.vx > arg5) {
                    goto next;
                }
                score = dir.vy + dir.vx + (d >> 7);
                if (arg7 > 0)
                    score += (rand() * arg7) >> 15;
                if (score < min) {
                    min = score;
                    best = e;
                    min_d = d;
                }
            }
        }
    next:
        e++;
    }
    *dist = min_d;
    return best;
}

/* Distance from (x, y, z) to the segment a..b, or -1 beyond r (r + extra
 * from its interior). */
ADDRESS(0x8004b8ec, 0x29c)
int vector_point_segment_distance(int x, int y, int z, int r, VECTOR *a, VECTOR *b, int extra)
{
    int dx;
    int dy;
    int dz;
    int t;
    int len2;
    int d;

    dx = b->vx - a->vx;
    dy = b->vy - a->vy;
    dz = b->vz - a->vz;
    if (dx != 0 && dy != 0 && dz != 0) {
        t = (x - a->vx) * dx + (y - a->vy) * dy + (z - a->vz) * dz;
        if (t >= 0) {
            len2 = dx * dx + dy * dy + dz * dz;
            if (t <= len2) {
                dx = dx * t / len2 + a->vx - x;
                dy = dy * t / len2 + a->vy - y;
                dz = dz * t / len2 + a->vz - z;
                d = SquareRoot0(dx * dx + dz * dz + dy * dy);
                if (r + extra >= d)
                    return d;
            } else {
                dx = b->vx - x;
                dz = b->vz - z;
                dy = b->vy - y;
                d = SquareRoot0(dx * dx + dz * dz + dy * dy);
                if (r >= d)
                    return d;
            }
        } else {
            dx = a->vx - x;
            dz = a->vz - z;
            dy = a->vy - y;
            d = SquareRoot0(dx * dx + dz * dz + dy * dy);
            if (r >= d)
                return d;
        }
    }
    return -1;
}

/* Finds the live entry in front of req within its reach: mode (arg1 & 5)
 * 1 and 4 also require it inside the viewer's view cone; mode 0 returns the
 * first hit, otherwise the nearest. */
ADDRESS(0x8004bb88, 0x2f8)
int creature_find_in_reach(VisLists *req, int arg1)
{
    Vis212 *e;
    Vis192 *r;
    int mode;
    int i;
    int best;
    int min;
    int dx;
    int dz;
    int dist;
    int fwd;
    int lim;
    int miss;

    min = 0x7fffffff;
    best = -1;
    mode = arg1 & 5;
    for (e = vis_table_a.entries, i = 0; i < 64; i++, e++) {
        if (e->unk05 != 1)
            continue;
        if (e->unk70 & vis_table_a.unk6101)
            continue;
        if (e == vis_table_a.unk6138)
            continue;
        r = &vis_table_a.unk3500[e->unk02];
        if (mode == 1 &&
            !angle_within(viewer.unk278.vy,
                           (short)angle_from_xz(e->unk74.vx - viewer.position.vx, e->unk74.vz - viewer.position.vz),
                           256))
            continue;
        if (mode == 4 &&
            !angle_within(viewer.unk278.vy,
                           (short)angle_from_xz(e->unk74.vx - viewer.position.vx, e->unk74.vz - viewer.position.vz),
                           384))
            continue;
        if (r->unk16 == 0)
            continue;
        dx = e->unk74.vx - req->unk4.vx;
        if (dx < -10240 || dx > 10240)
            continue;
        dz = e->unk74.vz - req->unk4.vz;
        if (dz < -10240 || dz > 10240)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        fwd = req->unk22 * rcos(req->unk20 - (short)ratan2(dz, dx)) / 4096;
        if (arg1 & 2)
            lim = e->unk2E + req->unk1C + 512;
        else
            lim = e->unk2E + req->unk1C;
        if (dist - fwd >= lim)
            continue;
        if (e->unk74.vy < req->unk4.vy) {
            if (e->unk74.vy < req->unk4.vy - req->unk1E) {
                if (mode == 0)
                    return -1;
                continue;
            }
        } else if (req->unk4.vy < e->unk74.vy - e->unk30) {
            if (mode == 0)
                return -1;
            continue;
        }
        if (mode == 0)
            return i;
        if (dist - fwd < min) {
            min = dist - fwd;
            best = i;
        }
    }
    if (mode == 0)
        return -1;
    return best;
}

/* Finds the first live entry whose body req touches. */
ADDRESS(0x8004be80, 0x1bc)
int creature_find_touching(VisLists *req)
{
    Vis212 *e;
    VECTOR v;
    int i;

    e = vis_table_a.entries;
    for (i = 0; i < 64; i++, e++) {
        if (e->unk05 != 1)
            continue;
        if (e->unk70 & vis_table_a.unk6101)
            continue;
        if (e == vis_table_a.unk6138)
            continue;
        if (e->unk70 & 0x10) {
            if (e->unk6A == vis_table_a.unk6144)
                continue;
            v.vx = e->unk74.vx;
            v.vz = e->unk74.vz;
            v.vy = e->unk74.vy - e->unk30 + e->unk6E;
            if (vector_distance_xz_within(&v, req->unk4.vx, req->unk4.vy, req->unk4.vz, e->unk2E + req->unk1C,
                              e->unk30, req->unk1E) != -1)
                return i;
        } else if (e->unk70 & 0x400000) {
            creature_vertex_offset(e, 0, &v);
            v.vx += e->unk74.vx;
            v.vz += e->unk74.vz;
            v.vy += e->unk74.vy;
            if (vector_distance_xz_within(&v, req->unk4.vx, req->unk4.vy, req->unk4.vz, e->unk2E + req->unk1C,
                              e->unk30, req->unk1E) != -1)
                return i;
        } else {
            v.vx = e->unk74.vx;
            v.vz = e->unk74.vz;
            v.vy = e->unk74.vy;
            if (vector_distance_xz_within(&v, req->unk4.vx, req->unk4.vy, req->unk4.vz, e->unk2E + req->unk1C,
                              e->unk30, req->unk1E) != -1)
                return i;
        }
    }
    return -1;
}

/* Makes e the current entry (unk6138) and caches its record, and for a
 * follower the entry it follows. */
ADDRESS(0x8004c03c, 0xb8)
void creature_set_current(Vis212 *e)
{
    Vis192 *r;
    Vis212 *t;

    if (e != NULL) {
        vis_table_a.unk6138 = e;
        vis_table_a.unk6134 = r = &vis_table_a.unk3500[e->unk02];
        vis_table_a.unk6148 = e->unk02;
        if (r->unk20 & 0x10) {
            vis_table_a.unk6140 = t = &vis_table_a.entries[e->unk6A];
            vis_table_a.unk613C = &vis_table_a.unk3500[t->unk02];
        }
    } else {
        vis_table_a.unk6138 = NULL;
        vis_table_a.unk6134 = NULL;
        vis_table_a.unk6148 = 0;
        vis_table_a.unk6140 = NULL;
        vis_table_a.unk613C = NULL;
    }
}

ADDRESS(0x8004c0f4, 0x34)
void creature_anim_step_wrap(Vis212 *e, short step)
{
    if (step < 0)
        e->unkB0 = -step;
    else
        e->unkB0 = step;
    e->unk2A = (e->unk2A + step) & 0xfff;
}

ADDRESS(0x8004c128, 0x5c)
void creature_anim_step_clamp(Vis212 *e, short step)
{
    if (step < 0)
        e->unkB0 = -step;
    else
        e->unkB0 = step;
    e->unk2A += step;
    if ((short)e->unk2A >= 4096)
        e->unk2A = 4095;
    else if ((short)e->unk2A < 0)
        e->unk2A = 0;
}

ADDRESS(0x8004c184, 0x30)
int creature_anim_passed_frame(Vis212 *e, u16 v)
{
    return v < (short)e->unk2A && v >= (short)e->unk2A - e->unkB0;
}

/* Moves the entry by its velocity, sliding along up to three walls the
 * request hits, then settles it on the floor. */
ADDRESS(0x8004c1b4, 0x654)
void creature_move(Vis212 *e)
{
    Vis192 *r;
    SVECTOR vel;
    VECTOR d;
    SVECTOR n;
    int speed;
    int heading;
    int hits;
    int tries;
    int slid;
    int wall0, wall1;
    int depth;
    int ang;
    int push;
    int dy;

    wall0 = 0;
    wall1 = 0;
    slid = 0;
    tries = 0;
    r = &vis_table_a.unk3500[e->unk02];
    speed = SquareRoot0(e->unk9C.vx * e->unk9C.vx + e->unk9C.vz * e->unk9C.vz);
    heading = ratan2(e->unk9C.vz, e->unk9C.vx);
    vel = e->unk9C;
    if (e->unk0E != 2)
        vel.vy = e->unk16;
    vis_lists.unk4 = e->unk74;
    vis_lists.unk4.vy -= e->unk30;
    vis_lists.unk1C = e->unk2E;
    vis_lists.unk1E = e->unk30;
    vis_lists.unk0 = e->unk50 | 0x800;
loop:
    e->unk9C.vx = vel.vx;
    e->unk9C.vy = vel.vy;
    e->unk9C.vz = vel.vz;
    vis_lists.unk14 = e->unk9C;
    vis_lists.unk20 = ratan2(e->unk9C.vz, e->unk9C.vx);
    vis_lists.unk22 = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
    hits = collision_query(&vis_lists);
    if (tries == 0) {
        if (vis_lists.n_sources != 0) {
            if (e->unk03 == 1)
                e->unk03 = 0;
            e->unk26 = vis_lists.unk47D8;
        } else if (e->unk03 == 0) {
            e->unk03 = 1;
        }
        if (vis_records[e->unk26].unk01 != e->unk06) {
            vel.vx = vel.vz = 0;
            goto done;
        }
    }
    if (hits == 0 || tries++ >= 3)
        goto done;
    if (hits & 2) {
        depth = 1 - (vis_lists.unk3C - e->unk2E);
        vel.vx = depth * vis_lists.unk8C / 4096;
        vel.vz = depth * vis_lists.unk90 / 4096;
        if (wall0)
            goto done;
        wall0 = 1;
        vis_lists.unk78 = ratan2(vis_lists.unk90, vis_lists.unk8C);
        ang = vis_lists.unk78 + 1024;
        e->unk5E = ang & 0xfff;
        push = speed * rcos(heading - (ang & 0xfff)) / 4096;
        vis_lists.unk44C6 = push;
    } else if (hits & 8) {
        depth = 1 - (vis_lists.unk44 - e->unk2E);
        vel.vx = depth * vis_lists.unk9C / 4096;
        vel.vz = depth * vis_lists.unkA0 / 4096;
        if (wall1)
            goto done;
        wall1 = 1;
        vis_lists.unk78 = ratan2(vis_lists.unkA0, vis_lists.unk9C);
        ang = vis_lists.unk78 + 1024;
        e->unk5E = ang & 0xfff;
        push = speed * rcos(heading - (ang & 0xfff)) / 4096;
        vis_lists.unk44C6 = push;
    } else if (hits & 0xd0) {
        if (hits & 0x80)
            e->unk39 = 1;
        collision_locate_hit_body(&e->unk74);
        d.vx = e->unk74.vx - vis_lists.unkAC.vx;
        d.vy = 0;
        d.vz = e->unk74.vz - vis_lists.unkAC.vz;
        VectorNormalS(&d, &n);
        vel.vx = 0;
        vel.vz = 0;
        if (slid)
            goto done;
        slid = 1;
        vis_lists.unk78 = ratan2(d.vz, d.vx);
        ang = vis_lists.unk78 + 1024;
        push = speed * rcos(heading - (ang & 0xfff)) / 4096;
        vis_lists.unk44C6 = push;
    } else {
        goto done;
    }
    if (push != 0) {
        vel.vx += rcos(ang) * push / 4096;
        vel.vz += rsin(ang) * push / 4096;
    }
    goto loop;
done:
    if (hits & 4) {
        dy = vis_lists.unk2C;
        vel.vy += dy - e->unk30;
    }
    dy = vis_lists.unk2C - e->unk30;
    e->unk4C = dy;
    if (e->unk0E == 1 || e->unk0E == 2) {
        vel.vy = 0;
    } else if (dy > 0 && e->unk0E != 4) {
        if (e->unk30 < dy && e->unk0E != 8) {
            vel.vx = vel.vz = 0;
        } else if (e->unk16 > 0) {
            if (dy < vel.vy) {
                vel.vy = dy;
                e->unk16 = 0;
            } else {
                e->unk16 += r->unk0E;
            }
        } else if (e->unk16 < 0) {
            if (dy < e->unk16) {
                vel.vy += dy;
                e->unk16 = 0;
            } else {
                e->unk16 += r->unk0E;
            }
        } else if (dy < r->unk0E) {
            vel.vy += dy;
            e->unk16 = 0;
        } else {
            e->unk16 += r->unk0E;
        }
    } else if (dy < 0) {
        if (-dy < e->unk30 >> 1) {
            vel.vy += dy;
            e->unk16 = 0;
        } else {
            vel.vx = vel.vz = 0;
        }
    } else {
        vel.vy = 0;
        e->unk16 = 0;
    }
    e->unk9C.vx = vel.vx;
    e->unk9C.vy = vel.vy;
    e->unk9C.vz = vel.vz;
}

/* Moves the current entry vertically, clamped between the floor and the
 * ceiling the height probe finds. */
ADDRESS(0x8004c808, 0x12c)
void creature_move_vertical(void)
{
    Vis212 *e = vis_table_a.unk6138;

    vis_lists.unk4 = e->unk74;
    vis_lists.unk4.vy += e->unk9C.vy - e->unk30;
    vis_lists.unk1C = e->unk2E;
    vis_lists.unk1E = e->unk30;
    vis_lists.unk14 = e->unk9C;
    vis_lists.unk20 = ratan2(e->unk9C.vz, e->unk9C.vx);
    vis_lists.unk0 = 2;
    collision_test_map(&vis_lists);
    if (vis_lists.unk47E4 < e->unk74.vy) {
        e->unk74.vy = vis_lists.unk47E4;
        e->unk9C.vy = 0;
    } else if (e->unk74.vy - e->unk30 < vis_lists.unk47E8) {
        e->unk74.vy = vis_lists.unk47E8 + e->unk30;
        e->unk9C.vy = 0;
    } else {
        e->unk74.vy += e->unk9C.vy;
    }
}

/* The world position of vertex `index` of the entry's model, or of its
 * centre when the model has none. */
ADDRESS(0x8004c934, 0x100)
int creature_vertex_offset(Vis212 *e, int index, VECTOR *out)
{
    SVECTOR rot;
    SVECTOR v;

    if (anim_get_vertex(e->unk0B + 128, e->unk08, (short)e->unk2A, index, &v)) {
        v.vx = 0;
        v.vy = -e->unk30 >> 1;
        v.vz = -e->unk2E;
    } else {
        v.vx = v.vx * e->unk94.vx >> 12;
        v.vy = v.vy * e->unk94.vy >> 12;
        v.vz = v.vz * e->unk94.vz >> 12;
    }
    rot.vx = e->unk8C;
    rot.vy = e->unk8E + 2048;
    rot.vz = e->unk90;
    vector_rotate(&rot, &v, out);
    return 1;
}

ADDRESS(0x8004ca34, 0x5c)
VECTOR *creature_get_position(Vis212 *e, VECTOR *out)
{
    switch (e->unk70 & 3) {
    case 0:
        return &e->unk74;
    case 1:
        out->vx = e->unk74.vx;
        out->vy = e->unk74.vy;
        out->vz = e->unk74.vz;
        return out;
    case 2:
        out->vx = e->unk74.vx;
        out->vy = e->unk74.vy;
        out->vz = e->unk74.vz;
        return out;
    }
    return &e->unk74;
}

/* A random offset around base: +-8 across, up to 31 below `up`; out is base
 * less the offset, raised by `height`. */
ADDRESS(0x8004ca90, 0x110)
void effect_random_offset(SVECTOR *off, VECTOR *out, VECTOR *base, int arg3, short height, short up)
{
    off->vx = 8 - rand() / 2024;
    off->vy = up - rand() / 1024;
    off->vz = 8 - rand() / 2024;
    out->vx = base->vx - off->vx;
    out->vy = base->vy + height;
    out->vz = base->vz - off->vz;
}

/* Jitters off and places out at base plus it. */
ADDRESS(0x8004cba0, 0xd0)
void effect_jitter_offset(SVECTOR *off, VECTOR *out, VECTOR *base)
{
    off->vx += 8 - rand() / 2048;
    off->vy += -16 - rand() / 1024;
    off->vz += 8 - rand() / 2048;
    out->vx = base->vx + off->vx;
    out->vy = base->vy;
    out->vz = base->vz + off->vz;
}

/* Aims dir (at `speed`) from `target` at `pos`: the names are swapped,
 * `pos` is the aim point (raised 800; its callers pass the player) and
 * `target` the shot's origin. Turns from the entry's facing by at most
 * `turn` per step; with pitch -1 the pitch turns too. n > 1 steps lead
 * the aim point by the player's motion (viewer.unk260). Stores the final
 * rotation (facing the other way) in out and returns its yaw. */
ADDRESS(0x8004cc70, 0x278)
int creature_aim_shot(e, pos, speed, target, dir, out, pitch, turn, n)
Vis212 *e;
VECTOR *pos;
int speed;
VECTOR *target;
SVECTOR *dir;
SVECTOR *out;
short pitch;
short turn;
int n;
{
    VECTOR p;
    SVECTOR rot;
    int k;
    int d;
    int t;

    p = *pos;
    k = turn << 12;
    p.vy += 800;
    for (;;) {
        vector_to_angles(p.vx - target->vx, p.vy - target->vy, p.vz - target->vz, &rot);
        d = (rot.vy - e->unk8E + 2048) & 0xfff;
        if (d >= 2048)
            d = 4096 - d;
        if (d != 0)
            d = k / d;
        if (d > 4096)
            d = 4096;
        t = angle_lerp(rot.vy, e->unk8E + 2048, d);
        rot.vy = t;
        t = pitch;
        if (t == -1) {
            d = (rot.vx - (short)e->unk8C) & 0xfff;
            if (d >= 2048)
                d = 4096 - d;
            rot.vx = angle_lerp(rot.vx, (short)e->unk8C, d);
        } else {
            rot.vx = pitch;
        }
        vector_from_angles(&rot, dir);
        vector_scale((short)speed, dir);
        if (--n == 0)
            break;
        vector_add_scaled(&p, &viewer.unk260,
                      vector_length_xyz(target->vx - p.vx, target->vy - p.vy, target->vz - p.vz) / speed,
                      &p);
    }
    rot.vy += 2048;
    *out = rot;
    return rot.vy;
}
