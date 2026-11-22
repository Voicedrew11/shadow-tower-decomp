/*
 * ST.EXE: table B (vis_table_b.entries: doors, traps, warps and the other
 * fixed things of an area) and table C (items lying in the areas) as one
 * translation unit (WIP module, -G8: the two shake amounts are
 * $gp-relative statics): loading and running table B, the per-frame pass
 * objects_update, warps, table C and its pick-ups, 0x8004f53c..0x80053d38.
 * Retail's switch tables of the table B code (0x800125d0), objects_update
 * and ground_items_update follow one another, each 8-aligned relative to
 * 0x800125d0, which is 4 mod 8 relative to st.creature_attack's tables
 * before them: one object's read-only data.
 */
/* The area request's prototype is not in scope before vis_c's code: the
 * calls in objects_update pass its last three arguments as sign-extended
 * bytes, as calls through an implicit declaration do (the headers leave it
 * out under this define; it is declared below, before player_warp).
 * Likewise the HUD drawer world_render_frame (st/hud.h is not included): every
 * call here sees it as a function returning int, implicitly at first and
 * then through the declaration before ground_item_pick_up. */
#define ST_AREA_REQUEST_LOAD_OLD_STYLE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/area.h>
#include <st/light.h>
#include <st/player.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/vis.h>
#include <kf/lib/null.h>
#include <st/menu.h>
#include <INLINE_C.H>

RODATA(0x800125d0, 0x17c)

/* The first entry (other than unk71B0) whose record radius, widened by the
 * request's and shortened along its heading, reaches the request at a
 * matching height; -1 when none. */
ADDRESS(0x8004f53c, 0x1dc)
int object_find_in_reach(CollisionState *req)
{
    AreaObject *e = vis_table_b.entries;
    AreaObjectType *r;
    int i;
    int dx;
    int dz;
    int dist;

    for (i = 0; i < 240; i++, e++) {
        if (e->type == 0xffff || e == vis_table_b.current)
            continue;
        r = &vis_table_b.types[e->type];
        if (r->radius == 0)
            continue;
        dx = e->position.vx - req->position.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = e->position.vz - req->position.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        if (dist - req->speed * rcos(req->heading - (short)ratan2(dz, dx)) / 4096
            >= r->radius + req->radius)
            continue;
        if (e->position.vy < req->position.vy) {
            if (e->position.vy < req->position.vy - req->height)
                return -1;
        } else if (req->position.vy < e->position.vy - e->height) {
            return -1;
        }
        return i;
    }
    return -1;
}

/* From entry `i`, the first switch-like entry whose trigger radius holds
 * the request at its height and, for a facing record, faces the viewer;
 * -1 when none. */
ADDRESS(0x8004f718, 0x25c)
int object_find_usable(int start, CollisionState *req)
{
    AreaObject *e = &vis_table_b.entries[start];
    AreaObjectType *r;
    short i;
    int dx;
    int dz;
    int dist;
    short d;
    u8 k;

    for (i = start; i < 240; i++, e++) {
        if (e->type == 0xffff)
            continue;
        r = &vis_table_b.types[e->type];
        if (r->use_radius == 0)
            continue;
        dx = e->position.vx - req->position.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = e->position.vz - req->position.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        ratan2(dz, dx);
        if (dist >= r->use_radius)
            continue;
        if (e->position.vy < req->position.vy ? e->position.vy < req->position.vy - req->height
                                       : req->position.vy < e->position.vy - (e->height + 0x300))
            continue;
        d = (viewer.yaw - e->player_heading) & 0xfff;
        if (d > 0x800)
            d |= 0xf000;
        if ((r->flags & 8) && (u16)(d + 400) > 800)
            continue;
        k = r->unk00[0];
        if ((k >= 50 && k <= 52) || k == 1 || k == 0 || k == 254 || k == 225 || k == 226
            || k == 14)
            return i;
    }
    return -1;
}

/* Starts kind `kind` on an idle entry. */
ADDRESS(0x8004f974, 0x20)
void object_start_kind(e, kind)
    AreaObject *e;
    u8 kind;
{
    if (e->kind == OBJECT_KIND_NONE) {
        e->kind = kind;
        e->state = 0;
    }
}

/* A free entry among `n` from `start`; exits when there is none. */
ADDRESS(0x8004f994, 0xa4)
AreaObject *object_alloc(int start, int n, int now)
{
    AreaObject *e = &vis_table_b.entries[start];
    int oldest = 0;
    int age;

    do {
        if (e->type == 0xffff)
            return e;
        if (now != -1) {
            age = now - *(u16 *)&e->params.snd.fade_range;
            if (age < 0)
                age += 0x10000;
            if (oldest < age)
                oldest = age;
        }
        e++;
    } while (--n);
    exit(1);
    return e;
}

/* Flips the switches keyed `key`. */
ADDRESS(0x8004fa38, 0x150)
void objects_toggle_by_link_id(key)
    u8 key;
{
    AreaObject *e = vis_table_b.entries;
    u16 i;

    if (key == 0xff)
        return;
    i = 240;
    while (i--) {
        switch (e->kind) {
        case 'T':
        case 0xa2:
        case 0xa3:
            if (e->unk48 == key)
                e->unk48 = 0xff;
            break;
        case '_':
            if (e->unk48 == key) {
                e->state = 1;
                ((u8 *)&e->params.snd.period)[1] = ((u8 *)&e->params.snd.period)[1] == 0;
            }
            break;
        case 'X':
            if (e->unk49 == key) {
                e->state = 1;
                e->unk48 = e->unk48 == 0;
            }
            break;
        case 'Q':
            if (e->params.snd.sound_flags == key)
                e->unk48 = e->unk48 ? 0 : 0xff;
            break;
        case 'R':
        case 'U':
        case 'Z':
            if (((u8 *)&e->params.snd.period)[1] == key)
                e->unk48 = e->unk48 ? 0 : 0xff;
            break;
        case 'Y':
            if (e->unk48 == key)
                e->state = 1;
            break;
        case 'V':
            if (e->params.snd.sound == key)
                e->unk49 = e->unk49 ? 0 : 0xff;
            break;
        }
        e++;
    }
}

/* For a switch-like entry at rest: 2 when it has no key, 1 (and the key is
 * used up) when `id` is its key item, 3 otherwise; 0 for other entries. */
ADDRESS(0x8004fb88, 0x84)
int object_try_key(AreaObject *e, int id)
{
    switch (e->kind) {
    case OBJECT_KIND_DOOR:
    case OBJECT_KIND_MODEL_DOOR:
    case OBJECT_KIND_GATE:
    case OBJECT_KIND_ITEM_GATE:
    case OBJECT_KIND_GROUP_GATE:
        if (e->state != 0)
            return 0;
        if (((u8 *)&e->params.snd.period)[1] >= 0xfe)
            return 2;
        if (((u8 *)&e->params.snd.period)[1] == id - KEY_ITEM_BASE) {
            ((u8 *)&e->params.snd.period)[1] = 0xff;
            return 1;
        }
        return 3;
    }
    return 0;
}

/* Offers item `key` to every entry. */
ADDRESS(0x8004fc0c, 0x78)
void objects_try_key(key)
    u8 key;
{
    AreaObject *e = vis_table_b.entries;
    u16 i;

    if (key == 0xff)
        return;
    i = 240;
    while (i--) {
        object_try_key(e, key);
        e++;
    }
}

