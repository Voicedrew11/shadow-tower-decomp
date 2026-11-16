/*
 * ST.EXE: angle, matrix, vector and distance helpers (WIP module; 4096 is
 * one turn and 1.0 in the fixed-point matrices).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/funcs.h>

/* Turns `angle` toward `to` the shorter way round by `step`, stopping at
 * `to`. */
ADDRESS(0x800496c8, 0xc4)
int func_800496c8(int a, int b, int step)
{
    short angle = a & 0xfff;
    short to = b & 0xfff;

    if (to == angle)
        return to;
    if (angle < to) {
        if (to - angle <= 2048) {
            angle += step;
            if (angle > to)
                return to;
        } else {
            angle = (angle - step) & 0xfff;
            if (angle >= 2048 && angle <= to)
                return to;
        }
    } else {
        if (angle - to <= 2048) {
            angle -= step;
            if (angle < to)
                return to;
        } else {
            angle = (angle + step) & 0xfff;
            if (angle <= 2048 && angle >= to)
                return to;
        }
    }
    return angle;
}

/* Moves `v` toward `to` by |step|, stopping at `to`. */
ADDRESS(0x8004978c, 0x68)
int func_8004978c(int v, int to, int step)
{
    if (step > 0) {
        if (v < to) {
            v += step;
            if (v > to)
                v = to;
        } else if (v > to) {
            v -= step;
            if (v < to)
                v = to;
        }
    } else {
        if (v > to) {
            v += step;
            if (v < to)
                v = to;
        } else if (v < to) {
            v -= step;
            if (v > to)
                v = to;
        }
    }
    return v;
}

ADDRESS(0x800497f4, 0xd8)
int func_800497f4(int angle, int heading, int speed, int accel, int decay)
{
    if (func_8004a148((short)angle, (short)heading, (short)(accel >> 1)) && speed <= accel &&
        speed >= -accel)
        return angle - heading;
    if (heading != angle) {
        if (func_8004a184((short)heading, (short)angle))
            speed -= accel;
        else
            speed += accel;
    }
    if (speed > 0)
        speed -= decay;
    else if (speed < 0)
        speed += decay;
    return speed;
}

/* The four quarter turns of `m` about y, into out[0..3]. */
ADDRESS(0x800498cc, 0x1e0)
void func_800498cc(MATRIX *m, MATRIX *out)
{
    out->m[0][0] = m->m[0][0];
    out->m[0][1] = m->m[0][1];
    out->m[0][2] = m->m[0][2];
    out->m[1][0] = m->m[1][0];
    out->m[1][1] = m->m[1][1];
    out->m[1][2] = m->m[1][2];
    out->m[2][0] = m->m[2][0];
    out->m[2][1] = m->m[2][1];
    out->m[2][2] = m->m[2][2];
    out++;
    out->m[0][0] = m->m[0][2];
    out->m[0][1] = m->m[0][1];
    out->m[0][2] = -m->m[0][0];
    out->m[1][0] = m->m[1][2];
    out->m[1][1] = m->m[1][1];
    out->m[1][2] = -m->m[1][0];
    out->m[2][0] = m->m[2][2];
    out->m[2][1] = m->m[2][1];
    out->m[2][2] = -m->m[2][0];
    out++;
    out->m[0][0] = -m->m[0][0];
    out->m[0][1] = m->m[0][1];
    out->m[0][2] = -m->m[0][2];
    out->m[1][0] = -m->m[1][0];
    out->m[1][1] = m->m[1][1];
    out->m[1][2] = -m->m[1][2];
    out->m[2][0] = -m->m[2][0];
    out->m[2][1] = m->m[2][1];
    out->m[2][2] = -m->m[2][2];
    out++;
    out->m[0][0] = -m->m[0][2];
    out->m[0][1] = m->m[0][1];
    out->m[0][2] = m->m[0][0];
    out->m[1][0] = -m->m[1][2];
    out->m[1][1] = m->m[1][1];
    out->m[1][2] = m->m[1][0];
    out->m[2][0] = -m->m[2][2];
    out->m[2][1] = m->m[2][1];
    out->m[2][2] = m->m[2][0];
}

