/*
 * FDAT_012.BIN: its last two handlers (WIP module; a unit of its own until
 * its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Arms the scene when the entry comes within 0xc00, then plays script 8
 * once it is defeated or beyond 0x1000. */
ADDRESS(0x801edf0c, 0x7c)
void map_trigger_on_approach(Creature *e)
{
    if (fdat_801edfac == 0) {
        if (e->player_dist_xz < 0xc00)
            fdat_801edfac = 1;
    } else if (fdat_801edfac == 1 && (e->draw_state == 0 || e->player_dist_xz > 0x1000)) {
        fdat_801edfac = 2;
        creature_start_script(e, 8);
    }
}

/* Ends the scene: state 3 and flag 0. */
ADDRESS(0x801edf88, 0x24)
void map_mark_scene_done(void)
{
    fdat_801edfac = 3;
    game_progress.event_flags.bit.f0 = 1;
}
