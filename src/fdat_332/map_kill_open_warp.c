/*
 * FDAT_332.BIN: its last handler (WIP module; a unit of its own until its neighbours in
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

/* Not exact: all body instructions agree, but retail reserves 40 bytes and
 * saves the four registers from sp+24; this source reserves 32 and saves
 * them from sp+16. The eight bytes at sp+16 are never accessed: either an
 * unused 8-byte local (the sibling scenes keep an SVECTOR there) or a third
 * word pair of outgoing arguments from a five-argument call that was
 * optimized away; neither is evidenced by an instruction. */
/* Entry 33's end: it faces the viewer, finishes its animation, leaves its
 * drops (20/30/100 percent), dies (animation 12) and table B entry 3
 * opens; the area's history is reset (scene word 2). */
ADDRESS(0x801ed9e8, 0x154)
void map_kill_creature_open_warp(void)
{
    Creature *e;
    Vis88 *d3;

    viewer.unk218 = 5;
    e = &vis_table_a.entries[33];
    d3 = &vis_table_b.entries[3];
    player_turn_to_creature(e);
    for (;;) {
        e->anim_frame += e->anim_step;
        if ((short)e->anim_frame >= 0x1000)
            goto animation_done;
        world_run_frame(NULL, NULL, 0);
    }
animation_done:
    creature_force_talk(e, 0, 0);
    creature_force_talk(e, 0, 0);
    vis_table_a.zone_history[3] = 0xff;
    vis_table_a.zone_history[2] = 0xff;
    vis_table_a.zone_history[1] = 0xff;
    vis_table_a.zone_history[0] = 0xff;
    e->drop_chance0 = 20;
    e->drop_chance1 = 30;
    e->drop_chance2 = 100;
    creature_roll_drops(e);
    creature_award_kill_bonuses(e);
    creature_start_script(e, 12);
    e->state = 5;
    e->hurt_timer = 0xfff;
    e->script_state = 0xf0;
    e->anim_frame = 0;
    e->hp = 0;
    d3->unk05 = 0xed;
    viewer.unk218 = 0;
    fdat_801edb3c = 2;
    vis_table_a.unk612C = 0;
    vis_table_a.spawn_cost_total = 10;
    e->status.bytes[2] |= 0x15;
}