/* Light matrix 0 of `light` turned by `index` quarter turns about y. */
ADDRESS(0x80049aac, 0x228)
void func_80049aac(StLight *light, StLightMatrix *out, int index)
{
    StLightMatrix *m = &light->light[0];

    switch (index) {
    case 0:
        out->m[0][0] = m->m[0][0];
        out->m[0][1] = m->m[0][1];
        out->m[0][2] = m->m[0][2];
        out->m[1][0] = m->m[1][0];
        out->m[1][1] = m->m[1][1];
        out->m[1][2] = m->m[1][2];
        out->m[2][0] = m->m[2][0];
        out->m[2][1] = m->m[2][1];
        out->m[2][2] = m->m[2][2];
        break;
    case 1:
        out->m[0][0] = m->m[0][2];
        out->m[0][1] = m->m[0][1];
        out->m[0][2] = -m->m[0][0];
        out->m[1][0] = m->m[1][2];
        out->m[1][1] = m->m[1][1];
        out->m[1][2] = -m->m[1][0];
        out->m[2][0] = m->m[2][2];
        out->m[2][1] = m->m[2][1];
        out->m[2][2] = -m->m[2][0];
        break;
    case 2:
        out->m[0][0] = -m->m[0][0];
        out->m[0][1] = m->m[0][1];
        out->m[0][2] = -m->m[0][2];
        out->m[1][0] = -m->m[1][0];
        out->m[1][1] = m->m[1][1];
        out->m[1][2] = -m->m[1][2];
        out->m[2][0] = -m->m[2][0];
        out->m[2][1] = m->m[2][1];
        out->m[2][2] = -m->m[2][2];
        break;
    case 3:
        out->m[0][0] = -m->m[0][2];
        out->m[0][1] = m->m[0][1];
        out->m[0][2] = m->m[0][0];
        out->m[1][0] = -m->m[1][2];
        out->m[1][1] = m->m[1][1];
        out->m[1][2] = m->m[1][0];
        out->m[2][0] = -m->m[2][2];
        out->m[2][1] = m->m[2][1];
        out->m[2][2] = m->m[2][0];
        break;
    }
}

/* Rotation about x. */
ADDRESS(0x80049cd4, 0x74)
void func_80049cd4(short angle, MATRIX *m)
{
    int s = rsin(angle);
    int c = rcos(angle);

    m->m[0][0] = 4096;
    m->m[0][1] = 0;
    m->m[0][2] = 0;
    m->m[1][0] = 0;
    m->m[1][1] = c;
    m->m[1][2] = -s;
    m->m[2][0] = 0;
    m->m[2][1] = s;
    m->m[2][2] = c;
}

/* Rotation about y. */
ADDRESS(0x80049d48, 0x74)
void func_80049d48(short angle, MATRIX *m)
{
    int s = rsin(angle);
    int c = rcos(angle);

    m->m[0][0] = c;
    m->m[0][1] = 0;
    m->m[0][2] = -s;
    m->m[1][0] = 0;
    m->m[1][1] = 4096;
    m->m[1][2] = 0;
    m->m[2][0] = s;
    m->m[2][1] = 0;
    m->m[2][2] = c;
}

/* Rotation about z. */
ADDRESS(0x80049dbc, 0x74)
void func_80049dbc(short angle, MATRIX *m)
{
    int s = rsin(angle);
    int c = rcos(angle);

    m->m[0][0] = c;
    m->m[0][1] = -s;
    m->m[0][2] = 0;
    m->m[1][0] = s;
    m->m[1][1] = c;
    m->m[1][2] = 0;
    m->m[2][0] = 0;
    m->m[2][1] = 0;
    m->m[2][2] = 4096;
}

