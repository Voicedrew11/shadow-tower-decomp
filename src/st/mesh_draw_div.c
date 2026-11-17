/*
 * ST.EXE subdividing mesh renderers (WIP module): like st.mesh_draw, but a
 * face whose NCLIP area is large is drawn through LIBGTE's RCpoly*
 * subdivision (one to three levels by area), and the lighting is NCDS with
 * depth cueing or, when the render mode asks for it, plain NCCS. Each
 * renderer has its own pair of tint callbacks linked before it.
 * mesh_div_draw_part (st.mesh_draw_div_setup) runs them.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>
#include <st/scratch.h>

ADDRESS(0x80019fb4, 0x40)
void mesh_div_ft3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x80019ff4, 0x44)
void mesh_div_ft3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001a038, 0x630)
void mesh_div_draw_ft3(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    MeshDiv div;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    int t;
    MeshFaceFT3 *next;
    void (*tint)(CVECTOR *);
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_ft3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_ft3_tint_b;
    }
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
                if (sp->opz > 0x4000) {
                    div.d3.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.d3.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.d3.ndiv = 3;
                        }
                    }
                    gte_ldv0(sp->normals + face->normal);
                    div.d3.pih = 320;
                    div.d3.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncds();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d3.rgbc);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d3.rgbc);
                        }
                    } else {
                        gte_nccs();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d3.rgbc);
                    }
                    div.d3.ot = ot + z;
                    div.d3.cr[0].r0 = &div.d3.r0;
                    div.d3.cr[0].r1 = &div.d3.r1;
                    div.d3.cr[0].r2 = &div.d3.r2;
                    div.d3.r0.sxy = v0->sxy;
                    div.d3.r1.sxy = v1->sxy;
                    div.d3.r2.sxy = v2->sxy;
                    div.d3.r0.sz = v0->z;
                    div.d3.r1.sz = v1->z;
                    div.d3.r2.sz = v2->z;
                    div.d3.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d3.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d3.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    ((u_long *)&div.d3.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.d3.r1)[2] = face->uv1_tpage | (flags & 0x600000);
                    ((u_long *)&div.d3.r2)[2] = *(u_long *)&face->uv2;
                    sp->prim = RCpolyFT3((POLY_FT3 *)sp->prim, &div.d3);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    p = (POLY_FT3 *)sp->prim;
                    sp->prim = (u_long *)(p + 1);
                    gte_stsxy3_ft3(p);
                    gte_ldv0(sp->normals + face->normal);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncds();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb(&p->r0);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint((CVECTOR *)&p->r0);
                        }
                    } else {
                        gte_nccs();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb(&p->r0);
                    }
                    mesh_add_prim(ot + z, p, 7);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 8;
}

ADDRESS(0x8001a668, 0x40)
void mesh_div_ft4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001a6a8, 0x44)
void mesh_div_ft4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001a6ec, 0x70c)
void mesh_div_draw_ft4(int count, MeshFaceFT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    MeshDiv div;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    int t;
    MeshFaceFT4 *next;
    void (*tint)(CVECTOR *);
    short z;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_ft4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_ft4_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        gte_nclip();
        p = (POLY_FT4 *)sp->prim;
        gte_stopz(&sp->opz);
        gte_stsxy3_ft4(p);
        v3 = (MeshVertex *)(sp->vertices + face->i3);
        gte_ldsxy0(*(u_long *)&v3->sxy);
        gte_nclip();
        next = face + 1;
        gte_stopz(&sp->opz2);
        if (sp->opz > 0 || sp->opz2 < 0) {
            gte_stsxy0(&p->x3);
            sp->rgb.cd = face->code | ((flags & 2) | 0x2c);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.d4.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.d4.ndiv = 2;
                        if (sp->opz > 0x10000 || sp->opz2 < -0x10000) {
                            div.d4.ndiv = 3;
                        }
                    }
                    gte_ldv0(sp->normals + face->normal);
                    div.d4.pih = 320;
                    div.d4.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncds();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d4.rgbc);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d4.rgbc);
                        }
                    } else {
                        gte_nccs();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb(&div.d4.rgbc);
                    }
                    div.d4.ot = ot + z;
                    div.d4.cr[0].r0 = &div.d4.r0;
                    div.d4.cr[0].r1 = &div.d4.r1;
                    div.d4.cr[0].r2 = &div.d4.r2;
                    div.d4.cr[0].r3 = &div.d4.r3;
                    div.d4.r0.sxy = v0->sxy;
                    div.d4.r1.sxy = v1->sxy;
                    div.d4.r2.sxy = v2->sxy;
                    div.d4.r3.sxy = v3->sxy;
                    div.d4.r0.sz = v0->z;
                    div.d4.r1.sz = v1->z;
                    div.d4.r2.sz = v2->z;
                    div.d4.r3.sz = v3->z;
                    div.d4.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d4.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d4.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    div.d4.r3.v = *(SVECTOR *)(sp->local_vertices + face->i3);
                    *(u_long *)div.d4.r0.uv = face->uv0_clut;
                    *(u_long *)div.d4.r1.uv = face->uv1_tpage | (flags & 0x600000);
                    *(u_long *)div.d4.r2.uv = *(u_long *)&face->uv2;
                    *(u_long *)div.d4.r3.uv = *(u_long *)&face->uv3;
                    sp->prim = RCpolyFT4((POLY_FT4 *)sp->prim, &div.d4);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    sp->prim = (u_long *)((POLY_FT4 *)sp->prim + 1);
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->rgb);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncds();
                        *(u_short *)&p->u2 = face->uv2;
                        *(u_short *)&p->u3 = face->uv3;
                        gte_strgb(&p->r0);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint((CVECTOR *)&p->r0);
                        }
                    } else {
                        gte_nccs();
                        *(u_short *)&p->u2 = face->uv2;
                        *(u_short *)&p->u3 = face->uv3;
                        gte_strgb(&p->r0);
                    }
                    mesh_add_prim(ot + z, p, 9);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x8001adf8, 0x40)
void mesh_div_gt3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001ae38, 0x44)
void mesh_div_gt3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001ae7c, 0x6dc)
void mesh_div_draw_gt3(int count, MeshFaceGT3 *face, u_long *ot, int flags)
{
    POLY_GT3 *p;
    int t;
    u_long *start;
    short z;
    MeshVertex *v0;
    MeshVertex *v2;
    MeshVertex *v1;
    void (*tint)(CVECTOR *);
    MeshDiv div;
    MeshFaceGT3 *next;
    ScratchPad *sp = SCRATCHPAD;

    tint = 0;
    start = sp->prim;
    sp->face_count += count;
    if (MESH_MODE & MESH_TINT_A) {
        tint = mesh_div_gt3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_gt3_tint_b;
    }
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
                if (sp->opz > 0x4000) {
                    div.d3.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.d3.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.d3.ndiv = 3;
                        }
                    }
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    div.d3.pih = 320;
                    div.d3.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncdt();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d3.r0.c, &div.d3.r1.c, &div.d3.r2.c);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d3.r0.c);
                            tint(&div.d3.r1.c);
                            tint(&div.d3.r2.c);
                        }
                    } else {
                        gte_ncct();
                        div.d3.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d3.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d3.r0.c, &div.d3.r1.c, &div.d3.r2.c);
                    }
                    div.d3.rgbc = sp->rgb;
                    div.d3.ot = ot + z;
                    div.d3.cr[0].r0 = &div.d3.r0;
                    div.d3.cr[0].r1 = &div.d3.r1;
                    div.d3.cr[0].r2 = &div.d3.r2;
                    div.d3.r0.sxy = v0->sxy;
                    div.d3.r1.sxy = v1->sxy;
                    div.d3.r2.sxy = v2->sxy;
                    div.d3.r0.sz = v0->z;
                    div.d3.r1.sz = v1->z;
                    div.d3.r2.sz = v2->z;
                    div.d3.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d3.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d3.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    ((u_long *)&div.d3.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.d3.r1)[2] = face->uv1_tpage | (flags & 0x600000);
                    ((u_long *)&div.d3.r2)[2] = *(u_long *)&face->uv2;
                    sp->prim = RCpolyGT3((POLY_GT3 *)sp->prim, &div.d3);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    p = (POLY_GT3 *)sp->prim;
                    sp->prim = (u_long *)(p + 1);
                    gte_stsxy3_gt3(p);
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p) / 3);
                        gte_ncdt();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint((CVECTOR *)&p->r0);
                            tint((CVECTOR *)&p->r1);
                            tint((CVECTOR *)&p->r2);
                        }
                    } else {
                        gte_ncct();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                    }
                    mesh_add_prim(ot + z, p, 9);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    sp->prim_count.count += (sp->prim - start) / 10;
}

ADDRESS(0x8001b558, 0x40)
void mesh_div_gt4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001b598, 0x44)
void mesh_div_gt4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

/* The tint callbacks take only the colour; this renderer also passes each
 * vertex's depth, through an unprototyped pointer. */
