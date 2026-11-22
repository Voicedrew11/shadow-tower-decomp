/*
 * VS.EXE: the mesh renderers and drawing one model part (WIP module). Each
 * set draws VS's untextured flat and Gouraud faces (vs/mesh.h) and ST.EXE's
 * textured ones without a mode argument: depth-cued (NCDS/NCDT) first, then
 * lit without depth cueing (NCCS/NCCT); the last textured four, ST.EXE's
 * src/st/mesh_draw_nccs.c renderers, add VS's semi-transparency, mirroring
 * and fixed-depth switches.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>
#include <vs/data.h>
#include <vs/mesh.h>
#include <vs/scratch.h>
#include <vs/funcs.h>

ADDRESS(0x800258f0, 0x1f0)
void mesh_draw_f3(int count, VsFaceF3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_F3 *p;
    VsFaceF3 *next;
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
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_F3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_f3(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&face->rgb);
                gte_lddp((v0->p + v1->p + v2->p) / 3);
                mesh_set_tag(ot + z, p, 4);
                gte_ncds_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 5;
}

ADDRESS(0x80025ae0, 0x1ec)
void mesh_draw_f4(int count, VsFaceF4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_F4 *p;
    VsFaceF4 *next;
    int t;
    int z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = (v0->z + v1->z + v2->z + v3->z) >> 7;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_F4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_f4(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&face->rgb);
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                mesh_set_tag(ot + z, p, 5);
                gte_ncds_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 6;
}

ADDRESS(0x80025ccc, 0x1f4)
void mesh_draw_g3(int count, VsFaceG3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_G3 *p;
    VsFaceG3 *next;
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
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            z = ((v0->z + v1->z + v2->z) >> 7) / 3;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_G3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_g3(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&face->rgb);
                gte_lddp((u_short)v0->p);
                mesh_set_tag(ot + z, p, 6);
                gte_ncdt_b();
                mesh_set_ot(ot + z, p);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 7;
}

ADDRESS(0x80025ec0, 0x238)
void mesh_draw_g4(int count, VsFaceG4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_G4 *p;
    VsFaceG4 *next;
    int t;
    int z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = (v0->z + v1->z + v2->z + v3->z) >> 7;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_G4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_g4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&face->rgb);
                gte_lddp((u_short)v0->p);
                mesh_set_tag(ot + z, p, 8);
                gte_ncdt_b();
                mesh_set_ot(ot + z, p);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_ldrgb(&face->rgb);
                gte_lddp((u_short)v0->p);
                gte_ncds();
                gte_strgb(&p->r3);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 9;
}

ADDRESS(0x800260f8, 0x22c)
void mesh_draw_ft3(int count, MeshFaceFT3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        sp->rgb.cd = face->code | 0x24;
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_FT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft3(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_lddp((v0->p + v1->p + v2->p) / 3);
                *(u_short *)&p->u2 = face->uv2;
                gte_ncds_b();
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 8;
}

ADDRESS(0x80026324, 0x248)
void mesh_draw_ft4(int count, MeshFaceFT4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        sp->rgb.cd = face->code | 0x2c;
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_FT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft4(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_ncds_b();
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

ADDRESS(0x8002656c, 0x25c)
void mesh_draw_gt3(int count, MeshFaceGT3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        sp->rgb.cd = face->code | 0x34;
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_GT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt3(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_lddp((v0->p * 2 + v1->p + v2->p) >> 2);
                *(u_short *)&p->u2 = face->uv2;
                gte_ncdt_b();
                mesh_add_prim(ot + z, p, 9);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 10;
}

ADDRESS(0x800267c8, 0x2c0)
void mesh_draw_gt4(int count, MeshFaceGT4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        sp->rgb.cd = face->code | 0x3c;
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_GT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_short *)&p->u0 = *(u_short *)&face->uv0_clut;
                gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                *(u_short *)&p->u1 = *(u_short *)&face->uv1_tpage;
                gte_ncdt_b();
                *(u_short *)&p->u2 = face->uv2;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_ldrgb(&sp->rgb);
                gte_lddp((u_short)v0->p);
                *(u_short *)&p->u3 = face->uv3;
                gte_ncds_b();
                mesh_add_prim(ot + z, p, 12);
                gte_strgb(&p->r3);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 12;
}

ADDRESS(0x80026a88, 0x184)
void mesh_draw_f3_nccs(int count, VsFaceF3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_F3 *p;
    VsFaceF3 *next;
    int t;
    int z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            z = (v0->z + v1->z + v2->z) >> 7;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_F3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_f3(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&face->rgb);
                mesh_set_tag(ot + z, p, 4);
                gte_nccs_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 5;
}

ADDRESS(0x80026c0c, 0x1b4)
void mesh_draw_f4_nccs(int count, VsFaceF4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_F4 *p;
    VsFaceF4 *next;
    int t;
    int z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = (v0->z + v1->z + v2->z + v3->z) >> 7;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_F4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_f4(p);
                gte_ldv0(sp->normals + face->normal);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&face->rgb);
                mesh_set_tag(ot + z, p, 5);
                gte_nccs_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 6;
}

ADDRESS(0x80026dc0, 0x1bc)
void mesh_draw_g3_nccs(int count, VsFaceG3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_G3 *p;
    VsFaceG3 *next;
    int t;
    int z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            z = (v0->z * 2 + v1->z + v2->z) >> 7;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_G3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_g3(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&face->rgb);
                mesh_set_tag(ot + z, p, 6);
                gte_ncct_b();
                mesh_set_ot(ot + z, p);
                gte_strgb3(&p->r0, &p->r1, &p->r2);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 7;
}

ADDRESS(0x80026f7c, 0x21c)
void mesh_draw_g4_nccs(int count, VsFaceG4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_G4 *p;
    VsFaceG4 *next;
    int t;
    int z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        gte_stopz(&sp->nclip);
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = (v0->z + v1->z + v2->z + v3->z) >> 7;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_G4 *)sp->prim;
                gte_stsxy3_g4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&face->rgb);
                sp->prim += sizeof(POLY_G4) / 4;
                gte_ncct_b();
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_ldrgb(&face->rgb);
                mesh_set_tag(ot + z, p, 8);
                gte_nccs_b();
                mesh_set_ot(ot + z, p);
                gte_strgb(&p->r3);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 9;
}

ADDRESS(0x80027198, 0x268)
void mesh_draw_ft3_nccs(int count, MeshFaceFT3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        u8 code;

        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        code = face->code;
        sp->rgb.cd = code | 0x24;
        if (mesh_semi_trans)
            sp->rgb.cd = code | 0x26;
        gte_stopz(&sp->nclip);
        if (mesh_mirror)
            sp->nclip = -sp->nclip;
        if (sp->nclip > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if ((z < sp->far_z && z > (short)sp->near_z) || mesh_fixed_depth) {
                p = (POLY_FT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft3(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_nccs_b();
                *(u_long *)&p->u1 = face->uv1_tpage;
                *(u_short *)&p->u2 = face->uv2;
                if (mesh_fixed_depth)
                    z = sp->far_z - mesh_fixed_depth;
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 8;
}

ADDRESS(0x80027400, 0x298)
void mesh_draw_ft4_nccs(int count, MeshFaceFT4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        u8 code;

        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        code = face->code;
        sp->rgb.cd = code | 0x2c;
        if (mesh_semi_trans)
            sp->rgb.cd = code | 0x2e;
        gte_stopz(&sp->nclip);
        if (mesh_mirror)
            sp->nclip = -sp->nclip;
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if ((z < sp->far_z && z > (short)sp->near_z) || mesh_fixed_depth) {
                p = (POLY_FT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft4(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_nccs_b();
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                if (mesh_fixed_depth)
                    z = sp->far_z - mesh_fixed_depth;
                mesh_add_prim(ot + z, p, 9);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count += (sp->prim - start) / 10;
}

ADDRESS(0x80027698, 0x260)
void mesh_draw_gt3_nccs(int count, MeshFaceGT3 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        u8 code;

        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        code = face->code;
        sp->rgb.cd = code | 0x34;
        if (mesh_semi_trans)
            sp->rgb.cd = code | 0x36;
        gte_stopz(&sp->nclip);
        if (mesh_mirror)
            sp->nclip = -sp->nclip;
        if (sp->nclip > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > (short)sp->near_z) {
                gte_stsxy3_gt3(sp->prim);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                p = (POLY_GT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u1 = face->uv1_tpage;
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

ADDRESS(0x800278f8, 0x2ac)
void mesh_draw_gt4_nccs(int count, MeshFaceGT4 *face, u_long *ot)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
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
        u8 code;

        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        next = face + 1;
        gte_nclip();
        code = face->code;
        sp->rgb.cd = code | 0x3c;
        if (mesh_semi_trans)
            sp->rgb.cd = code | 0x3e;
        gte_stopz(&sp->nclip);
        if (mesh_mirror)
            sp->nclip = -sp->nclip;
        if (sp->nclip > 0) {
            v3 = (MeshVertex *)(sp->vertices + face->i3);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > (short)sp->near_z) {
                p = (POLY_GT4 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_gt4(p);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                *(u_long *)&p->x3 = *(u_long *)&v3->sxy;
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_ncct_b();
                *(u_long *)&p->u1 = face->uv1_tpage;
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

/* Draws part `part` of the scratchpad model with the four renderers above
 * (ST.EXE's mesh_draw_part_nccs without its tint and far-colour setup). */
ADDRESS(0x80027ba4, 0xc0)
void mesh_draw_part_nccs(u_short part)
{
    VsScratchPad *sp = VS_SCRATCHPAD;

    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    sp->faces = sp->model + 12 + sp->header->faces;
    mesh_draw_ft3_nccs(sp->header->n_ft3, (MeshFaceFT3 *)sp->faces, sp->ot);
    mesh_draw_ft4_nccs(sp->header->n_ft4, (MeshFaceFT4 *)sp->faces, sp->ot);
    mesh_draw_gt3_nccs(sp->header->n_gt3, (MeshFaceGT3 *)sp->faces, sp->ot);
    mesh_draw_gt4_nccs(sp->header->n_gt4, (MeshFaceGT4 *)sp->faces, sp->ot);
}