/* m = Ry * Rx * Rz. */
ADDRESS(0x80049e30, 0x6c)
void func_80049e30(SVECTOR *rot, MATRIX *m)
{
    MATRIX t;

    func_80049dbc(rot->vz, &t);
    func_80049cd4(rot->vx, m);
    MulMatrix(m, &t);
    func_80049d48(rot->vy, &t);
    MulMatrix2(&t, m);
}

/* m = Rx * Rz * Ry. */
ADDRESS(0x80049e9c, 0x6c)
void func_80049e9c(SVECTOR *rot, MATRIX *m)
{
    MATRIX t;

    func_80049d48(rot->vy, &t);
    func_80049dbc(rot->vz, m);
    MulMatrix(m, &t);
    func_80049cd4(rot->vx, &t);
    MulMatrix2(&t, m);
}

/* The unit z vector turned by pitch rot->vx and yaw rot->vy. */
ADDRESS(0x80049f08, 0xdc)
void func_80049f08(SVECTOR *rot, SVECTOR *out)
{
    MATRIX mx;
    MATRIX my;
    SVECTOR v;
    VECTOR r;

    v.vx = 0;
    v.vy = 0;
    v.vz = 4096;
    func_80049cd4(-rot->vx, &mx);
    ApplyMatrix(&mx, &v, &r);
    v.vx = r.vx;
    v.vy = r.vy;
    v.vz = r.vz;
    func_80049d48(rot->vy, &my);
    ApplyMatrix(&my, &v, &r);
    out->vx = r.vx;
    out->vy = r.vy;
    out->vz = r.vz;
}

ADDRESS(0x80049fe4, 0x94)
void func_80049fe4(SVECTOR *rot, SVECTOR *out)
{
    MATRIX m;
    SVECTOR r;
    SVECTOR v;

    r = *rot;
    v = DAT_8008809c;
    r.vy = -r.vy;
    RotMatrixZYX_gte(&r, &m);
    ApplyMatrixSV(&m, &v, out);
}

/* v turned by rot (func_80049e30) into out. */
ADDRESS(0x8004a078, 0x48)
void func_8004a078(SVECTOR *rot, SVECTOR *v, VECTOR *out)
{
    MATRIX m;

    func_80049e30(rot, &m);
    ApplyMatrix(&m, v, out);
}

ADDRESS(0x8004a0c0, 0x38)
void func_8004a0c0(short scale, long *v)
{
    v[0] = v[0] * scale >> 11;
    v[1] = v[1] * scale >> 11;
}

/* Scales v by scale / 4096. */
ADDRESS(0x8004a0f8, 0x50)
void func_8004a0f8(scale, v)
short scale;
SVECTOR *v;
{
    v->vx = v->vx * scale >> 12;
    v->vy = v->vy * scale >> 12;
    v->vz = v->vz * scale >> 12;
}

/* Whether `angle` is within `range` of `heading`. */
ADDRESS(0x8004a148, 0x3c)
int func_8004a148(angle, heading, range)
int angle;
int heading;
short range;
{
    int d = (angle - heading) & 0xfff;
    int result = 0;

    if (d <= range || d >= 4096 - range)
        result = 1;
    return result;
}

ADDRESS(0x8004a184, 0x10)
int func_8004a184(int a, int b)
{
    return ((a - b) & 0xfff) <= 2048;
}

ADDRESS(0x8004a194, 0x20)
int func_8004a194(int a, int b)
{
    return (short)(((a + 2048) & 0xfff) - ((b + 2048) & 0xfff)) > 0;
}

/* The heading of (dx, dz). */
ADDRESS(0x8004a1b4, 0x150)
int func_8004a1b4(int dx, int dz)
{
    int adx = abs(dx);
    int adz = abs(dz);

    if (adz >= adx) {
        if (dz > 0)
            return -catan((dx << 12) / dz) & 0xfff;
        if (dz < 0)
            return 2048 - catan((dx << 12) / dz);
        return 0;
    }
    if (dx >= 0)
        return catan((dz << 12) / dx) + 3072;
    else
        return catan((dz << 12) / dx) + 1024;
}

