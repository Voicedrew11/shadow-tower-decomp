/*
 * ST.EXE world renderers (WIP module): world_draw transforms every visible
 * record's model part and runs the renderers below over the visible list.
 * A record flagged near by vis_cull_by_range is drawn with depth-cued
 * Gouraud primitives and RCpoly* subdivision (and the render mode's tint);
 * the others with flat NCCS primitives. Each renderer handles one face kind
 * and advances the shared transformed-vertex cursor (ScratchPad.unk34) past
 * every record's part.
 *
 * In this phase the scratchpad's 0x80 word (near_z for st.mesh_draw*) holds
 * a face's nearest vertex depth and the 0x8c word (face_count there) is the
 * near OT limit.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <RAND.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/vis.h>

/* A transformed vertex of the current record's part. */
#define PART_VERTEX(sp, i) ((MeshVertex *)((u8 *)(sp)->unk34 + (i)))


/*
 * Projects each visible record's part vertices (RTPT in threes, then RTPS)
 * into the shared transformed-vertex buffer, with a depth-cue value from
 * the lighting's near/far depths (ScratchPad.unk64/unk68).
 */
/*
 * Not exact (same size, same instructions): retail keeps the unk38 copy
 * (`lw v0,52(t0)` ... `sw v0,56(t0)`) behind the two vertex pointer stores;
 * here sched1 lifts it above the header->vertices load, which swaps v0/v1
 * in between. In the sched1 trace the copy's store has priority 4 (a load
 * at depth 3 plus its latency) against 5 for the vertex stores (their
 * chain starts with the rec->part load), so it is placed after them in
 * the backward pass; retail needs it at 5 or tied. Ruled out: every order
 * of the five header statements, scratch variables for the copied value,
 * `*(T *)&field` spellings, moving the n_vertices read ahead of the copy.
 */
ADDRESS(0x8001cdf8, 0x42c)
void world_project_parts(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    MeshVertex *out;
    SVECTOR *in;
    int i;
    int n;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        sp->light_index = rec->unk03 & 3;
        gte_SetRotMatrix(&world_state.unk1ED28[sp->light_index]);
        gte_SetTransVector(&trans[i]);
        sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
        sp->vertices = sp->part_vertices = sp->model + 12 + sp->header->vertices;
        sp->unk38 = sp->unk34;
        n = sp->header->n_vertices / 3;
        out = (MeshVertex *)sp->unk34;
        in = (SVECTOR *)sp->part_vertices;
        while (n--) {
            gte_ldv3c(in);
            gte_rtpt();
            gte_stsxy3(&out[0].sxy, &out[1].sxy, &out[2].sxy);
            gte_stsz3(&out[0].z, &out[1].z, &out[2].z);
            if (sp->unk64 != sp->unk68) {
                out[0].p = ((out[0].z - sp->unk64) << 12) / (sp->unk68 - sp->unk64);
                out[1].p = ((out[1].z - sp->unk64) << 12) / (sp->unk68 - sp->unk64);
                out[2].p = ((out[2].z - sp->unk64) << 12) / (sp->unk68 - sp->unk64);
            } else {
                out[0].p = 0;
                out[1].p = 0;
                out[2].p = 0;
            }
            if (out[0].p < 0) {
                out[0].p = 0;
            }
            if (out[1].p < 0) {
                out[1].p = 0;
            }
            if (out[2].p < 0) {
                out[2].p = 0;
            }
            out += 3;
            in += 3;
        }
        n = sp->header->n_vertices % 3;
        while (n--) {
            gte_ldv0(in);
            gte_rtps();
            gte_stsxy(&out->sxy);
            gte_stsz(&out->z);
            if (sp->unk64 != sp->unk68) {
                out->p = ((out->z - sp->unk64) << 12) / (sp->unk68 - sp->unk64);
            } else {
                out->p = 0;
            }
            if (out->p > 4065) {
                out->p = 4065;
            }
            if (out->p < 0) {
                out->p = 0;
            }
            out++;
            in++;
        }
        sp->part_vertex_count += sp->header->n_vertices;
        sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
    }
}

