/*
 * The round's overlays and HUD: the full-screen
 * overlay and lit model parts, the round clock, the fighters' gauges, the
 * round-win markers, timer, message banners and screen overlays, the
 * fighters' names and the round's opening sequence. One translation unit:
 * retail lays out its constants (0x8001142c, the overlay's initializers
 * and the opening's tables) before its switch tables (0x8001150c,
 * func_80028fcc's and func_8002a360's), the order cc1 writes one object's
 * read-only data in. -G8: the gauges' blink toggle and the overlay's
 * texture step are $gp-relative.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <GTEMAC.H>
#include <st/mesh.h>
#include <vs/data.h>
#include <vs/scratch.h>
#include <vs/buffer.h>
#include <vs/menu.h>
#include <vs/pad.h>

/* The module's constants, then its switch tables. */
RODATA(0x8001142c, 0x148)

/*
 * A full-screen semi-transparent overlay.
 */
/* src/vs/mesh_800258f0.c's part renderer, called here without a
 * prototype: retail passes the int part number unconverted. */
void mesh_draw_part_nccs();

/* func_800291d4's default NCCS colour (128, 128, 128). */
SDATA(0x800632dc, 0x4)
u_long DAT_800632dc = 0x808080;

/* The overlay's texture quarter, stepped 0..5 by each frame that asks. */
DATA(0x800632e0, 0x4, ".sdata")
static s32 DAT_800632e0 = 0;

/*
 * Adds a 320x240 semi-transparent textured quad at the back of the frame's
 * ordering table, its texture window chosen by DAT_800632e0; advances that
 * when `hold` is 0.
 */
ADDRESS(0x80028fcc, 0x208)
void func_80028fcc(int hold)
{
    StPacketBuffer *packet = vs_gfx_state.packet;
    POLY_FT4 *p = (POLY_FT4 *)packet->cur;

    packet->cur = (u_long *)(p + 1);
    SetPolyFT4(p);
    setXY4(p, 0, 0, 320, 0, 0, 240, 320, 240);
    setSemiTrans(p, 1);
    switch (DAT_800632e0) {
    case 0:
    case 1:
        setUV4(p, 0, 0, 127, 0, 0, 127, 127, 127);
        p->clut = 0x7f10;
        p->tpage = 142;
        break;
    case 2:
    case 3:
        setUV4(p, 0, 0, 127, 0, 0, 127, 127, 127);
        p->clut = 0x7f10;
        p->tpage = 143;
        break;
    case 4:
    case 5:
        setUV4(p, 0, 128, 127, 128, 0, 255, 127, 255);
        p->clut = 0x7f10;
        p->tpage = 142;
        break;
    default:
        setUV4(p, 128, 0, 255, 0, 128, 127, 255, 127);
        break;
    }
    setRGB0(p, 100, 100, 100);
    addPrim(&vs_gfx_state.ot_cur[VS_OT_LENGTH - 1], p);
    if (hold == 0) {
        if (++DAT_800632e0 >= 6)
            DAT_800632e0 = 0;
    }
}

/* Projects the current part's vertices: RTPT in threes, the first group
 * ahead of the loop so each pass stores one group and starts the next, then
 * RTPS for the rest. */
#define PROJECT(sp, out, in, n)                                                \
    {                                                                          \
        out = (MeshVertex *)(sp)->vertices;                                    \
        in = (SVECTOR *)(sp)->local_vertices;                                  \
        n = (sp)->header->n_vertices / 3;                                      \
        if (n--) {                                                             \
            gte_ldv3c(in);                                                     \
            gte_rtpt();                                                        \
            in += 3;                                                           \
            for (;;) {                                                         \
                gte_stsxy3(&out[0].sxy, &out[1].sxy, &out[2].sxy);             \
                gte_stsz3(&out[0].z, &out[1].z, &out[2].z);                    \
                out += 3;                                                      \
                if (!n--)                                                      \
                    break;                                                     \
                gte_ldv3c(in);                                                 \
                in += 3;                                                       \
                gte_rtpt();                                                    \
            }                                                                  \
        }                                                                      \
        n = (sp)->header->n_vertices % 3;                                      \
        while (n--) {                                                          \
            gte_ldv0(in);                                                      \
            gte_rtps();                                                        \
            in++;                                                              \
            gte_stsxy(&out->sxy);                                              \
            gte_stsz(&out->z);                                                 \
            out++;                                                             \
        }                                                                      \
    }

