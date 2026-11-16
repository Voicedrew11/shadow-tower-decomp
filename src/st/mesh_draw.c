/*
 * ST.EXE depth-cued mesh renderers (WIP module). Each renderer draws one
 * primitive kind of a model part's face list: NCLIP back-face culling, an
 * averaged OT depth clipped to the scratchpad's near/far range, and NCDS/NCDT
 * lighting with depth cueing. When the render-mode byte asks for a tint, the
 * lit colours go through one of the two tint callbacks linked before each
 * renderer. func_800192d4 sets up a part and runs the four renderers.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>

/* The FT3 renderer's tint callbacks (band B, 0x80018440 and 0x80018480). */
extern void func_80018440(CVECTOR *rgb);
extern void func_80018480(CVECTOR *rgb);

ADDRESS(0x800184c4, 0x2e0)
void func_800184c4(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    int t;
    MeshFaceFT3 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = func_80018440;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = func_80018480;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->v0);
        v1 = (MeshVertex *)(sp->vertices + face->v1);
        v2 = (MeshVertex *)(sp->vertices + face->v2);
        gte_ldsxy3(v0->sxy, v1->sxy, v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x24);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft3(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_lddp((v0->p + v1->p + v2->p) / 3);
                *(u_short *)&p->u2 = face->uv2;
                gte_ncds_b();
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                }
            }
        }
        face = next;
    }
    sp->faces = face;
    sp->prim_count += (sp->prim - start) / 8;
}

ADDRESS(0x800187a4, 0x40)
void func_800187a4(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x800187e4, 0x44)
void func_800187e4(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x80018828, 0x2f4)
void func_80018828(int count, MeshFaceFT4 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    int t;
    MeshFaceFT4 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = func_800187a4;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = func_800187e4;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->v0);
        v1 = (MeshVertex *)(sp->vertices + face->v1);
        v2 = (MeshVertex *)(sp->vertices + face->v2);
        gte_ldsxy3(v0->sxy, v1->sxy, v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x2c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->v3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft4(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->x3 = v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_ncds_b();
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb(&p->r0);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                }
            }
        }
        face = next;
    }
    sp->faces = face;
    sp->prim_count += (sp->prim - start) / 10;
}

ADDRESS(0x80018b1c, 0x40)
void func_80018b1c(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x80018b5c, 0x44)
void func_80018b5c(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x80018ba0, 0x324)
void func_80018ba0(int count, MeshFaceGT3 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int t;
    MeshFaceGT3 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = func_80018b1c;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = func_80018b5c;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->v0);
        v1 = (MeshVertex *)(sp->vertices + face->v1);
        v2 = (MeshVertex *)(sp->vertices + face->v2);
        gte_ldsxy3(v0->sxy, v1->sxy, v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x34);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_GT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt3(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_lddp((v0->p * 2 + v1->p + v2->p) >> 2);
                *(u_short *)&p->u2 = face->uv2;
                gte_ncdt_b();
                mesh_add_prim(ot + z, p, 9);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                    tint((CVECTOR *)&p->r1);
                    tint((CVECTOR *)&p->r2);
                }
            }
        }
        face = next;
    }
    sp->faces = face;
    sp->prim_count += (sp->prim - start) / 10;
}

ADDRESS(0x80018ec4, 0x40)
void func_80018ec4(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x80018f04, 0x44)
void func_80018f04(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x80018f48, 0x38c)
void func_80018f48(int count, MeshFaceGT4 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    void (*tint)(CVECTOR *);
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int t;
    MeshFaceGT4 *next;
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = func_80018ec4;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = func_80018f04;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->v0);
        v1 = (MeshVertex *)(sp->vertices + face->v1);
        v2 = (MeshVertex *)(sp->vertices + face->v2);
        gte_ldsxy3(v0->sxy, v1->sxy, v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x3c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->v3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_GT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_ncdt_b();
                *(u_short *)&p->u2 = face->uv2;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_ldrgb(&sp->rgb);
                gte_lddp(v0->p);
                *(u_short *)&p->u3 = face->uv3;
                gte_ncds_b();
                mesh_add_prim(ot + z, p, 12);
                gte_strgb(&p->r3);
                if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                    tint((CVECTOR *)&p->r0);
                    tint((CVECTOR *)&p->r1);
                    tint((CVECTOR *)&p->r2);
                    tint((CVECTOR *)&p->r3);
                }
            }
        }
        face = next;
    }
    sp->faces = face;
    /* 12 words per primitive, although a POLY_GT4 is 13. */
    sp->prim_count += (sp->prim - start) / 12;
}

ADDRESS(0x800192d4, 0x1f0)
void func_800192d4(u_short part, u_int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    int mode;

    mode = (flags >> 1) & 2;
    mode |= (flags & 3) << 21;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->rgb.r = sp->rgb.g = sp->rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    sp->faces = sp->model + 12 + sp->header->faces;
    func_800184c4(sp->header->n_ft3, sp->faces, sp->ot, mode);
    func_80018828(sp->header->n_ft4, sp->faces, sp->ot, mode);
    func_80018ba0(sp->header->n_gt3, sp->faces, sp->ot, mode);
    func_80018f48(sp->header->n_gt4, sp->faces, sp->ot, mode);
}