ADDRESS(0x8001d224, 0x40)
void world_ft3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001d264, 0x44)
void world_ft3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001d2a8, 0x838)
void world_draw_ft3_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON3 div;
    void (*tint)(CVECTOR *);
    VisRecord *rec;
    MeshFaceFT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int i;
    int n;
    int t;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_ft3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_ft3_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->unk04 & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT3 *)(sp->model + 12 + sp->header->faces);
            n = sp->header->n_ft3;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x34;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_gt3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.ndiv = 3;
                        }
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    gte_strgb(&div.r0.c);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    gte_strgb(&div.r1.c);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    gte_strgb(&div.r2.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                    }
                    gte_SetRotMatrix(&world_state.unk1ED28[sp->light_index]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    face++;
                    sp->prim = RCpolyGT3((POLY_GT3 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT3 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    *(u_long *)&p->u0 = face->uv0_clut;
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 9);
                    gte_strgb(&p->r1);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r2);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                    }
                    face++;
                }
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001dae0, 0x40)
void world_ft4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001db20, 0x44)
void world_ft4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001db64, 0x9b8)
void world_draw_ft4_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON4 div;
    void (*tint)(CVECTOR *);
    VisRecord *rec;
    MeshFaceFT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int i;
    int n;
    int t;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_ft4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_ft4_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->unk04 & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceFT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            n = sp->header->n_ft4;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x3c;
                gte_stopz(&sp->opz);
                v3 = PART_VERTEX(sp, face->i3);
                gte_stsxy3_gt4(sp->prim);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_GT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.ndiv = 2;
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    gte_strgb(&div.r0.c);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    gte_strgb(&div.r1.c);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    gte_strgb(&div.r2.c);
                    gte_lddp(v3->p);
                    gte_dpcs();
                    gte_strgb(&div.r3.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                        tint(&div.r3.c);
                    }
                    gte_SetRotMatrix(&world_state.unk1ED28[sp->light_index]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.cr[0].r3 = &div.r3;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r3.sxy = v3->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r3.sz = v3->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    div.r3.v = *(SVECTOR *)(sp->part_vertices + face->i3);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    ((u_long *)&div.r3)[2] = *(u_long *)&face->uv3;
                    face++;
                    sp->prim = RCpolyGT4((POLY_GT4 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT4 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldv0(sp->normals + face->normal);
                    gte_ldrgb(&sp->part_rgb);
                    gte_nccs();
                    *(u_long *)&p->u0 = face->uv0_clut;
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    gte_strgb(&sp->unk2C);
                    gte_ldrgb(&sp->unk2C);
                    gte_lddp(v0->p);
                    gte_dpcs();
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_lddp(v1->p);
                    gte_dpcs();
                    *(u_short *)&p->u3 = face->uv3;
                    gte_strgb(&p->r1);
                    gte_lddp(v2->p);
                    gte_dpcs();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 12);
                    gte_strgb(&p->r2);
                    gte_lddp(v3->p);
                    gte_dpcs();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r3);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                        tint((CVECTOR *)&p->r3);
                    }
                    face++;
                }
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001e51c, 0x40)
void world_gt3_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001e55c, 0x44)
void world_gt3_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001e5a0, 0x888)
void world_draw_gt3_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON3 div;
    void (*tint)(CVECTOR *);
    VisRecord *rec;
    MeshFaceGT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int i;
    int n;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_gt3_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_gt3_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->unk04 & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT3 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            n = sp->header->n_gt3;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x34;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_gt3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000) {
                        div.ndiv = 2;
                        if (sp->opz > 0x10000) {
                            div.ndiv = 3;
                        }
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    gte_ncds();
                    gte_strgb(&div.r0.c);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    gte_strgb(&div.r1.c);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    gte_strgb(&div.r2.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                    }
                    gte_SetRotMatrix(&world_state.unk1ED28[sp->light_index]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    face++;
                    sp->prim = RCpolyGT3((POLY_GT3 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT3 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ncds_b();
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 9);
                    gte_strgb(&p->r1);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r2);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                    }
                    face++;
                }
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001ee28, 0x40)
void world_gt4_tint_a(CVECTOR *rgb)
{
    if (rgb->b > 85) {
        rgb->b = 85;
    }
    rgb->r = rgb->b * 3;
    rgb->g = rgb->b * 2;
}

