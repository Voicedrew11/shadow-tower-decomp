/*
 * FDAT_122.BIN: its last two handlers (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Arms table A entries 32..34 and resets those that are not active. */
ADDRESS(0x801ed880, 0x78)
void map_spawn_creature_group(void)
{
    vis_table_a.entries[32].spawn_chance = vis_table_a.entries[33].spawn_chance = vis_table_a.entries[34].spawn_chance = 100;
    if (vis_table_a.entries[32].state == 0)
        creature_spawn_now(&vis_table_a.entries[32]);
    if (vis_table_a.entries[33].state == 0)
        creature_spawn_now(&vis_table_a.entries[33]);
    if (vis_table_a.entries[34].state == 0)
        creature_spawn_now(&vis_table_a.entries[34]);
}

/* Restarts script 0 of an entry of record 0x35 that is running script 1. */
ADDRESS(0x801ed8f8, 0x40)
void map_restart_script_on_block(Creature *e)
{
    if (e->block == 0x35 && e->script_index == 1)
        creature_start_script(e, 0);
}
