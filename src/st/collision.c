/*
 * ST.EXE: the collision test of a request against the collected sources'
 * collision parts: the parts' vertices in world space and relative to the
 * three test points, the triangle and quad face tests, and the request's
 * resolution into floor, ceiling and wall heights (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <INLINE_C.H>
#include <st/data.h>
#include <st/gfx.h>
#include <st/vis.h>

#define DOT(a, b) ((a).vx * (b).vx + (a).vy * (b).vy + (a).vz * (b).vz)
/* The edge cross products' sign along the vertical axis. */
#define UP(e) ((e).vy * 4096 >= 0)
#define DOWN(e) ((e).vy * -4096 >= 0)
/* A face vertex: the faces keep SVECTOR byte offsets, the VECTOR arrays
 * are twice as wide. */
/* Offsets in the collision file count from the end of its 12-byte header. */
#define HIT_DATA(sp, off) ((sp)->base + ((off) + 12))
#define AT(base, off) ((VECTOR *)((u8 *)(base) + (off) * 2))
/* Copies a face normal as two words. */
#define COPY_NORMAL(dst, src)                                                  \
    ((long *)&(dst))[0] = ((long *)(src))[0];                                  \
    ((long *)&(dst))[1] = ((long *)(src))[1]

/* Each collected source's part vertices in world space (rotated by the
 * record's quarter turn and moved to the source's cell), and relative to the
 * three test points. */
ADDRESS(0x80046604, 0x2bc)
void collision_transform_vertices(VECTOR *pos)
{
    CollisionScratch *sp = COLLISION_SCRATCH;
    MapBlock *rec;
    SVECTOR *v;
    VECTOR *o;
    VECTOR *o0;
    VECTOR *o1;
    VECTOR *o2;
    SVECTOR unused;
    int i;
    int n;

    i = vis_lists.n_sources;
    while (i--) {
        rec = &vis_records[vis_lists.sources[i]];
        sp->turn = (u8)(rec->turn & 3);
        gte_SetRotMatrix(&quarter_turn_matrices[sp->turn]);
        gte_SetTransVector(&pos[i]);
        o = sp->world;
        o0 = sp->rel[0];
        o1 = sp->rel[1];
        o2 = sp->rel[2];
        sp->part = (CollisionPart *)HIT_DATA(sp, rec->part * 40);
        sp->vertices = HIT_DATA(sp, sp->part->vertices);
        v = (SVECTOR *)sp->vertices;
        n = sp->part->n_vertices;
        if (&vis_lists.vertices[271] < o + n)
            return;
        while (n--) {
            gte_ldv0(v);
            gte_rtv0tr();
            gte_stlvnl(o);
            o0->vx = o->vx - sp->pos[0].vx;
            o0->vy = o->vy - sp->pos[0].vy;
            o0->vz = o->vz - sp->pos[0].vz;
            o0++;
            o1->vx = o->vx - sp->pos[1].vx;
            o1->vy = o->vy - sp->pos[1].vy;
            o1->vz = o->vz - sp->pos[1].vz;
            o1++;
            o2->vx = o->vx - sp->pos[2].vx;
            o2->vy = o->vy - sp->pos[2].vy;
            o2->vz = o->vz - sp->pos[2].vz;
            o2++;
            v++;
            o++;
        }
        sp->world += sp->part->n_vertices;
        sp->rel[0] += sp->part->n_vertices;
        sp->rel[1] += sp->part->n_vertices;
        sp->rel[2] += sp->part->n_vertices;
    }
}

/* Tests the parts' triangles against the three points: floors and
 * ceilings (flag 1), walls (flag 2) and flag 4 faces; the nearest heights
 * and their normals go to the scratchpad. */
