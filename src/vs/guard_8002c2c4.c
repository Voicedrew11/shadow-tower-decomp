/*
 * VS.EXE: the guard flash (WIP module).
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
#include <vs/buffer.h>
#include <vs/scratch.h>

/* func_8002c2c4's switch table. */
RODATA(0x80011574, 0x14)

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

/* A semi-transparent Gouraud triangle over vertices a, b, c in colours
 * ca, cb, cb at the OT depth of the three, then the mode triangle at the
 * same depth. */
#define FLASH_G3(sp, va, vb, vc, ca, cb)                                       \
    {                                                                          \
        int z;                                                                 \
                                                                               \
        p3 = (POLY_G3 *)vs_gfx_state.packet->cur;                              \
        vs_gfx_state.packet->cur = (u_long *)(p3 + 1);                         \
        SetPolyG3(p3);                                                         \
        setSemiTrans(p3, 1);                                                   \
        p3->x0 = VTX(sp, va).sxy.vx;                                           \
        p3->y0 = VTX(sp, va).sxy.vy;                                           \
        p3->x1 = VTX(sp, vb).sxy.vx;                                           \
        p3->y1 = VTX(sp, vb).sxy.vy;                                           \
        p3->x2 = VTX(sp, vc).sxy.vx;                                           \
        p3->y2 = VTX(sp, vc).sxy.vy;                                           \
        z = (VTX(sp, va).z + VTX(sp, vb).z + VTX(sp, vc).z) >> 7;              \
        setRGB0(p3, ca.r, ca.g, ca.b);                                         \
        setRGB1(p3, cb.r, cb.g, cb.b);                                         \
        setRGB2(p3, cb.r, cb.g, cb.b);                                         \
        addPrim(vs_gfx_state.ot_cur + z, p3);                                  \
        FLASH_MODE(z);                                                         \
    }

/* A semi-transparent Gouraud quad over vertices a, b, c, d in colours
 * ca, ca, cc, cc at the depth of the centre vertex and the quad's last
 * three corners of ring row i. */
#define FLASH_G4(sp, va, vb, vc, vd, ca, cc)                                   \
    {                                                                          \
        int z;                                                                 \
                                                                               \
        p4 = (POLY_G4 *)vs_gfx_state.packet->cur;                              \
        vs_gfx_state.packet->cur = (u_long *)(p4 + 1);                         \
        SetPolyG4(p4);                                                         \
        setSemiTrans(p4, 1);                                                   \
        p4->x0 = VTX(sp, va).sxy.vx;                                           \
        p4->y0 = VTX(sp, va).sxy.vy;                                           \
        p4->x1 = VTX(sp, vb).sxy.vx;                                           \
        p4->y1 = VTX(sp, vb).sxy.vy;                                           \
        p4->x2 = VTX(sp, vc).sxy.vx;                                           \
        p4->y2 = VTX(sp, vc).sxy.vy;                                           \
        p4->x3 = VTX(sp, vd).sxy.vx;                                           \
        p4->y3 = VTX(sp, vd).sxy.vy;                                           \
        z = (VTX(sp, 0).z + VTX(sp, i + 16).z + VTX(sp, i + 17).z              \
             + VTX(sp, i + 1).z) >> 7;                                         \
        setRGB0(p4, ca.r, ca.g, ca.b);                                         \
        setRGB1(p4, ca.r, ca.g, ca.b);                                         \
        setRGB2(p4, cc.r, cc.g, cc.b);                                         \
        setRGB3(p4, cc.r, cc.g, cc.b);                                         \
        addPrim(vs_gfx_state.ot_cur + z, p4);                                  \
        FLASH_MODE(z);                                                         \
    }

/* An empty textured triangle at OT depth z that sets the semi-transparency
 * mode (tpage 56) for the flash polygon before it. */
#define FLASH_MODE(z)                                                          \
    {                                                                          \
        t = (POLY_FT3 *)vs_gfx_state.packet->cur;                              \
        vs_gfx_state.packet->cur = (u_long *)(t + 1);                          \
        SetPolyFT3(t);                                                         \
        setXY3(t, 0, 0, 0, 0, 0, 0);                                           \
        setUV3(t, 0, 0, 0, 0, 0, 0);                                           \
        t->clut = 32641;                                                       \
        t->tpage = 56;                                                         \
        setRGB0(t, 0, 0, 0);                                                   \
        addPrim(vs_gfx_state.ot_cur + z, t);                                   \
    }

/* The scratchpad's projected vertex n. */
#define VTX(sp, n) (((MeshVertex *)(sp)->vertices)[n])

/* OT depths: a triangle's three vertices; a quad's, as retail sums them,
 * from the centre vertex and the corners of ring column i. */
#define Z3(sp, va, vb, vc) (VTX(sp, va).z + VTX(sp, vb).z + VTX(sp, vc).z)
#define Z4(sp, i) (VTX(sp, 0).z + VTX(sp, (i) + 16).z + VTX(sp, (i) + 17).z + VTX(sp, (i) + 1).z)

