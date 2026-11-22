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

void map_restart_script_on_block(Vis212 *e);

/* The current entry's handler: kind 0 dying sets fdat_801ed938 and the
 * second flag word's bit 15; then the animation steps as creature_step_script
 * does, kind 2 running map_restart_script_on_block after its next move. */
ADDRESS(0x801ed77c, 0x104)
void map_step_creature(void)
{
    Vis212 *e = vis_table_a.unk6138;
    u32 flags;

    if (e->unk02 == 0 && e->unk05 == 5) {
        Data801cba08 *story = &DAT_801cba08;

        flags = story->unk1444.word;
        fdat_801ed938 = 1;
        story->unk1444.word = flags | 0x8000;
    }
    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        creature_pick_move();
        if (e->unk02 == 2)
            map_restart_script_on_block(e);
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}
