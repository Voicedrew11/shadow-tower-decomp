/*
 * ST.EXE: resolving a request (vis_lists' head) against the viewer and
 * tables A and B (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/vis.h>
#include <st/funcs.h>

/*
 * Records the source the request resolved to (the viewer, table A entry
 * unk5C or table B entry unk64): its position, its horizontal distance
 * from `pos` and its two levels at unk44C0/unk44C2.
 */
ADDRESS(0x800492f4, 0x250)
void collision_locate_hit_body(VECTOR *pos)
{
    VisLists *v = &vis_lists;
    Creature *a;
    Vis88 *b;
    Vis24 *r;
    int dx, dz;
    int n;

    if (v->unk74 & 0x80) {
        dx = pos->vx - viewer.position.vx;
        dz = pos->vz - viewer.position.vz;
        v->unkBC = SquareRoot0(dx * dx + dz * dz);
        v->unkAC = *(VECTOR *)&viewer.position.vx;
        v->unk44C0 = 256;
        v->unk44C2 = 1000;
    } else if (v->unk5C != -1) {
        a = &vis_table_a.entries[v->unk5C];
        dx = pos->vx - a->position.vx;
        dz = pos->vz - a->position.vz;
        v->unkBC = SquareRoot0(dx * dx + dz * dz);
        v->unkAC = a->position;
        v->unk44C0 = a->radius;
        v->unk44C2 = a->height;
    } else if ((n = v->unk64) != -1) {
        b = &vis_table_b.entries[n];
        dx = pos->vx - b->unk24.vx;
        dz = pos->vz - b->unk24.vz;
        r = &vis_table_b.unk0000[b->unk0a];
        v->unkBC = SquareRoot0(dx * dx + dz * dz);
        v->unkAC = b->unk24;
        n = b->unk04;
        if (n & 0x10)
            v->unk44C0 = r->unk04 * b->unk48 >> 7;
        else
            v->unk44C0 = r->unk04;
        vis_lists.unk44C2 = r->unk08;
    }
}

/*
 * Resolves the request by its kind bits (2: collision_test_map, 0x10: a table A
 * target, 0x40: creature_find_touching, 0x20: a table B entry, 0x80: a projectile)
 * with its height unk1E added for the duration; returns and records the
 * bits that hit.
 */
ADDRESS(0x80049544, 0x184)
int collision_query(VisLists *req)
{
    int hits = 0;

    if (req->unk0 & 2)
        hits = collision_test_map(req);
    req->unk4.vy += req->unk1E;
    if (req->unk0 & 0x10) {
        if ((vis_lists.unk5C = creature_find_in_reach(req, (req->unk0 & 0x400) ? 2 : 0)) != -1)
            hits |= 0x10;
    } else
        vis_lists.unk5C = -1;
    if (req->unk0 & 0x40) {
        if ((vis_lists.unk60 = creature_find_touching(req)) != -1)
            hits |= 0x10;
    } else
        vis_lists.unk60 = -1;
    if (req->unk0 & 0x20) {
        if ((vis_lists.unk64 = object_find_in_reach(req)) != -1)
            hits |= 0x40;
    } else
        vis_lists.unk64 = -1;
    if (req->unk0 & 0x80) {
        if (req->unk0 & 0x800) {
            req->unk1C += 512;
            if (player_distance_in_reach(req) != -1)
                hits |= 0x80;
        } else if (player_distance_in_reach(req) != -1)
            hits |= 0x80;
    }
    req->unk4.vy -= req->unk1E;
    vis_lists.unk74 = hits;
    return hits;
}
