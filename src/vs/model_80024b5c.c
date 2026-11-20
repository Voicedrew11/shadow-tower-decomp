/*
 * VS.EXE: drawing the current fighter's model (WIP module): its object
 * parts through the model list (VS's copy of ST.EXE's model_draw_list,
 * src/st/model_draw.c) and the fighter itself, lit and posed.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <GTEMAC.H>
#include <st/mesh.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/scratch.h>
#include <vs/funcs.h>

RODATA(0x800112d0, 0x50)

/* The renderers' default NCCS colour (128, 128, 128). */
SDATA(0x800632d0, 0x4)
u_long DAT_800632d0 = 0x808080;

/* The frame slot of the model list's every-nth parts. */
DATA(0x800632d4, 0x4, ".sdata")
static s32 DAT_800632d4 = 0;

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
 * Draws each entry of the scratchpad's model list: kind 2 turns its parent
 * matrix to face the camera first, kinds 1, 3 and 4 draw scaled and only
 * on the entry whose index matches DAT_800632d4's slot (every 6th, 4th,
 * 8th), stepping it.
 */
ADDRESS(0x80024b5c, 0x6a8)
void func_80024b5c(void)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    ModelEntry *e = ((ModelList *)sp->anims)->entries;
    int i = ((ModelList *)sp->anims)->count;
    MATRIX scale = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    MeshVertex *out;
    SVECTOR *in;
    int n;

    while (i--) {
        e->obj->unk0 = 0;
        e++;
    }
    i = ((ModelList *)sp->anims)->count;
    e = ((ModelList *)sp->anims)->entries;
    while (i--) {
        e->obj->unk0 = 0;
        if (e->unk0C != -1 && e->part != 0xffff) {
            if (e->kind == 2) {
                MATRIX face = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
                SVECTOR angle;

                angle.vx = -DAT_80063300->unk4DF00 + vs_gfx_state.unk2CA88.vx;
                angle.vy = -DAT_80063300->unk4DF02 + vs_gfx_state.unk2CA88.vy;
                angle.vz = -DAT_80063300->unk4DF04 + vs_gfx_state.unk2CA88.vz;
                RotMatrixX(angle.vx, &face);
                RotMatrixY(angle.vy, &face);
                RotMatrixZ(angle.vz, &face);
                if (e->obj->unk48->unk2 == 0) {
                    RotMatrix(&e->obj->unk48->unk44->rot, &e->obj->unk48->unk4);
                }
                gte_MulMatrix0(&e->obj->unk48->unk4, &face, &e->obj->unk48->unk4);
                e->obj->unk48->unk2 = 0;
            }
            anim_record_compose(e->obj);
            if (e->kind == 1) {
                if (DAT_800632d4 >= 6)
                    DAT_800632d4 = 0;
                ScaleMatrix(&scale, &e->obj->unk44->scale);
                gte_SetRotMatrix(&scale);
                if (i % 6 != DAT_800632d4) {
                    e++;
                    continue;
                }
                DAT_800632d4 = i % 6 + 1;
            } else if (e->kind == 3) {
                if (DAT_800632d4 >= 4)
                    DAT_800632d4 = 0;
                ScaleMatrix(&scale, &e->obj->unk44->scale);
                gte_SetRotMatrix(&scale);
                if (i % 4 != DAT_800632d4) {
                    e++;
                    continue;
                }
                DAT_800632d4 = i % 4 + 1;
            } else if (e->kind == 4) {
                if (DAT_800632d4 >= 8)
                    DAT_800632d4 = 0;
                ScaleMatrix(&scale, &e->obj->unk44->scale);
                gte_SetRotMatrix(&scale);
                if (i % 8 != DAT_800632d4) {
                    e++;
                    continue;
                }
                DAT_800632d4 = i % 8 + 1;
            }
            sp->header = (MeshHeader *)(sp->model + 12) + e->part;
            sp->local_vertices = sp->model + 12 + sp->header->vertices;
            PROJECT(sp, out, in, n);
            DAT_80063360 = 0;
            func_80027ba4(e->part);
        }
        e++;
    }
}