/*
 * Draws part `part` of model `model` at camera-relative position `pos`
 * with rotation `rot` scaled by `scale` (4096ths), lit like the fighters.
 */
ADDRESS(0x800291d4, 0x628)
void func_800291d4(u8 *model, int part, SVECTOR *pos, SVECTOR *rot, SVECTOR *scale)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    VsLights lights = {
        {{3000, -3700, -1500}, {-2000, -3700, -3000}, {0, 1000, 0}}, 0,
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0,
        {200, 200, 200, 0}, {-1, -1}};
    MATRIX m = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    MATRIX unused = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    MeshVertex *out;
    SVECTOR *in;
    int n;

    ScaleMatrix(&m, &DAT_8005b2a8);
    RotMatrixX(DAT_80063218, &m);
    RotMatrixY(DAT_8006321c, &m);
    MulMatrix((MATRIX *)&lights.l, &m);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    sp->unk68 = lights.dq[0];
    sp->unk6C = lights.dq[1];
    gte_SetBackColor(lights.bk.r, lights.bk.g, lights.bk.b);
    sp->matrix.t[0] = pos->vx - vs_gfx_state.unk2CA78.vx;
    sp->matrix.t[1] = pos->vy - vs_gfx_state.unk2CA78.vy;
    sp->matrix.t[2] = pos->vz - vs_gfx_state.unk2CA78.vz;
    RotMatrixZYX(rot, &sp->matrix);
    sp->matrix.m[0][0] *= scale->vx;
    sp->matrix.m[0][1] *= scale->vx;
    sp->matrix.m[0][2] *= scale->vx;
    sp->matrix.m[1][0] *= scale->vy;
    sp->matrix.m[1][1] *= scale->vy;
    sp->matrix.m[1][2] *= scale->vy;
    sp->matrix.m[2][0] *= scale->vz;
    sp->matrix.m[2][1] *= scale->vz;
    sp->matrix.m[2][2] *= scale->vz;
    CompMatrix(&vs_gfx_state.unk2C998, &sp->matrix, &sp->matrix);
    gte_SetRotMatrix(&sp->matrix);
    gte_SetTransMatrix(&sp->matrix);
    sp->far_z = 4096;
    sp->ot = vs_gfx_state.ot_cur;
    sp->near_z = 4;
    sp->face_count = vs_gfx_state.unk831C[1];
    sp->prim_count = vs_gfx_state.unk8324;
    sp->prim = vs_gfx_state.packet->cur;
    sp->prim_end = vs_gfx_state.packet->end;
    sp->vertices = vs_gfx_state.unkA7EC;
    sp->clut_base = 0;
    sp->model = model;
    sp->header = (MeshHeader *)(model + 12) + part;
    sp->local_vertices = model + (sp->header->vertices + 12);
    sp->normals = model + (sp->header->normals + 12);
    sp->faces = model + (sp->header->faces + 12);
    *(u_long *)&sp->rgb = DAT_800632dc;
    PROJECT(sp, out, in, n);
    DAT_80063360 = 0;
    if (DAT_80063224 != 3)
        DAT_80063350 = 0;
    mesh_draw_part_nccs(part);
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.unk831C[1] = sp->face_count;
    vs_gfx_state.unk8324 = sp->prim_count;
}

/*
 * The round clock.
 */
/*
 * Draws the seconds left of `time` (frames, rounded up) as two digits
 * cross-fading from the previous second's digits over the first 16 frames
 * of each second, reddening over the last 14 seconds.
 */
