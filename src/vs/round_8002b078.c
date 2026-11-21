/*
 * VS.EXE: the round's opening sequence (WIP module, -G8).
 *
 * Parked: no unit claims this file (config/units.toml) so VS.EXE stays a
 * partial image. Claiming 0x8002b078 completes VS's function coverage, and
 * kf build then runs the native PSYLINK link, which fails until the VS data
 * globals have defining objects. Built locally as unit vs.round_8002b078
 * (profile probe-gcc272cdk-o2-g8, between vs.name_8002ad40 and
 * vs.shadow_8002bfe8) the function and its constant pool match retail.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/pad.h>

/* The ring's fade by column and the sixteen ring directions. */
RODATA(0x8001147c, 0x90)

void func_8002a360(int kind, int level);                 /* src/vs/hud_80029f28.c */
void func_8002a124(int value, int level);
void func_8002a9c0(int kind, int level, int size);
void func_80029b10(int level);                           /* src/vs/gauge_80029b10.c */
void func_80029f28(int phase, int scale);
void func_8002ad40(int level);                           /* src/vs/name_8002ad40.c */
void func_80025204(void);                                /* src/vs/model_80024b5c.c */
void func_80028914(void);                                /* src/vs/model_80028914.c */
void func_8002bfe8(int n, int scale);                    /* src/vs/shadow_8002bfe8.c */

/* x, or 0 when it is negative. */
#define FLOOR0(x) ((x) < 0 ? 0 : (x))

/* A ring radius by the fighter's hit radius. */
#define RADIUS(size, a, b, c, d)                                               \
    if ((size) < 400)                                                          \
        r = a;                                                                 \
    else if ((size) < 900)                                                     \
        r = b;                                                                 \
    else if ((size) < 1400)                                                    \
        r = c;                                                                 \
    else                                                                       \
        r = d

/* The fighter's place seen from the camera, unrotated. */
#define PLACE(m, f)                                                            \
    (m).t[0] = (f).unk4DEF0.vx - DAT_801624d0.unk2CA78.vx;                    \
    (m).t[1] = -DAT_801624d0.unk2CA78.vy;                                      \
    (m).t[2] = (f).unk4DEF0.vz - DAT_801624d0.unk2CA78.vz;                    \
    (m).m[0][0] = 4096;                                                        \
    (m).m[0][1] = 0;                                                           \
    (m).m[0][2] = 0;                                                           \
    (m).m[1][0] = 0;                                                           \
    (m).m[1][1] = 4096;                                                        \
    (m).m[1][2] = 0;                                                           \
    (m).m[2][0] = 0;                                                           \
    (m).m[2][1] = 0;                                                           \
    (m).m[2][2] = 4096;                                                        \
    CompMatrix(&DAT_801624d0.unk2C998, &(m), &(m));                            \
    gte_SetRotMatrix(&(m));                                                    \
    gte_SetTransMatrix(&(m))

/* An empty textured triangle at OT depth z that sets the semi-transparency
 * mode (tpage 56) for the ring quad before it. */
#define RING_MODE(z)                                                           \
    {                                                                          \
        t3 = (POLY_FT3 *)DAT_801624d0.packet->cur;                             \
        DAT_801624d0.packet->cur = (u_long *)(t3 + 1);                         \
        SetPolyFT3(t3);                                                        \
        if (i != 15)                                                           \
            z = (u_long)(v[i + 16].vz + v[i + 17].vz + v[i].vz + v[i + 1].vz) >> 7; \
        else                                                                   \
            z = (u_long)(v[31].vz + v[16].vz + v[15].vz + v[0].vz) >> 7;       \
        setXY3(t3, 0, 0, 0, 0, 0, 0);                                          \
        setUV3(t3, 0, 0, 0, 0, 0, 0);                                          \
        t3->clut = 32641;                                                      \
        t3->tpage = 56;                                                        \
        setRGB0(t3, 0, 0, 0);                                                  \
        addPrim(DAT_801624d0.ot_cur + z, t3);                                  \
    }

/*
 * Draws frame `frame` of the round's opening: the banners and gauges
 * fading in, a ring of light opening around each fighter (frames 20..76),
 * a glow on the floor under each (to frame 99) and, from frame 46, the
 * fighters themselves. Returns 1 once the sequence is over.
 */