/*
 * Draws the current fighter (DAT_80063300) unless it has no animation
 * frame: lights it, places it at its position turned by its rotation and
 * the camera, poses its model and draws it (dimmed by its brightness
 * below 128) with its object parts.
 */
ADDRESS(0x80025204, 0x46c)
void func_80025204(void)
{
    if (DAT_80063300->unk4DEEA != -1) {
    VsScratchPad *sp = VS_SCRATCHPAD;
    VsLights lights = {
        {{3000, 3700, -1500}, {-2000, 3700, -3000}, {-1300, -1000, 1024}}, 0,
        {{4096, 3000, 3000}, {4096, 3000, 3000}, {4096, 3000, 3000}}, 0,
        {100, 100, 100, 0}, {-1, -1}};
    MATRIX m = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    VECTOR unused0;      /* unused: retail reserves their frame slots */
    SVECTOR unused1;
    int rx;
    int ry;
    int n;

    rx = DAT_80063218 + DAT_80063300->unk4DF00;
    ry = DAT_8006321c + DAT_80063300->unk4DF02;
    ScaleMatrix(&m, &DAT_8005b2a8);
    RotMatrixX(rx, &m);
    RotMatrixY(ry, &m);
    MulMatrix((MATRIX *)&lights.l, &m);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    sp->unk68 = lights.dq[0];
    sp->unk6C = lights.dq[1];
    gte_SetBackColor(lights.bk.r, lights.bk.g, lights.bk.b);
    sp->matrix.t[0] = DAT_80063300->unk4DEF0.vx - vs_gfx_state.unk2CA78.vx;
    sp->matrix.t[1] = DAT_80063300->unk4DEF0.vy - vs_gfx_state.unk2CA78.vy;
    sp->matrix.t[2] = DAT_80063300->unk4DEF0.vz - vs_gfx_state.unk2CA78.vz;
    RotMatrix((SVECTOR *)&DAT_80063300->unk4DF00, &sp->matrix);
    CompMatrix(&vs_gfx_state.unk2C998, &sp->matrix, &sp->matrix);
    n = DAT_80063300 - DAT_800633b0.fighter;
    sp->model = (u8 *)vs_gfx_state.unk834C[n] + ((ModelFile *)vs_gfx_state.unk834C[n])->unk8;
    if (func_800246d8(&DAT_80063300->unk4DEE0, n, DAT_80063300->unk4DEE8,
                      DAT_80063300->unk4DEEA) == 0)
        return;
    sp->ot = vs_gfx_state.ot_cur;
    sp->far_z = 4096;
    if (DAT_80063300->unk4DF12 < 128) {
        sp->rgb.r = DAT_80063300->unk4DF12;
        sp->rgb.g = DAT_80063300->unk4DF12;
        sp->rgb.b = DAT_80063300->unk4DF12;
        sp->rgb.cd = 0;
        DAT_80063350 = 1;
    } else {
        *(u_long *)&sp->rgb = DAT_800632d0;
        DAT_80063350 = 0;
    }
    sp->face_count = vs_gfx_state.unk831C[1];
    sp->prim_count = vs_gfx_state.unk8324;
    sp->unk94 = vs_gfx_state.unk8328;
    sp->prim = vs_gfx_state.packet->cur;
    sp->prim_end = vs_gfx_state.packet->end;
    sp->vertices = vs_gfx_state.unkA7EC;
    sp->clut_base = 0;
    if (DAT_80063300->unk4DEEA != -1)
        func_80024b5c();
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.unk831C[1] = sp->face_count;
    vs_gfx_state.unk8328 = sp->unk94;
    vs_gfx_state.unk8324 = sp->prim_count;
    }
}
