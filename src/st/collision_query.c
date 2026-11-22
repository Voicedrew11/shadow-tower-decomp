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
    CollisionState *v = &vis_lists;
    Creature *a;
    AreaObject *b;
    AreaObjectType *r;
    int dx, dz;
    int n;

    if (v->hits & 0x80) {
        dx = pos->vx - viewer.position.vx;
        dz = pos->vz - viewer.position.vz;
        v->hit_body_dist = SquareRoot0(dx * dx + dz * dz);
        v->hit_body_position = *(VECTOR *)&viewer.position.vx;
        v->hit_body_radius = 256;
        v->hit_body_height = 1000;
    } else if (v->creature_in_reach != -1) {
        a = &vis_table_a.entries[v->creature_in_reach];
        dx = pos->vx - a->position.vx;
        dz = pos->vz - a->position.vz;
        v->hit_body_dist = SquareRoot0(dx * dx + dz * dz);
        v->hit_body_position = a->position;
        v->hit_body_radius = a->radius;
        v->hit_body_height = a->height;
    } else if ((n = v->object_in_reach) != -1) {
        b = &vis_table_b.entries[n];
        dx = pos->vx - b->position.vx;
        dz = pos->vz - b->position.vz;
        r = &vis_table_b.types[b->type];
        v->hit_body_dist = SquareRoot0(dx * dx + dz * dz);
        v->hit_body_position = b->position;
        n = b->flags;
        if (n & 0x10)
            v->hit_body_radius = r->radius * b->unk48 >> 7;
        else
            v->hit_body_radius = r->radius;
        vis_lists.hit_body_height = r->height;
    }
}

/*
 * Resolves the request by its kind bits (2: collision_test_map, 0x10: a table A
 * target, 0x40: creature_find_touching, 0x20: a table B entry, 0x80: a projectile)
 * with its height unk1E added for the duration; returns and records the
 * bits that hit.
 */
ADDRESS(0x80049544, 0x184)
int collision_query(CollisionState *req)
{
    int hits = 0;

    if (req->tests & 2)
        hits = collision_test_map(req);
    req->position.vy += req->height;
    if (req->tests & 0x10) {
        if ((vis_lists.creature_in_reach = creature_find_in_reach(req, (req->tests & 0x400) ? 2 : 0)) != -1)
            hits |= 0x10;
    } else
        vis_lists.creature_in_reach = -1;
    if (req->tests & 0x40) {
        if ((vis_lists.creature_touching = creature_find_touching(req)) != -1)
            hits |= 0x10;
    } else
        vis_lists.creature_touching = -1;
    if (req->tests & 0x20) {
        if ((vis_lists.object_in_reach = object_find_in_reach(req)) != -1)
            hits |= 0x40;
    } else
        vis_lists.object_in_reach = -1;
    if (req->tests & 0x80) {
        if (req->tests & 0x800) {
            req->radius += 512;
            if (player_distance_in_reach(req) != -1)
                hits |= 0x80;
        } else if (player_distance_in_reach(req) != -1)
            hits |= 0x80;
    }
    req->position.vy -= req->height;
    vis_lists.hits = hits;
    return hits;
}