ADDRESS(0x8002b078, 0xf70)
int func_8002b078(int frame)
{
    SVECTOR v[32];
    u8 fade[16] = { 100, 50, 20, 10, 0, 10, 20, 50, 100, 50, 20, 10, 0, 10, 20, 50 };
    CVECTOR col[4];
    MATRIX m;
    CVECTOR ring[2];
    int sins[16] = { 0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
                     0, -1567, -2896, -3784, -4096, -3784, -2896, -1567 };
    int coss[16] = { 4096, 3784, 2896, 1567, 0, -1567, -2896, -3784,
                     -4096, -3784, -2896, -1567, 0, 1567, 2896, 3784 };
    long sz;
    int n;
    int i;
    int r;
    int open;
    int c;
    short z;
    POLY_G4 *p;
    POLY_FT3 *t3;
    POLY_FT4 *q;

    if (frame > 120)
        return 1;
    if (frame > 90) {
        func_8002a360(7, (106 - frame) * 2);
        func_8002a360(3, (frame - 100) * 2);
        func_8002a124(30, 32);
        func_80029b10(32);
        func_80029f28((u32)DAT_801603c0.unk3C % 96, 32);
        func_8002ad40(32);
    } else if (frame > 70) {
        func_8002a360(7, (frame - 70) * 2);
        func_8002a124(30, frame - 70);
        func_80029b10((frame - 70) * 2);
        func_80029f28((u32)DAT_801603c0.unk3C % 96, (frame - 70) * 2);
        func_8002ad40(frame - 70);
    } else if (frame > 40) {
        func_8002a9c0(DAT_80063338 + 16, (48 - frame) * 2, 50 - frame);
    } else {
        func_8002a9c0(DAT_80063338 + 16, (frame - 10) * 2, 50 - frame);
    }
    if (frame >= 20 && frame <= 76) {
        if (frame < 45) {
            open = (frame - 20) * 6;
            ring[0].r = ring[0].g = ring[0].b = 255;
            ring[1].r = ring[1].g = ring[1].b = 255;
        } else if (frame < 61) {
            open = 150;
            ring[0].r = ring[0].g = ring[0].b = FLOOR0(255 - (frame - 45) * 16);
            ring[1].r = ring[1].g = ring[1].b = 255;
        } else {
            open = 150;
            ring[0].r = ring[0].g = ring[0].b = 0;
            ring[1].r = ring[1].g = ring[1].b = FLOOR0(255 - (frame - 61) * 16);
        }
        for (n = 0; n < 2; n++) {
            RADIUS(DAT_800633b0.fighter[n].unk4DF18.unk1B4, 500, 1000, 1500, 2000);
            for (i = 0; i < 16; i++) {
                v[i].vx = r * coss[i] * open / 409600;
                v[i].vy = 0;
                v[i].vz = r * sins[i] * open / 409600;
                v[i + 16].vx = r * coss[i] * open / 409600;
                v[i + 16].vy = DAT_801624d0.unk2CA78.vy;
                v[i + 16].vz = r * sins[i] * open / 409600;
            }
            PLACE(m, DAT_800633b0.fighter[n]);
            for (i = 0; i < 30; i += 3) {
                gte_ldv3c(&v[i]);
                gte_rtpt();
                gte_stsxy3(&v[i], &v[i + 1], &v[i + 2]);
                gte_stsz3(&v[i].vz, &v[i + 1].vz, &v[i + 2].vz);
            }
            gte_ldv01c(&v[30]);
            gte_rtpt();
            gte_stsxy01(&v[30], &v[31]);
            gte_stsz3(&v[30].vz, &v[31].vz, &sz);
            for (i = 0; i < 16; i++) {
                p = (POLY_G4 *)DAT_801624d0.packet->cur;
                DAT_801624d0.packet->cur = (u_long *)(p + 1);
                SetPolyG4(p);
                if (frame > 45)
                    setSemiTrans(p, 1);
                if (i < 15) {
                    p->x0 = v[i + 16].vx;
                    p->y0 = v[i + 16].vy;
                    p->x1 = v[i + 17].vx;
                    p->y1 = v[i + 17].vy;
                    p->x2 = v[i].vx;
                    p->y2 = v[i].vy;
                    p->x3 = v[i + 1].vx;
                    p->y3 = v[i + 1].vy;
                    z = (u_long)(v[i + 16].vz + v[i + 17].vz + v[i].vz + v[i + 1].vz) >> 7;
                    col[0].b = FLOOR0(ring[0].b - fade[i]);
                    col[1].b = FLOOR0(ring[0].b - fade[i + 1]);
                    col[2].b = FLOOR0(ring[1].b - fade[i]);
                    col[3].b = FLOOR0(ring[1].b - fade[i + 1]);
                } else {
                    p->x0 = v[31].vx;
                    p->y0 = v[31].vy;
                    p->x1 = v[16].vx;
                    p->y1 = v[16].vy;
                    p->x2 = v[15].vx;
                    p->y2 = v[15].vy;
                    p->x3 = v[0].vx;
                    p->y3 = v[0].vy;
                    z = (u_long)(v[31].vz + v[16].vz + v[15].vz + v[0].vz) >> 7;
                    col[0].b = FLOOR0(ring[0].b - fade[i]);
                    col[1].b = FLOOR0(ring[0].b - fade[0]);
                    col[2].b = FLOOR0(ring[1].b - fade[i]);
                    col[3].b = FLOOR0(ring[1].b - fade[0]);
                }
                setRGB0(p, ring[1].r, ring[1].g, col[2].b);
                setRGB1(p, ring[1].r, ring[1].g, col[3].b);
                setRGB2(p, ring[0].r, ring[0].g, col[0].b);
                setRGB3(p, ring[0].r, ring[0].g, col[1].b);
                addPrim(DAT_801624d0.ot_cur + z, p);
                RING_MODE(z);
            }
        }
    }
    if (frame < 100) {
        for (i = 0; i < 2; i++) {
            RADIUS(DAT_800633b0.fighter[i].unk4DF18.unk1B4, 625, 1250, 1875, 2500);
            v[i].vx = -r;
            v[i].vy = 0;
            v[i].vz = -r;
            v[i + 1].vx = r;
            v[i + 1].vy = 0;
            v[i + 1].vz = -r;
            v[i + 2].vx = -r;
            v[i + 2].vy = 0;
            v[i + 2].vz = r;
            v[i + 3].vx = r;
            v[i + 3].vy = 0;
            v[i + 3].vz = r;
            PLACE(m, DAT_800633b0.fighter[i]);
            gte_ldv3c(&v[i]);
            gte_rtpt();
            gte_stsxy3(&v[i], &v[i + 1], &v[i + 2]);
            gte_ldv0(&v[i + 3]);
            gte_rtps();
            gte_stsxy(&v[i + 3]);
            q = (POLY_FT4 *)DAT_801624d0.packet->cur;
            DAT_801624d0.packet->cur = (u_long *)(q + 1);
            setPolyFT4(q);
            setSemiTrans(q, 1);
            setXY4(q, v[i].vx, v[i].vy, v[i + 1].vx, v[i + 1].vy,
                   v[i + 2].vx, v[i + 2].vy, v[i + 3].vx, v[i + 3].vy);
            setUV4(q, 4, 128, 127, 128, 4, 253, 127, 253);
            q->clut = 30752;
            q->tpage = 61;
            if (frame > 45) {
                if (frame > 77) {
                    setRGB0(q, 0, 0, 0);
                } else {
                    c = frame < 46 ? 255 : (77 - frame) * 8;
                    setRGB0(q, c, c, c);
                }
            } else {
                c = frame * 4;
                setRGB0(q, c, c, c);
            }
            addPrim(DAT_801624d0.ot_cur + 4091, q);
        }
    }
    if (frame > 45) {
        for (i = 0; i < 2; i++) {
            DAT_80063300 = &DAT_800633b0.fighter[i];
            switch (DAT_800633b0.fighter[i].unk4DF10) {
            case 1:
                func_80025204();
                break;
            case 2:
                DAT_80063360 = 0;
                func_80028914();
                break;
            }
            DAT_80063300->unk4DEEA =
                (DAT_80063300->unk4DEEA + DAT_80063300->unk4DEEC * DAT_80063220) & 0xfff;
            func_8002bfe8(i, 4096);
        }
    }
    return 0;
}
