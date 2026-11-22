/*
 * FDAT_332.BIN: a handler after creature_pick_move (WIP module; a unit of its own
 * until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/fdat.h>

void map_kill_creature_open_warp(void);

/*
 * The step of the overlay's entries, like creature_step_script: an entry of record
 * 4 plays the scene at the end of a run while it is armed.
 */
ADDRESS(0x801ed904, 0xe4)
void map_step_creature(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 4 && fdat_801edb3c == 1) {
            map_kill_creature_open_warp();
            return;
        }
        e->unk1A = 0xff;
        creature_pick_move();
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}