ADDRESS(0x8004fc84, 0x10)
void func_8004fc84(void)
{
    int unused[4];
}

/* Whether the viewer's cell lies in the w x d cells at (x, z) and, unless
 * y is 0x8000, within 4096 below height y. */
ADDRESS(0x8004fc94, 0x94)
int player_in_cell_rect(int x, int z, int w, int d, int y)
{
    int cx = (viewer.position.vx + 0x800) >> 12;
    int cz = (viewer.position.vz + 0x800) >> 12;

    if (cx < x || cx >= x + w)
        return 0;
    if (cz < z)
        return 0;
    if (cz >= z + d)
        return 0;
    if (y == 0x8000 || (y >= viewer.position.vy && viewer.position.vy >= y - 0x1000))
        return 1;
    return 0;
}

/* The same test for the point half a w x d box past the viewer. */
ADDRESS(0x8004fd28, 0xa8)
int player_in_centred_rect(int x, int z, int w, int d, int y)
{
    int px = viewer.position.vx + w / 2;
    int pz = viewer.position.vz + d / 2;

    if (px < x)
        return 0;
    if (px >= x + w)
        return 0;
    if (pz < z)
        return 0;
    if (pz >= z + d)
        return 0;
    if (y == 0x8000 || (y >= viewer.position.vy && viewer.position.vy >= y - 0x1000))
        return 1;
    return 0;
}

/* Moves `e` from `from` to `to` (both turned by src's rotation, relative to
 * src) over `steps` calls, fading it in when `fade`; returns 1 when done.
 */
ADDRESS(0x8004fdd0, 0x2dc)
int object_move_child(AreaObject *src, AreaObject *e, SVECTOR *from, SVECTOR *to, int fade, int steps)
{
    VECTOR a;
    VECTOR b;
    int t;

    switch (src->state) {
    case 0:
        if (src->unk48 == 0xff)
            return 0;
        if (e->unk48 == 0)
            *(u8 *)&src->unk50 = 0;
        else
            *(u8 *)&src->unk50 = steps - 1;
        src->state = 1;
        object_reset(e);
        e->kind = 112;
        e->unk01 = src->unk01;
        switch (e->type) {
        case 136:
            e->rot_x = 0x400;
            e->depth_bias = 100;
            break;
        case 133:
            e->depth_bias = -220;
            break;
        }
        if (fade)
            e->anim = 0;
        else
            e->anim = 0x80;
        vector_rotate((SVECTOR *)&src->rot_x, from, &e->position);
        e->position.vx += src->position.vx;
        e->position.vy += src->position.vy;
        e->position.vz += src->position.vz;
        e->facing = src->facing;
        *(s16 *)&e->unk50 = src - vis_table_b.entries;
    case 1:
        break;
    default:
        return 0;
    }
    if (fade) {
        e->anim_frame += 204;
        if (e->anim_frame >= 4096)
            e->anim_frame = 4095;
    }
    vector_rotate((SVECTOR *)&src->rot_x, from, &a);
    vector_rotate((SVECTOR *)&src->rot_x, to, &b);
    (*(u8 *)&src->unk50)++;
    t = (*(u8 *)&src->unk50 << 12) / steps;
    e->position.vx = src->position.vx + math_lerp(a.vx, b.vx, t);
    e->position.vy = src->position.vy + math_lerp(a.vy, b.vy, t);
    e->position.vz = src->position.vz + math_lerp(a.vz, b.vz, t);
    if (*(u8 *)&src->unk50 >= steps) {
        if (fade)
            e->anim_frame = 4095;
        if (area_state.loading == 0)
            e->unk48 = 0xff;
        src->state = 99;
        return 1;
    }
    return 0;
}

/* Runs frames while the screen fades from level `start` to `end` by
 * `delta`. */
ADDRESS(0x800500ac, 0xc4)
void fade_run_frames(int mode, int start, int end, int delta)
{
    VECTOR pos;
    SVECTOR rot;
    int c;
    int step = delta;
    int v = start;

    for (;;) {
        c = v * v >> 16;
        if (c > 255)
            c = 255;
        fade_add_color(c, c, c, 4096, mode);
        cd_wait_idle();
        sound_bank_transfer_body();
        image_stream_update();
        player_get_camera(&pos, &rot);
        world_render_frame(&pos, &rot);
        player_update_status(0);
        if (v == end)
            break;
        v += step;
    }
}

/* Shifts the viewer, the effects and the table A entries by (dx, dy, dz). */
ADDRESS(0x80050170, 0x118)
void world_shift_position(int dx, int dy, int dz)
{
    StEffect *f = effects.slots;
    Creature *e;
    u16 i;
    u16 j;

    viewer.position.vx += dx;
    viewer.position.vz += dz;
    viewer.position.vy += dy;
    i = 128;
    while (i--) {
        if (f->flags != ST_EFFECT_FREE && (f->draw_mode & 0xc) != 0xc) {
            f->position.vx += dx;
            f->position.vz += dz;
            f->position.vy += dy;
        }
        f++;
    }
    e = vis_table_a.entries;
    j = 64;
    while (j--) {
        if (e->spawn_chance != 0xff) {
            e->position.vx += dx;
            e->position.vz += dz;
            e->position.vy += dy;
        }
        e++;
    }
}

/* Spawns a debris entry of `kind` beside src, `size` units out along its
 * yaw turned by `angle`. */
ADDRESS(0x80050288, 0x1e0)
AreaObject *object_spawn_part(src, kind, state, arg3, size, angle, ox, oy, oz)
    AreaObject *src;
    u8 kind;
    u16 state;
    s16 arg3;
    u8 size;
    short angle;
    int ox;
    int oy;
    int oz;
{
    AreaObject *e = object_alloc(0, 300, -1);
    int c;
    int n;

    e->type = kind;
    e->unk48 = 0;
    e->block = src->block;
    e->kind = state;
    *(s16 *)&e->unk50 = arg3;
    e->state = size;
    c = rcos(src->facing - angle) / 16;
    e->unk56 = rsin(src->facing - angle) / 16;
    e->flags = 0x40;
    ((s16 *)&e->unk50)[1] = c;
    e->unk54 = 0;
    e->visibility = 0;
    e->anim = 0x80;
    e->rot_x = 0;
    e->rot_z = rand() / 8;
    e->facing = src->facing;
    e->depth_bias = src->depth_bias;
    e->draw_flags = 15;
    e->unk06 = 0xff;
    e->light = 0x800;
    n = size;
    e->scale.vx = e->scale.vy = e->scale.vz = n << 10;
    e->position.vx = src->position.vx + (n * 256 - ox) * rcos(src->facing) / 4096;
    e->position.vy = src->position.vy - oy;
    e->position.vz = src->position.vz + (n * 256 - oz) * rsin(src->facing) / 4096;
    return e;
}

/* Breaks src into four kind-175 pieces. */
ADDRESS(0x80050468, 0xfc)
void object_spawn_side_parts(e, arg)
    AreaObject *e;
    u16 arg;
{
    object_spawn_part(e, 175, 4, arg, 4, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 175, 4, arg, 3, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 175, 4, arg, 2, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 175, 4, arg, 1, 0x400, 0, 0x500, 0);
    e->kind = OBJECT_KIND_NONE;
}

