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

/* Each collected source's part vertices in world space (rotated by the
 * record's light set and moved to the source's cell), and relative to the
 * three test points. */
ADDRESS(0x80046604, 0x2bc)
void func_80046604(VECTOR *pos)
{
    VisHitScratch *sp = VIS_HIT_SCRATCH;
    VisRecord *rec;
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
        sp->unk20 = (u8)(rec->unk03 & 3);
        gte_SetRotMatrix(&DAT_8008042c[sp->unk20]);
        gte_SetTransVector(&pos[i]);
        o = sp->world;
        o0 = sp->rel[0];
        o1 = sp->rel[1];
        o2 = sp->rel[2];
        sp->part = (VisHitPart *)HIT_DATA(sp, rec->part * 40);
        sp->vertices = HIT_DATA(sp, sp->part->vertices);
        v = (SVECTOR *)sp->vertices;
        n = sp->part->n_vertices;
        if (&vis_lists.unkC0[271] < o + n)
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
void func_800468c0(VECTOR *pos, int kind)
{
    VisHitScratch *sp = VIS_HIT_SCRATCH;
    SVECTOR nrm;
    SVECTOR *fn;
    VisRecord *rec;
    VisHitTri *f;
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
    int i;
    int d;
    int h0;
    int h1;
    int h2;
    u_int n;

    i = vis_lists.n_sources;
    while (i--) {
        rec = &vis_records[vis_lists.sources[i]];
        sp->part = (VisHitPart *)HIT_DATA(sp, rec->part * 40);
        sp->vertices = HIT_DATA(sp, sp->part->vertices);
        sp->normals = HIT_DATA(sp, sp->part->normals);
        n = sp->part->n_tris;
        f = (VisHitTri *)(HIT_DATA(sp, sp->part->faces) + sp->part->n18 * 20 +
                          sp->part->n1c * 28 + sp->part->n1a * 24 + sp->part->n1e * 32);
        m = &DAT_8008042c[rec->unk03 & 3];
        while (n--) {
            fn = (SVECTOR *)(sp->normals + f->normal);
            gte_SetRotMatrix(m);
            gte_ldv0(fn);
            gte_rtv0();
            gte_stsv(&nrm);
            /* flags 1 and 4, read as one word with the push bytes */
            if (DOT(nrm, sp->dir) > 0 && !(*(u32 *)f & 0x50000))
                goto next;
            w = AT(sp->world, f->v[0]);
            d = -DOT(nrm, *w);
            h0 = (DOT(nrm, sp->pos[0]) + d) / 4096;
            h1 = (DOT(nrm, sp->pos[1]) + d) / 4096;
            h2 = (DOT(nrm, sp->pos[2]) + d) / 4096;
            if (h0 < -256)
                goto next;
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
                    h0 < sp->h120) {
                    sp->h120 = h0;
                    sp->n150 = nrm;
                }
                if (h1 < -256)
                    goto next;
                if (DOWN(sp->e[1][0]) && DOWN(sp->e[1][1]) && DOWN(sp->e[1][2]) &&
                    h1 < sp->h124)
                    sp->h124 = h1;
            }
            if (f->flags & 1) {
                if (UP(sp->e[0][0]) && UP(sp->e[0][1]) && UP(sp->e[0][2]) && h0 < sp->h114) {
                    if ((f->flags & 0x800) && !(vis_lists.unk47E0 & 1)) {
                        sp->flags &= ~0x10;
                        sp->flags &= ~0x8;
                        sp->flags &= ~0x20;
                    }
                    if ((f->flags & 0x10) && !(vis_lists.unk47E0 & 1))
                        sp->flags = 1;
                    if ((f->flags & 0x20) && !((vis_lists.unk47E0 >> 1) & 1))
                        sp->flags = 2;
                    if ((f->flags & 0x40) && !((vis_lists.unk47E0 >> 2) & 1)) {
                        sp->flags |= 4;
                        sp->h138 = w->vy + sp->pos[0].vy;
                    }
                    if ((f->flags & 0x80) && !((vis_lists.unk47E0 >> 3) & 1))
                        sp->flags = 8;
                    if ((f->flags & 0x100) && !((vis_lists.unk47E0 >> 4) & 1))
                        sp->flags = 16;
                    if ((f->flags & 0x200) && !((vis_lists.unk47E0 >> 5) & 1)) {
                        sp->flags |= 0x20;
                        sp->h144 = w->vy + sp->pos[0].vy;
                    }
                    if (f->flags & 0x400)
                        goto next;
                    sp->h114 = h0;
                    sp->hit = vis_lists.sources[i];
                    sp->n158 = nrm;
                    sp->v190.vx = (s8)f->x;
                    sp->v190.vy = 0;
                    sp->v190.vz = (s8)f->z;
                    gte_SetRotMatrix(m);
                    gte_ldv0(&sp->v190);
                    gte_rtv0();
                    gte_stsv(&sp->v190);
                    vis_lists.unk47CE += sp->v190.vx;
                    vis_lists.unk47D0 += sp->v190.vy;
                    vis_lists.unk47D2 += sp->v190.vz;
                }
                if (h1 < -256)
                    goto next;
                if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && h1 < sp->h118) {
                    sp->h118 = h1;
                    /* the record index is stored as a word here */
                    *(u32 *)&vis_lists.unk47D8 = vis_lists.sources[i];
                    sp->n160 = nrm;
                }
            }
            if (!(f->flags & 2))
                goto next;
            if (DOT(nrm, sp->e[0][0]) > 0 || DOT(nrm, sp->e[0][1]) > 0 ||
                DOT(nrm, sp->e[0][2]) > 0) {
                if (DOT(sp->dir, sp->e[0][0]) >= 0 && DOT(sp->dir, sp->e[0][1]) >= 0 &&
                    DOT(sp->dir, sp->e[0][2]) >= 0 && h0 < sp->h12c) {
                    sp->h12c = h0;
                    sp->n178 = nrm;
                }
            } else if (h0 < sp->h128) {
                sp->h128 = h0;
                sp->hit = vis_lists.sources[i];
                sp->n170 = nrm;
            }
            if (h1 < -256)
                goto next;
            if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && h1 < sp->h11c) {
                sp->h11c = h1;
                sp->hit = vis_lists.sources[i];
                sp->n168 = nrm;
            }
            if (h2 < -256)
                goto next;
            if (DOT(nrm, sp->e[2][0]) > 0 || DOT(nrm, sp->e[2][1]) > 0 ||
                DOT(nrm, sp->e[2][2]) > 0) {
                if (DOT(sp->dir, sp->e[2][0]) >= 0 && DOT(sp->dir, sp->e[2][1]) >= 0 &&
                    DOT(sp->dir, sp->e[2][2]) >= 0 && h2 < sp->h134) {
                    sp->h134 = h2;
                    sp->n188 = nrm;
                }
            } else if (h2 < sp->h130) {
                sp->h130 = h2;
                sp->n180 = nrm;
            }
        next:
            f++;
        }
        sp->world += sp->part->n_vertices;
        sp->rel[0] += sp->part->n_vertices;
        sp->rel[1] += sp->part->n_vertices;
        sp->rel[2] += sp->part->n_vertices;
    }
}

