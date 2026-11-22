/*
 * FDAT_012.BIN: the per-entry handler (WIP module; a unit of its own until its neighbours in
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

void map_start_creature_fall(Creature *e);
void map_step_falling_creature(Creature *e);
void map_trigger_on_approach(Creature *e);

/*
 * The current entry's handler: kind 4 drops once record's stream counter
 * reaches 4 (or when hit) and then runs map_step_falling_creature; kind 10 plays the
 * scene word fdat_801edfac's stages; kinds 1 and 9 bob by their frame;
 * then the animation steps as creature_step_script does.
 */
ADDRESS(0x801ed76c, 0x274)
void map_step_creature(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    int a;
    int offset;

    if (e->type == 4) {
        if (game_progress.talk_counts[r->talk_id] == 4 && e->status.bytes[0] == 0)
            map_start_creature_fall(e);
        if (e->command == 0x80) {
            map_step_falling_creature(e);
            return;
        }
        if (e->hit != 0)
            map_start_creature_fall(e);
    } else if (e->type == 10) {
        if ((unsigned)fdat_801edfac < 2) {
            creature_start_script(e, 8);
            e->anim_step = 0;
            e->status.bytes[3] = 1;
            map_trigger_on_approach(e);
            return;
        }
        if (fdat_801edfac == 2) {
            e->anim_frame += e->anim_step;
            creature_update_script_sound(e);
            if (0xfff - e->anim_step < (short)e->anim_frame) {
                fdat_801edfac = 3;
                e->status.bytes[3] = 0;
            }
            return;
        }
    } else if (e->type == 1 || e->type == 9) {
        if (e->command == 0x20) {
            /* The phase is sign-extended in the variable itself (retail
             * loads it with LH and halves that register); through a (short)
             * cast cse merges the halving into the extension's shift. */
            a = e->anim_frame << 16;
            a >>= 16;
            if (a < 0x400) {
                a >>= 1;
                e->position.vy = e->spawn_y - a;
            } else if (a < 0x800) {
                offset = (a - 0x400) * 5 >> 2;
                offset -= 0x200;
                e->position.vy = e->spawn_y + offset;
            } else {
                offset = (a - 0x800) * 3 >> 3;
                offset -= 0x300;
                e->position.vy = e->spawn_y - offset;
            }
        } else {
            e->position.vy = e->spawn_y;
        }
    }
    e->anim_frame += e->anim_step;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->anim_step >= 0 ? (short)e->anim_frame >= 4096 : (short)e->anim_frame <= 0) {
        e->script_state = 0xff;
        creature_pick_move();
    }
    if (e->script != NULL) {
        creature_run_script_command(e);
        e->script_state = 0xf0;
    }
}
