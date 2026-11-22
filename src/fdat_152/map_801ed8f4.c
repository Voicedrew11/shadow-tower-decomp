/*
 * FDAT_152.BIN: the per-entry handler (WIP module; a unit of its own until its neighbours in
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
 * The current entry's handler: the animation steps as creature_step_script does,
 * except that a script of kind 0x17 flies the entry: it keeps 0x640 under
 * the viewer and turns, moves and climbs by its arguments scaled by the
 * animation speed.
 */
ADDRESS(0x801ed8f4, 0x198)
void map_step_creature(void)
{
    Vis212 *e = vis_table_a.unk6138;
    VisScript *s;
    int speed;

    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        creature_pick_move();
    }
    s = e->unkA8;
    if (s == NULL)
        return;
    speed = abs(e->unkB0);
    switch (e->unk0F) {
    case 0x17:
        *(s32 *)&e->unk48 = viewer.position.vy - 0x640;
        break;
    }
    if (e->unk0F == 0x17) {
        e->unk08 = s->unk01;
        creature_turn_toward(e->unk5C, s->arg.s[1] * speed >> 12);
        creature_set_velocity(e->unk8E + s->arg.u[2] - 0x400, s->arg.s[0] * speed >> 12);
        creature_steer_height(viewer.position.vy + s->arg.s[4], s->arg.s[3] * speed >> 12);
        e->unk1A = 0xf0;
    } else {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}