/* func_800468c0 for the parts' quads (vertex order 0, 1, 3, 2). */
ADDRESS(0x80047884, 0x124c)
void func_80047884(VECTOR *pos, int kind)
{
    VisHitScratch *sp = VIS_HIT_SCRATCH;
    SVECTOR nrm;
    SVECTOR *fn;
    VisRecord *rec;
    VisHitQuad *f;
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
    int i;
    int d;
    int h0;
    int h1;
    int h2;
    u_int n;

    i = vis_lists.n_sources;
    while (i--) {
        rec = &vis_records[vis_lists.sources[i]];
        sp->part = (VisHitPart *)HIT_DATA(sp, rec->part * 40);
        sp->vertices = HIT_DATA(sp, sp->part->vertices);
        sp->normals = HIT_DATA(sp, sp->part->normals);
        n = sp->part->n_quads;
        f = (VisHitQuad *)(HIT_DATA(sp, sp->part->faces) + sp->part->n18 * 20 +
                           sp->part->n1c * 28 + sp->part->n1a * 24 + sp->part->n1e * 32 +
                           sp->part->n_tris * 12);
        m = &DAT_8008042c[rec->unk03 & 3];
        while (n--) {
            fn = (SVECTOR *)(sp->normals + f->normal);
            gte_SetRotMatrix(m);
            gte_ldv0(fn);
            gte_rtv0();
            gte_stsv(&nrm);
            /* flags 1 and 4, read as one word with the push bytes */
            if (DOT(nrm, sp->dir) > 0 && !(*(u32 *)f & 0x50000))
                goto next;
            w = AT(sp->world, f->v[0]);
            d = -DOT(nrm, *w);
            h0 = (DOT(nrm, sp->pos[0]) + d) / 4096;
            h1 = (DOT(nrm, sp->pos[1]) + d) / 4096;
            h2 = (DOT(nrm, sp->pos[2]) + d) / 4096;
            if (h0 < -256)
                goto next;
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
                    DOWN(sp->e[0][3]) && h0 < sp->h120) {
                    sp->h120 = h0;
                    sp->n150 = nrm;
                }
                if (h1 < -256)
                    goto next;
                if (DOWN(sp->e[1][0]) && DOWN(sp->e[1][1]) && DOWN(sp->e[1][2]) &&
                    DOWN(sp->e[1][3]) && h1 < sp->h124)
                    sp->h124 = h1;
            }
            if (f->flags & 1) {
                if (UP(sp->e[0][0]) && UP(sp->e[0][1]) && UP(sp->e[0][2]) && UP(sp->e[0][3]) &&
                    h0 < sp->h114) {
                    if ((f->flags & 0x800) && !(vis_lists.unk47E0 & 1)) {
                        sp->flags &= ~0x10;
                        sp->flags &= ~0x8;
                        sp->flags &= ~0x20;
                    }
                    if ((f->flags & 0x10) && !(vis_lists.unk47E0 & 1))
                        sp->flags = 1;
                    if ((f->flags & 0x20) && !((vis_lists.unk47E0 >> 1) & 1))
                        sp->flags = 2;
                    if ((f->flags & 0x40) && !((vis_lists.unk47E0 >> 2) & 1)) {
                        sp->flags |= 4;
                        sp->h138 = w->vy + sp->pos[0].vy;
                    }
                    if ((f->flags & 0x80) && !((vis_lists.unk47E0 >> 3) & 1))
                        sp->flags = 8;
                    if ((f->flags & 0x100) && !((vis_lists.unk47E0 >> 4) & 1))
                        sp->flags = 16;
                    if ((f->flags & 0x200) && !((vis_lists.unk47E0 >> 5) & 1)) {
                        sp->flags |= 0x20;
                        sp->h144 = w->vy + sp->pos[0].vy;
                    }
                    if (f->flags & 0x400)
                        goto next;
                    sp->h114 = h0;
                    sp->hit = vis_lists.sources[i];
                    sp->n158 = nrm;
                    sp->v190.vx = (s8)f->x;
                    sp->v190.vy = 0;
                    sp->v190.vz = (s8)f->z;
                    gte_SetRotMatrix(m);
                    gte_ldv0(&sp->v190);
                    gte_rtv0();
                    gte_stsv(&sp->v190);
                    vis_lists.unk47CE += sp->v190.vx;
                    vis_lists.unk47D0 += sp->v190.vy;
                    vis_lists.unk47D2 += sp->v190.vz;
                }
                if (h1 < -256)
                    goto next;
                if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && UP(sp->e[1][3]) &&
                    h1 < sp->h118) {
                    sp->h118 = h1;
                    /* the record index is stored as a word here */
                    *(u32 *)&vis_lists.unk47D8 = vis_lists.sources[i];
                    sp->n160 = nrm;
                }
            }
            if (!(f->flags & 2))
                goto next;
            if (DOT(nrm, sp->e[0][0]) > 0 || DOT(nrm, sp->e[0][1]) > 0 ||
                DOT(nrm, sp->e[0][2]) > 0 || DOT(nrm, sp->e[0][3]) > 0) {
                if (DOT(sp->dir, sp->e[0][0]) >= 0 && DOT(sp->dir, sp->e[0][1]) >= 0 &&
                    DOT(sp->dir, sp->e[0][2]) >= 0 && DOT(sp->dir, sp->e[0][3]) >= 0 &&
                    h0 < sp->h12c) {
                    sp->h12c = h0;
                    sp->n178 = nrm;
                }
            } else if (h0 < sp->h128) {
                sp->h128 = h0;
                sp->hit = vis_lists.sources[i];
                sp->n170 = nrm;
            }
            if (h1 < -256)
                goto next;
            if (UP(sp->e[1][0]) && UP(sp->e[1][1]) && UP(sp->e[1][2]) && UP(sp->e[1][3]) &&
                h1 < sp->h11c) {
                sp->h11c = h1;
                sp->hit = vis_lists.sources[i];
                sp->n168 = nrm;
            }
            if (h2 < -256)
                goto next;
            if (DOT(nrm, sp->e[2][0]) > 0 || DOT(nrm, sp->e[2][1]) > 0 ||
                DOT(nrm, sp->e[2][2]) > 0 || DOT(nrm, sp->e[2][3]) > 0) {
                if (DOT(sp->dir, sp->e[2][0]) >= 0 && DOT(sp->dir, sp->e[2][1]) >= 0 &&
                    DOT(sp->dir, sp->e[2][2]) >= 0 && DOT(sp->dir, sp->e[2][3]) >= 0 &&
                    h2 < sp->h134) {
                    sp->h134 = h2;
                    sp->n188 = nrm;
                }
            } else if (h2 < sp->h130) {
                sp->h130 = h2;
                sp->n180 = nrm;
            }
        next:
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
 * normals go to vis_lists.
 * Not exact: unset-constant setup and the record/last-array-pointer
 * preparation are scheduled in another order before the triangle pass. */