/* The same with state 6, offset pieces. */
ADDRESS(0x80050564, 0x108)
void object_spawn_forward_parts(e, arg)
    AreaObject *e;
    u16 arg;
{
    object_spawn_part(e, 175, 6, arg, 4, 0x400, 0x500, 2000, 0x500);
    object_spawn_part(e, 175, 6, arg, 3, 0x400, 0x500, 2000, 0x500);
    object_spawn_part(e, 175, 6, arg, 2, 0x400, 0x500, 2000, 0x500);
    object_spawn_part(e, 175, 6, arg, 1, 0x400, 0x500, 2000, 0x500);
    e->kind = OBJECT_KIND_NONE;
}

/* Four kind-171 pieces with state 8. */
ADDRESS(0x8005066c, 0xfc)
void object_spawn_side_parts_alt(e, arg)
    AreaObject *e;
    u16 arg;
{
    object_spawn_part(e, 171, 8, arg, 4, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 171, 8, arg, 3, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 171, 8, arg, 2, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 171, 8, arg, 1, 0x400, 0, 0x500, 0);
    e->kind = OBJECT_KIND_NONE;
}

/* objects_update's switch table (a door's swing side). */

/* The (x, z) push a kind 224 entry gives the player while its entries rise:
 * written to the object push words vis_table_b.push_x/unk71A8, which
 * player_update_walk adds to the viewer's push (viewer.push_x/unk1D0). */
DATA(0x800881e0, 0x8, ".sbss")
static SVECTOR object_sequence_push;

/* The halfwords and bytes other kinds keep from 0x50. */
#define H50(e) (*(s16 *)&(e)->unk50)
#define H52(e) (((s16 *)&(e)->unk50)[1])
#define B50(e) (*(u8 *)&(e)->unk50)
#define B51(e) (((s8 *)&(e)->unk50)[1])
#define B52(e) (((s8 *)&(e)->unk50)[2])

/* The three bytes from 0x48 and from 0x50, 0xffffff when unset. */
#define W48(e) (*(u_long *)&(e)->unk48 & 0xffffff)
#define W50(e) (*(u_long *)&(e)->unk50 & 0xffffff)

/* Starts the entry's sound n (its record's sounds start at 208). */
#define SOUND(e, n) object_play_sound(e, (e)->type * 3 + (n))

/* The moving sound as the entry leaves its rest position. */
#define START(e, step)                                                         \
    if ((e)->anim_frame == 0 && (e)->anim_frame + (step) * 4 > 0)                        \
        SOUND(e, 208)

/* Swaps the first bytes of the entry's two record ranges. */
#define SWAP_RECORDS(e)                                                            n = (e)->params.door.n_blocks;                                                     while (n--) {                                                                      u8 t = vis_records[(e)->params.door.block_b + n].part;                                                                                                          vis_records[(e)->params.door.block_b + n].part =                                     vis_records[(e)->params.door.block_a + n].part;                              vis_records[(e)->params.door.block_a + n].part = t;                          }

/*
 * Updates every table B entry by its kind.
 */
