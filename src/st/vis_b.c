/*
 * ST.EXE: table B (vis_table_b.entries: doors, traps, warps and the other
 * fixed things of an area) and table C (items lying in the areas) as one
 * translation unit (WIP module, -G8: the two shake amounts are
 * $gp-relative statics): loading and running table B, the per-frame pass
 * objects_update, warps, table C and its pick-ups, 0x8004f53c..0x80053d38.
 * Retail's switch tables of vis_b (0x800125d0), objects_update and
 * ground_items_update follow one another, each 8-aligned relative to
 * 0x800125d0, which is 4 mod 8 relative to vis_attack's tables before
 * them: one object's read-only data.
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
int object_find_in_reach(VisLists *req)
{
    Vis88 *e = vis_table_b.entries;
    Vis24 *r;
    int i;
    int dx;
    int dz;
    int dist;

    for (i = 0; i < 240; i++, e++) {
        if (e->unk0a == 0xffff || e == vis_table_b.unk71B0)
            continue;
        r = &vis_table_b.unk0000[e->unk0a];
        if (r->unk04 == 0)
            continue;
        dx = e->unk24.vx - req->unk4.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = e->unk24.vz - req->unk4.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        if (dist - req->unk22 * rcos(req->unk20 - (short)ratan2(dz, dx)) / 4096
            >= r->unk04 + req->unk1C)
            continue;
        if (e->unk24.vy < req->unk4.vy) {
            if (e->unk24.vy < req->unk4.vy - req->unk1E)
                return -1;
        } else if (req->unk4.vy < e->unk24.vy - e->unk12) {
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
int object_find_usable(int start, VisLists *req)
{
    Vis88 *e = &vis_table_b.entries[start];
    Vis24 *r;
    short i;
    int dx;
    int dz;
    int dist;
    short d;
    u8 k;

    for (i = start; i < 240; i++, e++) {
        if (e->unk0a == 0xffff)
            continue;
        r = &vis_table_b.unk0000[e->unk0a];
        if (r->unk06 == 0)
            continue;
        dx = e->unk24.vx - req->unk4.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = e->unk24.vz - req->unk4.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        ratan2(dz, dx);
        if (dist >= r->unk06)
            continue;
        if (e->unk24.vy < req->unk4.vy ? e->unk24.vy < req->unk4.vy - req->unk1E
                                       : req->unk4.vy < e->unk24.vy - (e->unk12 + 0x300))
            continue;
        d = (viewer.unk26A - e->unk20) & 0xfff;
        if (d > 0x800)
            d |= 0xf000;
        if ((r->unk03 & 8) && (u16)(d + 400) > 800)
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
    Vis88 *e;
    u8 kind;
{
    if (e->unk05 == 0xff) {
        e->unk05 = kind;
        e->unk0C = 0;
    }
}

/* A free entry among `n` from `start`; exits when there is none. */
ADDRESS(0x8004f994, 0xa4)
Vis88 *object_alloc(int start, int n, int now)
{
    Vis88 *e = &vis_table_b.entries[start];
    int oldest = 0;
    int age;

    do {
        if (e->unk0a == 0xffff)
            return e;
        if (now != -1) {
            age = now - *(u16 *)&e->unk4A.snd.unk4C;
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
    Vis88 *e = vis_table_b.entries;
    u16 i;

    if (key == 0xff)
        return;
    i = 240;
    while (i--) {
        switch (e->unk05) {
        case 'T':
        case 0xa2:
        case 0xa3:
            if (e->unk48 == key)
                e->unk48 = 0xff;
            break;
        case '_':
            if (e->unk48 == key) {
                e->unk0C = 1;
                ((u8 *)&e->unk4A.snd.unk4E)[1] = ((u8 *)&e->unk4A.snd.unk4E)[1] == 0;
            }
            break;
        case 'X':
            if (e->unk49 == key) {
                e->unk0C = 1;
                e->unk48 = e->unk48 == 0;
            }
            break;
        case 'Q':
            if (e->unk4A.snd.unk4D == key)
                e->unk48 = e->unk48 ? 0 : 0xff;
            break;
        case 'R':
        case 'U':
        case 'Z':
            if (((u8 *)&e->unk4A.snd.unk4E)[1] == key)
                e->unk48 = e->unk48 ? 0 : 0xff;
            break;
        case 'Y':
            if (e->unk48 == key)
                e->unk0C = 1;
            break;
        case 'V':
            if (e->unk4A.snd.unk4A == key)
                e->unk49 = e->unk49 ? 0 : 0xff;
            break;
        }
        e++;
    }
}

/* For a switch-like entry at rest: 2 when it has no key, 1 (and the key is
 * used up) when `id` is its key item, 3 otherwise; 0 for other entries. */
ADDRESS(0x8004fb88, 0x84)
int object_try_key(Vis88 *e, int id)
{
    switch (e->unk05) {
    case 0:
    case 1:
    case 50:
    case 51:
    case 52:
        if (e->unk0C != 0)
            return 0;
        if (((u8 *)&e->unk4A.snd.unk4E)[1] >= 0xfe)
            return 2;
        if (((u8 *)&e->unk4A.snd.unk4E)[1] == id - 272) {
            ((u8 *)&e->unk4A.snd.unk4E)[1] = 0xff;
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
    Vis88 *e = vis_table_b.entries;
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
int object_move_child(Vis88 *src, Vis88 *e, SVECTOR *from, SVECTOR *to, int fade, int steps)
{
    VECTOR a;
    VECTOR b;
    int t;

    switch (src->unk0C) {
    case 0:
        if (src->unk48 == 0xff)
            return 0;
        if (e->unk48 == 0)
            *(u8 *)&src->unk50 = 0;
        else
            *(u8 *)&src->unk50 = steps - 1;
        src->unk0C = 1;
        object_reset(e);
        e->unk05 = 112;
        e->unk01 = src->unk01;
        switch (e->unk0a) {
        case 136:
            e->unk34 = 0x400;
            e->unk14 = 100;
            break;
        case 133:
            e->unk14 = -220;
            break;
        }
        if (fade)
            e->unk02 = 0;
        else
            e->unk02 = 0x80;
        func_8004a078((SVECTOR *)&src->unk34, from, &e->unk24);
        e->unk24.vx += src->unk24.vx;
        e->unk24.vy += src->unk24.vy;
        e->unk24.vz += src->unk24.vz;
        e->unk36 = src->unk36;
        *(s16 *)&e->unk50 = src - vis_table_b.entries;
    case 1:
        break;
    default:
        return 0;
    }
    if (fade) {
        e->unk10 += 204;
        if (e->unk10 >= 4096)
            e->unk10 = 4095;
    }
    func_8004a078((SVECTOR *)&src->unk34, from, &a);
    func_8004a078((SVECTOR *)&src->unk34, to, &b);
    (*(u8 *)&src->unk50)++;
    t = (*(u8 *)&src->unk50 << 12) / steps;
    e->unk24.vx = src->unk24.vx + func_8004a830(a.vx, b.vx, t);
    e->unk24.vy = src->unk24.vy + func_8004a830(a.vy, b.vy, t);
    e->unk24.vz = src->unk24.vz + func_8004a830(a.vz, b.vz, t);
    if (*(u8 *)&src->unk50 >= steps) {
        if (fade)
            e->unk10 = 4095;
        if (area_state.unk0 == 0)
            e->unk48 = 0xff;
        src->unk0C = 99;
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
        func_80058620();
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
    Vis212 *e;
    u16 i;
    u16 j;

    viewer.position.vx += dx;
    viewer.position.vz += dz;
    viewer.position.vy += dy;
    i = 128;
    while (i--) {
        if (f->unk0 != ST_EFFECT_FREE && (f->unk6 & 0xc) != 0xc) {
            f->unk20.vx += dx;
            f->unk20.vz += dz;
            f->unk20.vy += dy;
        }
        f++;
    }
    e = vis_table_a.entries;
    j = 64;
    while (j--) {
        if (e->unk00 != 0xff) {
            e->unk74.vx += dx;
            e->unk74.vz += dz;
            e->unk74.vy += dy;
        }
        e++;
    }
}

/* Spawns a debris entry of `kind` beside src, `size` units out along its
 * yaw turned by `angle`. */
ADDRESS(0x80050288, 0x1e0)
Vis88 *object_spawn_part(src, kind, state, arg3, size, angle, ox, oy, oz)
    Vis88 *src;
    u8 kind;
    u16 state;
    s16 arg3;
    u8 size;
    short angle;
    int ox;
    int oy;
    int oz;
{
    Vis88 *e = object_alloc(0, 300, -1);
    int c;
    int n;

    e->unk0a = kind;
    e->unk48 = 0;
    e->unk08 = src->unk08;
    e->unk05 = state;
    *(s16 *)&e->unk50 = arg3;
    e->unk0C = size;
    c = rcos(src->unk36 - angle) / 16;
    e->unk56 = rsin(src->unk36 - angle) / 16;
    e->unk04 = 0x40;
    ((s16 *)&e->unk50)[1] = c;
    e->unk54 = 0;
    e->unk00 = 0;
    e->unk02 = 0x80;
    e->unk34 = 0;
    e->unk38 = rand() / 8;
    e->unk36 = src->unk36;
    e->unk14 = src->unk14;
    e->unk03 = 15;
    e->unk06 = 0xff;
    e->unk16 = 0x800;
    n = size;
    e->unk3C.vx = e->unk3C.vy = e->unk3C.vz = n << 10;
    e->unk24.vx = src->unk24.vx + (n * 256 - ox) * rcos(src->unk36) / 4096;
    e->unk24.vy = src->unk24.vy - oy;
    e->unk24.vz = src->unk24.vz + (n * 256 - oz) * rsin(src->unk36) / 4096;
    return e;
}

/* Breaks src into four kind-175 pieces. */
ADDRESS(0x80050468, 0xfc)
void object_spawn_side_parts(e, arg)
    Vis88 *e;
    u16 arg;
{
    object_spawn_part(e, 175, 4, arg, 4, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 175, 4, arg, 3, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 175, 4, arg, 2, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 175, 4, arg, 1, 0x400, 0, 0x500, 0);
    e->unk05 = 0xff;
}

/* The same with state 6, offset pieces. */
ADDRESS(0x80050564, 0x108)
void object_spawn_forward_parts(e, arg)
    Vis88 *e;
    u16 arg;
{
    object_spawn_part(e, 175, 6, arg, 4, 0x400, 0x500, 2000, 0x500);
    object_spawn_part(e, 175, 6, arg, 3, 0x400, 0x500, 2000, 0x500);
    object_spawn_part(e, 175, 6, arg, 2, 0x400, 0x500, 2000, 0x500);
    object_spawn_part(e, 175, 6, arg, 1, 0x400, 0x500, 2000, 0x500);
    e->unk05 = 0xff;
}

/* Four kind-171 pieces with state 8. */
ADDRESS(0x8005066c, 0xfc)
void object_spawn_side_parts_alt(e, arg)
    Vis88 *e;
    u16 arg;
{
    object_spawn_part(e, 171, 8, arg, 4, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 171, 8, arg, 3, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 171, 8, arg, 2, 0x400, 0, 0x500, 0);
    object_spawn_part(e, 171, 8, arg, 1, 0x400, 0, 0x500, 0);
    e->unk05 = 0xff;
}

/* objects_update's switch table (a door's swing side). */

/* The shake (x, z) a kind 224 entry gives the view while its entries rise. */
DATA(0x800881e0, 0x8, ".sbss")
static SVECTOR DAT_800881e0;

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
#define SOUND(e, n) object_play_sound(e, (e)->unk0a * 3 + (n))

/* The moving sound as the entry leaves its rest position. */
#define START(e, step)                                                         \
    if ((e)->unk10 == 0 && (e)->unk10 + (step) * 4 > 0)                        \
        SOUND(e, 208)

/* Swaps the first bytes of the entry's two record ranges. */
#define SWAP_RECORDS(e)                                                            n = (e)->unk4A.door.unk4E;                                                     while (n--) {                                                                      u8 t = vis_records[(e)->unk4A.door.unk4C + n].part;                                                                                                          vis_records[(e)->unk4A.door.unk4C + n].part =                                     vis_records[(e)->unk4A.door.unk4A + n].part;                              vis_records[(e)->unk4A.door.unk4A + n].part = t;                          }

/*
 * Updates every table B entry by its kind.
 */
ADDRESS(0x80050768, 0x212c)
void objects_update(void)
{
    u16 dmg[9];
    u16 dmg2[9];
    Vis88 *e = vis_table_b.entries;
    Vis24 *rec;
    int i;
    int dx;
    int dy;
    int dz;
    int angle;
    int off;
    u16 n;

    for (i = 240; i != 0; e++, i--) {
        if (e->unk0a == 0xffff)
            continue;
        vis_table_b.unk71B0 = e;
        vis_table_b.unk71AC = rec = &vis_table_b.unk0000[e->unk0a];
        dx = e->unk24.vx - viewer.position.vx;
        dy = e->unk24.vy - viewer.position.vy;
        dz = e->unk24.vz - viewer.position.vz;
        if (dx + 0x8000 > 0x10000u || dy > 0x8000 || dy < -0x8000 || dz > 0x8000 || dz < -0x8000) {
            e->unk18 = 0x40000000;
            e->unk1C = 0x8000;
        } else {
            e->unk18 = dx * dx + dy * dy + dz * dz;
            e->unk1C = SquareRoot0(dx * dx + dz * dz);
        }
        angle = (ratan2(dz, dx) - 1024) & 0xfff;
        e->unk20 = angle;
        off = ((u16)e->unk36 - angle) & 0xfff;
        e->unk22 = off;
        if (off > 2048)
            e->unk22 = 4096 - off;
        if (e->unk05 == 255)
            continue;
        switch (e->unk05) {
        case 0:
        case 1: {
            int step = e->unk48;
            short speed = step;

            speed *= 4;
            switch (e->unk0C) {
            case 0:
                if (e->unk05 == 1) {
                    if (world_state.unk834C[e->unk0a + 256] == NULL)
                        e->unk00 = 0;
                    else
                        e->unk00 = 2;
                }
                if (e->unk49 >= 249 && e->unk49 <= 250 && e->unk1C <= 8192)
                    e->unk0C = 1;
                break;
            case 1:
                if (e->unk10 == 0 && e->unk10 + step * 4 > 0) {
                    SWAP_RECORDS(e);
                    if (e->unk05 == 1)
                        e->unk00 = 1;
                    SOUND(e, 208);
                }
                if (speed + e->unk10 >= 4095) {
                    e->unk10 = 4095;
                    e->unk0C = 2;
                } else {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                }
                if (e->unk10 < 3072 && speed + e->unk10 >= 3072)
                    SOUND(e, 210);
                if (speed != 0 && e->unk10 <= 2048 && e->unk18 <= 2048)
                    player_stop_motion();
                if (e->unk1C <= 2048 && e->unk10 <= 2048) {
                    int push = 0;

                    switch (rec->unk12) {
                    case 0: {
                        short side = e->unk22;

                        if (side > -1024 && side < 1024)
                            push = 1;
                        break;
                    }
                    case 1:
                        if (e->unk22 > 0 && e->unk22 < 2048)
                            push = 1;
                        break;
                    case 2:
                        if (e->unk22 > 1024 && e->unk22 < 3072)
                            push = 1;
                        break;
                    case 3:
                        if (e->unk22 > 2048 && e->unk22 < 4096)
                            push = 1;
                        break;
                    case 5:
                        push = 1;
                        break;
                    case 4:
                        break;
                    }
                    if (speed != 0 && push) {
                        short away = e->unk20;

                        player_stop_motion();
                        away -= 1024;
                        vis_table_b.unk71A4 += rcos(away) / 32;
                        vis_table_b.unk71A8 += rsin(away) / 32;
                    }
                }
                break;
            case 2:
                if (e->unk49 == 252 || e->unk49 == 250) {
                    e->unk0C = 255;
                    break;
                }
                if (e->unk1C < 8192)
                    break;
                if (!(e->unk49 & 0x80)) {
                    e->unk02 = e->unk49 == 0;
                    e->unk10 = 0;
                }
                e->unk0C = 3;
                SOUND(e, 208);
                break;
            case 3:
                if (e->unk49 & 0x80) {
                    if (step * 4 < e->unk10) {
                        e->unk10 = (e->unk10 - speed) & 0xfff;
                        break;
                    }
                    SWAP_RECORDS(e);
                } else {
                    if (e->unk10 + step * 4 < 4095) {
                        e->unk10 = (e->unk10 + speed) & 0xfff;
                        break;
                    }
                    SWAP_RECORDS(e);
                    if (!(e->unk49 & 0x80))
                        e->unk02 = e->unk49;
                }
                e->unk10 = 0;
                e->unk0C = 0;
                SOUND(e, 209);
                break;
            case 7:
                if (e->unk05 == 1)
                    e->unk00 = 2;
                break;
            case 8:
                e->unk00 = 1;
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                } else {
                    e->unk00 = 0;
                    e->unk10 = 4095;
                    e->unk0C = 255;
                }
                break;
            case 11:
                break;
            case 12:
                START(e, step);
                e->unk10 += speed;
                if (speed + e->unk10 >= 4095) {
                    e->unk10 = 4095;
                    e->unk00 = 0;
                    e->unk0C = 255;
                }
                break;
            case 255:
                e->unk10 = 4095;
                break;
            }
            break;
        }
        case 50:
        case 51: {
            int step = e->unk48;
            short speed = step;

            speed *= 4;
            switch (e->unk0C) {
            case 0:
                if (e->unk05 == 51)
                    ground_item_set_hidden(e->unk4A.snd.unk4C, 0);
                if (e->unk49 >= 249 && e->unk49 <= 250 && e->unk1C <= 8192)
                    e->unk0C = 1;
                break;
            case 1:
                if (e->unk10 == 0 && e->unk10 + step * 4 > 0) {
                    SOUND(e, 208);
                    if (e->unk05 == 51)
                        ground_item_set_hidden(e->unk4A.snd.unk4C, 1);
                }
                if (e->unk10 < 3072 && speed + e->unk10 >= 3072)
                    SOUND(e, 210);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                } else {
                    e->unk10 = 4095;
                    e->unk0C = 2;
                }
                break;
            case 2:
                if (e->unk49 == 252 || e->unk49 == 250) {
                    e->unk0C = 255;
                    break;
                }
                if (e->unk1C < 8192)
                    break;
                if (!(e->unk49 & 0x80)) {
                    e->unk02 = e->unk49 == 0;
                    e->unk10 = 0;
                }
                e->unk0C = 3;
                SOUND(e, 208);
                break;
            case 3:
                if (e->unk49 & 0x80) {
                    if (speed < e->unk10) {
                        e->unk10 = (e->unk10 - e->unk48) & 0xfff;
                        break;
                    }
                } else {
                    if (speed + e->unk10 < 4095) {
                        e->unk10 = (e->unk10 + speed) & 0xfff;
                        break;
                    }
                    e->unk02 = e->unk49;
                }
                e->unk10 = 0;
                e->unk0C = 0;
                break;
            case 4:
                START(e, step);
                e->unk10 += speed;
                if (e->unk10 >= 4095)
                    e->unk10 &= 0xfff;
                break;
            case 5:
                if (world_state.tint_level < 54 || H50(e) >= 600) {
                    e->unk0C = 6;
                    H50(e) = 0;
                    SOUND(e, 208);
                } else {
                    H50(e)++;
                }
                break;
            case 6:
                e->unk10 += speed;
                if (e->unk10 + step * 4 >= 4095) {
                    e->unk10 = 0;
                    e->unk0C = 5;
                }
                break;
            case 7:
                break;
            case 8:
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                } else {
                    e->unk10 = 4095;
                    e->unk0C = 255;
                }
                break;
            case 9:
                if ((rand() >> 12) == 0) {
                    if (H50(e) % 20 == 0) {
                        e->unk0C = 10;
                        e->unk00 = 0;
                        H50(e) = 0;
                        SOUND(e, 208);
                    }
                } else {
                    H50(e)++;
                }
                break;
            case 10:
                e->unk10 += speed;
                if (e->unk10 + step * 4 >= 4096) {
                    e->unk10 = 0;
                    e->unk00 = 0;
                    e->unk0C = 9;
                }
                break;
            case 11:
                break;
            case 12:
                START(e, step);
                e->unk10 += speed;
                if (speed + e->unk10 >= 4095) {
                    e->unk10 = 4095;
                    e->unk00 = 0;
                    e->unk0C = 255;
                }
                break;
            case 255:
                e->unk10 = 4095;
                break;
            }
            break;
        }
        case 52: {
            int step = e->unk48;
            short speed = step;
            Vis88 *o;
            int j;
            int group;

            speed *= 4;
            switch (e->unk0C) {
            case 0:
                break;
            case 1:
                START(e, step);
                if (speed + e->unk10 < 4095) {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
                    break;
                }
                if (e->unk49 != 255 && e->unk49 != 252) {
                    e->unk02 = 1;
                    e->unk10 = 0;
                }
                e->unk0C = 255;
                if (e->unk4A.snd.unk4B == 255)
                    break;
                group = e->unk4A.snd.unk4B;
                for (o = vis_table_b.entries, j = 0; j < 240; j++, o++) {
                    if ((o->unk05 == 0 || o->unk05 == 50) && o->unk4A.door.unk4F == group) {
                        if (o->unk0C == 0) {
                            o->unk49 = 252;
                            o->unk0C = 1;
                            o->unk02 = 0;
                            o->unk10 = 0;
                        } else if (o->unk0C == 255) {
                            o->unk0C = 3;
                            o->unk02 = 1;
                            o->unk10 = 0;
                            SOUND(o, 208);
                        }
                    }
                }
                break;
            case 3:
                START(e, step);
                if (speed + e->unk10 >= 4095) {
                    e->unk02 = 0;
                    e->unk10 = 0;
                    e->unk0C = 0;
                    if (e->unk4A.snd.unk4B == 255)
                        break;
                    group = e->unk4A.snd.unk4B;
                    for (o = vis_table_b.entries, j = 0; j < 240; j++, o++) {
                        if (o->unk05 == 0 && o->unk4A.door.unk4F == group) {
                            if (o->unk0C == 0) {
                                o->unk49 = 252;
                                o->unk0C = 1;
                                o->unk02 = 0;
                                o->unk10 = 0;
                            } else if (o->unk0C == 255) {
                                o->unk49 = 0;
                                o->unk0C = 3;
                                o->unk02 = 1;
                                o->unk10 = 0;
                                SOUND(o, 208);
                            }
                        }
                    }
                } else {
                    e->unk10 = (e->unk10 + speed) & 0xfff;
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
            if (e->unk0C == 0) {
                if (e->unk1C < 1024 && B50(e) == 0) {
                    memset(dmg, 0, 18);
                    dmg[3] = 100;
                    player_hit(dmg, 4096, 0, (long *)&e->unk24);
                    B50(e) = 20;
                }
                if (e->unk3C.vy > 4096) {
                    e->unk3C.vx = e->unk3C.vx - 512 - a;
                    e->unk3C.vz = e->unk3C.vz - 512 - c;
                } else {
                    e->unk3C.vx = a + (e->unk3C.vx + 512);
                    e->unk3C.vz = c + (e->unk3C.vz + 512);
                }
                e->unk3C.vy = b + (e->unk3C.vy + 1256);
                if (e->unk3C.vy > 8192)
                    e->unk0C = 1;
            } else if (e->unk0C == 1) {
                e->unk3C.vx = 0;
                e->unk3C.vy = 0;
                e->unk3C.vz = 0;
                e->unk0C = 2;
            } else if (rand() < 2048 && B50(e) == 0) {
                e->unk0C = 0;
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
            if (e->unk1C < e->unk3C.vx / 4 && e->unk48 == 0) {
                if (e->unk05 == 4) {
                    memset(dmg, 0, 18);
                    dmg[3] = 180;
                    player_hit(dmg, 4096, 0, (long *)&e->unk24);
                    e->unk48 = 20;
                } else if (e->unk05 == 8) {
                    memset(dmg2, 0, 18);
                    dmg2[6] = 135;
                    player_hit(dmg2, 4096, 5120, (long *)&e->unk24);
                    e->unk48 = 20;
                }
            }
            if (e->unk0C >= 5) {
                e->unk24 = vis_table_b.entries[H50(e)].unk24;
                e->unk24.vy -= 1280;
                e->unk3C.vx = e->unk3C.vy = e->unk3C.vz = 0;
                H52(e) = rcos(e->unk36 - 1024) * 384 / 4096;
                e->unk54 = 0;
                e->unk56 = rsin(e->unk36 - 1024) * 384 / 4096;
                e->unk24.vx += H52(e);
                e->unk24.vz += e->unk56;
                e->unk0C = 0;
            } else if (e->unk0C >= 3) {
                H52(e) = e->unk0C * (rcos(e->unk36 - 1024) << 7) / 4096;
                e->unk54 = e->unk0C << 5;
                e->unk56 = e->unk0C * (rsin(e->unk36 - 1024) << 7) / 4096;
                e->unk24.vx += H52(e) - rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 - rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            } else {
                H52(e) = e->unk0C * (rcos(e->unk36 - 1024) << 7) / 4096;
                e->unk54 = e->unk0C << 6;
                e->unk56 = e->unk0C * (rsin(e->unk36 - 1024) << 7) / 4096;
                e->unk24.vx += H52(e) + rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 + rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            }
            break;
        case 5:
            object_spawn_forward_parts(e, (u16)(240 - i));
            break;
        case 6:
            if (e->unk48)
                e->unk48--;
            if (e->unk1C < e->unk3C.vx / 4 && e->unk48 == 0) {
                memset(dmg, 0, 18);
                dmg[3] = 100;
                player_hit(dmg, 4096, 0, (long *)&e->unk24);
                e->unk48 = 20;
            }
            if (e->unk0C >= 5) {
                e->unk24 = vis_table_b.entries[H50(e)].unk24;
                e->unk24.vx -= rcos(e->unk36) * 1280 / 4096;
                e->unk24.vy -= 2000;
                e->unk24.vz -= rsin(e->unk36) * 1280 / 4096;
                e->unk3C.vx = e->unk3C.vy = e->unk3C.vz = 0;
                H52(e) = rcos(e->unk36) * 384 / 4096;
                e->unk54 = 128;
                e->unk56 = rsin(e->unk36) * 384 / 4096;
                e->unk24.vx += H52(e);
                e->unk24.vz += e->unk56;
                e->unk0C = 0;
            } else if (e->unk0C >= 3) {
                H52(e) = e->unk0C * (rcos(e->unk36) << 7) / 4096;
                e->unk54 = e->unk0C << 6;
                e->unk56 = e->unk0C * (rsin(e->unk36) << 7) / 4096;
                e->unk24.vx += H52(e) - rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 - rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            } else {
                H52(e) = e->unk0C * (rcos(e->unk36) << 7) / 4096;
                e->unk54 = e->unk0C << 7;
                e->unk56 = e->unk0C * (rsin(e->unk36) << 7) / 4096;
                e->unk24.vx += H52(e) + rand() / 256;
                e->unk24.vy += e->unk54 + rand() / 2048;
                e->unk24.vz += e->unk56 + rand() / 256;
                e->unk3C.vx += 1024;
                e->unk3C.vy += 1024;
                e->unk3C.vz += 1024;
                e->unk0C++;
            }
            break;
        case 7:
            object_spawn_side_parts_alt(e, (u16)(240 - i));
            break;
        case 224: {
            Vis212 *a;
            int j;
            int rising;

            if (player_in_cell_rect(e->unk24.vx >> 12, e->unk24.vz >> 12, 1, 1, e->unk24.vy) == 0
                && area_state.unk0 == 0)
                break;
            if ((DAT_801cd180.unk2 & DAT_801cd180.unk22[10]) && vis_table_a.unk6169 == 0
                && W50(e) != 0xffffff && W48(e) != 0xffffff) {
                vis_table_a.unk6169 = 1;
                player_stop_motion();
                func_80057a3c();
            }
            if (vis_table_a.unk6169 == 1) {
                a = &vis_table_a.entries[56];
                rising = 0;
                cd_wait_idle();
                viewer.unk21E = 1;
                for (j = 7; j != -1; a++, j--) {
                    a->unk2A = 0;
                    if (a->unk00 != 255) {
                        func_80058220(a->unk0B * 10 + 48, &a->unk74, 127, (rand() * 5 >> 15) - 2);
                        a->unkB0 = 128;
                        DAT_800881e0.vx = rcos((s16)a->unk56 - 1024) / 256;
                        DAT_800881e0.vz = rsin((s16)a->unk56 - 1024) / 256;
                        rising = 1;
                    }
                }
                if (rising)
                    vis_table_a.unk6169 = 2;
            }
            if (vis_table_a.unk6169 == 2) {
                a = &vis_table_a.entries[56];
                player_stop_motion();
                vis_table_b.unk71A4 = DAT_800881e0.vx;
                vis_table_b.unk71A8 = DAT_800881e0.vz;
                for (rising = 7; rising != -1; a++, rising--) {
                    if (a->unk00 != 255 && (s16)a->unk2A + a->unkB0 > 3584) {
                        a->unkB0 = 0;
                        a->unk2A = 4095;
                        vis_table_a.unk6169 = 3;
                        func_80057a3c();
                    }
                }
                break;
            }
            if (vis_table_a.unk6169 != 3) {
                if (W50(e) != 0xffffff)
                    break;
            } else {
                DAT_800881e0.vx = 0;
                DAT_800881e0.vz = 0;
            }
            vis_table_a.unk6169 = 4;
            area_request_load(e->unk4A.snd.unk4D, e->unk4A.snd.unk4D, e->unk4A.snd.unk4D, e->unk4A.snd.unk4D,
                          e->unk4A.snd.unk4D, (s8)B50(e), B52(e), B51(e), (s8)e->unk4A.snd.unk4B,
                          (s8)e->unk4A.door.unk4F, (s8)e->unk4A.snd.unk4C);
            if (W48(e) != 0xffffff) {
                vis_table_b.unk71A4 = -DAT_800881e0.vx * 2;
                vis_table_b.unk71A8 = -DAT_800881e0.vz * 2;
                viewer.position.vx = e->unk24.vx - 2048;
                viewer.position.vy = e->unk24.vy - 1600;
                viewer.position.vz = e->unk24.vz - 2048;
                viewer.unk2B8.vx = 0;
                viewer.unk2B8.vz = 0;
            }
            if (e->unk4A.door.unk4E != 255) {
                int yaw = e->unk4A.door.unk4E << 10;

                viewer.unk278.vy += yaw;
            }
            break;
        }
        case 225:
            break;
        case 237:
        case 239: {
            Vis88 *o;
            int j;
            short turn;
            u8 x;
            u8 y;
            u8 z;
            u8 area;
            u8 lit;
            u8 lit2;

            if (player_in_centred_rect(e->unk24.vx, e->unk24.vz, 3072, 3072, e->unk24.vy) == 0)
                goto away;
            turn = (viewer.unk26A - e->unk20) & 0xfff;
            if (turn > 2048)
                turn |= 0xf000;
            if (!((DAT_801cd180.unk2 & DAT_801cd180.unk22[10]) && turn > -400 && turn < 400)
                && e->unk05 != 239)
                goto away;
            if (B50(e))
                break;
            viewer.unk220.warp.x = viewer.position.vx >> 12;
            viewer.unk220.warp.area = area_state.unk4;
            viewer.unk220.warp.y = viewer.position.vy >> 12;
            viewer.unk220.warp.z = viewer.position.vz >> 12;
            viewer.unk220.warp.dx = viewer.position.vx - (viewer.unk220.warp.x << 12);
            viewer.unk220.warp.dy = viewer.position.vy - (viewer.unk220.warp.y << 12);
            viewer.unk220.warp.dz = viewer.position.vz - (viewer.unk220.warp.z << 12);
            viewer.unk220.warp.yaw = e->unk36 + 2048;
            for (j = 240, o = vis_table_b.entries; j != 0; j--, o++) {
                if (o->unk05 == e->unk05)
                    B50(o) = 1;
            }
            x = e->unk49 + 64;
            y = e->unk4A.snd.unk4A + 64;
            z = e->unk4A.snd.unk4B + 64;
            func_80057c54();
            fade_run_frames(2, 0, 0x1000, 0x100);
            area = e->unk48;
            lit = area;
            lit2 = area;
            do {
                cd_wait_frame();
                area_load_update();
            } while (area_state.unk0 != 0);
            viewer.unk278.vy = e->unk4A.door.unk4F << 10;
            viewer.unk278.vx = 0;
            if (area != area_state.unk4) {
                area_request_load(area, area, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
            } else {
                area_request_load(0xff, 0xff, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
                viewer.unk21C = 1;
                creatures_bind_previous_area();
                vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] =
                    vis_table_a.unk6158[3] = 0xff;
                vis_table_a.unk6103 = 0xff;
            }
            viewer.unk218 = 0;
            viewer.unk28C = 0;
            viewer.unk28A = 0;
            viewer.unk288 = 0;
            *(SVECTOR *)&viewer.unk268 = viewer.unk278;
            viewer.position.vx = (x << 12) + ((s8)e->unk4A.snd.unk4C << 7);
            viewer.position.vz = (z << 12) + ((s8)e->unk4A.door.unk4E << 7);
            viewer.position.vy = (y << 12) + ((s8)e->unk4A.snd.unk4D << 7) - 1600;
            viewer.unk2F0 = 0;
            viewer.unk2EC = 0;
            player_stop_motion();
            vis_table_b.unk71A8 = 0;
            vis_table_b.unk71A6 = 0;
            vis_table_b.unk71A4 = 0;
            vis_lists.unk47D2 = 0;
            vis_lists.unk47D0 = 0;
            vis_lists.unk47CE = 0;
            vis_table_a.unk6164 = 0;
            vis_table_a.unk6162 = 0;
            vis_table_a.unk6160 = 0;
            viewer.unk1D0 = 0;
            viewer.unk1CE = 0;
            viewer.unk1CC = 0;
            do {
                cd_wait_frame();
                area_load_update();
            } while (area_state.unk0 != 0);
            vis_lists.n_sources = 0;
            vis_collect_sources((VECTOR *)&viewer.position.vx, NULL, NULL);
            if (vis_lists.n_sources)
                viewer.unk174 = &vis_records[vis_lists.sources[0]];
            creatures_update();
            fade_run_frames(2, 0x1000, 0x1000, 0);
            cd_wait_idle();
            func_80057c54();
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
            area_state.hooks->unk24();
            break;
        }
    }
    vis_table_b.unk71B0 = NULL;
    vis_table_b.unk71A4 /= 2;
    vis_table_b.unk71A6 /= 2;
    vis_table_b.unk71A8 /= 2;
    if (vis_table_b.unk71A4 > -4 && vis_table_b.unk71A4 < 4)
        vis_table_b.unk71A4 = 0;
    if (vis_table_b.unk71A6 > -4 && vis_table_b.unk71A6 < 4)
        vis_table_b.unk71A6 = 0;
    if (vis_table_b.unk71A8 > -4 && vis_table_b.unk71A8 < 4)
        vis_table_b.unk71A8 = 0;
}

/* src/st/area_a.c: from here on its calls see the prototype. */
KF_VALUELESS_S32 area_request_load(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5);

/* Fades out, loads `to`'s area and places the viewer there, then fades in;
 * `keep` skips re-collecting the visible sources and the light reset. */
ADDRESS(0x80052894, 0x390)
void player_warp(to, keep)
    StWarp *to;
    u8 keep;
{
    Vis88 *e = vis_table_b.entries;
    StEffect *f;
    int i;
    u8 x;
    u8 y;
    u8 z;
    u8 area;
    int j;

    for (i = 240; i != 0; i--) {
        if (e->unk05 == 237)
            *(u8 *)&e->unk50 = 1;
        e++;
    }
    x = to->x;
    y = to->y;
    z = to->z;
    func_80057c54();
    fade_run_frames(2, 0, 0x1000, 0x100);
    area = to->area;
    do {
        cd_wait_frame();
        area_load_update();
    } while (area_state.unk0 != 0);
    viewer.unk278.vy = to->yaw;
    viewer.unk278.vx = 0;
    if (area != area_state.unk4) {
        area_request_load(area, area, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
    } else {
        area_request_load(0xff, 0xff, area, 0xff, 0xff, 127, 127, 127, 0, 0, 0);
        viewer.unk21C = 1;
        creatures_bind_previous_area();
        vis_table_a.unk6158[0] = vis_table_a.unk6158[1] = vis_table_a.unk6158[2] = vis_table_a.unk6158[3] = 0xff;
        vis_table_a.unk6103 = 0xff;
    }
    viewer.unk218 = 0;
    viewer.unk28C = 0;
    viewer.unk28A = 0;
    viewer.unk288 = 0;
    *(SVECTOR *)&viewer.unk268 = viewer.unk278;
    viewer.position.vx = (x << 12) + to->dx;
    viewer.position.vz = (z << 12) + to->dz;
    viewer.position.vy = (y << 12) + to->dy;
    player_stop_motion();
    f = effects.slots;
    vis_table_b.unk71A8 = 0;
    vis_table_b.unk71A6 = 0;
    vis_table_b.unk71A4 = 0;
    vis_lists.unk47D2 = 0;
    vis_lists.unk47D0 = 0;
    vis_lists.unk47CE = 0;
    vis_table_a.unk6164 = 0;
    vis_table_a.unk6162 = 0;
    vis_table_a.unk6160 = 0;
    viewer.unk1D0 = 0;
    viewer.unk1CE = 0;
    viewer.unk1CC = 0;
    j = 128;
    while (j--) {
        f->unk0 = ST_EFFECT_FREE;
        f++;
    }
    do {
        cd_wait_frame();
        area_load_update();
    } while (area_state.unk0 != 0);
    if (!keep) {
        vis_lists.n_sources = 0;
        vis_collect_sources((VECTOR *)&viewer.position.vx, NULL, NULL);
        if (vis_lists.n_sources)
            viewer.unk174 = &vis_records[vis_lists.sources[0]];
        creatures_update();
    }
    fade_run_frames(2, 0x1000, 0x1000, 0);
    cd_wait_idle();
    func_80057c54();
    fade_run_frames(2, 0x1000, 0, -0x100);
    if (!keep) {
        area_request_load(0xff, 0xff, 0xff, area, area, 127, 127, 127, 0, 0, 0);
        lighting_clear_requests();
    }
    func_80057a3c();
    player_enter_normal_state();
}

/* Warps to viewer.unk22C. */
ADDRESS(0x80052c24, 0x38)
void player_warp_to_save_point(void)
{
    player_warp((StWarp *)&viewer.unk22C, 0);
    viewer.unk2B = 0;
}

/* Warps to viewer.unk220 keeping the current sources. */
ADDRESS(0x80052c5c, 0x48)
void player_warp_back(void)
{
    viewer.unk218 = 5;
    player_update_durability_warning();
    player_warp(&viewer.unk220.warp, 1);
    DAT_801cd0e4 = 0;
}

/* Frees every table C entry and clears the group counts and id bits. */
ADDRESS(0x80052ca4, 0x6c)
void ground_items_clear(void)
{
    Vis48 *c = vis_table_c.entries;
    int i;

    vis_table_c.unk0000[5] = 0;
    vis_table_c.unk0000[4] = 0;
    vis_table_c.unk0000[3] = 0;
    vis_table_c.unk0000[2] = 0;
    i = 64;
    while (i--) {
        c->unk00 = 0xffff;
        c->unk22 = 0;
        c++;
    }
    /* Ten words each; the tenth of unk0070 is the first of unk0094. */
    i = 10;
    while (i--) {
        vis_table_c.unk0048[i] = 0;
        vis_table_c.unk0070[i] = 0;
    }
}

/* Unassigns every Vis44 slot. */
ADDRESS(0x80052d10, 0x30)
void item_model_slots_clear(void)
{
    int i = 128;

    while (i--)
        vis_table_c.unk0094[i] = 0xff;
    vis_table_c.unk0114 = 0;
}

/* 0 when id has been collected (ids below 266 keep a bit), else 1. */
ADDRESS(0x80052d40, 0x58)
int item_is_uncollected(int id)
{
    if (id < 266) {
        if (vis_table_c.unk0070[id / 32] & (1 << (id % 32)))
            return 0;
    }
    return 1;
}

/* Counts the pick-up of item *id and marks it collected. */
ADDRESS(0x80052d98, 0xb8)
void item_record_pickup(u16 *id)
{
    DAT_801c7590.counts[*id].count++;
    if (*id == 266)
        viewer.unk27++;
    if (*id < 266) {
        viewer.unk28++;
        vis_table_c.unk0070[*id >> 5] |= 1 << (*id & 0x1f);
    }
}

/* Removes an entry, closing up the rest of its group (32..47 or 48..63). */
ADDRESS(0x80052e50, 0xe0)
void ground_item_remove(Vis48 *c)
{
    int i = ((u8 *)c - (u8 *)vis_table_c.entries) / sizeof(Vis48);
    int n;

    if (i < 32) {
        vis_table_c.unk0000[1]--;
    } else {
        if (i < 48) {
            n = 47 - i;
            vis_table_c.unk0000[2]--;
        } else {
            n = 63 - i;
            vis_table_c.unk0000[3]--;
        }
        while (n--) {
            c[0] = c[1];
            c++;
        }
    }
    c->unk22 = 0;
}

/* Loads the first 32 entries from an area file, then drops the collected
 * ones of the 16 from entry 32. */
ADDRESS(0x80052f30, 0x244)
void ground_items_load(VisCFileEntry *src)
{
    Vis48 *c = vis_table_c.entries;
    Vis44 *rec;
    int i;

    vis_table_c.unk0000[0] = 0;
    for (i = 0; i < 32; c++, src++, i++) {
        c->unk00 = src->unk00;
        c->unk02 = src->unk16;
        c->unk08.vx = (src->unk04 << 12) + src->unk08;
        c->unk08.vy = (src->unk05 << 12) + src->unk0A;
        c->unk08.vz = (src->unk06 << 12) + src->unk0C;
        c->unk18 = src->unk0E & 0xfff;
        c->unk1A = src->unk10 & 0xfff;
        c->unk1C = src->unk12 & 0xfff;
        c->unk20 = src->unk02;
        c->unk26 = 0;
        if (c->unk00 == 0xffff) {
            c->unk22 = 0;
            continue;
        }
        rec = &vis_table_c.unk0118[c->unk00];
        c->unk2c = area_state.area;
        c->unk2D = rec->unk5[1];
        vis_table_c.unk0048[c->unk00 >> 5] |= 1 << (c->unk00 & 0x1f);
        vis_table_c.unk0000[1]++;
        vis_table_c.unk0000[0]++;
        if (vis_table_c.unk0094[rec->unk4] == 0xff)
            vis_table_c.unk0094[rec->unk4] = vis_table_c.unk0114++;
        if (i >= 28 || item_is_uncollected(c->unk00))
            c->unk22 = 2;
        else
            c->unk22 = 0;
    }
    c = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++) {
        if (item_is_uncollected(c->unk00) == 0)
            ground_item_remove(c);
        else
            c++;
    }
}

/* The nearest lying item in view of the viewer within the request's reach
 * (its record's radius plus req->unk1C); -1 when none. */
ADDRESS(0x80053174, 0x1e8)
int ground_item_find_in_reach(VisLists *req, int unused)
{
    Vis48 *c;
    Vis44 *rec;
    int best = -1;
    int nearest = 0x7fffffff;
    int i;
    int dx;
    int dz;
    int dist;

    c = vis_table_c.entries;
    for (i = 0; i < 64; i++, c++) {
        if (c->unk22 != 1 && c->unk22 != 2)
            continue;
        if (c->unk2c != area_state.area)
            continue;
        rec = &vis_table_c.unk0118[c->unk00];
        if (rec->unk0[1] == 0)
            continue;
        if (c->unk08.vy < viewer.position.vy - 0x400 || c->unk08.vy > viewer.position.vy + 0xa40)
            continue;
        if (!func_8004a148(viewer.unk278.vy,
                           func_8004a1b4(c->unk08.vx - viewer.position.vx, c->unk08.vz - viewer.position.vz), 300))
            continue;
        dx = c->unk08.vx - req->unk4.vx;
        if (dx < -0x2800 || dx > 0x2800)
            continue;
        dz = c->unk08.vz - req->unk4.vz;
        if (dz < -0x2800 || dz > 0x2800)
            continue;
        dist = SquareRoot0(dx * dx + dz * dz);
        if (dist < rec->unk0[1] * 256 + req->unk1C && dist < nearest) {
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
void ground_item_pick_up(Vis48 *c, int forced)
{
    Vis48 save;
    VECTOR a;
    VECTOR b;
    SVECTOR r;
    SVECTOR r2;
    Vis44 *info;
    int take;
    int i;
    int j;

    info = &vis_table_c.unk0118[c->unk00];
    save = *c;
    viewer.unk218 = 3;
    gte_SetRotMatrix(&viewer.unk298);
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
        r.vx = -viewer.unk268;
        r.vy = r.vz = 0;
        break;
    case 1:
        r.vx = 1024 - viewer.unk268;
        r.vy = r.vz = 0;
        break;
    case 2:
        r.vx = -1024 - viewer.unk268;
        r.vy = r.vz = 0;
        break;
    case 3:
        r.vx = 0;
        r.vy = 1024;
        r.vz = viewer.unk268 - 256;
        break;
    }
    r.vy += viewer.unk26A;
    if (r.vx - c->unk18 > 2048)
        r.vx -= 4096;
    if (r.vx - c->unk18 < -2048)
        r.vx += 4096;
    if (r.vy - c->unk1A > 2048)
        r.vy -= 4096;
    if (r.vy - c->unk1A < -2048)
        r.vy += 4096;
    i = 16;
    c->unk2D -= 8;
    while (1) {
        if (forced == 0 && (DAT_801cd180.unk4 & DAT_801cd180.unk22[11])) {
            take = 0;
            a = c->unk08;
            b = save.unk08;
            r = *(SVECTOR *)&c->unk18;
            r2 = *(SVECTOR *)&save.unk18;
            i = 16 - i;
            break;
        }
        if ((DAT_801cd180.unk4 & DAT_801cd180.unk22[10]) && !DAT_801cd180.unk12) {
            take = 1;
            a = c->unk08;
            b = viewer.position;
            b.vy += 800;
            r = *(SVECTOR *)&c->unk18;
            r2 = r;
            i = 16;
            break;
        }
        j = 16 - i;
        c->unk08.vx = (i * save.unk08.vx + j * a.vx) >> 4;
        c->unk08.vy = (i * save.unk08.vy + j * a.vy) >> 4;
        c->unk08.vz = (i * save.unk08.vz + j * a.vz) >> 4;
        if (i >= 9) {
            c->unk18 = save.unk18;
            c->unk1A = ((i - 8) * save.unk1A + j * r.vy) >> 3;
            c->unk1C = save.unk1C;
        } else {
            c->unk18 = (i * save.unk18 + (8 - i) * r.vx) >> 3;
            c->unk1A = r.vy;
            c->unk1C = (i * save.unk1C + (8 - i) * r.vz) >> 3;
        }
        world_render_frame(0, 0);
        player_update_status(0);
        if (i > 0)
            i--;
    }
    DAT_801cd180.unk4 = 0;
    while (i--) {
        j = 16 - i;
        c->unk08.vx = (i * a.vx + j * b.vx) >> 4;
        c->unk08.vy = (i * a.vy + j * b.vy) >> 4;
        c->unk08.vz = (i * a.vz + j * b.vz) >> 4;
        if (i >= 9) {
            c->unk18 = ((i - 8) * r.vx + j * r2.vx) >> 3;
            c->unk1A = r.vy;
            c->unk1C = ((i - 8) * r.vz + j * r2.vz) >> 3;
        } else {
            c->unk18 = r2.vx;
            c->unk1A = (i * r.vy + (8 - i) * r2.vy) >> 3;
            c->unk1C = r2.vz;
        }
        world_render_frame(0, 0);
        player_update_status(0);
    }
    c->unk2D += 8;
    if (!take) {
        *c = save;
        c->unk22 = c->unk23;
    } else {
        item_record_pickup(c);
        ground_item_remove(c);
    }
}

/* The per-frame table C pass: drops fall and bounce, a pick-up runs. */
ADDRESS(0x80053aa4, 0x294)
void ground_items_update(void)
{
    VisRequest req;
    Vis48 *c = vis_table_c.entries;
    Vis44 *rec;
    int i = 64;
    int h;

    while (i--) {
        if (c->unk00 == 0xffff || c->unk2c != area_state.unk4) {
            c++;
            continue;
        }
        rec = &vis_table_c.unk0118[c->unk00];
        c->unk24 = 0;
        c->unk28 = SquareRoot0((c->unk08.vx - viewer.position.vx) * (c->unk08.vx - viewer.position.vx)
                               + (c->unk08.vz - viewer.position.vz) * (c->unk08.vz - viewer.position.vz));
        switch (c->unk22) {
        case 0:
        case 2:
        case 4:
            break;
        case 1:
            if (c->unk25 < 4) {
                req.unk4 = c->unk08;
                req.unk1C = *(u16 *)&rec->unk5[9];
                req.unk1E = 0x200;
                req.unk0 = 2;
                req.unk14.vz = 0;
                req.unk14.vx = 0;
                req.unk4.vy -= 0x200;
                req.unk14.vy = c->unk26;
                req.unk22 = 0;
                req.unk20 = 0;
                collision_test_map((VisLists *)&req);
                h = vis_lists.unk2C - 0x200;
                if (h > 0) {
                    c->unk08.vy += c->unk26;
                    if (c->unk26 < 256)
                        c->unk26 += 8;
                } else {
                    h = 0;
                    c->unk25++;
                    c->unk08.vy -= 0x200 - vis_lists.unk2C;
                    c->unk26 = -(c->unk26 >> 1);
                }
                if (viewer.position.vy + 0x8000 < c->unk08.vy)
                    ground_item_remove(c);
                if (c->unk00 < 71)
                    c->unk18 = h;
            } else if (c->unk25 == 4) {
                c->unk22 = 2;
                c->unk18 = 0;
            }
            break;
        case 3:
            ground_item_pick_up(c, 0);
            viewer.unk218 = 0;
            break;
        }
        c++;
    }
}