ADDRESS(0x8001ee68, 0x44)
void world_gt4_tint_b(CVECTOR *rgb)
{
    if (rgb->r > 85) {
        rgb->r = 85;
    }
    rgb->b = rgb->r * 3;
    rgb->g = rgb->r * 3;
}

ADDRESS(0x8001eeac, 0xaa8)
void world_draw_gt4_near(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    DIVPOLYGON4 div;
    VisRecord *rec;
    MeshFaceGT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int i;
    void (*tint)(CVECTOR *);
    int n;

    tint = 0;
    i = vis_lists.n_visible;
    if (MESH_MODE & MESH_TINT_A) {
        tint = world_gt4_tint_a;
    } else if (MESH_MODE & MESH_TINT_B) {
        tint = world_gt4_tint_b;
    }
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (!(rec->unk04 & 3)) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_gt3 * sizeof(MeshFaceGT3));
            n = sp->header->n_gt4;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x3c;
                gte_stopz(&sp->opz);
                gte_stsxy3_gt4(sp->prim);
                v3 = PART_VERTEX(sp, face->i3);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_GT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                if (sp->opz > 0x4000 || sp->opz2 < -0x4000) {
                    div.ndiv = 1;
                    if (sp->opz > 0x8000 || sp->opz2 < -0x8000) {
                        div.ndiv = 2;
                        if (sp->opz > 0x10000 || sp->opz2 < -0x10000) {
                            div.ndiv = 3;
                        }
                    }
                    div.pih = 320;
                    div.piv = 240;
                    div.clut = MESH_WORD_HI(face->uv0_clut);
                    div.tpage = MESH_WORD_HI(face->uv1_tpage);
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    gte_ncds();
                    gte_strgb(&div.r0.c);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    gte_strgb(&div.r1.c);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    gte_strgb(&div.r2.c);
                    gte_ldv0(sp->normals + face->n3);
                    gte_lddp(v3->p);
                    gte_ncds();
                    gte_strgb(&div.r3.c);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint(&div.r0.c);
                        tint(&div.r1.c);
                        tint(&div.r2.c);
                        tint(&div.r3.c);
                    }
                    gte_SetRotMatrix(&world_state.unk1ED28[sp->light_index]);
                    gte_SetTransVector(&trans[i]);
                    div.rgbc = sp->part_rgb;
                    div.cr[0].r0 = &div.r0;
                    div.cr[0].r1 = &div.r1;
                    div.cr[0].r2 = &div.r2;
                    div.cr[0].r3 = &div.r3;
                    div.ot = sp->ot + sp->z_ot;
                    div.r0.sxy = v0->sxy;
                    div.r1.sxy = v1->sxy;
                    div.r2.sxy = v2->sxy;
                    div.r3.sxy = v3->sxy;
                    div.r0.sz = v0->z;
                    div.r1.sz = v1->z;
                    div.r2.sz = v2->z;
                    div.r3.sz = v3->z;
                    div.r0.v = *(SVECTOR *)(sp->part_vertices + face->i0);
                    div.r1.v = *(SVECTOR *)(sp->part_vertices + face->i1);
                    div.r2.v = *(SVECTOR *)(sp->part_vertices + face->i2);
                    div.r3.v = *(SVECTOR *)(sp->part_vertices + face->i3);
                    ((u_long *)&div.r0)[2] = face->uv0_clut;
                    ((u_long *)&div.r1)[2] = face->uv1_tpage;
                    ((u_long *)&div.r2)[2] = *(u_long *)&face->uv2;
                    ((u_long *)&div.r3)[2] = *(u_long *)&face->uv3;
                    face++;
                    sp->prim = RCpolyGT4((POLY_GT4 *)sp->prim, &div);
                    sp->part_div_prims += (1 << div.ndiv) << div.ndiv;
                } else {
                    p = (POLY_GT4 *)sp->prim;
                    if (sp->prim_end < (u_long *)(p + 1)) {
                        return;
                    }
                    sp->prim = (u_long *)(p + 1);
                    sp->part_prims++;
                    gte_ldrgb(&sp->part_rgb);
                    gte_ldv0(sp->normals + face->n0);
                    gte_lddp(v0->p);
                    *(u_long *)&p->u0 = face->uv0_clut;
                    gte_ncds_b();
                    *(u_long *)&p->u1 = face->uv1_tpage;
                    *(u_short *)&p->u2 = face->uv2;
                    gte_strgb(&p->r0);
                    gte_ldv0(sp->normals + face->n1);
                    gte_lddp(v1->p);
                    gte_ncds();
                    *(u_short *)&p->u3 = face->uv3;
                    gte_strgb(&p->r1);
                    gte_ldv0(sp->normals + face->n2);
                    gte_lddp(v2->p);
                    gte_ncds();
                    mesh_set_tag(sp->ot + sp->z_ot, p, 12);
                    gte_strgb(&p->r2);
                    gte_ldv0(sp->normals + face->n3);
                    gte_lddp(v3->p);
                    gte_ncds();
                    mesh_set_ot(sp->ot + sp->z_ot, p);
                    gte_strgb(&p->r3);
                    if (MESH_MODE & (MESH_TINT_A | MESH_TINT_B)) {
                        tint((CVECTOR *)&p->r0);
                        tint((CVECTOR *)&p->r1);
                        tint((CVECTOR *)&p->r2);
                        tint((CVECTOR *)&p->r3);
                    }
                    face++;
                }
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001f954, 0x3ac)
void world_draw_ft3_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    MeshFaceFT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    int i;
    int n;
    int t;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->unk04 & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT3 *)(sp->model + 12 + sp->header->faces);
            n = sp->header->n_ft3;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x24;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_ft3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_FT3 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->part_rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_nccs_b();
                *(u_short *)&p->u2 = face->uv2;
                mesh_set_tag(sp->ot + sp->z_ot, p, 7);
                gte_strgb(&p->r0);
                mesh_set_ot(sp->ot + sp->z_ot, p);
                face++;
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x8001fd00, 0x464)
void world_draw_ft4_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    MeshFaceFT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_FT4 *p;
    int i;
    int n;
    int t;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->unk04 & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceFT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceFT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            n = sp->header->n_ft4;
            sp->part_faces += n;
            while ((t = n, n = n - 1, t)) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x2c;
                gte_stopz(&sp->opz);
                v3 = PART_VERTEX(sp, face->i3);
                gte_stsxy3_ft4(sp->prim);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_FT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_FT4 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                p->clut = MESH_WORD_HI(face->uv0_clut);
                p->tpage = MESH_WORD_HI(face->uv1_tpage);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->part_rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                gte_nccs_b();
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                gte_strgb(&p->r0);
                face++;
                mesh_add_prim(sp->ot + sp->z_ot, p, 9);
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x80020164, 0x424)
void world_draw_gt3_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    MeshFaceGT3 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_GT3 *p;
    int i;
    int n;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->unk04 & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT3 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT3 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            n = sp->header->n_gt3;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x34;
                gte_stopz(&sp->opz);
                if (sp->opz <= 0) {
                    face++;
                    continue;
                }
                gte_stsxy3_gt3(sp->prim);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_GT3 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                p->clut = MESH_WORD_HI(face->uv0_clut);
                p->tpage = MESH_WORD_HI(face->uv1_tpage);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&sp->part_rgb);
                gte_ncct();
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                *(u_short *)&p->u2 = face->uv2;
                face++;
                mesh_add_prim(sp->ot + sp->z_ot, p, 9);
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

