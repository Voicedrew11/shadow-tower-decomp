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

void map_start_creature_fall(Vis212 *e);
void map_step_falling_creature(Vis212 *e);
void map_trigger_on_approach(Vis212 *e);

/*
 * The current entry's handler: kind 4 drops once record's stream counter
 * reaches 4 (or when hit) and then runs map_step_falling_creature; kind 10 plays the
 * scene word fdat_801edfac's stages; kinds 1 and 9 bob by their frame;
 * then the animation steps as creature_step_script does.
 */
ADDRESS(0x801ed76c, 0x274)
void map_step_creature(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    int a;
    int offset;

    if (e->unk02 == 4) {
        if (DAT_801cba08.unk1448[r->unk09] == 4 && e->unk1C.bytes[0] == 0)
            map_start_creature_fall(e);
        if (e->unk0F == 0x80) {
            map_step_falling_creature(e);
            return;
        }
        if (e->unk64 != 0)
            map_start_creature_fall(e);
    } else if (e->unk02 == 10) {
        if ((unsigned)fdat_801edfac < 2) {
            creature_start_script(e, 8);
            e->unkB0 = 0;
            e->unk1C.bytes[3] = 1;
            map_trigger_on_approach(e);
            return;
        }
        if (fdat_801edfac == 2) {
            e->unk2A += e->unkB0;
            creature_update_script_sound(e);
            if (0xfff - e->unkB0 < (short)e->unk2A) {
                fdat_801edfac = 3;
                e->unk1C.bytes[3] = 0;
            }
            return;
        }
    } else if (e->unk02 == 1 || e->unk02 == 9) {
        if (e->unk0F == 0x20) {
            /* The phase is sign-extended in the variable itself (retail
             * loads it with LH and halves that register); through a (short)
             * cast cse merges the halving into the extension's shift. */
            a = e->unk2A << 16;
            a >>= 16;
            if (a < 0x400) {
                a >>= 1;
                e->unk74.vy = e->unk84 - a;
            } else if (a < 0x800) {
                offset = (a - 0x400) * 5 >> 2;
                offset -= 0x200;
                e->unk74.vy = e->unk84 + offset;
            } else {
                offset = (a - 0x800) * 3 >> 3;
                offset -= 0x300;
                e->unk74.vy = e->unk84 - offset;
            }
        } else {
            e->unk74.vy = e->unk84;
        }
    }
    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        creature_pick_move();
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}
