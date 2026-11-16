/*
 * ST.EXE mesh renderers without depth cueing (WIP module): the NCCS/NCCT-lit
 * counterparts of the st.mesh_draw renderers, with no tint callbacks.
 * mesh_draw_part_nccs sets up a model part and runs them.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>

ADDRESS(0x800194c4, 0x1fc)
void mesh_draw_ft3_nccs(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    MeshFaceFT3 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
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
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_nccs_b();
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 8;
}

ADDRESS(0x800196c0, 0x234)
void mesh_draw_ft4_nccs(int count, MeshFaceFT4 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    MeshFaceFT4 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x2c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft4(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_nccs_b();
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 10;
}

ADDRESS(0x800198f4, 0x240)
void mesh_draw_gt3_nccs(int count, MeshFaceGT3 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    MeshFaceGT3 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x34);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                gte_stsxy3_gt3(sp->prim);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                p = (POLY_GT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                gte_ncct_b();
                *(u_short *)&p->u2 = face->uv2;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 10;
}

ADDRESS(0x80019b34, 0x294)
void mesh_draw_gt4_nccs(int count, MeshFaceGT4 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    MeshFaceGT4 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x3c);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_GT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ncct_b();
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                *(u_short *)&p->u3 = face->uv3;
                gte_ldrgb(&sp->rgb);
                mesh_set_tag(ot + z, p, 12);
                gte_nccs_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r3);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    /* 12 words per primitive, although a POLY_GT4 is 13. */
    sp->prim_count += (sp->prim - start) / 12;
}

ADDRESS(0x80019dc8, 0x1ec)
void mesh_draw_part_nccs(u_short part, u_int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    int mode;

    mode = (flags >> 1) & 2;
    mode |= (flags & 3) << 21;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    sp->faces = sp->model + 12 + sp->header->faces;
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
    mesh_draw_ft3_nccs(sp->header->n_ft3, (MeshFaceFT3 *)sp->faces, sp->ot, mode);
    mesh_draw_ft4_nccs(sp->header->n_ft4, (MeshFaceFT4 *)sp->faces, sp->ot, mode);
    mesh_draw_gt3_nccs(sp->header->n_gt3, (MeshFaceGT3 *)sp->faces, sp->ot, mode);
    mesh_draw_gt4_nccs(sp->header->n_gt4, (MeshFaceGT4 *)sp->faces, sp->ot, mode);
}