ADDRESS(0x80048ad0, 0x824)
int func_80048ad0(VisLists *req)
{
    VisHitScratch *sp = VIS_HIT_SCRATCH;
    VECTOR pos[384];
    VisLists *lists;
    u32 record;
    int hits;
    int r;
    s16 ang;
    int t;
    int floor_motion;
    int unset;

    sp->base = (u8 *)world_state.unk8308[1];
    sp->pos[0].vx = req->unk4.vx;
    sp->pos[0].vy = req->unk4.vy;
    sp->pos[0].vz = req->unk4.vz;
    sp->pos[1].vx = req->unk4.vx + req->unk14.vx;
    sp->pos[1].vy = req->unk4.vy + req->unk14.vy;
    sp->pos[1].vz = req->unk4.vz + req->unk14.vz;
    sp->pos[2].vx = req->unk4.vx;
    sp->pos[2].vy = req->unk4.vy + req->unk1E - 1027;
    sp->pos[2].vz = req->unk4.vz;
    vis_lists.n_sources = 0;
    vis_collect_sources(&sp->pos[0], &sp->pos[1], NULL);
    hits = 0;
    if (vis_lists.n_sources == 0) {
        vis_lists.unk2C = 0x80000;
        vis_lists.unk30 = 0x80000;
        return hits;
    }
    func_8004656c(pos);
    sp->world = vis_lists.unkC0;
    sp->rel[0] = vis_lists.unk11C0;
    sp->rel[1] = vis_lists.unk22C0;
    sp->rel[2] = vis_lists.unk33C0;
    func_80046604(pos);
    if (req->unk20 == 9999) {
        if (req->unk14.vx == 0 && req->unk14.vz == 0) {
            sp->dir.vx = 0;
            sp->dir.vy = req->unk14.vy + req->unk1E;
            sp->dir.vz = 0;
            goto heights;
        }
        sp->dir.vx = req->unk14.vx + rcos(ratan2(req->unk14.vz, req->unk14.vx)) * req->unk1C / 4096;
        sp->dir.vy = req->unk14.vy + req->unk1E;
        sp->dir.vz = req->unk14.vz + rsin(ratan2(req->unk14.vz, req->unk14.vx)) * req->unk1C / 4096;
    } else {
        sp->dir.vx = req->unk14.vx + rcos(req->unk20) * req->unk1C / 4096;
        sp->dir.vy = req->unk14.vy;
        sp->dir.vz = req->unk14.vz + rsin(req->unk20) * req->unk1C / 4096;
    }
heights:
    unset = 0x80000;
    sp->hit = -1;
    lists = &vis_lists;
    sp->h124 = unset;
    sp->h144 = unset;
    sp->h138 = unset;
    sp->h120 = unset;
    sp->h11c = unset;
    sp->h118 = unset;
    sp->h114 = unset;
    sp->h128 = unset;
    sp->h12c = unset;
    sp->h130 = unset;
    sp->h134 = unset;
    sp->flags = 0;
    sp->world = lists->unkC0;
    sp->rel[0] = lists->unk11C0;
    sp->rel[1] = lists->unk22C0;
    /* the record index is stored as a word here */
    record = lists->sources[0];
    sp->rel[2] = lists->unk33C0;
    *(u32 *)&lists->unk47D8 = record;
    func_800468c0(pos, req->unk0);
    sp->world = vis_lists.unkC0;
    sp->rel[0] = vis_lists.unk11C0;
    sp->rel[1] = vis_lists.unk22C0;
    sp->rel[2] = vis_lists.unk33C0;
    func_80047884(pos, req->unk0);
    if (sp->h128 == 0x80000) {
        sp->n170 = sp->n178;
        sp->h128 = sp->h12c;
    }
    if (sp->h130 == 0x80000) {
        sp->n180 = sp->n188;
        sp->h130 = sp->h134;
    }
    if (sp->h114 == 0x80000) {
        sp->n158 = sp->n168;
        sp->h114 = sp->h11c;
    }
    if (sp->h118 == 0x80000) {
        sp->n160 = sp->n158;
        sp->h118 = sp->h114;
    }
    if (sp->h114 != 0x80000 && sp->n158.vy != 0) {
        r = SquareRoot0(sp->n158.vx * sp->n158.vx + sp->n158.vz * sp->n158.vz);
        ang = ratan2(-sp->n158.vy, r);
        if (r != 0)
            sp->h114 = (sp->h114 << 12) / rsin(ang);
    }
    if (sp->h118 != 0x80000 && sp->n160.vy != 0) {
        /* retail squares the second normal's z with the first's */
        r = SquareRoot0(sp->n160.vx * sp->n160.vx + sp->n160.vz * sp->n158.vz);
        ang = ratan2(-sp->n160.vy, r);
        if (r != 0)
            sp->h118 = (sp->h118 << 12) / rsin(ang);
    }
    if (sp->h120 < 256 - req->unk14.vy)
        hits |= 4;
    floor_motion = req->unk14.vy * -4096 / 4096;
    if (sp->h114 < req->unk1E - floor_motion)
        hits |= 1;
    t = DOT(req->unk14, sp->n170) / 4096;
    if (sp->h128 < req->unk1C - t)
        hits |= 2;
    t = DOT(req->unk14, sp->n180) / 4096;
    if (sp->h130 < req->unk1C - t)
        hits |= 8;
    vis_lists.unk38 = sp->h120;
    vis_lists.unk34 = sp->h124;
    vis_lists.unk30 = sp->h114;
    vis_lists.unk2C = sp->h118;
    vis_lists.unk3C = sp->h128;
    vis_lists.unk40 = sp->h12c;
    vis_lists.unk44 = sp->h130;
    vis_lists.unk48 = sp->h134;
    vis_lists.unk4C = sp->h138;
    vis_lists.unk50 = sp->h13c;
    vis_lists.unk54 = sp->h140;
    vis_lists.unk58 = sp->h144;
    vis_lists.unk7C = sp->n150;
    vis_lists.unk84 = sp->n158;
    /* the wall normals: SVECTORs over vis_lists' unk8C and unk9C fields */
    *(SVECTOR *)&vis_lists.unk8C = sp->n170;
    vis_lists.unk94 = sp->n178;
    *(SVECTOR *)&vis_lists.unk9C = sp->n180;
    vis_lists.unkA4 = sp->n188;
    vis_lists.unk44C8 = sp->hit;
    vis_lists.unk47DC = sp->flags;
    return hits;
}
