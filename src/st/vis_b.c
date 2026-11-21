/*
 * ST.EXE: table B (vis_table_b.entries) queries and helpers: the entries a
 * request touches, switch toggles, region tests, a moving entry, fades, the
 * world shift and spawned debris (WIP module; the per-frame pass
 * func_80050768 that calls most of these follows and is not reconstructed).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/hud.h>
#include <st/area.h>
#include <st/light.h>
#include <st/player.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/vis.h>

JUMPTABLES(0x800125d0, 0x14c)

/* The first entry (other than unk71B0) whose record radius, widened by the
 * request's and shortened along its heading, reaches the request at a
 * matching height; -1 when none. */
ADDRESS(0x8004f53c, 0x1dc)
int func_8004f53c(VisLists *req)
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
int func_8004f718(int start, VisLists *req)
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
void func_8004f974(e, kind)
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
Vis88 *func_8004f994(int start, int n, int now)
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
void func_8004fa38(key)
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
int func_8004fb88(Vis88 *e, int id)
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
void func_8004fc0c(key)
    u8 key;
{
    Vis88 *e = vis_table_b.entries;
    u16 i;

    if (key == 0xff)
        return;
    i = 240;
    while (i--) {
        func_8004fb88(e, key);
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
int func_8004fc94(int x, int z, int w, int d, int y)
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
int func_8004fd28(int x, int z, int w, int d, int y)
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
int func_8004fdd0(Vis88 *src, Vis88 *e, SVECTOR *from, SVECTOR *to, int fade, int steps)
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
        func_8004ee00(e);
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
void func_800500ac(int mode, int start, int end, int delta)
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
        func_80021994(c, c, c, 4096, mode);
        func_80015ad4();
        func_80058620();
        func_80045ffc();
        func_8003e734(&pos, &rot);
        func_80026afc(&pos, &rot);
        player_update_status(0);
        if (v == end)
            break;
        v += step;
    }
}

/* Shifts the viewer, the effects and the table A entries by (dx, dy, dz). */
ADDRESS(0x80050170, 0x118)
void func_80050170(int dx, int dy, int dz)
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
Vis88 *func_80050288(src, kind, state, arg3, size, angle, ox, oy, oz)
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
    Vis88 *e = func_8004f994(0, 300, -1);
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
void func_80050468(e, arg)
    Vis88 *e;
    u16 arg;
{
    func_80050288(e, 175, 4, arg, 4, 0x400, 0, 0x500, 0);
    func_80050288(e, 175, 4, arg, 3, 0x400, 0, 0x500, 0);
    func_80050288(e, 175, 4, arg, 2, 0x400, 0, 0x500, 0);
    func_80050288(e, 175, 4, arg, 1, 0x400, 0, 0x500, 0);
    e->unk05 = 0xff;
}

/* The same with state 6, offset pieces. */
ADDRESS(0x80050564, 0x108)
void func_80050564(e, arg)
    Vis88 *e;
    u16 arg;
{
    func_80050288(e, 175, 6, arg, 4, 0x400, 0x500, 2000, 0x500);
    func_80050288(e, 175, 6, arg, 3, 0x400, 0x500, 2000, 0x500);
    func_80050288(e, 175, 6, arg, 2, 0x400, 0x500, 2000, 0x500);
    func_80050288(e, 175, 6, arg, 1, 0x400, 0x500, 2000, 0x500);
    e->unk05 = 0xff;
}

/* Four kind-171 pieces with state 8. */
ADDRESS(0x8005066c, 0xfc)
void func_8005066c(e, arg)
    Vis88 *e;
    u16 arg;
{
    func_80050288(e, 171, 8, arg, 4, 0x400, 0, 0x500, 0);
    func_80050288(e, 171, 8, arg, 3, 0x400, 0, 0x500, 0);
    func_80050288(e, 171, 8, arg, 2, 0x400, 0, 0x500, 0);
    func_80050288(e, 171, 8, arg, 1, 0x400, 0, 0x500, 0);
    e->unk05 = 0xff;
}