ADDRESS(0x800468c0, 0xfc4)
void collision_test_triangles(VECTOR *pos, int kind)
{
    CollisionScratch *sp = COLLISION_SCRATCH;
    SVECTOR nrm;
    SVECTOR *nv = &nrm;
    SVECTOR *fn;
    MapBlock *rec;
    CollisionTri *f;
    int i;
    MATRIX *m;
    VECTOR *w;
    VECTOR *a0;
    VECTOR *a1;
    VECTOR *a2;
    VECTOR *b0;
    VECTOR *b1;
    VECTOR *b2;
    VECTOR *c0;
    VECTOR *c1;
    VECTOR *c2;
    int d;
    int h0;
    int h1;
    int h2;
    u_int n;

    i = vis_lists.n_sources;
    while (i--) {
        rec = &vis_records[vis_lists.sources[i]];
        sp->part = (CollisionPart *)HIT_DATA(sp, rec->part * 40);
        sp->vertices = HIT_DATA(sp, sp->part->vertices);
        sp->normals = HIT_DATA(sp, sp->part->normals);
        n = sp->part->n_tris;
        f = (CollisionTri *)HIT_DATA(sp, sp->part->faces);
        f = (CollisionTri *)((u8 *)f + sp->part->n18 * 20);
        f = (CollisionTri *)((u8 *)f + sp->part->n1c * 28);
        f = (CollisionTri *)((u8 *)f + sp->part->n1a * 24);
        f = (CollisionTri *)((u8 *)f + sp->part->n1e * 32);
        m = &quarter_turn_matrices[rec->turn & 3];
        while (n-- > 0) {
            fn = (SVECTOR *)(sp->normals + f->normal);
            gte_SetRotMatrix(m);
            gte_ldv0(fn);
            gte_rtv0();
            gte_stsv(nv);
            /* flags 1 and 4, read as one word with the push bytes */
            if (DOT(*nv, sp->dir) > 0 && !(*(u32 *)f & 0x50000))
                { f++; continue; }
            w = AT(sp->world, f->v[0]);
            d = -DOT(*nv, *w);
            h0 = (DOT(*nv, sp->pos[0]) + d) / 4096;
            h1 = (DOT(*nv, sp->pos[1]) + d) / 4096;
            h2 = (DOT(*nv, sp->pos[2]) + d) / 4096;
            if (h0 < -256)
                { f++; continue; }
            a0 = AT(sp->rel[0], f->v[0]);
            a1 = AT(sp->rel[0], f->v[1]);
            a2 = AT(sp->rel[0], f->v[2]);
            b0 = AT(sp->rel[1], f->v[0]);
            b1 = AT(sp->rel[1], f->v[1]);
            b2 = AT(sp->rel[1], f->v[2]);
            c0 = AT(sp->rel[2], f->v[0]);
            c1 = AT(sp->rel[2], f->v[1]);
            c2 = AT(sp->rel[2], f->v[2]);
            gte_ldopv1(a0);
            gte_ldopv2(a1);
            gte_op12();
            gte_stlvnl(&sp->e[0][0]);
            gte_ldopv1(a1);
            gte_ldopv2(a2);
            gte_op12();
            gte_stlvnl(&sp->e[0][1]);
            gte_ldopv1(a2);
            gte_ldopv2(a0);
            gte_op12();
            gte_stlvnl(&sp->e[0][2]);
            gte_ldopv1(b0);
            gte_ldopv2(b1);
            gte_op12();
            gte_stlvnl(&sp->e[1][0]);
            gte_ldopv1(b1);
            gte_ldopv2(b2);
            gte_op12();
            gte_stlvnl(&sp->e[1][1]);
            gte_ldopv1(b2);
            gte_ldopv2(b0);
            gte_op12();
            gte_stlvnl(&sp->e[1][2]);
            gte_ldopv1(c0);
            gte_ldopv2(c1);
            gte_op12();
            gte_stlvnl(&sp->e[2][0]);
            gte_ldopv1(c1);
            gte_ldopv2(c2);
            gte_op12();
            gte_stlvnl(&sp->e[2][1]);
            gte_ldopv1(c2);
            gte_ldopv2(c0);
            gte_op12();
            gte_stlvnl(&sp->e[2][2]);
            if (f->flags & 4) {
                if (DOWN(sp->e[0][0]) && DOWN(sp->e[0][1]) && DOWN(sp->e[0][2]) &&
                    h0 < sp->ceiling_dist) {
                    sp->ceiling_dist = h0;
                    COPY_NORMAL(sp->ceiling_normal, nv);
                }
                if (h1 < -256)
                    { f++; continue; }
                if (DOWN(sp->e[1][0]) && DOWN(sp->e[1][1]) && DOWN(sp->e[1][2]) &&
                    h1 < sp->next_ceiling_dist)
                    sp->next_ceiling_dist = h1;
            }
            if (f->flags & 1) {
                if (UP(sp->e[0][0]) && UP(sp->e[0][1]) && UP(sp->e[0][2]) && h0 < sp->floor_dist) {
                    if ((f->flags & 0x800) && !(vis_lists.floor_kinds_off & 1)) {
                        sp->flags &= ~0x10;
                        sp->flags &= ~0x8;
                        sp->flags &= ~0x20;
                    }
                    if ((f->flags & 0x10) && !(vis_lists.floor_kinds_off & 1))
                        sp->flags = 1;
                    if ((f->flags & 0x20) && !((vis_lists.floor_kinds_off >> 1) & 1))
                        sp->flags = 2;
                    if ((f->flags & 0x40) && !((vis_lists.floor_kinds_off >> 2) & 1)) {
                        sp->flags |= 4;
                        sp->surface_y = a0->vy + sp->pos[0].vy;
                    }
                    if ((f->flags & 0x80) && !((vis_lists.floor_kinds_off >> 3) & 1))
                        sp->flags = 8;
                    if ((f->flags & 0x100) && !((vis_lists.floor_kinds_off >> 4) & 1))
                        sp->flags = 16;
                    if ((f->flags & 0x200) && !((vis_lists.floor_kinds_off >> 5) & 1)) {
                        sp->flags |= 0x20;
                        sp->acid_surface_y = a0->vy + sp->pos[0].vy;
                    }
                    if (f->flags & 0x400)
                        { f++; continue; }
                    sp->hit = vis_lists.sources[i];
                    sp->floor_dist = h0;
                    COPY_NORMAL(sp->floor_normal, nv);
                    sp->push.vx = (s8)f->push_x;
                    sp->push.vy = 0;
                    sp->push.vz = (s8)f->push_z;
                    gte_SetRotMatrix(m);
                    gte_ldv0(&sp->push);
                    gte_rtv0();
                    gte_stsv(&sp->push);
                    vis_lists.floor_push_x += sp->push.vx;
                    vis_lists.floor_push_y += sp->push.vy;
                    vis_lists.floor_push_z += sp->push.vz;
                }
                if (h1 < -256)
                    { f++; continue; }
                if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && h1 < sp->next_floor_dist) {
                    sp->next_floor_dist = h1;
                    vis_lists.next_floor_block = vis_lists.sources[i];
                    COPY_NORMAL(sp->next_floor_normal, nv);
                }
            }
            if (!(f->flags & 2))
                { f++; continue; }
            if (DOT(*nv, sp->e[0][0]) > 0 || DOT(*nv, sp->e[0][1]) > 0 ||
                DOT(*nv, sp->e[0][2]) > 0) {
                if (DOT(sp->dir, sp->e[0][0]) >= 0 && DOT(sp->dir, sp->e[0][1]) >= 0 &&
                    DOT(sp->dir, sp->e[0][2]) >= 0 && h0 < sp->reach_wall_dist) {
                    sp->reach_wall_dist = h0;
                    COPY_NORMAL(sp->reach_wall_normal, nv);
                }
            } else if (h0 < sp->wall_dist) {
                sp->hit = vis_lists.sources[i];
                sp->wall_dist = h0;
                COPY_NORMAL(sp->wall_normal, nv);
            }
            if (h1 < -256)
                { f++; continue; }
            if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && h1 < sp->wall_floor_dist) {
                sp->hit = vis_lists.sources[i];
                sp->wall_floor_dist = h1;
                COPY_NORMAL(sp->wall_floor_normal, nv);
            }
            if (h2 < -256)
                { f++; continue; }
            if (DOT(*nv, sp->e[2][0]) > 0 || DOT(*nv, sp->e[2][1]) > 0 ||
                DOT(*nv, sp->e[2][2]) > 0) {
                if (DOT(sp->dir, sp->e[2][0]) >= 0 && DOT(sp->dir, sp->e[2][1]) >= 0 &&
                    DOT(sp->dir, sp->e[2][2]) >= 0 && h2 < sp->low_reach_wall_dist) {
                    sp->low_reach_wall_dist = h2;
                    COPY_NORMAL(sp->low_reach_wall_normal, nv);
                }
            } else if (h2 < sp->low_wall_dist) {
                sp->low_wall_dist = h2;
                COPY_NORMAL(sp->low_wall_normal, nv);
            }
            f++;
        }
        sp->world += sp->part->n_vertices;
        sp->rel[0] += sp->part->n_vertices;
        sp->rel[1] += sp->part->n_vertices;
        sp->rel[2] += sp->part->n_vertices;
    }
}