ADDRESS(0x80020588, 0x4f8)
void world_draw_gt4_far(VECTOR *trans)
{
    ScratchPad *sp = SCRATCHPAD;
    VisRecord *rec;
    MeshFaceGT4 *face;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    MeshVertex *v3;
    POLY_GT4 *p;
    int i;
    int n;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->unk04 & 3) {
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        } else {
            sp->light_index = rec->unk03 & 3;
            gte_SetLightMatrix(&sp->light->light[sp->light_index]);
            sp->header = (MeshHeader *)(sp->model + 12) + rec->part;
            sp->part_vertices = sp->model + 12 + sp->header->vertices;
            sp->normals = sp->model + 12 + sp->header->normals;
            face = (MeshFaceGT4 *)(sp->model + 12 + sp->header->faces);
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft3 * sizeof(MeshFaceFT3));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_ft4 * sizeof(MeshFaceFT4));
            face = (MeshFaceGT4 *)((u8 *)face + sp->header->n_gt3 * sizeof(MeshFaceGT3));
            n = sp->header->n_gt4;
            sp->faces_left = n;
            sp->part_faces += n;
            while (--sp->faces_left != -1) {
                v0 = PART_VERTEX(sp, face->i0);
                v1 = PART_VERTEX(sp, face->i1);
                v2 = PART_VERTEX(sp, face->i2);
                gte_ldsxy3(*(u_long *)&v0->sxy, *(u_long *)&v1->sxy, *(u_long *)&v2->sxy);
                gte_nclip();
                sp->part_rgb.cd = face->code | 0x3c;
                gte_stopz(&sp->opz);
                v3 = PART_VERTEX(sp, face->i3);
                gte_stsxy3_gt4(sp->prim);
                gte_ldsxy0(*(u_long *)&v3->sxy);
                gte_nclip();
                gte_stopz(&sp->opz2);
                if (sp->opz <= 0 && sp->opz2 >= 0) {
                    face++;
                    continue;
                }
                gte_stsxy0(&((POLY_GT4 *)sp->prim)->x3);
                sp->near_z = sp->z_max = v0->z;
                if (sp->z_max < v1->z) {
                    sp->z_max = v1->z;
                }
                if (v1->z < sp->near_z) {
                    sp->near_z = v1->z;
                }
                if (sp->z_max < v2->z) {
                    sp->z_max = v2->z;
                }
                if (v2->z < sp->near_z) {
                    sp->near_z = v2->z;
                }
                if (sp->z_max < v3->z) {
                    sp->z_max = v3->z;
                }
                if (v3->z < sp->near_z) {
                    sp->near_z = v3->z;
                }
                sp->z_ot = (sp->z_max >> 5) + face->depth_bias;
                if (sp->z_ot >= 4096 || sp->z_ot < sp->face_count) {
                    face++;
                    continue;
                }
                p = (POLY_GT4 *)sp->prim;
                if (sp->prim_end < (u_long *)(p + 1)) {
                    return;
                }
                sp->prim = (u_long *)(p + 1);
                sp->part_prims++;
                p->clut = MESH_WORD_HI(face->uv0_clut);
                p->tpage = MESH_WORD_HI(face->uv1_tpage);
                gte_ldv3(sp->normals + face->n0, sp->normals + face->n1, sp->normals + face->n2);
                gte_ldrgb(&sp->part_rgb);
                gte_ncct();
                gte_strgb3(&p->r0, &p->r1, &p->r2);
                gte_ldv0(sp->normals + face->n3);
                gte_nccs();
                gte_strgb(&p->r3);
                *(u_long *)&p->u0 = face->uv0_clut;
                *(u_long *)&p->u1 = face->uv1_tpage;
                *(u_short *)&p->u2 = face->uv2;
                *(u_short *)&p->u3 = face->uv3;
                face++;
                /* 12 words, although a POLY_GT4 is 13. */
                mesh_add_prim(sp->ot + sp->z_ot, p, 12);
            }
            sp->unk34 = (u_long *)((u8 *)sp->unk34 + sp->header->n_vertices * 8);
        }
    }
}