ADDRESS(0x800297fc, 0x314)
void func_800297fc(int time)
{
    int frame;
    int warn;
    int i;
    int digit;
    StPacketBuffer *packet;
    POLY_FT4 *p;

    time += 59;
    frame = time % 60;
    time /= 60;
    warn = (14 - time) * 2;
    if (warn < 0)
        warn = 0;
    if (warn > 8)
        warn = 8;
    for (i = 0; i < 4; i++) {
        packet = vs_gfx_state.packet;
        p = (POLY_FT4 *)packet->cur;
        packet->cur = (u_long *)(p + 1);
        setPolyFT4(p);
        setSemiTrans(p, 1);
        setXY4(p, i % 2 * 32 + 128, 16, i % 2 * 32 + 160, 16,
               i % 2 * 32 + 128, 48, i % 2 * 32 + 160, 48);
        switch (i) {
        case 0:
            digit = time / 10;
            break;
        case 1:
            digit = time % 10;
            break;
        case 2:
            digit = (time - 1) / 10;
            break;
        case 3:
            digit = (time - 1) % 10;
            break;
        }
        if (digit < 5) {
            setUV4(p, digit * 32, 0, digit * 32 + 32, 0,
                   digit * 32, 32, digit * 32 + 32, 32);
        } else {
            digit -= 5;
            setUV4(p, digit * 32, 32, digit * 32 + 32, 32,
                   digit * 32, 64, digit * 32 + 32, 64);
        }
        p->clut = 32290;
        p->tpage = 61;
        if (i < 2) {
            if (frame >= 16) {
                setRGB0(p, warn * 16 + 127, -128 - warn * 16, -128 - warn * 16);
            } else {
                setRGB0(p, frame * (warn + 8), frame * (8 - warn), frame * (8 - warn));
            }
        } else if (frame >= 16) {
            setRGB0(p, 0, 0, 0);
        } else {
            setRGB0(p, warn * 16 + 127 - frame * (warn + 8),
                    -128 - warn * 16 - frame * (8 - warn),
                    -128 - warn * 16 - frame * (8 - warn));
        }
        addPrim(&vs_gfx_state.ot_cur[3], p);
    }
}

/*
 * The fighters' gauges. -G8: the blink toggle is a
 * $gp-relative .sdata static.
 */
/* Flips every frame; a low hit-point bar blinks with it. */
DATA(0x800632e4, 0x1, ".sdata")
static u8 DAT_800632e4 = 0;

/*
 * Draws each fighter's hit-point bar (growing outwards from the screen
 * centre, white while below 31% on alternate frames) and, when it has
 * one, its second gauge under it; `level` (at most 32) scales the colours.
 */
ADDRESS(0x80029b10, 0x418)
void func_80029b10(int level)
{
    int i = 0;
    int w;
    VsFighter *f;
    POLY_G4 *p;
    StPacketBuffer *packet;

    DAT_800632e4 = 1 - DAT_800632e4;
    if (level > 32)
        level = 32;
    for (; i < 2; i++) {
        f = &DAT_800633b0.fighter[i];
        packet = vs_gfx_state.packet;
        p = (POLY_G4 *)packet->cur;
        packet->cur = (u_long *)(p + 1);
        setPolyG4(p);
        setSemiTrans(p, 1);
        w = f->unk4DF08 * 100 / f->unk4DF18.unk1B8;
        if (i == 0)
            setXY4(p, 115 - w, 20, 115, 20, 120 - w, 25, 120, 25);
        else
            setXY4(p, w + 205, 20, 205, 20, w + 200, 25, 200, 25);
        setRGB0(p, 0, 0, 0);
        setRGB1(p, 0, 0, 0);
        if (w >= 31 || DAT_800632e4 != 0) {
            setRGB3(p, level * 250 / 32, 0, 0);
            setRGB2(p, 250 - w * 5 / 2, w * 5 / 2 * level / 32, 0);
        } else {
            setRGB3(p, 200, 200, 200);
            setRGB2(p, 200, 200, 200);
        }
        addPrim(&vs_gfx_state.ot_cur[1], p);
    }
    for (i = 0; i < 2; i++) {
        f = &DAT_800633b0.fighter[i];
        if (f->unk4DF18.unk1BA != 0) {
            packet = vs_gfx_state.packet;
            p = (POLY_G4 *)packet->cur;
            packet->cur = (u_long *)(p + 1);
            setPolyG4(p);
            setSemiTrans(p, 1);
            w = f->unk4DF0A * 100 / f->unk4DF18.unk1BA;
            if (i == 0)
                setXY4(p, 118 - w, 25, 118, 25, 120 - w, 28, 120, 28);
            else
                setXY4(p, w + 202, 25, 202, 25, w + 200, 28, 200, 28);
            setRGB0(p, 0, 0, 0);
            setRGB1(p, 0, 0, 0);
            setRGB3(p, level * 250 / 32, 0, 0);
            setRGB2(p, level * 250 / 32, w * 5 / 2 * level / 32, 0);
            addPrim(&vs_gfx_state.ot_cur[1], p);
        }
    }
}