/* collision_test_triangles for the parts' quads (vertex order 0, 1, 3, 2); its
 * floor-kind bit 0x800 is gated by bit 6 of unk47E0, not bit 0. */
ADDRESS(0x80047884, 0x124c)
void collision_test_quads(VECTOR *pos, int kind)
{
    CollisionScratch *sp = COLLISION_SCRATCH;
    SVECTOR nrm;
    SVECTOR *nv = &nrm;
    SVECTOR *fn;
    MapBlock *rec;
    CollisionQuad *f;
    int i;
    MATRIX *m;
    VECTOR *w;
    VECTOR *a0;
    VECTOR *a1;
    VECTOR *a2;
    VECTOR *a3;
    VECTOR *b0;
    VECTOR *b1;
    VECTOR *b2;
    VECTOR *b3;
    VECTOR *c0;
    VECTOR *c1;
    VECTOR *c2;
    VECTOR *c3;
    int d;
    int h0;
    int h1;
    int h2;
    u_int n;

    i = vis_lists.n_sources;
    while (i--) {
        rec = &vis_records[vis_lists.sources[i]];
        sp->part = (CollisionPart *)HIT_DATA(sp, rec->part * 40);
        sp->vertices = HIT_DATA(sp, sp->part->vertices);
        sp->normals = HIT_DATA(sp, sp->part->normals);
        n = sp->part->n_quads;
        f = (CollisionQuad *)HIT_DATA(sp, sp->part->faces);
        f = (CollisionQuad *)((u8 *)f + sp->part->n18 * 20);
        f = (CollisionQuad *)((u8 *)f + sp->part->n1c * 28);
        f = (CollisionQuad *)((u8 *)f + sp->part->n1a * 24);
        f = (CollisionQuad *)((u8 *)f + sp->part->n1e * 32);
        f = (CollisionQuad *)((u8 *)f + sp->part->n_tris * 12);
        m = &quarter_turn_matrices[rec->turn & 3];
        while (n-- > 0) {
            fn = (SVECTOR *)(sp->normals + f->normal);
            gte_SetRotMatrix(m);
            gte_ldv0(fn);
            gte_rtv0();
            gte_stsv(nv);
            /* flags 1 and 4, read as one word with the push bytes */
            if (DOT(*nv, sp->dir) > 0 && !(*(u32 *)f & 0x50000))
                { f++; continue; }
            w = AT(sp->world, f->v[0]);
            d = -DOT(*nv, *w);
            h0 = (DOT(*nv, sp->pos[0]) + d) / 4096;
            h1 = (DOT(*nv, sp->pos[1]) + d) / 4096;
            h2 = (DOT(*nv, sp->pos[2]) + d) / 4096;
            if (h0 < -256)
                { f++; continue; }
            a0 = AT(sp->rel[0], f->v[0]);
            a1 = AT(sp->rel[0], f->v[1]);
            a2 = AT(sp->rel[0], f->v[2]);
            a3 = AT(sp->rel[0], f->v[3]);
            b0 = AT(sp->rel[1], f->v[0]);
            b1 = AT(sp->rel[1], f->v[1]);
            b2 = AT(sp->rel[1], f->v[2]);
            b3 = AT(sp->rel[1], f->v[3]);
            c0 = AT(sp->rel[2], f->v[0]);
            c1 = AT(sp->rel[2], f->v[1]);
            c2 = AT(sp->rel[2], f->v[2]);
            c3 = AT(sp->rel[2], f->v[3]);
            gte_ldopv1(a0);
            gte_ldopv2(a1);
            gte_op12();
            gte_stlvnl(&sp->e[0][0]);
            gte_ldopv1(a1);
            gte_ldopv2(a3);
            gte_op12();
            gte_stlvnl(&sp->e[0][1]);
            gte_ldopv1(a3);
            gte_ldopv2(a2);
            gte_op12();
            gte_stlvnl(&sp->e[0][2]);
            gte_ldopv1(a2);
            gte_ldopv2(a0);
            gte_op12();
            gte_stlvnl(&sp->e[0][3]);
            gte_ldopv1(b0);
            gte_ldopv2(b1);
            gte_op12();
            gte_stlvnl(&sp->e[1][0]);
            gte_ldopv1(b1);
            gte_ldopv2(b3);
            gte_op12();
            gte_stlvnl(&sp->e[1][1]);
            gte_ldopv1(b3);
            gte_ldopv2(b2);
            gte_op12();
            gte_stlvnl(&sp->e[1][2]);
            gte_ldopv1(b2);
            gte_ldopv2(b0);
            gte_op12();
            gte_stlvnl(&sp->e[1][3]);
            gte_ldopv1(c0);
            gte_ldopv2(c1);
            gte_op12();
            gte_stlvnl(&sp->e[2][0]);
            gte_ldopv1(c1);
            gte_ldopv2(c3);
            gte_op12();
            gte_stlvnl(&sp->e[2][1]);
            gte_ldopv1(c3);
            gte_ldopv2(c2);
            gte_op12();
            gte_stlvnl(&sp->e[2][2]);
            gte_ldopv1(c2);
            gte_ldopv2(c0);
            gte_op12();
            gte_stlvnl(&sp->e[2][3]);
            if (f->flags & 4) {
                if (DOWN(sp->e[0][0]) && DOWN(sp->e[0][1]) && DOWN(sp->e[0][2]) &&
                    DOWN(sp->e[0][3]) && h0 < sp->ceiling_dist) {
                    sp->ceiling_dist = h0;
                    COPY_NORMAL(sp->ceiling_normal, nv);
                }
                if (h1 < -256)
                    { f++; continue; }
                if (DOWN(sp->e[1][0]) && DOWN(sp->e[1][1]) && DOWN(sp->e[1][2]) &&
                    DOWN(sp->e[1][3]) && h1 < sp->next_ceiling_dist)
                    sp->next_ceiling_dist = h1;
            }
            if (f->flags & 1) {
                if (UP(sp->e[0][0]) && UP(sp->e[0][1]) && UP(sp->e[0][2]) && UP(sp->e[0][3]) &&
                    h0 < sp->floor_dist) {
                    if ((f->flags & 0x800) && !((vis_lists.floor_kinds_off >> 6) & 1)) {
                        sp->flags &= ~0x10;
                        sp->flags &= ~0x8;
                        sp->flags &= ~0x20;
                    }
                    if ((f->flags & 0x10) && !(vis_lists.floor_kinds_off & 1))
                        sp->flags = 1;
                    if ((f->flags & 0x20) && !((vis_lists.floor_kinds_off >> 1) & 1))
                        sp->flags = 2;
                    if ((f->flags & 0x40) && !((vis_lists.floor_kinds_off >> 2) & 1)) {
                        sp->flags |= 4;
                        sp->surface_y = a0->vy + sp->pos[0].vy;
                    }
                    if ((f->flags & 0x80) && !((vis_lists.floor_kinds_off >> 3) & 1))
                        sp->flags = 8;
                    if ((f->flags & 0x100) && !((vis_lists.floor_kinds_off >> 4) & 1))
                        sp->flags = 16;
                    if ((f->flags & 0x200) && !((vis_lists.floor_kinds_off >> 5) & 1)) {
                        sp->flags |= 0x20;
                        sp->acid_surface_y = a0->vy + sp->pos[0].vy;
                    }
                    if (f->flags & 0x400)
                        { f++; continue; }
                    sp->hit = vis_lists.sources[i];
                    sp->floor_dist = h0;
                    COPY_NORMAL(sp->floor_normal, nv);
                    sp->push.vx = (s8)f->push_x;
                    sp->push.vy = 0;
                    sp->push.vz = (s8)f->push_z;
                    gte_SetRotMatrix(m);
                    gte_ldv0(&sp->push);
                    gte_rtv0();
                    gte_stsv(&sp->push);
                    vis_lists.floor_push_x += sp->push.vx;
                    vis_lists.floor_push_y += sp->push.vy;
                    vis_lists.floor_push_z += sp->push.vz;
                }
                if (h1 < -256)
                    { f++; continue; }
                if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && UP(sp->e[1][3]) &&
                    h1 < sp->next_floor_dist) {
                    sp->next_floor_dist = h1;
                    vis_lists.next_floor_block = vis_lists.sources[i];
                    COPY_NORMAL(sp->next_floor_normal, nv);
                }
            }
            if (!(f->flags & 2))
                { f++; continue; }
            if (DOT(*nv, sp->e[0][0]) > 0 || DOT(*nv, sp->e[0][1]) > 0 ||
                DOT(*nv, sp->e[0][2]) > 0 || DOT(*nv, sp->e[0][3]) > 0) {
                if (DOT(sp->dir, sp->e[0][0]) >= 0 && DOT(sp->dir, sp->e[0][1]) >= 0 &&
                    DOT(sp->dir, sp->e[0][2]) >= 0 && DOT(sp->dir, sp->e[0][3]) >= 0 &&
                    h0 < sp->reach_wall_dist) {
                    sp->reach_wall_dist = h0;
                    COPY_NORMAL(sp->reach_wall_normal, nv);
                }
            } else if (h0 < sp->wall_dist) {
                sp->hit = vis_lists.sources[i];
                sp->wall_dist = h0;
                COPY_NORMAL(sp->wall_normal, nv);
            }
            if (h1 < -256)
                { f++; continue; }
            if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && UP(sp->e[1][3]) &&
                h1 < sp->wall_floor_dist) {
                sp->hit = vis_lists.sources[i];
                sp->wall_floor_dist = h1;
                COPY_NORMAL(sp->wall_floor_normal, nv);
            }
            if (h2 < -256)
                { f++; continue; }
            if (DOT(*nv, sp->e[2][0]) > 0 || DOT(*nv, sp->e[2][1]) > 0 ||
                DOT(*nv, sp->e[2][2]) > 0 || DOT(*nv, sp->e[2][3]) > 0) {
                if (DOT(sp->dir, sp->e[2][0]) >= 0 && DOT(sp->dir, sp->e[2][1]) >= 0 &&
                    DOT(sp->dir, sp->e[2][2]) >= 0 && DOT(sp->dir, sp->e[2][3]) >= 0 &&
                    h2 < sp->low_reach_wall_dist) {
                    sp->low_reach_wall_dist = h2;
                    COPY_NORMAL(sp->low_reach_wall_normal, nv);
                }
            } else if (h2 < sp->low_wall_dist) {
                sp->low_wall_dist = h2;
                COPY_NORMAL(sp->low_wall_normal, nv);
            }
            f++;
        }
        sp->world += sp->part->n_vertices;
        sp->rel[0] += sp->part->n_vertices;
        sp->rel[1] += sp->part->n_vertices;
        sp->rel[2] += sp->part->n_vertices;
    }
}