ADDRESS(0x80050768, 0x212c)
void objects_update(void)
{
    u16 dmg[9];
    u16 dmg2[9];
    AreaObject *e = vis_table_b.entries;
    AreaObjectType *rec;
    int i;
    int dx;
    int dy;
    int dz;
    int angle;
    int off;
    u16 n;

    for (i = 240; i != 0; e++, i--) {
        if (e->type == 0xffff)
            continue;
        vis_table_b.current = e;
        vis_table_b.current_type = rec = &vis_table_b.types[e->type];
        dx = e->position.vx - viewer.position.vx;
        dy = e->position.vy - viewer.position.vy;
        dz = e->position.vz - viewer.position.vz;
        if (dx + 0x8000 > 0x10000u || dy > 0x8000 || dy < -0x8000 || dz > 0x8000 || dz < -0x8000) {
            e->player_dist_sq = 0x40000000;
            e->player_dist_xz = 0x8000;
        } else {
            e->player_dist_sq = dx * dx + dy * dy + dz * dz;
            e->player_dist_xz = SquareRoot0(dx * dx + dz * dz);
        }
        angle = (ratan2(dz, dx) - 1024) & 0xfff;
        e->player_heading = angle;
        off = ((u16)e->facing - angle) & 0xfff;
        e->player_bearing = off;
        if (off > 2048)
            e->player_bearing = 4096 - off;
        if (e->kind == OBJECT_KIND_NONE)
            continue;
        switch (e->kind) {
        case OBJECT_KIND_DOOR:
        case OBJECT_KIND_MODEL_DOOR: {
            int step = e->unk48;
            short speed = step;

            speed *= 4;
            switch (e->state) {
            case 0:
                if (e->kind == OBJECT_KIND_MODEL_DOOR) {
                    if (world_state.model_files[e->type + 256] == NULL)
                        e->visibility = 0;
                    else
                        e->visibility = 2;
                }
                if (e->unk49 >= 249 && e->unk49 <= 250 && e->player_dist_xz <= 8192)
                    e->state = 1;
                break;
            case 1:
                if (e->anim_frame == 0 && e->anim_frame + step * 4 > 0) {
                    SWAP_RECORDS(e);
                    if (e->kind == OBJECT_KIND_MODEL_DOOR)
                        e->visibility = 1;
                    SOUND(e, 208);
                }
                if (speed + e->anim_frame >= 4095) {
                    e->anim_frame = 4095;
                    e->state = 2;
                } else {
                    e->anim_frame = (e->anim_frame + speed) & 0xfff;
                }
                if (e->anim_frame < 3072 && speed + e->anim_frame >= 3072)
                    SOUND(e, 210);
                if (speed != 0 && e->anim_frame <= 2048 && e->player_dist_sq <= 2048)
                    player_stop_motion();
                if (e->player_dist_xz <= 2048 && e->anim_frame <= 2048) {
                    int push = 0;

                    switch (rec->push_side) {
                    case 0: {
                        short side = e->player_bearing;

                        if (side > -1024 && side < 1024)
                            push = 1;
                        break;
                    }
                    case 1:
                        if (e->player_bearing > 0 && e->player_bearing < 2048)
                            push = 1;
                        break;
                    case 2:
                        if (e->player_bearing > 1024 && e->player_bearing < 3072)
                            push = 1;
                        break;
                    case 3:
                        if (e->player_bearing > 2048 && e->player_bearing < 4096)
                            push = 1;
                        break;
                    case 5:
                        push = 1;
                        break;
                    case 4:
                        break;
                    }
                    if (speed != 0 && push) {
                        short away = e->player_heading;

                        player_stop_motion();
                        away -= 1024;
                        vis_table_b.push_x += rcos(away) / 32;
                        vis_table_b.push_z += rsin(away) / 32;
                    }
                }
                break;
            case 2:
                if (e->unk49 == 252 || e->unk49 == 250) {
                    e->state = 255;
                    break;
                }
                if (e->player_dist_xz < 8192)
                    break;
                if (!(e->unk49 & 0x80)) {
                    e->anim = e->unk49 == 0;
                    e->anim_frame = 0;
                }
                e->state = 3;
                SOUND(e, 208);
                break;
            case 3:
                if (e->unk49 & 0x80) {
                    if (step * 4 < e->anim_frame) {
                        e->anim_frame = (e->anim_frame - speed) & 0xfff;
                        break;
                    }
                    SWAP_RECORDS(e);
                } else {
                    if (e->anim_frame + step * 4 < 4095) {
                        e->anim_frame = (e->anim_frame + speed) & 0xfff;
                        break;
                    }
                    SWAP_RECORDS(e);
                    if (!(e->unk49 & 0x80))
                        e->anim = e->unk49;
                }
                e->anim_frame = 0;
                e->state = 0;
                SOUND(e, 209);
                break;
            case 7:
                if (e->kind == OBJECT_KIND_MODEL_DOOR)
                    e->visibility = 2;
                break;
            case 8:
                e->visibility = 1;
                START(e, step);
                if (speed + e->anim_frame < 4095) {
                    e->anim_frame = (e->anim_frame + speed) & 0xfff;
                } else {
                    e->visibility = 0;
                    e->anim_frame = 4095;
                    e->state = 255;
                }
                break;
            case 11:
                break;
            case 12:
                START(e, step);
                e->anim_frame += speed;
                if (speed + e->anim_frame >= 4095) {
                    e->anim_frame = 4095;
                    e->visibility = 0;
                    e->state = 255;
                }
                break;
            case 255:
                e->anim_frame = 4095;
                break;
            }
            break;
        }
        case OBJECT_KIND_GATE:
        case OBJECT_KIND_ITEM_GATE: {
            int step = e->unk48;
            short speed = step;

            speed *= 4;
            switch (e->state) {
            case 0:
                if (e->kind == OBJECT_KIND_ITEM_GATE)
                    ground_item_set_hidden(e->params.snd.fade_range, 0);
                if (e->unk49 >= 249 && e->unk49 <= 250 && e->player_dist_xz <= 8192)
                    e->state = 1;
                break;
            case 1:
                if (e->anim_frame == 0 && e->anim_frame + step * 4 > 0) {
                    SOUND(e, 208);
                    if (e->kind == OBJECT_KIND_ITEM_GATE)
                        ground_item_set_hidden(e->params.snd.fade_range, 1);
                }
                if (e->anim_frame < 3072 && speed + e->anim_frame >= 3072)
                    SOUND(e, 210);
                if (speed + e->anim_frame < 4095) {
                    e->anim_frame = (e->anim_frame + speed) & 0xfff;
                } else {
                    e->anim_frame = 4095;
                    e->state = 2;
                }
                break;
            case 2:
                if (e->unk49 == 252 || e->unk49 == 250) {
                    e->state = 255;
                    break;
                }
                if (e->player_dist_xz < 8192)
                    break;
                if (!(e->unk49 & 0x80)) {
                    e->anim = e->unk49 == 0;
                    e->anim_frame = 0;
                }
                e->state = 3;
                SOUND(e, 208);
                break;
            case 3:
                if (e->unk49 & 0x80) {
                    if (speed < e->anim_frame) {
                        e->anim_frame = (e->anim_frame - e->unk48) & 0xfff;
                        break;
                    }
                } else {
                    if (speed + e->anim_frame < 4095) {
                        e->anim_frame = (e->anim_frame + speed) & 0xfff;
                        break;
                    }
                    e->anim = e->unk49;
                }
                e->anim_frame = 0;
                e->state = 0;
                break;
            case 4:
                START(e, step);
                e->anim_frame += speed;
                if (e->anim_frame >= 4095)
                    e->anim_frame &= 0xfff;
                break;
            case 5:
                if (world_state.tint_level < 54 || H50(e) >= 600) {
                    e->state = 6;
                    H50(e) = 0;
                    SOUND(e, 208);
                } else {
                    H50(e)++;
                }
                break;
            case 6:
                e->anim_frame += speed;
                if (e->anim_frame + step * 4 >= 4095) {
                    e->anim_frame = 0;
                    e->state = 5;
                }
                break;
            case 7:
                break;
            case 8:
                START(e, step);
                if (speed + e->anim_frame < 4095) {
                    e->anim_frame = (e->anim_frame + speed) & 0xfff;
                } else {
                    e->anim_frame = 4095;
                    e->state = 255;
                }
                break;
            case 9:
                if ((rand() >> 12) == 0) {
                    if (H50(e) % 20 == 0) {
                        e->state = 10;
                        e->visibility = 0;
                        H50(e) = 0;
                        SOUND(e, 208);
                    }
                } else {
                    H50(e)++;
                }
                break;
            case 10:
                e->anim_frame += speed;
                if (e->anim_frame + step * 4 >= 4096) {
                    e->anim_frame = 0;
                    e->visibility = 0;
                    e->state = 9;
                }
                break;
            case 11:
                break;
            case 12:
                START(e, step);
                e->anim_frame += speed;
                if (speed + e->anim_frame >= 4095) {
                    e->anim_frame = 4095;
                    e->visibility = 0;
                    e->state = 255;
                }
                break;
            case 255:
                e->anim_frame = 4095;
                break;
            }
            break;
        }
        case OBJECT_KIND_GROUP_GATE: {
            int step = e->unk48;
            short speed = step;
            AreaObject *o;
            int j;
            int group;

            speed *= 4;
            switch (e->state) {
            case 0:
                break;
            case 1:
                START(e, step);
                if (speed + e->anim_frame < 4095) {
                    e->anim_frame = (e->anim_frame + speed) & 0xfff;
                    break;
                }
                if (e->unk49 != 255 && e->unk49 != 252) {
                    e->anim = 1;
                    e->anim_frame = 0;
                }
                e->state = 255;
                if (e->params.snd.volume == 255)
                    break;
                group = e->params.snd.volume;
                for (o = vis_table_b.entries, j = 0; j < 240; j++, o++) {
                    if ((o->kind == OBJECT_KIND_DOOR || o->kind == OBJECT_KIND_GATE)
                        && o->params.door.lock == group) {
                        if (o->state == 0) {
                            o->unk49 = 252;
                            o->state = 1;
                            o->anim = 0;
                            o->anim_frame = 0;
                        } else if (o->state == 255) {
                            o->state = 3;
                            o->anim = 1;
                            o->anim_frame = 0;
                            SOUND(o, 208);
                        }
                    }
                }
                break;
            case 3:
                START(e, step);
                if (speed + e->anim_frame >= 4095) {
                    e->anim = 0;
                    e->anim_frame = 0;
                    e->state = 0;
                    if (e->params.snd.volume == 255)
                        break;
                    group = e->params.snd.volume;
                    for (o = vis_table_b.entries, j = 0; j < 240; j++, o++) {
                        if (o->kind == OBJECT_KIND_DOOR && o->params.door.lock == group) {
                            if (o->state == 0) {
                                o->unk49 = 252;
                                o->state = 1;
                                o->anim = 0;
                                o->anim_frame = 0;
                            } else if (o->state == 255) {
                                o->unk49 = 0;
                                o->state = 3;
                                o->anim = 1;
                                o->anim_frame = 0;
                                SOUND(o, 208);
                            }
                        }
                    }
                } else {
                    e->anim_frame = (e->anim_frame + speed) & 0xfff;
                }
                break;
            }
            break;
        }
        case 2: {
            u_long a = (u_long)rand() >> 8;
            u_long b = (u_long)rand() >> 6;
            u_long c = (u_long)rand() >> 8;
            if (B50(e))
                B50(e)--;
            if (e->state == 0) {
                if (e->player_dist_xz < 1024 && B50(e) == 0) {
                    memset(dmg, 0, 18);
                    dmg[3] = 100;
                    player_hit(dmg, 4096, 0, (long *)&e->position);
                    B50(e) = 20;
                }
                if (e->scale.vy > 4096) {
                    e->scale.vx = e->scale.vx - 512 - a;
                    e->scale.vz = e->scale.vz - 512 - c;
                } else {
                    e->scale.vx = a + (e->scale.vx + 512);
                    e->scale.vz = c + (e->scale.vz + 512);
                }
                e->scale.vy = b + (e->scale.vy + 1256);
                if (e->scale.vy > 8192)
                    e->state = 1;
            } else if (e->state == 1) {
                e->scale.vx = 0;
                e->scale.vy = 0;
                e->scale.vz = 0;
                e->state = 2;
            } else if (rand() < 2048 && B50(e) == 0) {
                e->state = 0;
            }
            break;
        }
        case 3:
            object_spawn_side_parts(e, (u16)(240 - i));
            break;
        case 4:
        case 8:
            if (e->unk48)
                e->unk48--;
            if (e->player_dist_xz < e->scale.vx / 4 && e->unk48 == 0) {
                if (e->kind == 4) {
                    memset(dmg, 0, 18);
                    dmg[3] = 180;
                    player_hit(dmg, 4096, 0, (long *)&e->position);
                    e->unk48 = 20;
                } else if (e->kind == 8) {
                    memset(dmg2, 0, 18);
                    dmg2[6] = 135;
                    player_hit(dmg2, 4096, 5120, (long *)&e->position);
                    e->unk48 = 20;
                }
            }
            if (e->state >= 5) {
                e->position = vis_table_b.entries[H50(e)].position;
                e->position.vy -= 1280;
                e->scale.vx = e->scale.vy = e->scale.vz = 0;
                H52(e) = rcos(e->facing - 1024) * 384 / 4096;
                e->unk54 = 0;
                e->unk56 = rsin(e->facing - 1024) * 384 / 4096;
                e->position.vx += H52(e);
                e->position.vz += e->unk56;
                e->state = 0;
            } else if (e->state >= 3) {
                H52(e) = e->state * (rcos(e->facing - 1024) << 7) / 4096;
                e->unk54 = e->state << 5;
                e->unk56 = e->state * (rsin(e->facing - 1024) << 7) / 4096;
                e->position.vx += H52(e) - rand() / 256;
                e->position.vy += e->unk54 + rand() / 2048;
                e->position.vz += e->unk56 - rand() / 256;
                e->scale.vx += 1024;
                e->scale.vy += 1024;
                e->scale.vz += 1024;
                e->state++;
            } else {
                H52(e) = e->state * (rcos(e->facing - 1024) << 7) / 4096;
                e->unk54 = e->state << 6;
                e->unk56 = e->state * (rsin(e->facing - 1024) << 7) / 4096;
                e->position.vx += H52(e) + rand() / 256;
                e->position.vy += e->unk54 + rand() / 2048;
                e->position.vz += e->unk56 + rand() / 256;
                e->scale.vx += 1024;
                e->scale.vy += 1024;
                e->scale.vz += 1024;
                e->state++;
            }
            break;
        case 5:
            object_spawn_forward_parts(e, (u16)(240 - i));
            break;
        case 6:
            if (e->unk48)
                e->unk48--;
            if (e->player_dist_xz < e->scale.vx / 4 && e->unk48 == 0) {
                memset(dmg, 0, 18);
                dmg[3] = 100;
                player_hit(dmg, 4096, 0, (long *)&e->position);
                e->unk48 = 20;
            }
            if (e->state >= 5) {
                e->position = vis_table_b.entries[H50(e)].position;
                e->position.vx -= rcos(e->facing) * 1280 / 4096;
                e->position.vy -= 2000;
                e->position.vz -= rsin(e->facing) * 1280 / 4096;
                e->scale.vx = e->scale.vy = e->scale.vz = 0;
                H52(e) = rcos(e->facing) * 384 / 4096;
                e->unk54 = 128;
                e->unk56 = rsin(e->facing) * 384 / 4096;
                e->position.vx += H52(e);
                e->position.vz += e->unk56;
                e->state = 0;
            } else if (e->state >= 3) {
                H52(e) = e->state * (rcos(e->facing) << 7) / 4096;
                e->unk54 = e->state << 6;
                e->unk56 = e->state * (rsin(e->facing) << 7) / 4096;
                e->position.vx += H52(e) - rand() / 256;
                e->position.vy += e->unk54 + rand() / 2048;
                e->position.vz += e->unk56 - rand() / 256;
                e->scale.vx += 1024;
                e->scale.vy += 1024;
                e->scale.vz += 1024;
                e->state++;
            } else {
                H52(e) = e->state * (rcos(e->facing) << 7) / 4096;
                e->unk54 = e->state << 7;
                e->unk56 = e->state * (rsin(e->facing) << 7) / 4096;
                e->position.vx += H52(e) + rand() / 256;
                e->position.vy += e->unk54 + rand() / 2048;
                e->position.vz += e->unk56 + rand() / 256;
                e->scale.vx += 1024;
                e->scale.vy += 1024;
                e->scale.vz += 1024;
                e->state++;
            }
            break;
        case 7:
            object_spawn_side_parts_alt(e, (u16)(240 - i));
            break;
        case OBJECT_KIND_SEQUENCE: {
            Creature *a;
            int j;
            int rising;

            if (player_in_cell_rect(e->position.vx >> 12, e->position.vz >> 12, 1, 1, e->position.vy) == 0
                && area_state.loading == 0)
                break;
            if ((pad_state.held & pad_state.buttons[PAD_ACTION_EVENT])
                && vis_table_a.object_sequence_state == 0
                && W50(e) != 0xffffff && W48(e) != 0xffffff) {
                vis_table_a.object_sequence_state = 1;
                player_stop_motion();
                pad_clear_input();
            }
            if (vis_table_a.object_sequence_state == 1) {
                a = &vis_table_a.entries[56];
                rising = 0;
                cd_wait_idle();
                viewer.motion_stopped = 1;
                for (j = 7; j != -1; a++, j--) {
                    a->anim_frame = 0;
                    if (a->spawn_chance != 255) {
                        sound_play_at(a->model_slot * 10 + 48, &a->position, 127, (rand() * 5 >> 15) - 2);
                        a->anim_step = 128;
                        object_sequence_push.vx = rcos((s16)a->player_heading - 1024) / 256;
                        object_sequence_push.vz = rsin((s16)a->player_heading - 1024) / 256;
                        rising = 1;
                    }
                }
                if (rising)
                    vis_table_a.object_sequence_state = 2;
            }
            if (vis_table_a.object_sequence_state == 2) {
                a = &vis_table_a.entries[56];
                player_stop_motion();
                vis_table_b.push_x = object_sequence_push.vx;
                vis_table_b.push_z = object_sequence_push.vz;
                for (rising = 7; rising != -1; a++, rising--) {
                    if (a->spawn_chance != 255 && (s16)a->anim_frame + a->anim_step > 3584) {
                        a->anim_step = 0;
                        a->anim_frame = 4095;
                        vis_table_a.object_sequence_state = 3;
                        pad_clear_input();
                    }
                }
                break;
            }
            if (vis_table_a.object_sequence_state != 3) {
                if (W50(e) != 0xffffff)
                    break;
            } else {
                object_sequence_push.vx = 0;
                object_sequence_push.vz = 0;
            }
            vis_table_a.object_sequence_state = 4;
            area_request_load(e->params.snd.sound_flags, e->params.snd.sound_flags, e->params.snd.sound_flags, e->params.snd.sound_flags,
                          e->params.snd.sound_flags, (s8)B50(e), B52(e), B51(e), (s8)e->params.snd.volume,
                          (s8)e->params.door.lock, (s8)e->params.snd.fade_range);
            if (W48(e) != 0xffffff) {
                vis_table_b.push_x = -object_sequence_push.vx * 2;
                vis_table_b.push_z = -object_sequence_push.vz * 2;
                viewer.position.vx = e->position.vx - 2048;
                viewer.position.vy = e->position.vy - 1600;
                viewer.position.vz = e->position.vz - 2048;
                viewer.walk_velocity.vx = 0;
                viewer.walk_velocity.vz = 0;
            }
            if (e->params.door.n_blocks != 255) {
                int yaw = e->params.door.n_blocks << 10;

                viewer.view_rot.vy += yaw;
            }
            break;
        }
        case OBJECT_KIND_MESSAGE:
            break;
        case OBJECT_KIND_WARP:
        case OBJECT_KIND_WALK_IN_WARP: {
            AreaObject *o;
            int j;
            short turn;
            u8 x;
            u8 y;
            u8 z;
            u8 area;
            u8 lit;
            u8 lit2;

            if (player_in_centred_rect(e->position.vx, e->position.vz, 3072, 3072, e->position.vy) == 0)
                goto away;
            turn = (viewer.yaw - e->player_heading) & 0xfff;
            if (turn > 2048)
                turn |= 0xf000;
            if (!((pad_state.held & pad_state.buttons[PAD_ACTION_EVENT]) && turn > -400 && turn < 400)
                && e->kind != OBJECT_KIND_WALK_IN_WARP)
                goto away;
            if (B50(e))
                break;
            viewer.warp_return.warp.x = viewer.position.vx >> 12;
            viewer.warp_return.warp.area = area_state.loaded_area;
            viewer.warp_return.warp.y = viewer.position.vy >> 12;
            viewer.warp_return.warp.z = viewer.position.vz >> 12;
            viewer.warp_return.warp.dx = viewer.position.vx - (viewer.warp_return.warp.x << 12);
            viewer.warp_return.warp.dy = viewer.position.vy - (viewer.warp_return.warp.y << 12);
            viewer.warp_return.warp.dz = viewer.position.vz - (viewer.warp_return.warp.z << 12);
            viewer.warp_return.warp.yaw = e->facing + 2048;
            for (j = 240, o = vis_table_b.entries; j != 0; j--, o++) {
                if (o->kind == e->kind)
                    B50(o) = 1;
            }
            x = e->unk49 + 64;
            y = e->params.snd.sound + 64;
            z = e->params.snd.volume + 64;
            play_warp_sound();
            fade_run_frames(2, 0, 0x1000, 0x100);
            area = e->unk48;
            lit = area;
            lit2 = area;
            do {
                cd_wait_frame();
                area_load_update();
            } while (area_state.loading != 0);
            viewer.view_rot.vy = e->params.door.lock << 10;
            viewer.view_rot.vx = 0;
            if (area != area_state.loaded_area) {
                area_request_load(area, area, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
            } else {
                area_request_load(0xff, 0xff, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
                viewer.update_all_spawns = 1;
                creatures_bind_previous_area();
                vis_table_a.zone_history[0] = vis_table_a.zone_history[1] = vis_table_a.zone_history[2] =
                    vis_table_a.zone_history[3] = 0xff;
                vis_table_a.prev_zone = 0xff;
            }
            viewer.state = VIEWER_STATE_NORMAL;
            viewer.shake_roll = 0;
            viewer.shake_yaw = 0;
            viewer.shake_pitch = 0;
            *(SVECTOR *)&viewer.pitch = viewer.view_rot;
            viewer.position.vx = (x << 12) + ((s8)e->params.snd.fade_range << 7);
            viewer.position.vz = (z << 12) + ((s8)e->params.door.n_blocks << 7);
            viewer.position.vy = (y << 12) + ((s8)e->params.snd.sound_flags << 7) - 1600;
            viewer.submerged_acid = 0;
            viewer.submerged = 0;
            player_stop_motion();
            vis_table_b.push_z = 0;
            vis_table_b.push_y = 0;
            vis_table_b.push_x = 0;
            vis_lists.floor_push_z = 0;
            vis_lists.floor_push_y = 0;
            vis_lists.floor_push_x = 0;
            vis_table_a.push_z = 0;
            vis_table_a.push_y = 0;
            vis_table_a.push_x = 0;
            viewer.push_z = 0;
            viewer.push_y = 0;
            viewer.push_x = 0;
            do {
                cd_wait_frame();
                area_load_update();
            } while (area_state.loading != 0);
            vis_lists.n_sources = 0;
            vis_collect_sources((VECTOR *)&viewer.position.vx, NULL, NULL);
            if (vis_lists.n_sources)
                viewer.block_record = &vis_records[vis_lists.sources[0]];
            creatures_update();
            fade_run_frames(2, 0x1000, 0x1000, 0);
            cd_wait_idle();
            play_warp_sound();
            fade_run_frames(2, 0x1000, 0, -0x100);
            if (area < 44)
                area_request_load(0xff, 0xff, 0xff, lit2, lit, 127, 127, 127, 0, 0, 0);
            lighting_clear_requests();
            break;
        away:
            B50(e) = 0;
            break;
        }
        default:
            area_state.hooks->update_object();
            break;
        }
    }
    vis_table_b.current = NULL;
    vis_table_b.push_x /= 2;
    vis_table_b.push_y /= 2;
    vis_table_b.push_z /= 2;
    if (vis_table_b.push_x > -4 && vis_table_b.push_x < 4)
        vis_table_b.push_x = 0;
    if (vis_table_b.push_y > -4 && vis_table_b.push_y < 4)
        vis_table_b.push_y = 0;
    if (vis_table_b.push_z > -4 && vis_table_b.push_z < 4)
        vis_table_b.push_z = 0;
}

/* src/st/area_load.c: from here on its calls see the prototype. */
KF_VALUELESS_S32 area_request_load(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5);

/* Fades out, loads `to`'s area and places the viewer there, then fades in;
 * `keep` skips re-collecting the visible sources and the light reset. */
ADDRESS(0x80052894, 0x390)
void player_warp(to, keep)
    StWarp *to;
    u8 keep;
{
    AreaObject *e = vis_table_b.entries;
    StEffect *f;
    int i;
    u8 x;
    u8 y;
    u8 z;
    u8 area;
    int j;

    for (i = 240; i != 0; i--) {
        if (e->kind == OBJECT_KIND_WARP)
            *(u8 *)&e->unk50 = 1;
        e++;
    }
    x = to->x;
    y = to->y;
    z = to->z;
    play_warp_sound();
    fade_run_frames(2, 0, 0x1000, 0x100);
    area = to->area;
    do {
        cd_wait_frame();
        area_load_update();
    } while (area_state.loading != 0);
    viewer.view_rot.vy = to->yaw;
    viewer.view_rot.vx = 0;
    if (area != area_state.loaded_area) {
        area_request_load(area, area, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
    } else {
        area_request_load(0xff, 0xff, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
        viewer.update_all_spawns = 1;
        creatures_bind_previous_area();
        vis_table_a.zone_history[0] = vis_table_a.zone_history[1] = vis_table_a.zone_history[2] = vis_table_a.zone_history[3] = 0xff;
        vis_table_a.prev_zone = 0xff;
    }
    viewer.state = VIEWER_STATE_NORMAL;
    viewer.shake_roll = 0;
    viewer.shake_yaw = 0;
    viewer.shake_pitch = 0;
    *(SVECTOR *)&viewer.pitch = viewer.view_rot;
    viewer.position.vx = (x << 12) + to->dx;
    viewer.position.vz = (z << 12) + to->dz;
    viewer.position.vy = (y << 12) + to->dy;
    player_stop_motion();
    f = effects.slots;
    vis_table_b.push_z = 0;
    vis_table_b.push_y = 0;
    vis_table_b.push_x = 0;
    vis_lists.floor_push_z = 0;
    vis_lists.floor_push_y = 0;
    vis_lists.floor_push_x = 0;
    vis_table_a.push_z = 0;
    vis_table_a.push_y = 0;
    vis_table_a.push_x = 0;
    viewer.push_z = 0;
    viewer.push_y = 0;
    viewer.push_x = 0;
    j = 128;
    while (j--) {
        f->flags = ST_EFFECT_FREE;
        f++;
    }
    do {
        cd_wait_frame();
        area_load_update();
    } while (area_state.loading != 0);
    if (!keep) {
        vis_lists.n_sources = 0;
        vis_collect_sources((VECTOR *)&viewer.position.vx, NULL, NULL);
        if (vis_lists.n_sources)
            viewer.block_record = &vis_records[vis_lists.sources[0]];
        creatures_update();
    }
    fade_run_frames(2, 0x1000, 0x1000, 0);
    cd_wait_idle();
    play_warp_sound();
    fade_run_frames(2, 0x1000, 0, -0x100);
    if (!keep) {
        area_request_load(0xff, 0xff, 0xff, area, area, 127, 127, 127, 0, 0, 0);
        lighting_clear_requests();
    }
    pad_clear_input();
    player_enter_normal_state();
}

/* Warps to viewer.save_area. */
ADDRESS(0x80052c24, 0x38)
void player_warp_to_save_point(void)
{
    player_warp((StWarp *)&viewer.save_area, 0);
    viewer.save_warp_request = 0;
}

/* Warps to viewer.warp_return keeping the current sources. */
ADDRESS(0x80052c5c, 0x48)
void player_warp_back(void)
{
    viewer.state = VIEWER_STATE_SCRIPTED;
    player_update_durability_warning();
    player_warp(&viewer.warp_return.warp, 1);
    DAT_801cd0e4 = 0;
}

/* Frees every table C entry and clears the group counts and id bits. */
ADDRESS(0x80052ca4, 0x6c)
void ground_items_clear(void)
{
    GroundItem *c = vis_table_c.entries;
    int i;

    vis_table_c.ground_item_counts[5] = 0;
    vis_table_c.ground_item_counts[4] = 0;
    vis_table_c.ground_item_counts[3] = 0;
    vis_table_c.ground_item_counts[2] = 0;
    i = 64;
    while (i--) {
        c->item = 0xffff;
        c->state = GROUND_ITEM_NONE;
        c++;
    }
    /* Ten words each; the tenth of unk0070 is the first of unk0094. */
    i = 10;
    while (i--) {
        vis_table_c.placed_bits[i] = 0;
        vis_table_c.collected_bits[i] = 0;
    }
}

/* Unassigns every ItemType slot. */
ADDRESS(0x80052d10, 0x30)
void item_model_slots_clear(void)
{
    int i = 128;

    while (i--)
        vis_table_c.model_slots[i] = 0xff;
    vis_table_c.next_model_slot = 0;
}

/* 0 when id has been collected (ids below 266 keep a bit), else 1. */
ADDRESS(0x80052d40, 0x58)
int item_is_uncollected(int id)
{
    if (id < 266) {
        if (vis_table_c.collected_bits[id / 32] & (1 << (id % 32)))
            return 0;
    }
    return 1;
}

/* Counts the pick-up of item *id and marks it collected. */
ADDRESS(0x80052d98, 0xb8)
void item_record_pickup(u16 *id)
{
    inventory.counts[*id].count++;
    if (*id == 266)
        viewer.cunes_found++;
    if (*id < 266) {
        viewer.items_found++;
        vis_table_c.collected_bits[*id >> 5] |= 1 << (*id & 0x1f);
    }
}

/* Removes an entry, closing up the rest of its group (32..47 or 48..63). */
ADDRESS(0x80052e50, 0xe0)
void ground_item_remove(GroundItem *c)
{
    int i = ((u8 *)c - (u8 *)vis_table_c.entries) / sizeof(GroundItem);
    int n;

    if (i < 32) {
        vis_table_c.ground_item_counts[1]--;
    } else {
        if (i < 48) {
            n = 47 - i;
            vis_table_c.ground_item_counts[2]--;
        } else {
            n = 63 - i;
            vis_table_c.ground_item_counts[3]--;
        }
        while (n--) {
            c[0] = c[1];
            c++;
        }
    }
    c->state = GROUND_ITEM_NONE;
}

/* Loads the first 32 entries from an area file, then drops the collected
 * ones of the 16 from entry 32. */
ADDRESS(0x80052f30, 0x244)
void ground_items_load(GroundItemFileEntry *src)
{
    GroundItem *c = vis_table_c.entries;
    ItemType *rec;
    int i;

    vis_table_c.ground_item_counts[0] = 0;
    for (i = 0; i < 32; c++, src++, i++) {
        c->item = src->item;
        c->block = src->block;
        c->position.vx = (src->cell_x << 12) + src->offset_x;
        c->position.vy = (src->cell_y << 12) + src->offset_y;
        c->position.vz = (src->cell_z << 12) + src->offset_z;
        c->rot_x = src->rot_x & 0xfff;
        c->facing = src->facing & 0xfff;
        c->rot_z = src->rot_z & 0xfff;
        c->unk20 = src->unk02;
        c->vertical_speed = 0;
        if (c->item == 0xffff) {
            c->state = GROUND_ITEM_NONE;
            continue;
        }
        rec = &vis_table_c.types[c->item];
        c->area = area_state.area;
        c->depth_bias = rec->unk5[1];
        vis_table_c.placed_bits[c->item >> 5] |= 1 << (c->item & 0x1f);
        vis_table_c.ground_item_counts[1]++;
        vis_table_c.ground_item_counts[0]++;
        if (vis_table_c.model_slots[rec->model] == 0xff)
            vis_table_c.model_slots[rec->model] = vis_table_c.next_model_slot++;
        if (i >= 28 || item_is_uncollected(c->item))
            c->state = GROUND_ITEM_RESTING;
        else
            c->state = GROUND_ITEM_NONE;
    }
    c = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++) {
        if (item_is_uncollected(c->item) == 0)
            ground_item_remove(c);
        else
            c++;
    }
}

/* The nearest lying item in view of the viewer within the request's reach
 * (its record's radius plus req->radius); -1 when none. */
ADDRESS(0x80053174, 0x1e8)
int ground_item_find_in_reach(CollisionState *req, int unused)
{
    GroundItem *c;
    ItemType *rec;
    int best = -1;
    int nearest = 0x7fffffff;
    int i;
    int dx;
    int dz;
    int dist;

    c = vis_table_c.entries;
    for (i = 0; i < 64; i++, c++) {
        if (c->state != GROUND_ITEM_FALLING && c->state != GROUND_ITEM_RESTING)
            continue;
        if (c->area != area_state.area)
            continue;
        rec = &vis_table_c.types[c->item];
        if (rec->unk0[1] == 0)
            continue;
        if (c->position.vy < viewer.position.vy - 0x400 || c->position.vy > viewer.position.vy + 0xa40)
            continue;
        if (!angle_within(viewer.view_rot.vy,
                           angle_from_xz(c->position.vx - viewer.position.vx, c->position.vz - viewer.position.vz), 300))
            continue;
        dx = c->position.vx - req->position.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = c->position.vz - req->position.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        if (dist < rec->unk0[1] * 256 + req->radius && dist < nearest) {
            nearest = dist;
            best = i;
        }
    }
    return best;
}

/* Draws the HUD (src/st/hud.c). Retail's caller saw it without a
 * declaration, as a function returning a word: nothing reads the result,
 * but v0 counts as used, which keeps it out of reach of reload. */
int world_render_frame();

/* Shows entry c in front of the viewer until a button takes it (bit 10)
 * or puts it back (bit 11, unless forced). */
ADDRESS(0x8005335c, 0x748)
void ground_item_pick_up(GroundItem *c, int forced)
{
    GroundItem save;
    VECTOR a;
    VECTOR b;
    SVECTOR r;
    SVECTOR r2;
    ItemType *info;
    int take;
    int i;
    int j;

    info = &vis_table_c.types[c->item];
    save = *c;
    viewer.state = VIEWER_STATE_PICK_UP;
    gte_SetRotMatrix(&viewer.camera_matrix);
    gte_ldv0((SVECTOR *)&info->unk5[3]);
    gte_rtv0();
    gte_stlvnl(&a);
    {
        VECTOR *position = &viewer.position;

        a.vx += position->vx;
        a.vy += position->vy;
        a.vz += position->vz;
    }
    switch (info->unk5[2]) {
    case 0:
        r.vx = -viewer.pitch;
        r.vy = r.vz = 0;
        break;
    case 1:
        r.vx = 1024 - viewer.pitch;
        r.vy = r.vz = 0;
        break;
    case 2:
        r.vx = -1024 - viewer.pitch;
        r.vy = r.vz = 0;
        break;
    case 3:
        r.vx = 0;
        r.vy = 1024;
        r.vz = viewer.pitch - 256;
        break;
    }
    r.vy += viewer.yaw;
    if (r.vx - c->rot_x > 2048)
        r.vx -= 4096;
    if (r.vx - c->rot_x < -2048)
        r.vx += 4096;
    if (r.vy - c->facing > 2048)
        r.vy -= 4096;
    if (r.vy - c->facing < -2048)
        r.vy += 4096;
    i = 16;
    c->depth_bias -= 8;
    while (1) {
        if (forced == 0 && (pad_state.pressed & pad_state.buttons[PAD_ACTION_MAGIC_MENU])) {
            take = 0;
            a = c->position;
            b = save.position;
            r = *(SVECTOR *)&c->rot_x;
            r2 = *(SVECTOR *)&save.rot_x;
            i = 16 - i;
            break;
        }
        if ((pad_state.pressed & pad_state.buttons[PAD_ACTION_EVENT]) && !pad_state.combo_timer) {
            take = 1;
            a = c->position;
            b = viewer.position;
            b.vy += 800;
            r = *(SVECTOR *)&c->rot_x;
            r2 = r;
            i = 16;
            break;
        }
        j = 16 - i;
        c->position.vx = (i * save.position.vx + j * a.vx) >> 4;
        c->position.vy = (i * save.position.vy + j * a.vy) >> 4;
        c->position.vz = (i * save.position.vz + j * a.vz) >> 4;
        if (i >= 9) {
            c->rot_x = save.rot_x;
            c->facing = ((i - 8) * save.facing + j * r.vy) >> 3;
            c->rot_z = save.rot_z;
        } else {
            c->rot_x = (i * save.rot_x + (8 - i) * r.vx) >> 3;
            c->facing = r.vy;
            c->rot_z = (i * save.rot_z + (8 - i) * r.vz) >> 3;
        }
        world_render_frame(0, 0);
        player_update_status(0);
        if (i > 0)
            i--;
    }
    pad_state.pressed = 0;
    while (i--) {
        j = 16 - i;
        c->position.vx = (i * a.vx + j * b.vx) >> 4;
        c->position.vy = (i * a.vy + j * b.vy) >> 4;
        c->position.vz = (i * a.vz + j * b.vz) >> 4;
        if (i >= 9) {
            c->rot_x = ((i - 8) * r.vx + j * r2.vx) >> 3;
            c->facing = r.vy;
            c->rot_z = ((i - 8) * r.vz + j * r2.vz) >> 3;
        } else {
            c->rot_x = r2.vx;
            c->facing = (i * r.vy + (8 - i) * r2.vy) >> 3;
            c->rot_z = r2.vz;
        }
        world_render_frame(0, 0);
        player_update_status(0);
    }
    c->depth_bias += 8;
    if (!take) {
        *c = save;
        c->state = c->prev_state;
    } else {
        item_record_pickup(c);
        ground_item_remove(c);
    }
}

/* The per-frame table C pass: drops fall and bounce, a pick-up runs. */
ADDRESS(0x80053aa4, 0x294)
void ground_items_update(void)
{
    CollisionRequest req;
    GroundItem *c = vis_table_c.entries;
    ItemType *rec;
    int i = 64;
    int h;

    while (i--) {
        if (c->item == 0xffff || c->area != area_state.loaded_area) {
            c++;
            continue;
        }
        rec = &vis_table_c.types[c->item];
        c->draw_flags = 0;
        c->player_dist_xz = SquareRoot0((c->position.vx - viewer.position.vx) * (c->position.vx - viewer.position.vx)
                               + (c->position.vz - viewer.position.vz) * (c->position.vz - viewer.position.vz));
        switch (c->state) {
        case GROUND_ITEM_NONE:
        case GROUND_ITEM_RESTING:
        case GROUND_ITEM_HIDDEN:
            break;
        case GROUND_ITEM_FALLING:
            if (c->bounces < 4) {
                req.position = c->position;
                req.radius = *(u16 *)&rec->unk5[9];
                req.height = 0x200;
                req.tests = 2;
                req.velocity.vz = 0;
                req.velocity.vx = 0;
                req.position.vy -= 0x200;
                req.velocity.vy = c->vertical_speed;
                req.speed = 0;
                req.heading = 0;
                collision_test_map((CollisionState *)&req);
                h = vis_lists.next_floor_dist - 0x200;
                if (h > 0) {
                    c->position.vy += c->vertical_speed;
                    if (c->vertical_speed < 256)
                        c->vertical_speed += 8;
                } else {
                    h = 0;
                    c->bounces++;
                    c->position.vy -= 0x200 - vis_lists.next_floor_dist;
                    c->vertical_speed = -(c->vertical_speed >> 1);
                }
                if (viewer.position.vy + 0x8000 < c->position.vy)
                    ground_item_remove(c);
                if (c->item < 71)
                    c->rot_x = h;
            } else if (c->bounces == 4) {
                c->state = GROUND_ITEM_RESTING;
                c->rot_x = 0;
            }
            break;
        case GROUND_ITEM_PICKED_UP:
            ground_item_pick_up(c, 0);
            viewer.state = VIEWER_STATE_NORMAL;
            break;
        }
        c++;
    }
}