/*
 * Draws fighter n's guard flash while it blocks (state 1..5 in unk4DF14
 * picks the rings' colours): a three-ring disc at its hit radius in front
 * of it, scaled by its hit radius and height.
 */
ADDRESS(0x8002c2c4, 0x144c)
void func_8002c2c4(int id)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    VsFighter *f = &DAT_800633b0.fighter[id];
    VsFighterData *d = &f->unk4DF18;
    CVECTOR c[4];
    SVECTOR rot;
    MATRIX m;
    MeshVertex *out;
    SVECTOR *in;
    int n;
    POLY_G3 *p3;
    POLY_G4 *p4;
    POLY_FT3 *t;
    int i;

    switch (f->unk4DF14) {
    case 5:
        c[0].r = 255;
        c[0].g = 255;
        c[0].b = 0;
        c[1].r = 0;
        c[1].g = 0;
        c[1].b = 0;
        c[2].r = 0;
        c[2].g = 0;
        c[2].b = 0;
        c[3].r = 0;
        c[3].g = 0;
        c[3].b = 0;
        break;
    case 4:
        c[0].r = 0;
        c[0].g = 0;
        c[0].b = 0;
        c[1].r = 255;
        c[1].g = 255;
        c[1].b = 0;
        c[2].r = 0;
        c[2].g = 0;
        c[2].b = 0;
        c[3].r = 0;
        c[3].g = 0;
        c[3].b = 0;
        break;
    case 3:
        c[0].r = 0;
        c[0].g = 0;
        c[0].b = 0;
        c[1].r = 0;
        c[1].g = 0;
        c[1].b = 0;
        c[2].r = 255;
        c[2].g = 255;
        c[2].b = 0;
        c[3].r = 0;
        c[3].g = 0;
        c[3].b = 0;
        break;
    case 2:
        c[0].r = 0;
        c[0].g = 0;
        c[0].b = 0;
        c[1].r = 0;
        c[1].g = 0;
        c[1].b = 0;
        c[2].r = 0;
        c[2].g = 0;
        c[2].b = 0;
        c[3].r = 64;
        c[3].g = 64;
        c[3].b = 0;
        break;
    case 1:
        c[0].r = 0;
        c[0].g = 0;
        c[0].b = 0;
        c[1].r = 0;
        c[1].g = 0;
        c[1].b = 0;
        c[2].r = 0;
        c[2].g = 0;
        c[2].b = 0;
        c[3].r = 32;
        c[3].g = 32;
        c[3].b = 0;
        break;
    }
    m.t[0] = f->unk4DEF0.vx + rsin(f->unk4DF02 - 2048) * d->unk1B4 / 4096 - vs_gfx_state.unk2CA78.vx;
    m.t[1] = f->unk4DEF0.vy - d->unk1B6 / 2 - vs_gfx_state.unk2CA78.vy;
    m.t[2] = f->unk4DEF0.vz + rcos(f->unk4DF02 - 2048) * d->unk1B4 / 4096 - vs_gfx_state.unk2CA78.vz;
    rot.vx = f->unk4DF00;
    rot.vy = f->unk4DF02;
    rot.vz = f->unk4DF04;
    RotMatrix(&rot, &m);
    CompMatrix(&vs_gfx_state.unk2C998, &m, &m);
    m.m[0][0] = m.m[0][0] * d->unk1B4 / 500;
    m.m[0][1] = m.m[0][1] * d->unk1B4 / 500;
    m.m[0][2] = m.m[0][2] * d->unk1B4 / 500;
    m.m[1][0] = m.m[1][0] * d->unk1B6 / 1000;
    m.m[1][1] = m.m[1][1] * d->unk1B6 / 1000;
    m.m[1][2] = m.m[1][2] * d->unk1B6 / 1000;
    m.m[2][0] = m.m[2][0] * d->unk1B4 / 1000;
    m.m[2][1] = m.m[2][1] * d->unk1B4 / 1000;
    m.m[2][2] = m.m[2][2] * d->unk1B4 / 1000;
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    sp->model = DAT_8015ec38;
    sp->header = (MeshHeader *)(DAT_8015ec38 + 12);
    sp->local_vertices = DAT_8015ec38 + 12 + sp->header->vertices;
    PROJECT(sp, out, in, n);
    for (i = 1; i < 16; i++) {
        FLASH_G3(sp, 0, i, i + 1, c[0], c[1]);
        FLASH_G4(sp, i, i + 1, i + 16, i + 17, c[1], c[2]);
        FLASH_G4(sp, i + 16, i + 17, i + 32, i + 33, c[2], c[3]);
    }
    FLASH_G3(sp, 0, 16, 1, c[0], c[1]);
    FLASH_G4(sp, 16, 1, 32, 17, c[1], c[2]);
    FLASH_G4(sp, 32, 17, 48, 33, c[2], c[3]);
}
