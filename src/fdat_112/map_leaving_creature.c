/*
 * FDAT_112.BIN: the entry's exit (WIP module; a unit of its own until its neighbours in
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

/* The leaving entry: animation 7 (turning to 0x800 at half its speed),
 * then 1 walking along z until 0x4f000. */
ADDRESS(0x801edb90, 0x114)
void map_step_leaving_creature(Creature *e)
{
    e->anim_frame += e->anim_step;
    if (e->script_index == 7) {
        if ((short)e->anim_frame >= 0x1000)
            creature_start_script(e, 1);
        else
            creature_turn_toward(0x800, (short)e->anim_step / 2);
    }
    if (e->script_index == 1) {
        if ((short)e->anim_frame >= 0x1000)
            e->anim_frame = 0;
        e->move_mode = 1;
        e->collision_tests &= ~0x82;
        creature_turn_toward(0x800, 0x40);
        creature_set_velocity(e->facing - 0x400, 0x40);
        if (e->position.vz > 0x4f000) {
            creature_despawn(e);
            viewer.state = VIEWER_STATE_NORMAL;
        }
    }
}