/* The length of (dx, dz). */
ADDRESS(0x8004a304, 0x3c)
int func_8004a304(int dx, int dz)
{
    dx >>= 3;
    dz >>= 3;
    return SquareRoot0(dx * dx + dz * dz) << 3;
}

/* The length of (x, y, z). */
ADDRESS(0x8004a340, 0x50)
int func_8004a340(int x, int y, int z)
{
    x >>= 3;
    y >>= 3;
    z >>= 3;
    return SquareRoot0(x * x + y * y + z * z) << 3;
}

/* The pitch and yaw of (dx, dy, dz). */
ADDRESS(0x8004a390, 0x7c)
void func_8004a390(int dx, int dy, int dz, SVECTOR *out)
{
    out->vy = func_8004a1b4(dx, dz);
    out->vx = -func_8004a1b4(dy, func_8004a304(dx, dz)) & 0xfff;
    out->vz = 0;
}

ADDRESS(0x8004a40c, 0x78)
void func_8004a40c(int dx, int dy, int dz, SVECTOR *out)
{
    out->vy = -ratan2(dx, dz);
    out->vx = ratan2(-dy, func_8004a304(dx, dz));
    out->vz = 0;
}

/* The horizontal distance from (x, z) to *p when it is within `range`
 * and y (unless 0xffff) is within `below`/`above` of p->vy, else -1. */
ADDRESS(0x8004a484, 0xfc)
int func_8004a484(VECTOR *p, int x, int y, int z, int range, int above, int below)
{
    int dx, dz;
    int d;

    dx = p->vx - x;
    if (dx < -range || dx > range)
        return -1;
    dz = p->vz - z;
    if (dz < -range || dz > range)
        return -1;
    if (y != 0xffff) {
        if (p->vy < y) {
            if (p->vy < y - below)
                return -1;
        } else if (y < p->vy - above)
            return -1;
    }
    dx >>= 3;
    dz >>= 3;
    d = SquareRoot0(dx * dx + dz * dz) << 3;
    if (d <= range)
        return d;
    return -1;
}

ADDRESS(0x8004a580, 0xfc)
int func_8004a580(VECTOR *p, int x, int y, int z, int range, int above, int below)
{
    int dx, dz;
    int d;

    dx = p->vx - x;
    if (dx < -range || dx > range)
        return -1;
    dz = p->vz - z;
    if (dz < -range || dz > range)
        return -1;
    if (y != 0xffff) {
        if (p->vy < y) {
            if (p->vy + above < y)
                return -1;
        } else if (y + below < p->vy)
            return -1;
    }
    dx >>= 3;
    dz >>= 3;
    d = SquareRoot0(dx * dx + dz * dz) << 3;
    if (d <= range)
        return d;
    return -1;
}

/* How far apart the spheres (a, ra) and (b, rb) are (b raised by half of
 * `height`): -9999999 when beyond ra + rb, 0 when b's centre is inside
 * rb. */
ADDRESS(0x8004a67c, 0x11c)
int func_8004a67c(VECTOR *a, int ra, VECTOR *b, int rb, int height)
{
    int r = ra + rb;
    int dx, dy, dz;
    int d;

    dx = b->vx - a->vx;
    if (dx < -r || dx > r)
        return -9999999;
    dz = b->vz - a->vz;
    if (dz < -r || dz > r)
        return -9999999;
    dy = b->vy - a->vy - (height >> 1);
    if (dy < -r || dy > r)
        return -9999999;
    dx >>= 3;
    dz >>= 3;
    dy >>= 3;
    d = SquareRoot0(dx * dx + dz * dz + dy * dy) << 3;
    if (d <= r) {
        if (d > rb)
            return d - rb;
        return 0;
    }
    return -9999999;
}