/*
 * Not exact: at the loop latch retail reloads the spilled face cursor
 * (next) before the spilled count and leaves a load-delay nop. This build
 * fills the count's load delay with the face reload, making it four bytes
 * short; every other instruction agrees.
 */
ADDRESS(0x8001b5dc, 0x934)
void mesh_div_draw_gt4(int count, MeshFaceGT4 *face, u_long *ot, int flags)
{
    ScratchPad *sp = SCRATCHPAD;
    MeshDiv div;
    u_long *start;
    void (*tint)();
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
        tint = mesh_div_gt4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = mesh_div_gt4_tint_b;
    }
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->i0);
        v1 = (MeshVertex *)(sp->vertices + face->i1);
        v2 = (MeshVertex *)(sp->vertices + face->i2);
        gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
        gte_nclip();
        p = (POLY_GT4 *)sp->prim;
        gte_stopz(&sp->opz);
        gte_stsxy3_gt4(p);
        v3 = (MeshVertex *)(sp->vertices + face->i3);
        gte_ldsxy0(*(u_long *)&v3->sxy);
        gte_nclip();
        next = face + 1;
        gte_stopz(&sp->opz2);
        if (sp->opz > 0 || sp->opz2 < 0) {
            gte_stsxy0(&p->x3);
            sp->rgb.cd = face->code | ((flags & 2) | 0x3c);
            z = ((v0->z + v1->z + v2->z + v3->z) >> 7) + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.d4.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.d4.ndiv = 2;
                        if (sp->opz > 0x10000 || sp->opz2 < -0x10000) {
                            div.d4.ndiv = 3;
                        }
                    }
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    div.d4.pih = 320;
                    div.d4.piv = 240;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncdt();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d4.r0.c, &div.d4.r1.c, &div.d4.r2.c);
                        gte_ldv0(sp->normals + face->n3);
                        gte_ncds();
                        div.d4.rgbc = sp->rgb;
                        div.d4.ot = ot + z;
                        gte_strgb(&div.d4.r3.c);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&div.d4.r0.c, v0->z);
                            tint(&div.d4.r1.c, v1->z);
                            tint(&div.d4.r2.c, v2->z);
                            tint(&div.d4.r3.c, v3->z);
                        }
                    } else {
                        gte_ncct();
                        div.d4.clut = MESH_WORD_HI(face->uv0_clut) + sp->clut_base;
                        div.d4.tpage = MESH_WORD_HI(face->uv1_tpage) | ((flags >> 16) & 0x60);
                        gte_strgb3(&div.d4.r0.c, &div.d4.r1.c, &div.d4.r2.c);
                        gte_ldv0(sp->normals + face->n3);
                        gte_nccs();
                        div.d4.rgbc = sp->rgb;
                        div.d4.ot = ot + z;
                        gte_strgb(&div.d4.r3.c);
                    }
                    div.d4.cr[0].r0 = &div.d4.r0;
                    div.d4.cr[0].r1 = &div.d4.r1;
                    div.d4.cr[0].r2 = &div.d4.r2;
                    div.d4.cr[0].r3 = &div.d4.r3;
                    div.d4.r0.sxy = v0->sxy;
                    div.d4.r1.sxy = v1->sxy;
                    div.d4.r2.sxy = v2->sxy;
                    div.d4.r3.sxy = v3->sxy;
                    div.d4.r0.sz = v0->z;
                    div.d4.r1.sz = v1->z;
                    div.d4.r2.sz = v2->z;
                    div.d4.r3.sz = v3->z;
                    div.d4.r0.v = *(SVECTOR *)(sp->local_vertices + face->i0);
                    div.d4.r1.v = *(SVECTOR *)(sp->local_vertices + face->i1);
                    div.d4.r2.v = *(SVECTOR *)(sp->local_vertices + face->i2);
                    div.d4.r3.v = *(SVECTOR *)(sp->local_vertices + face->i3);
                    *(u_long *)div.d4.r0.uv = face->uv0_clut;
                    *(u_long *)div.d4.r1.uv = face->uv1_tpage | (flags & 0x600000);
                    *(u_long *)div.d4.r2.uv = *(u_long *)&face->uv2;
                    *(u_long *)div.d4.r3.uv = *(u_long *)&face->uv3;
                    sp->prim = RCpolyGT4((POLY_GT4 *)sp->prim, &div.d4);
                    if (sp->prim_end < sp->prim) {
                        return;
                    }
                } else {
                    gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ldrgb(&sp->rgb);
                    if (!(MESH_MODE & MESH_NO_DEPTH_CUE)) {
                        gte_lddp((v0->p + v1->p + v2->p + v3->p) >> 2);
                        gte_ncdt();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                        gte_ldv0(sp->normals + face->n3);
                        gte_ncds();
                        *(u_short *)&p->u3 = face->uv3;
                        mesh_add_prim(ot + z, p, 12);
                        gte_strgb(&p->r3);
                        if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                            tint(&p->r0, v0->z);
                            tint(&p->r1, v1->z);
                            tint(&p->r2, v2->z);
                            tint(&p->r3, v3->z);
                        }
                    } else {
                        gte_ncct();
                        *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                        *(u_short *)&p->u2 = face->uv2;
                        gte_strgb3(&p->r0, &p->r1, &p->r2);
                        gte_ldv0(sp->normals + face->n3);
                        gte_nccs();
                        *(u_short *)&p->u3 = face->uv3;
                        mesh_add_prim(ot + z, p, 12);
                        gte_strgb(&p->r3);
                    }
                    sp->prim = (u_long *)((POLY_GT4 *)sp->prim + 1);
                }
            }
        }
        face = next;
    }
    sp->faces = (u8 *)face;
    /* 12 words per primitive, although a POLY_GT4 is 13. */
    sp->prim_count.count += (sp->prim - start) / 12;
}