/*
 * The round-win markers, the round timer, the message banners and
 * two screen overlays, all drawn near the front of the frame's ordering
 * table.
 */

/*
 * One pulsing red marker per round won (DAT_80063370) for each fighter,
 * left to right for the first and right to left for the second; `phase`
 * (0..95) cycles the gradient, `scale` its brightness.
 */
ADDRESS(0x80029f28, 0x1fc)
void func_80029f28(int phase, int scale)
{
    POLY_G4 *p;
    int i;
    int j;
    int left;
    int top;
    int bottom;

    if (phase < 32) {
        bottom = phase * 4 + 64;
        top = 64;
    } else if (phase < 64) {
        bottom = 192 - (phase - 32) * 4;
        top = (phase - 32) * 4 + 64;
    } else if (phase < 96) {
        bottom = 64;
        top = 192 - (phase - 64) * 4;
    }
    for (j = 0; j < 2; j++) {
        for (i = 0; i < DAT_80063370[j]; i++) {
            p = (POLY_G4 *)vs_gfx_state.packet->cur;
            vs_gfx_state.packet->cur = (u_long *)(p + 1);
            setlen(p, 8);
            setcode(p, 58);
            if (j == 0)
                left = i * 15 + 20;
            else
                left = 290 - i * 15;
            setXY4(p, left, 210, left + 10, 210, left, 220, left + 10, 220);
            setRGB0(p, top * scale / 32, 0, 0);
            setRGB1(p, top * scale / 32, 0, 0);
            setRGB2(p, bottom * scale / 32, 0, 0);
            setRGB3(p, bottom * scale / 32, 0, 0);
            addPrim(&vs_gfx_state.ot_cur[1], p);
        }
    }
}

/*
 * The two-digit round timer `value` at level `level` (0..16); the digit
 * cases for value - 1 are unused.
 */
ADDRESS(0x8002a124, 0x23c)
void func_8002a124(int value, int level)
{
    POLY_FT4 *p;
    int i;
    int digit;

    if (level > 16)
        level = 16;
    if (level < 0)
        level = 0;
    for (i = 0; i < 2; i++) {
        p = (POLY_FT4 *)vs_gfx_state.packet->cur;
        vs_gfx_state.packet->cur = (u_long *)(p + 1);
        setlen(p, 9);
        setcode(p, 46);
        setXY4(p, 128 + i * 32, 16, 160 + i * 32, 16, 128 + i * 32, 48, 160 + i * 32, 48);
        switch (i) {
        case 0:
            digit = value / 10;
            break;
        case 1:
            digit = value % 10;
            break;
        case 2:
            digit = (value - 1) / 10;
            break;
        case 3:
            digit = (value - 1) % 10;
            break;
        }
        if (digit < 5) {
            setUV4(p, digit * 32, 0, digit * 32 + 32, 0, digit * 32, 32, digit * 32 + 32, 32);
        } else {
            digit -= 5;
            setUV4(p, digit * 32, 32, digit * 32 + 32, 32, digit * 32, 64, digit * 32 + 32, 64);
        }
        p->clut = 32290;
        p->tpage = 61;
        setRGB0(p, level * 8, level * 8, level * 8);
        addPrim(&vs_gfx_state.ot_cur[3], p);
    }
}

/*
 * Message banner `kind` (0..19; 6 and 8 set no rectangle):
 * its screen rectangle and texture rectangle, the first nine from texture
 * page 62, the rest from 63, at level `level` (0..15; white for kinds 17
 * and 18, else red).
 */
