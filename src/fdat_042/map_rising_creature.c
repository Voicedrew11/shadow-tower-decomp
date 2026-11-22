/*
 * FDAT_042.BIN: its last two handlers and the scene word fdat_801ed970 (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ed970, 0x4, ".bss")
int fdat_801ed970;

void map_pick_creature_move(void);

/*
 * The current entry's handler: while the scene runs (fdat_801ed970 is 1),
 * table A entry 32 sinks 0x180 a frame down to y 0x40000 and then wakes;
 * then the animation steps as creature_step_script does.
 */
ADDRESS(0x801ed7b4, 0x11c)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;

    if (fdat_801ed970 == 1 && e == &vis_table_a.entries[32] && e->anim_slot != 0) {
        e->anim_frame += e->anim_step;
        if (e->position.vy > 0x3ffff) {
            e->position.vy -= 0x180;
            return;
        }
        e->move_mode = 0;
        e->status.bytes[3] = 0;
        fdat_801ed970 = 0xff;
    }
    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        e->script_state = 0xff;
        map_pick_creature_move();
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}

/* The scene: table A entry 32 wakes (animation 2), rises 0xc00 into the
 * current area, and flag 1 is set. */
ADDRESS(0x801ed8d0, 0x9c)
void map_spawn_rising_creature(void)
{
    Creature *e = &vis_table_a.entries[32];

    creature_spawn_now(e);
    e->fade = -1;
    fdat_801ed970 = 1;
    e->status.bytes[2] &= 0xf2;
    game_progress.event_flags.word |= 2;
    creature_start_script(e, 2);
    e->move_mode = 1;
    e->status.bytes[3] = 1;
    fdat_801ed96c = 0;
    e->position.vy += 0xc00;
    e->zone = vis_table_a.prev_zone;
}
