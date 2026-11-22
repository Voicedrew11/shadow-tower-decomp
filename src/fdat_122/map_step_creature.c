/*
 * FDAT_122.BIN: the per-entry handler (WIP module; a unit of its own until its neighbours in
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

void map_restart_script_on_block(Creature *e);

/* The current entry's handler: kind 0 dying sets fdat_801ed938 and the
 * second flag word's bit 15; then the animation steps as creature_step_script
 * does, kind 2 running map_restart_script_on_block after its next move. */
ADDRESS(0x801ed77c, 0x104)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;
    u32 flags;

    if (e->type == 0 && e->state == 5) {
        GameProgress *story = &game_progress;

        flags = story->event_flags2.word;
        fdat_801ed938 = 1;
        story->event_flags2.word = flags | 0x8000;
    }
    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        e->script_state = 0xff;
        creature_pick_move();
        if (e->type == 2)
            map_restart_script_on_block(e);
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}
