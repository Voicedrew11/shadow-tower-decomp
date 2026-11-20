/*
 * VS.EXE: drawing one effect model part and the effects' models (WIP
 * module). Their constant pool (0x800118c4) sits just before the switch
 * tables of func_80032404 and func_800335e8 (0x80011934), so these two
 * and src/vs/effect_move_8003333c.c / effect_run_800352cc.c are most
 * likely one file with those two functions (not reconstructed yet).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <GTEMAC.H>
#include <st/mesh.h>
#include <st/effect.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/scratch.h>
#include <vs/effect.h>
#include <vs/funcs.h>

RODATA(0x800118c4, 0x70)

/* Projects the current part's vertices: RTPT in threes, the first group
 * ahead of the loop so each pass stores one group and starts the next, then
 * RTPS for the rest. */
#define PROJECT(sp, out, in, n)                                                \
    {                                                                          \
        (sp)->z_ot += (sp)->header->n_vertices;                                \
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

ADDRESS(0x800353d0, 0x798)
void func_800353d0(u8 *model, VECTOR *pos, SVECTOR *rot, SVECTOR *scale, MATRIX *view,
                   u8 part, short light, u8 flags, int z)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    SVECTOR rel;
    VECTOR s;
    MATRIX work0;
    MATRIX work1;
    VsLights lights = {
        {{3000, -3700, -1500}, {-2000, -3700, -3000}, {0, 1000, 0}}, 0,
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0,
        {200, 200, 200, 0}, {-1, -1}};
    MATRIX unit = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};
    long flag;
    MeshVertex *out;
    SVECTOR *in;
    int n;

    sp->vertices = vs_gfx_state.unkA7EC;
    sp->unk48 = vs_gfx_state.unkA7EC;
    sp->z_ot = vs_gfx_state.unk831C[0];
    sp->face_count = vs_gfx_state.unk831C[1];
    sp->prim_count = vs_gfx_state.unk8324;
    sp->unk94 = vs_gfx_state.unk8328;
    sp->prim = vs_gfx_state.packet->cur;
    sp->prim_end = vs_gfx_state.packet->end;
    gte_SetRotMatrix(&vs_gfx_state.unk2C998);
    SetTransMatrix(&vs_gfx_state.unk2C998);
    if (view) {
        rel.vx = pos->vx - vs_gfx_state.unk2CA78.vx;
        rel.vy = pos->vy - vs_gfx_state.unk2CA78.vy;
        rel.vz = pos->vz - vs_gfx_state.unk2CA78.vz;
        gte_RotTrans(&rel, (VECTOR *)sp->matrix.t, &flag);
    } else {
        sp->matrix.t[0] = pos->vx;
        sp->matrix.t[1] = pos->vy;
        sp->matrix.t[2] = pos->vz;
    }
    RotMatrixYXZ(rot, &sp->matrix);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    if (scale) {
        s.vx = scale->vx;
        s.vy = scale->vy;
        s.vz = scale->vz;
        ScaleMatrix(&sp->matrix, &s);
    }
    if (view) {
        gte_MulMatrix0(view, &sp->matrix, &sp->matrix);
    }
    gte_SetRotMatrix(&sp->matrix);
    SetTransMatrix(&sp->matrix);
    sp->model = model;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->local_vertices = sp->model + 12 + sp->header->vertices;
    if (view) {
        PROJECT(sp, out, in, n);
    } else {
        PROJECT(sp, out, in, n);
    }
    sp->rgb.r = light / 32;
    sp->rgb.g = light / 32;
    sp->rgb.b = light / 32;
    sp->rgb.cd = 0;
    func_80027ba4(part);
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.unk831C[0] = sp->z_ot;
    vs_gfx_state.unk831C[1] = sp->face_count;
    vs_gfx_state.unk8324 = sp->prim_count;
    vs_gfx_state.unk8328 = sp->unk94;
}

/* Draws every live effect with a model (VS's copy of ST.EXE's
 * func_800255f0, src/st/objects2.c): kind 0 through the camera, kind 4
 * without a view. */
ADDRESS(0x80035b68, 0x204)
void func_80035b68(void)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    StEffect *o = DAT_801ce698.slots;
    MATRIX unit = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}};
    int i;

    for (i = 127; i != -1; o++, i--) {
        if (o->unk0 == 0xff)
            continue;
        if ((o->unk6 & 3) == 0)
            continue;
        sp->unk13C = o->unk20;
        sp->clut_base = o->unkE;
        switch (o->unk6 & 0xc) {
        case 0:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = vs_gfx_state.ot_cur - 7;
            func_800353d0((u8 *)DAT_800633b0.fighter[o->unk4C].unk4B000, &o->unk20, &o->unk30,
                          &o->unk38, &vs_gfx_state.unk2C998, o->unk2 & 0x7f, o->unk12, o->unk7, 0);
            break;
        case 4:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = vs_gfx_state.ot_cur - 7;
            func_800353d0((u8 *)DAT_800633b0.fighter[o->unk4C].unk4B000, &o->unk20, &o->unk30,
                          &o->unk38, &unit, o->unk2 & 0x7f, o->unk12, o->unk7, 0);
            break;
        case 8:
        case 12:
            break;
        }
    }
}
