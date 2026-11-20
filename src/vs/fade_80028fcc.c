/*
 * VS.EXE: a full-screen semi-transparent overlay (WIP module).
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

/* src/vs/mesh_800258f0.c's part renderer, called here without a
 * prototype: retail passes the int part number unconverted. */
void func_80027ba4();

/* func_800291d4's initializers. */
RODATA(0x8001142c, 0x50)

/* func_80028fcc's switch table. */
JUMPTABLES(0x8001150c, 0x18)

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
    func_80027ba4(part);
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.unk831C[1] = sp->face_count;
    vs_gfx_state.unk8324 = sp->prim_count;
}
