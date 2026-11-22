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
void creature_return_home(Creature *e)
{
    MapBlock *r;
    u16 i = e->home_block;

    e->block = i;
    r = &vis_records[i];
    e->position.vx = (r->cell_x << 12) + e->home_offset_x;
    e->position.vy = (r->cell_y << 12) + e->home_offset_y;
    e->position.vz = (r->cell_z << 12) + e->home_offset_z;
}

/* Clears the rotation and sets the facing to unk68, or a random one when
 * unk68 is 0xffff. */
ADDRESS(0x8004ac9c, 0x50)
void creature_reset_facing(Creature *e)
{
    e->rot_z = 0;
    e->rot_x = 0;
    if (e->home_facing == 0xffff)
        e->facing = rand() >> 3;
    else
        e->facing = e->home_facing;
}

/* Copies the entry's settings from its record and resets its scale. */
ADDRESS(0x8004acec, 0x84)
void creature_copy_type_stats(Creature *e)
{
    CreatureType *r = &vis_table_a.types[e->type];

    e->kind = r->kind;
    e->hp = r->max_hp;
    e->radius = r->radius;
    e->height = r->height;
    e->shadow_size = r->shadow_size;
    e->flags = r->flags;
    e->hurt_timer = -1;
    e->fade = -1;
    e->scale.vx = e->scale.vy = e->scale.vz = 4096;
}

/* Resets the entry's state for a fresh start. */
ADDRESS(0x8004ad70, 0xc8)
void creature_reset_for_spawn(Creature *e)
{
    CreatureType *r = &vis_table_a.types[e->type];

    e->state = 1;
    e->anim = 0;
    e->anim_frame = 0;
    e->unk1B = 0;
    e->move_mode = r->move_mode;
    e->command = 2;
    e->script_state = 255;
    e->voice = -1;
    e->fade = 4095;
    e->status.bytes[2] = 13;
    e->script = NULL;
    e->unk23 = 1;
    if (e->home_facing == 0xffff)
        e->facing = rand() >> 3;
    e->unk20 = 55;
    e->velocity.vx = e->velocity.vy = e->velocity.vz = 0;
    e->unkB2 = 0;
    e->unk24 = 2048;
}

/* Retires the entry: counts a kill unless it is state 5, then frees its
 * share of the totals and puts it back in its cell. */
ADDRESS(0x8004ae38, 0xec)
void creature_despawn(Creature *e)
{
    CreatureType *r = &vis_table_a.types[e->type];

    if (e->state == 5) {
        vis_table_a.zone_history[0] = vis_table_a.zone_history[1] = vis_table_a.zone_history[2] =
            vis_table_a.zone_history[3] = 255;
    } else if (e < &vis_table_a.entries[48]) {
        game_progress.unk16E4[area_state.area]++;
        viewer.creatures_killed++;
    }
    e->state = 3;
    vis_table_a.unk612C -= r->unk12;
    vis_table_a.spawn_cost_total -= e->spawn_cost;
    creature_return_home(e);
}

ADDRESS(0x8004af24, 0x58)
void creature_spawn(Creature *e, CreatureType *r)
{
    creature_return_home(e);
    creature_reset_facing(e);
    creature_reset_for_spawn(e);
    if (r->unk00 == 2)
        e->visibility = 2;
}

ADDRESS(0x8004af7c, 0x60)
void creature_spawn_by_index(u16 i)
{
    Creature *e = &vis_table_a.entries[i];

    creature_spawn(e, &vis_table_a.types[e->type]);
}

/* Frees every entry and clears the totals. */
ADDRESS(0x8004afdc, 0x7c)
void creatures_clear(void)
{
    Creature *e;
    u16 i;

    vis_table_a.frame = 0;
    e = vis_table_a.entries;
    for (i = 0; i < 64; i++, e++) {
        e->spawn_chance = 255;
        e->state = 0;
        e->anim_slot = 0;
    }
    vis_table_a.zone = 254;
    vis_table_a.skip_flags = 0;
    vis_table_a.zone_history[0] = vis_table_a.zone_history[1] = vis_table_a.zone_history[2] =
        vis_table_a.zone_history[3] = 255;
    vis_table_a.prev_zone = 255;
    vis_table_a.unk6130 = 0;
    vis_table_a.unk612C = 0;
    vis_table_a.spawn_cost_total = 0;
}