/*
 * Draws the world: sets up the scratchpad for the visible records, builds
 * and culls the visible list, transforms every record's part, then runs
 * the far and near renderers and saves the counters and the packet cursor
 * back.
 *
 * The 0x90/0x92 halves (st.mesh_draw*'s prim_count) hold 240 and 320 here.
 */
ADDRESS(0x80020a80, 0x428)
void world_draw(void)
{
    ScratchPad *sp = SCRATCHPAD;
    VECTOR trans[256];

    if (world_state.unk1EDF4) {
        sp->model = world_state.unk8308[1];
    } else {
        sp->model = world_state.unk8308[0];
    }
    sp->face_count = 4;
    sp->far_z = 4096;
    sp->ot = world_state.ot_cur;
    sp->prim = world_state.packet->cur;
    sp->prim_end = world_state.packet->end;
    sp->prim_count.halves[1] = 320;
    sp->prim_count.halves[0] = 240;
    sp->part_vertex_count = world_state.unk831C[0];
    sp->part_faces = world_state.unk831C[1];
    sp->unk1B8 = world_state.unk8324;
    sp->part_prims = world_state.unk8328;
    sp->part_div_prims = world_state.unk832C;
    sp->unk1C4 = world_state.unk8330;
    sp->unk1CC = world_state.unk8334;
    sp->unk1D0 = world_state.unk8338[0];
    sp->unk1D4 = world_state.unk8338[1];
    sp->unk1D8 = world_state.unk8338[2];
    sp->unk1DC = world_state.unk8338[3];
    sp->unk1E0 = world_state.unk8338[4];
    MESH_TINT_LEVEL = rand();
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->part_rgb.r = sp->part_rgb.g = sp->part_rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->part_rgb.r = sp->part_rgb.g = sp->part_rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->part_rgb.r = sp->part_rgb.g = sp->part_rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    vis_build_visible((u8 *)&viewer.position.vx);
    vis_build_planes();
    vis_cull_by_range();
    gte_SetRotMatrix(&world_state.unk1ECE8);
    gte_SetTransMatrix(&world_state.unk1ECE8);
    world_transform_parts(trans);
    sp->light = world_state.light;
    gte_SetBackColor(sp->light->back.r, sp->light->back.g, sp->light->back.b);
    gte_SetColorMatrix(sp->light->color.m);
    sp->unk64 = sp->light->unk68;
    sp->unk68 = sp->light->unk6A;
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_project_parts(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_ft3_far(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_ft4_far(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_gt3_far(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_gt4_far(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_ft3_near(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_ft4_near(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_gt3_near(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_draw_gt4_near(trans);
    sp->unk34 = (u_long *)world_state.world_vertices;
    world_state.unk831C[0] = sp->part_vertex_count;
    world_state.unk831C[1] = sp->part_faces;
    world_state.unk8328 = sp->part_prims;
    world_state.unk8324 = sp->unk1B8;
    world_state.unk832C = sp->part_div_prims;
    world_state.unk8330 = sp->unk1C4;
    world_state.unk8334 = sp->unk1CC;
    world_state.unk8338[0] = sp->unk1D0;
    world_state.unk8338[1] = sp->unk1D4;
    world_state.unk8338[2] = sp->unk1D8;
    world_state.unk8338[3] = sp->unk1DC;
    world_state.unk8338[4] = sp->unk1E0;
    world_state.packet->cur = sp->prim;
    world_state.packet->end = sp->prim_end;
}