/* A random value in 0..n. */
ADDRESS(0x8004a798, 0x48)
int func_8004a798(int n)
{
    return (rand() + rand()) * n >> 15;
}

/* A random value in -n..n. */
ADDRESS(0x8004a7e0, 0x50)
int func_8004a7e0(int n)
{
    return (rand() + rand() - 32768) * n >> 15;
}

/* from + (to - from) * t / 4096. */
ADDRESS(0x8004a830, 0x18)
int func_8004a830(int from, int to, int t)
{
    return ((to - from) * t >> 12) + from;
}

/* The angle t / 4096 of the shorter way from `from` to `to`. */
ADDRESS(0x8004a848, 0x40)
int func_8004a848(int from, short to, int t)
{
    int d = (to - (short)from) & 0xfff;

    if (d > 2048)
        d -= 4096;
    return (from + (d * t >> 12)) & 0xfff;
}

/* out = a + (b - a) * t / 4096 for each element. */
ADDRESS(0x8004a888, 0x60)
void func_8004a888(StLightMatrix *a, StLightMatrix *b, StLightMatrix *out, short t)
{
    short *pa = &a->m[0][0];
    short *pb = &b->m[0][0];
    short *po = &out->m[0][0];
    int n = 9;

    while (n--) {
        short va = *pa++;
        short vb = *pb++;

        *po++ = va + ((vb - va) * t >> 12);
    }
}

/* out = p + v * s. */
ADDRESS(0x8004a8e8, 0x58)
void func_8004a8e8(VECTOR *p, SVECTOR *v, int s, VECTOR *out)
{
    out->vx = p->vx + v->vx * s;
    out->vy = p->vy + v->vy * s;
    out->vz = p->vz + v->vz * s;
}

/* The distance from p to the segment a..b, or -1 when p is not beside it. */
ADDRESS(0x8004a940, 0x17c)
int func_8004a940(SVECTOR *p, SVECTOR *a, SVECTOR *b)
{
    int t;
    int x, y, z;
    int result = -1;

    t = (((p->vx - a->vx) * (b->vx - a->vx) + (p->vy - a->vy) * (b->vy - a->vy) +
          (p->vz - a->vz) * (b->vz - a->vz))
         << 8) /
        ((b->vx - a->vx) * (b->vx - a->vx) + (b->vy - a->vy) * (b->vy - a->vy) +
         (b->vz - a->vz) * (b->vz - a->vz));
    if ((u_int)t <= 256) {
        x = (b->vx - a->vx) * t / 256 + a->vx;
        y = (b->vy - a->vy) * t / 256 + a->vy;
        z = (b->vz - a->vz) * t / 256 + a->vz;
        result = SquareRoot0((x - p->vx) * (x - p->vx) + (y - p->vy) * (y - p->vy) +
                             (z - p->vz) * (z - p->vz));
    }
    return result;
}

ADDRESS(0x8004aabc, 0x17c)
int func_8004aabc(VECTOR *p, VECTOR *a, VECTOR *b)
{
    int t;
    int x, y, z;
    int result = -1;

    t = (((p->vx - a->vx) * (b->vx - a->vx) + (p->vy - a->vy) * (b->vy - a->vy) +
          (p->vz - a->vz) * (b->vz - a->vz))
         << 8) /
        ((b->vx - a->vx) * (b->vx - a->vx) + (b->vy - a->vy) * (b->vy - a->vy) +
         (b->vz - a->vz) * (b->vz - a->vz));
    if ((u_int)t <= 256) {
        x = (b->vx - a->vx) * t / 256 + a->vx;
        y = (b->vy - a->vy) * t / 256 + a->vy;
        z = (b->vz - a->vz) * t / 256 + a->vz;
        result = SquareRoot0((x - p->vx) * (x - p->vx) + (y - p->vy) * (y - p->vy) +
                             (z - p->vz) * (z - p->vz));
    }
    return result;
}