ADDRESS(0x8004b058, 0x38)
void creature_set_script(Creature *e, u8 *p)
{
    e->prev_command = e->command;
    if (p != NULL) {
        e->script = (CreatureScript *)p;
        e->command = *p;
        e->script_state = 0;
    } else {
        e->script = NULL;
        e->command = 255;
        e->script_state = 255;
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
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    u8 **list;
    u8 *p;
    u8 *best;
    int score;
    int max;
    int n;

    if (e->script_state == 240 || e->script_state == 0)
        return;
    list = (u8 **)r->scripts;
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
    if (best != NULL && (best != (u8 *)e->script || e->script_state == 255))
        creature_set_script(e, best);
}

ADDRESS(0x8004b19c, 0x4c)
void creature_pick_script_by_distance(void)
{
    creature_pick_script(vector_length_xz(vis_table_a.current->position.vx - viewer.position.vx,
                                vis_table_a.current->position.vz - viewer.position.vz));
}

/* Finds the entry of r's list whose first byte is id. */
ADDRESS(0x8004b1e8, 0x48)
u8 *creature_type_find_script(CreatureType *r, u8 id)
{
    u8 **list = (u8 **)r->scripts;
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
void creature_set_script_by_id(Creature *e, u8 id)
{
    creature_set_script(e, creature_type_find_script(vis_table_a.types, id));
}

ADDRESS(0x8004b270, 0x44)
void creature_end_script(void)
{
    vis_table_a.current->script_state = 255;
    area_state.hooks->creature_pick_move();
}

ADDRESS(0x8004b2b4, 0x28)
void creature_set_anim(u8 v)
{
    Creature *e;

    if (v != 255) {
        e = vis_table_a.current;
        e->anim = v;
        e->anim_frame = 0;
    }
}

/* Steps the current entry's state: 0 waits to spawn, 1 and 2 despawn. */
ADDRESS(0x8004b2dc, 0x2fc)
void creature_update_spawn(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    u8 cur;

    switch (e->state) {
    case 0:
        if (e->zone != 255 || e->player_dist_xz >= 28672) {
            cur = vis_table_a.zone;
            if (cur == vis_table_a.zone_history[0] && cur == vis_table_a.zone_history[1] &&
                cur == vis_table_a.zone_history[2] && cur == vis_table_a.zone_history[3])
                return;
            if (vis_table_a.zone == 255 || vis_table_a.zone != e->zone)
                return;
        }
        if (vis_table_a.spawn_cost_total + e->spawn_cost > 100 && vis_table_a.current_index < 32)
            return;
        if (e->player_dist_xz < 4096 && vis_table_a.current_index < 56)
            return;
        if (e->spawn_chance != 0 && e->spawn_chance >= rand() / 327) {
            vis_table_a.unk612C += r->unk12;
            vis_table_a.spawn_cost_total += e->spawn_cost;
            creature_spawn(vis_table_a.current, vis_table_a.current_type);
            e->unk1B = 255;
            e->aggressive = e->range_bits = 0xffff;
            e->action = 0;
            if (r->actions & 0x80)
                e->action = 128;
            area_state.hooks->creature_pick_move();
        } else if (e->spawn_chance != 0 && vis_table_a.current_index < 32) {
            e->zone = 253;
        }
        return;
    case 1:
        cur = vis_table_a.zone;
        if (cur == 255 || cur == e->zone)
            return;
        if (e->zone == 255 && e->player_dist_xz < 28672)
            return;
        vis_table_a.unk612C -= r->unk12;
        vis_table_a.spawn_cost_total -= e->spawn_cost;
        e->state = 0;
        creature_return_home(e);
        return;
    case 2:
        if (e->player_dist_xz < 28672) {
            cur = vis_table_a.zone;
            if (cur == 255 || cur == e->zone)
                return;
        }
        vis_table_a.unk612C -= r->unk12;
        vis_table_a.spawn_cost_total -= e->spawn_cost;
        e->state = 0;
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
    Creature *e;
    Creature *best;
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
        if (e->state == 1 && e != vis_table_a.current) {
            r = range;
            if (e->flags & 0x20000)
                r <<= 1;
            v.vx = e->position.vx;
            v.vy = e->position.vy - e->height;
            v.vz = e->position.vz;
            d = vector_distance_xz_within(&e->position, pos->vx, 0xffff, pos->vz, r, 0, 0);
            if (d != -1) {
                vector_to_angles(e->position.vx - pos->vx, e->position.vy - pos->vy, e->position.vz - pos->vz,
                              &dir);
                dir.vy = (dir.vy - yaw) & 0xfff;
                if (dir.vy >= 2048)
                    dir.vy = 4096 - dir.vy;
                dir.vx = (dir.vx - pitch) & 0xfff;
                if (dir.vx >= 2048)
                    dir.vx = 4096 - dir.vx;
                if ((e->flags & 0x20000) && arg7 >= 0) {
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
int creature_find_in_reach(CollisionState *req, int arg1)
{
    Creature *e;
    CreatureType *r;
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
        if (e->state != 1)
            continue;
        if (e->flags & vis_table_a.skip_flags)
            continue;
        if (e == vis_table_a.current)
            continue;
        r = &vis_table_a.types[e->type];
        if (mode == 1 &&
            !angle_within(viewer.view_rot.vy,
                           (short)angle_from_xz(e->position.vx - viewer.position.vx, e->position.vz - viewer.position.vz),
                           256))
            continue;
        if (mode == 4 &&
            !angle_within(viewer.view_rot.vy,
                           (short)angle_from_xz(e->position.vx - viewer.position.vx, e->position.vz - viewer.position.vz),
                           384))
            continue;
        if (r->radius == 0)
            continue;
        dx = e->position.vx - req->position.vx;
        if (dx < -10240 || dx > 10240)
            continue;
        dz = e->position.vz - req->position.vz;
        if (dz < -10240 || dz > 10240)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        fwd = req->speed * rcos(req->heading - (short)ratan2(dz, dx)) / 4096;
        if (arg1 & 2)
            lim = e->radius + req->radius + 512;
        else
            lim = e->radius + req->radius;
        if (dist - fwd >= lim)
            continue;
        if (e->position.vy < req->position.vy) {
            if (e->position.vy < req->position.vy - req->height) {
                if (mode == 0)
                    return -1;
                continue;
            }
        } else if (req->position.vy < e->position.vy - e->height) {
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
int creature_find_touching(CollisionState *req)
{
    Creature *e;
    VECTOR v;
    int i;

    e = vis_table_a.entries;
    for (i = 0; i < 64; i++, e++) {
        if (e->state != 1)
            continue;
        if (e->flags & vis_table_a.skip_flags)
            continue;
        if (e == vis_table_a.current)
            continue;
        if (e->flags & 0x10) {
            if (e->home_offset_x == vis_table_a.current_index)
                continue;
            v.vx = e->position.vx;
            v.vz = e->position.vz;
            v.vy = e->position.vy - e->height + e->home_offset_z;
            if (vector_distance_xz_within(&v, req->position.vx, req->position.vy, req->position.vz, e->radius + req->radius,
                              e->height, req->height) != -1)
                return i;
        } else if (e->flags & 0x400000) {
            creature_vertex_offset(e, 0, &v);
            v.vx += e->position.vx;
            v.vz += e->position.vz;
            v.vy += e->position.vy;
            if (vector_distance_xz_within(&v, req->position.vx, req->position.vy, req->position.vz, e->radius + req->radius,
                              e->height, req->height) != -1)
                return i;
        } else {
            v.vx = e->position.vx;
            v.vz = e->position.vz;
            v.vy = e->position.vy;
            if (vector_distance_xz_within(&v, req->position.vx, req->position.vy, req->position.vz, e->radius + req->radius,
                              e->height, req->height) != -1)
                return i;
        }
    }
    return -1;
}

/* Makes e the current entry (unk6138) and caches its record, and for a
 * follower the entry it follows. */
ADDRESS(0x8004c03c, 0xb8)
void creature_set_current(Creature *e)
{
    CreatureType *r;
    Creature *t;

    if (e != NULL) {
        vis_table_a.current = e;
        vis_table_a.current_type = r = &vis_table_a.types[e->type];
        vis_table_a.current_type_index = e->type;
        if (r->flags & 0x10) {
            vis_table_a.leader = t = &vis_table_a.entries[e->home_offset_x];
            vis_table_a.leader_type = &vis_table_a.types[t->type];
        }
    } else {
        vis_table_a.current = NULL;
        vis_table_a.current_type = NULL;
        vis_table_a.current_type_index = 0;
        vis_table_a.leader = NULL;
        vis_table_a.leader_type = NULL;
    }
}

ADDRESS(0x8004c0f4, 0x34)
void creature_anim_step_wrap(Creature *e, short step)
{
    if (step < 0)
        e->anim_step = -step;
    else
        e->anim_step = step;
    e->anim_frame = (e->anim_frame + step) & 0xfff;
}

ADDRESS(0x8004c128, 0x5c)
void creature_anim_step_clamp(Creature *e, short step)
{
    if (step < 0)
        e->anim_step = -step;
    else
        e->anim_step = step;
    e->anim_frame += step;
    if ((short)e->anim_frame >= 4096)
        e->anim_frame = 4095;
    else if ((short)e->anim_frame < 0)
        e->anim_frame = 0;
}

ADDRESS(0x8004c184, 0x30)
int creature_anim_passed_frame(Creature *e, u16 v)
{
    return v < (short)e->anim_frame && v >= (short)e->anim_frame - e->anim_step;
}

/* Moves the entry by its velocity, sliding along up to three walls the
 * request hits, then settles it on the floor. */
ADDRESS(0x8004c1b4, 0x654)
void creature_move(Creature *e)
{
    CreatureType *r;
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
    r = &vis_table_a.types[e->type];
    speed = SquareRoot0(e->velocity.vx * e->velocity.vx + e->velocity.vz * e->velocity.vz);
    heading = ratan2(e->velocity.vz, e->velocity.vx);
    vel = e->velocity;
    if (e->move_mode != 2)
        vel.vy = e->vertical_speed;
    vis_lists.position = e->position;
    vis_lists.position.vy -= e->height;
    vis_lists.radius = e->radius;
    vis_lists.height = e->height;
    vis_lists.tests = e->collision_tests | 0x800;
loop:
    e->velocity.vx = vel.vx;
    e->velocity.vy = vel.vy;
    e->velocity.vz = vel.vz;
    vis_lists.velocity = e->velocity;
    vis_lists.heading = ratan2(e->velocity.vz, e->velocity.vx);
    vis_lists.speed = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
    hits = collision_query(&vis_lists);
    if (tries == 0) {
        if (vis_lists.n_sources != 0) {
            if (e->visibility == 1)
                e->visibility = 0;
            e->block = vis_lists.next_floor_block;
        } else if (e->visibility == 0) {
            e->visibility = 1;
        }
        if (vis_records[e->block].region != e->region) {
            vel.vx = vel.vz = 0;
            goto done;
        }
    }
    if (hits == 0 || tries++ >= 3)
        goto done;
    if (hits & 2) {
        depth = 1 - (vis_lists.wall_dist - e->radius);
        vel.vx = depth * vis_lists.wall_normal_x / 4096;
        vel.vz = depth * vis_lists.wall_normal_z / 4096;
        if (wall0)
            goto done;
        wall0 = 1;
        vis_lists.hit_normal_heading = ratan2(vis_lists.wall_normal_z, vis_lists.wall_normal_x);
        ang = vis_lists.hit_normal_heading + 1024;
        e->wall_heading = ang & 0xfff;
        push = speed * rcos(heading - (ang & 0xfff)) / 4096;
        vis_lists.slide_speed = push;
    } else if (hits & 8) {
        depth = 1 - (vis_lists.low_wall_dist - e->radius);
        vel.vx = depth * vis_lists.low_wall_normal_x / 4096;
        vel.vz = depth * vis_lists.low_wall_normal_z / 4096;
        if (wall1)
            goto done;
        wall1 = 1;
        vis_lists.hit_normal_heading = ratan2(vis_lists.low_wall_normal_z, vis_lists.low_wall_normal_x);
        ang = vis_lists.hit_normal_heading + 1024;
        e->wall_heading = ang & 0xfff;
        push = speed * rcos(heading - (ang & 0xfff)) / 4096;
        vis_lists.slide_speed = push;
    } else if (hits & 0xd0) {
        if (hits & 0x80)
            e->touched_player = 1;
        collision_locate_hit_body(&e->position);
        d.vx = e->position.vx - vis_lists.hit_body_position.vx;
        d.vy = 0;
        d.vz = e->position.vz - vis_lists.hit_body_position.vz;
        VectorNormalS(&d, &n);
        vel.vx = 0;
        vel.vz = 0;
        if (slid)
            goto done;
        slid = 1;
        vis_lists.hit_normal_heading = ratan2(d.vz, d.vx);
        ang = vis_lists.hit_normal_heading + 1024;
        push = speed * rcos(heading - (ang & 0xfff)) / 4096;
        vis_lists.slide_speed = push;
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
        dy = vis_lists.next_floor_dist;
        vel.vy += dy - e->height;
    }
    dy = vis_lists.next_floor_dist - e->height;
    e->height_above_floor = dy;
    if (e->move_mode == 1 || e->move_mode == 2) {
        vel.vy = 0;
    } else if (dy > 0 && e->move_mode != 4) {
        if (e->height < dy && e->move_mode != 8) {
            vel.vx = vel.vz = 0;
        } else if (e->vertical_speed > 0) {
            if (dy < vel.vy) {
                vel.vy = dy;
                e->vertical_speed = 0;
            } else {
                e->vertical_speed += r->gravity;
            }
        } else if (e->vertical_speed < 0) {
            if (dy < e->vertical_speed) {
                vel.vy += dy;
                e->vertical_speed = 0;
            } else {
                e->vertical_speed += r->gravity;
            }
        } else if (dy < r->gravity) {
            vel.vy += dy;
            e->vertical_speed = 0;
        } else {
            e->vertical_speed += r->gravity;
        }
    } else if (dy < 0) {
        if (-dy < e->height >> 1) {
            vel.vy += dy;
            e->vertical_speed = 0;
        } else {
            vel.vx = vel.vz = 0;
        }
    } else {
        vel.vy = 0;
        e->vertical_speed = 0;
    }
    e->velocity.vx = vel.vx;
    e->velocity.vy = vel.vy;
    e->velocity.vz = vel.vz;
}

/* Moves the current entry vertically, clamped between the floor and the
 * ceiling the height probe finds. */
ADDRESS(0x8004c808, 0x12c)
void creature_move_vertical(void)
{
    Creature *e = vis_table_a.current;

    vis_lists.position = e->position;
    vis_lists.position.vy += e->velocity.vy - e->height;
    vis_lists.radius = e->radius;
    vis_lists.height = e->height;
    vis_lists.velocity = e->velocity;
    vis_lists.heading = ratan2(e->velocity.vz, e->velocity.vx);
    vis_lists.tests = 2;
    collision_test_map(&vis_lists);
    if (vis_lists.floor_y < e->position.vy) {
        e->position.vy = vis_lists.floor_y;
        e->velocity.vy = 0;
    } else if (e->position.vy - e->height < vis_lists.ceiling_y) {
        e->position.vy = vis_lists.ceiling_y + e->height;
        e->velocity.vy = 0;
    } else {
        e->position.vy += e->velocity.vy;
    }
}

/* The world position of vertex `index` of the entry's model, or of its
 * centre when the model has none. */
ADDRESS(0x8004c934, 0x100)
int creature_vertex_offset(Creature *e, int index, VECTOR *out)
{
    SVECTOR rot;
    SVECTOR v;

    if (anim_get_vertex(e->model_slot + 128, e->anim, (short)e->anim_frame, index, &v)) {
        v.vx = 0;
        v.vy = -e->height >> 1;
        v.vz = -e->radius;
    } else {
        v.vx = v.vx * e->scale.vx >> 12;
        v.vy = v.vy * e->scale.vy >> 12;
        v.vz = v.vz * e->scale.vz >> 12;
    }
    rot.vx = e->rot_x;
    rot.vy = e->facing + 2048;
    rot.vz = e->rot_z;
    vector_rotate(&rot, &v, out);
    return 1;
}

ADDRESS(0x8004ca34, 0x5c)
VECTOR *creature_get_position(Creature *e, VECTOR *out)
{
    switch (e->flags & 3) {
    case 0:
        return &e->position;
    case 1:
        out->vx = e->position.vx;
        out->vy = e->position.vy;
        out->vz = e->position.vz;
        return out;
    case 2:
        out->vx = e->position.vx;
        out->vy = e->position.vy;
        out->vz = e->position.vz;
        return out;
    }
    return &e->position;
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
 * the aim point by the player's motion (viewer.velocity). Stores the final
 * rotation (facing the other way) in out and returns its yaw. */
ADDRESS(0x8004cc70, 0x278)
int creature_aim_shot(e, pos, speed, target, dir, out, pitch, turn, n)
Creature *e;
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
        d = (rot.vy - e->facing + 2048) & 0xfff;
        if (d >= 2048)
            d = 4096 - d;
        if (d != 0)
            d = k / d;
        if (d > 4096)
            d = 4096;
        t = angle_lerp(rot.vy, e->facing + 2048, d);
        rot.vy = t;
        t = pitch;
        if (t == -1) {
            d = (rot.vx - (short)e->rot_x) & 0xfff;
            if (d >= 2048)
                d = 4096 - d;
            rot.vx = angle_lerp(rot.vx, (short)e->rot_x, d);
        } else {
            rot.vx = pitch;
        }
        vector_from_angles(&rot, dir);
        vector_scale((short)speed, dir);
        if (--n == 0)
            break;
        vector_add_scaled(&p, &viewer.velocity,
                      vector_length_xyz(target->vx - p.vx, target->vy - p.vy, target->vz - p.vz) / speed,
                      &p);
    }
    rot.vy += 2048;
    *out = rot;
    return rot.vy;
}
