/*
 * FDAT_012.BIN: the entry's fall (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

/*
 * The falling entry's stages (Creature.status byte 0): 1 falls and pushes the
 * viewer away while near, 2 lands (animation 6), 3 walks off (animation
 * 7) until x 0x3c800. Its hit flash (unk34) runs down meanwhile.
 */
ADDRESS(0x801edbec, 0x320)
void map_step_falling_creature(Creature *e)
{
    CreatureType *r = &vis_table_a.types[e->type];
    VECTOR pos;
    int d;

    e->anim_frame += e->anim_step;
    switch (e->status.bytes[0]) {
    case 1:
        if (e->height_above_floor == 0 && (short)e->anim_frame > 0x800) {
            e->anim = 6;
            e->anim_frame = 0;
            e->anim_step = 0x100;
            e->status.bytes[0] = 2;
            break;
        }
        if ((short)e->anim_frame < 0x400) {
            e->anim_frame += e->anim_step;
        } else {
            if ((short)e->anim_frame < 0x680)
                e->vertical_speed -= r->gravity;
            e->anim_frame += e->anim_step;
            if ((short)e->anim_frame >= 0x1000)
                e->anim_frame = 0xfff;
            creature_set_velocity(0xc00, 0x80);
        }
        pos.vx = e->position.vx + e->velocity.vx;
        pos.vz = e->position.vz + e->velocity.vz;
        d = e->player_dist_xz;
        if (d < e->radius + 0x420) {
            d -= 0x420;
            d = e->radius - d;
            viewer.position.vx += d * rcos((short)e->player_heading - 0x400) >> 12;
            viewer.position.vz += d * rsin((short)e->player_heading - 0x400) >> 12;
        }
        break;
    case 2:
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000) {
            e->anim = 7;
            e->anim_step = 0xa0;
            e->status.bytes[0] = 3;
            e->move_mode = 1;
            e->anim_frame = 0;
            e->collision_tests &= ~2;
        }
        break;
    case 3: {
        int target_x = 0x3c800;
        int target_z = 0x47000;

        if (e->position.vx < target_x) {
            creature_despawn(e);
            break;
        }
        creature_turn_toward(ratan2(e->position.vz - target_z, e->position.vx - target_x) + 0xc00, 0x20);
        creature_set_velocity(e->facing - 0x400, 0x40);
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000)
            e->anim_frame = 0;
        break;
    }
    }
    if (e->hurt_timer >= 0) {
        e->hurt_timer -= abs(e->anim_step);
        if (e->hurt_timer >= 0)
            return;
    }
    e->hurt_timer = -1;
    e->status.bytes[2] &= 0xdf;
}