ADDRESS(0x8002a360, 0x660)
void func_8002a360(int kind, int level)
{
    POLY_FT4 *p;

    if (level > 15)
        level = 15;
    if (level < 0)
        level = 0;
    p = (POLY_FT4 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(p + 1);
    setlen(p, 9);
    setcode(p, 46);
    switch (kind) {
    case 0:
        setXY4(p, 96, 80, 223, 80, 96, 135, 223, 135);
        setUV4(p, 0, 0, 127, 0, 0, 55, 126, 55);
        break;
    case 1:
        setXY4(p, 32, 80, 159, 80, 32, 135, 159, 135);
        setUV4(p, 128, 0, 255, 0, 128, 55, 255, 55);
        break;
    case 2:
        setXY4(p, 112, 80, 207, 80, 112, 135, 207, 135);
        setUV4(p, 0, 56, 95, 56, 0, 111, 95, 111);
        break;
    case 3:
        setXY4(p, 128, 80, 191, 80, 128, 135, 191, 135);
        setUV4(p, 96, 56, 159, 56, 96, 111, 159, 111);
        break;
    case 4:
        setXY4(p, 176, 80, 271, 80, 176, 135, 271, 135);
        setUV4(p, 160, 56, 255, 56, 160, 111, 255, 111);
        break;
    case 5:
        setXY4(p, 32, 80, 287, 80, 32, 135, 287, 135);
        setUV4(p, 0, 112, 255, 112, 0, 167, 255, 167);
        break;
    case 7:
        setXY4(p, 80, 80, 239, 80, 80, 135, 239, 135);
        setUV4(p, 0, 168, 159, 168, 0, 223, 159, 223);
        break;
    case 9:
        setXY4(p, 74, 99, 154, 99, 74, 115, 154, 115);
        setUV4(p, 124, 1, 204, 1, 124, 15, 204, 15);
        break;
    case 10:
        setXY4(p, 102, 100, 172, 100, 102, 115, 172, 115);
        setUV4(p, 130, 33, 200, 33, 130, 47, 200, 47);
        break;
    case 11:
        setXY4(p, 175, 99, 273, 99, 175, 115, 273, 115);
        setUV4(p, 3, 1, 111, 1, 3, 15, 111, 15);
        break;
    case 12:
        setXY4(p, 120, 90, 199, 90, 120, 105, 199, 105);
        setUV4(p, 106, 198, 186, 198, 106, 214, 186, 214);
        break;
    case 13:
        setXY4(p, 43, 100, 101, 100, 43, 115, 101, 115);
        setUV4(p, 0, 17, 58, 17, 0, 31, 58, 31);
        break;
    case 14:
        setXY4(p, 110, 130, 224, 130, 110, 145, 224, 145);
        setUV4(p, 60, 17, 174, 17, 60, 31, 174, 31);
        break;
    case 15:
        setXY4(p, 51, 100, 172, 100, 51, 115, 172, 115);
        setUV4(p, 0, 34, 121, 34, 0, 48, 121, 48);
        break;
    case 16:
        setXY4(p, 173, 99, 273, 99, 173, 115, 273, 115);
        setUV4(p, 3, 1, 111, 1, 3, 15, 111, 15);
        break;
    case 17:
        setXY4(p, 110, 130, 209, 130, 110, 146, 209, 146);
        setUV4(p, 0, 199, 100, 199, 0, 216, 100, 216);
        break;
    case 18:
        setXY4(p, 110, 155, 209, 155, 110, 171, 209, 171);
        setUV4(p, 0, 218, 99, 218, 0, 234, 99, 234);
        break;
    case 19:
        setXY4(p, 154, 99, 252, 99, 154, 115, 252, 115);
        setUV4(p, 3, 1, 111, 1, 3, 15, 111, 15);
        break;
    }
    if (kind < 9) {
        p->clut = 32162;
        p->tpage = 62;
    } else {
        p->clut = 32226;
        p->tpage = 63;
    }
    if (kind == 17 || kind == 18)
        setRGB0(p, level * 8, level * 8, level * 8);
    else
        setRGB0(p, level * 16, 0, 0);
    addPrim(&vs_gfx_state.ot_cur[5], p);
}

/*
 * A textured banner of half-height `size` centred at (160, 107), red at
 * `level` (0..15); `kind` 16, 17 or 18 picks its texture row.
 */
ADDRESS(0x8002a9c0, 0x198)
void func_8002a9c0(int kind, int level, int size)
{
    POLY_FT4 *p;

    if (level > 15)
        level = 15;
    if (level < 0)
        level = 0;
    p = (POLY_FT4 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(p + 1);
    setlen(p, 9);
    setcode(p, 46);
    switch (kind) {
    case 16:
        setUV4(p, 0, 48, 209, 48, 0, 97, 209, 97);
        break;
    case 17:
        setUV4(p, 0, 98, 209, 98, 0, 147, 209, 147);
        break;
    case 18:
        setUV4(p, 0, 148, 209, 148, 0, 197, 209, 197);
        break;
    }
    setXY4(p, 160 - size * 4, 107 - size, 160 + size * 4, 107 - size, 160 - size * 4, 107 + size,
           160 + size * 4, 107 + size);
    p->clut = 32226;
    p->tpage = 63;
    setRGB0(p, level * 16, 0, 0);
    addPrim(&vs_gfx_state.ot_cur[5], p);
}

/*
 * A semi-transparent blue box (wide or narrow) at `level` (0..15), then an
 * empty textured triangle that sets the draw mode for it.
 */
ADDRESS(0x8002ab58, 0x1e8)
void func_8002ab58(int level, int wide)
{
    POLY_F4 *p;
    POLY_FT3 *t;

    if (level > 15)
        level = 15;
    if (level < 0)
        level = 0;
    p = (POLY_F4 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(p + 1);
    setlen(p, 5);
    setcode(p, 42);
    if (wide)
        setXY4(p, 10, 80, 309, 80, 10, 185, 309, 185);
    else
        setXY4(p, 80, 80, 239, 80, 80, 185, 239, 185);
    setRGB0(p, 0, 0, level * 4);
    addPrim(&vs_gfx_state.ot_cur[5], p);
    t = (POLY_FT3 *)vs_gfx_state.packet->cur;
    vs_gfx_state.packet->cur = (u_long *)(t + 1);
    SetPolyFT3(t);
    t->clut = 32641;
    setXY3(t, 0, 0, 0, 0, 0, 0);
    setUV3(t, 0, 0, 0, 0, 0, 0);
    t->tpage = 24;
    setRGB0(t, 0, 0, 0);
    addPrim(&vs_gfx_state.ot_cur[5], t);
}

/*
 * Drawing the fighters' names.
 */
/*
 * Draws both fighters' names (item names DAT_801624b8[n].slot[DAT_80063338]) in
 * 11-pixel glyphs from the font page at VRAM (640, 0): the first on the
 * left, the second on the right, wrapping to a second line at glyph 49;
 * `bright` sets their green tint.
 */
/* The four x positions are not stepped variables: they are written from a
 * column counter that the line break sets back to 0. loop.c accepts a
 * constant assignment to a counter in an innermost loop, turns the four
 * sums into stepped registers, initialises them after the constants it
 * hoists and sets them again next to the `col = 0`. */
ADDRESS(0x8002ad40, 0x338)
void func_8002ad40(int bright)
{
    u16 *s;
    int line = 0;
    int n;
    int col;
    u16 glyph;

    for (n = 0; n < 2;) {
        s = VS_ITEM_NAME(DAT_801624b8[n].slot[DAT_80063338]);
        glyph = *s;
        if (glyph == 0xffff || glyph == 0)
            goto next;

        col = 0;
        do {
            POLY_FT4 *p;
            int c;
            int x0, x1, x2, x3;

            if (glyph == 1)
                break;
            c = *s - 2;
            if (c == 47) {
                line = 1;
                col = 0;
            }
            x0 = 20 + col * 11;
            x1 = 31 + col * 11;
            x2 = 201 + col * 11;
            x3 = 212 + col * 11;
            p = (POLY_FT4 *)vs_gfx_state.packet->cur;
            vs_gfx_state.packet->cur = (u_long *)(p + 1);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            setUV4(p, c % 18 * 12, c / 18 * 10, c % 18 * 12 + 11, c / 18 * 10,
                   c % 18 * 12, c / 18 * 10 + 10, c % 18 * 12 + 11, c / 18 * 10 + 10);
            if (n == 0) {
                if (line) {
                    int y = 40;

                    setXY4(p, x0, y, x1, y, x0, y + 10, x1, y + 10);
                } else {
                    int y = 29;

                    setXY4(p, x0, y, x1, y, x0, y + 10, x1, y + 10);
                }
            } else {
                if (line) {
                    int y = 40;

                    setXY4(p, x2, y, x3, y, x2, y + 10, x3, y + 10);
                } else {
                    int y = 29;

                    setXY4(p, x2, y, x3, y, x2, y + 10, x3, y + 10);
                }
            }
            p->tpage = GetTPage(0, 1, 640, 0);
            p->clut = GetClut(640, 480);
            setRGB0(p, bright * 3, bright * 3 + 16, bright * 3);
            addPrim(vs_gfx_state.ot_cur + 5, p);
            s++;
            col++;
            glyph = *s;
        } while (glyph != 0xffff && glyph != 0);
    next:
        n++;
        line = 0;
    }
}

/*
 * The round's opening sequence.
 */

void func_8002a360(int kind, int level);                 /* src/vs/hud_80029f28.c */
void func_8002a124(int value, int level);
void func_8002a9c0(int kind, int level, int size);
void func_80029b10(int level);                           /* src/vs/gauge_80029b10.c */
void func_80029f28(int phase, int scale);
void func_8002ad40(int level);                           /* src/vs/name_8002ad40.c */
void fighter_draw_skeletal(void);                                /* src/vs/model_80024b5c.c */
void fighter_draw_vertex_anim(void);                                /* src/vs/model_80028914.c */
void fighter_draw_shadow(int n, int scale);                    /* src/vs/shadow_8002bfe8.c */

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
    (m).t[0] = (f).unk4DEF0.vx - vs_gfx_state.unk2CA78.vx;                    \
    (m).t[1] = -vs_gfx_state.unk2CA78.vy;                                      \
    (m).t[2] = (f).unk4DEF0.vz - vs_gfx_state.unk2CA78.vz;                    \
    (m).m[0][0] = 4096;                                                        \
    (m).m[0][1] = 0;                                                           \
    (m).m[0][2] = 0;                                                           \
    (m).m[1][0] = 0;                                                           \
    (m).m[1][1] = 4096;                                                        \
    (m).m[1][2] = 0;                                                           \
    (m).m[2][0] = 0;                                                           \
    (m).m[2][1] = 0;                                                           \
    (m).m[2][2] = 4096;                                                        \
    CompMatrix(&vs_gfx_state.unk2C998, &(m), &(m));                            \
    gte_SetRotMatrix(&(m));                                                    \
    gte_SetTransMatrix(&(m))

/* An empty textured triangle at OT depth z that sets the semi-transparency
 * mode (tpage 56) for the ring quad before it. */
#define RING_MODE(z)                                                           \
    {                                                                          \
        t3 = (POLY_FT3 *)vs_gfx_state.packet->cur;                             \
        vs_gfx_state.packet->cur = (u_long *)(t3 + 1);                         \
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
        addPrim(vs_gfx_state.ot_cur + z, t3);                                  \
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
        func_80029f28((u32)pad_state.unk3C % 96, 32);
        func_8002ad40(32);
    } else if (frame > 70) {
        func_8002a360(7, (frame - 70) * 2);
        func_8002a124(30, frame - 70);
        func_80029b10((frame - 70) * 2);
        func_80029f28((u32)pad_state.unk3C % 96, (frame - 70) * 2);
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
                v[i + 16].vy = vs_gfx_state.unk2CA78.vy;
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
                p = (POLY_G4 *)vs_gfx_state.packet->cur;
                vs_gfx_state.packet->cur = (u_long *)(p + 1);
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
                addPrim(vs_gfx_state.ot_cur + z, p);
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
            q = (POLY_FT4 *)vs_gfx_state.packet->cur;
            vs_gfx_state.packet->cur = (u_long *)(q + 1);
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
            addPrim(vs_gfx_state.ot_cur + 4091, q);
        }
    }
    if (frame > 45) {
        for (i = 0; i < 2; i++) {
            DAT_80063300 = &DAT_800633b0.fighter[i];
            switch (DAT_800633b0.fighter[i].unk4DF10) {
            case 1:
                fighter_draw_skeletal();
                break;
            case 2:
                DAT_80063360 = 0;
                fighter_draw_vertex_anim();
                break;
            }
            DAT_80063300->unk4DEEA =
                (DAT_80063300->unk4DEEA + DAT_80063300->unk4DEEC * DAT_80063220) & 0xfff;
            fighter_draw_shadow(i, 4096);
        }
    }
    return 0;
}