/* Resolves a request against the collision parts around its position, its
 * position plus velocity and a point 1027 below its height: bit 0 floor,
 * bit 1 wall, bit 2 ceiling, bit 3 the second wall. The heights and wall
 * normals go to vis_lists. */
ADDRESS(0x80048ad0, 0x824)
int collision_test_map(CollisionState *req)
{
    CollisionScratch *sp = COLLISION_SCRATCH;
    VECTOR pos[384];
    CollisionState *lists;
    int hits;
    int r;
    s16 ang;
    int t;
    int floor_motion;

    sp->base = (u8 *)world_state.area_geometry[1];
    sp->pos[0].vx = req->position.vx;
    sp->pos[0].vy = req->position.vy;
    sp->pos[0].vz = req->position.vz;
    sp->pos[1].vx = req->position.vx + req->velocity.vx;
    sp->pos[1].vy = req->position.vy + req->velocity.vy;
    sp->pos[1].vz = req->position.vz + req->velocity.vz;
    sp->pos[2].vx = req->position.vx;
    sp->pos[2].vy = req->position.vy + req->height - 1027;
    sp->pos[2].vz = req->position.vz;
    vis_lists.n_sources = 0;
    vis_collect_sources(&sp->pos[0], &sp->pos[1], NULL);
    hits = 0;
    if (vis_lists.n_sources == 0) {
        vis_lists.next_floor_dist = 0x80000;
        vis_lists.floor_dist = 0x80000;
        return hits;
    }
    collision_source_positions(pos);
    sp->world = vis_lists.vertices;
    sp->rel[0] = vis_lists.rel_vertices;
    sp->rel[1] = vis_lists.next_rel_vertices;
    sp->rel[2] = vis_lists.low_rel_vertices;
    collision_transform_vertices(pos);
    if (req->heading == 9999) {
        if (req->velocity.vx == 0 && req->velocity.vz == 0) {
            sp->dir.vx = 0;
            sp->dir.vy = req->velocity.vy + req->height;
            sp->dir.vz = 0;
            goto heights;
        }
        sp->dir.vx = req->velocity.vx + rcos(ratan2(req->velocity.vz, req->velocity.vx)) * req->radius / 4096;
        sp->dir.vy = req->velocity.vy + req->height;
        sp->dir.vz = req->velocity.vz + rsin(ratan2(req->velocity.vz, req->velocity.vx)) * req->radius / 4096;
    } else {
        sp->dir.vx = req->velocity.vx + rcos(req->heading) * req->radius / 4096;
        sp->dir.vy = req->velocity.vy;
        sp->dir.vz = req->velocity.vz + rsin(req->heading) * req->radius / 4096;
    }
heights:
    lists = &vis_lists;
    sp->next_ceiling_dist = 0x80000;
    sp->acid_surface_y = 0x80000;
    sp->surface_y = 0x80000;
    sp->ceiling_dist = 0x80000;
    sp->wall_floor_dist = 0x80000;
    sp->next_floor_dist = 0x80000;
    sp->floor_dist = 0x80000;
    sp->wall_dist = 0x80000;
    sp->reach_wall_dist = 0x80000;
    sp->low_wall_dist = 0x80000;
    sp->low_reach_wall_dist = 0x80000;
    sp->hit = -1;
    sp->flags = 0;
    sp->world = lists->vertices;
    sp->rel[0] = lists->rel_vertices;
    sp->rel[1] = lists->next_rel_vertices;
    sp->rel[2] = lists->low_rel_vertices;
    lists->next_floor_block = lists->sources[0];
    collision_test_triangles(pos, req->tests);
    sp->world = vis_lists.vertices;
    sp->rel[0] = vis_lists.rel_vertices;
    sp->rel[1] = vis_lists.next_rel_vertices;
    sp->rel[2] = vis_lists.low_rel_vertices;
    collision_test_quads(pos, req->tests);
    if (sp->wall_dist == 0x80000) {
        sp->wall_normal = sp->reach_wall_normal;
        sp->wall_dist = sp->reach_wall_dist;
    }
    if (sp->low_wall_dist == 0x80000) {
        sp->low_wall_normal = sp->low_reach_wall_normal;
        sp->low_wall_dist = sp->low_reach_wall_dist;
    }
    if (sp->floor_dist == 0x80000) {
        sp->floor_normal = sp->wall_floor_normal;
        sp->floor_dist = sp->wall_floor_dist;
    }
    if (sp->next_floor_dist == 0x80000) {
        sp->next_floor_normal = sp->floor_normal;
        sp->next_floor_dist = sp->floor_dist;
    }
    if (sp->floor_dist != 0x80000 && sp->floor_normal.vy != 0) {
        r = SquareRoot0(sp->floor_normal.vx * sp->floor_normal.vx + sp->floor_normal.vz * sp->floor_normal.vz);
        ang = ratan2(-sp->floor_normal.vy, r);
        if (r != 0)
            sp->floor_dist = (sp->floor_dist << 12) / rsin(ang);
    }
    if (sp->next_floor_dist != 0x80000 && sp->next_floor_normal.vy != 0) {
        /* retail squares the second normal's z with the first's */
        r = SquareRoot0(sp->next_floor_normal.vx * sp->next_floor_normal.vx + sp->next_floor_normal.vz * sp->floor_normal.vz);
        ang = ratan2(-sp->next_floor_normal.vy, r);
        if (r != 0)
            sp->next_floor_dist = (sp->next_floor_dist << 12) / rsin(ang);
    }
    if (sp->ceiling_dist < 256 - req->velocity.vy)
        hits |= 4;
    floor_motion = req->velocity.vy * -4096 / 4096;
    if (sp->floor_dist < req->height - floor_motion)
        hits |= 1;
    t = DOT(req->velocity, sp->wall_normal) / 4096;
    if (sp->wall_dist < req->radius - t)
        hits |= 2;
    t = DOT(req->velocity, sp->low_wall_normal) / 4096;
    if (sp->low_wall_dist < req->radius - t)
        hits |= 8;
    vis_lists.ceiling_dist = sp->ceiling_dist;
    vis_lists.next_ceiling_dist = sp->next_ceiling_dist;
    vis_lists.floor_dist = sp->floor_dist;
    vis_lists.next_floor_dist = sp->next_floor_dist;
    vis_lists.wall_dist = sp->wall_dist;
    vis_lists.reach_wall_dist = sp->reach_wall_dist;
    vis_lists.low_wall_dist = sp->low_wall_dist;
    vis_lists.low_reach_wall_dist = sp->low_reach_wall_dist;
    vis_lists.surface_y = sp->surface_y;
    vis_lists.unk50 = sp->h13c;
    vis_lists.unk54 = sp->h140;
    vis_lists.acid_surface_y = sp->acid_surface_y;
    vis_lists.ceiling_normal = sp->ceiling_normal;
    vis_lists.floor_normal = sp->floor_normal;
    /* the wall normals: SVECTORs over vis_lists' unk8C and unk9C fields */
    *(SVECTOR *)&vis_lists.wall_normal_x = sp->wall_normal;
    vis_lists.reach_wall_normal = sp->reach_wall_normal;
    *(SVECTOR *)&vis_lists.low_wall_normal_x = sp->low_wall_normal;
    vis_lists.low_reach_wall_normal = sp->low_reach_wall_normal;
    vis_lists.hit_block = sp->hit;
    vis_lists.floor_kinds = sp->flags;
    return hits;
}
