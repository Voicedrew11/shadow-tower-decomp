/*
 * FDAT_112.BIN: the per-entry handler (WIP module; a unit of its own until its neighbours in
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

void map_step_leaving_creature(Vis212 *e);

/*
 * The current entry's handler: kind 10 wakes once its record's stream
 * counter is 3 (flag 6) and, when hit, gives the viewer status timer 4 and
 * flag 7; kind 5 carries table C item 7 on its hand (node 7, vertex 13) and
 * then walks off (map_step_leaving_creature); then the animation steps as
 * creature_step_script does, with each kind's own next move.
 */
ADDRESS(0x801ed770, 0x420)
void map_step_creature(void)
{
    VisTableA *a = &vis_table_a;
    Vis48 *c7 = &vis_table_c.entries[7];
    Vis212 *e = a->unk6138;
    Vis192 *r = a->unk6134;
    VECTOR v;

    if (e->unk02 == 10) {
        if (!(game_progress.unk1440.word >> 6 & 1)) {
            if (game_progress.unk1448[r->unk09] == 3) {
                e->unk70 &= ~0x2000000;
                game_progress.unk1440.word |= 0x40;
                e->unk1C.bytes[0] = 0;
                e->unk1C.bytes[3] = 1;
                goto next;
            }
        } else if (e->unk04 == 0) {
            creature_despawn(e);
            goto next;
        }
        if (!(game_progress.unk1440.word >> 7 & 1) && e->unk64 != 0) {
            viewer.unk190[4][1] = 6000;
            viewer.unk190[4][0] = 0;
            game_progress.unk1440.word |= 0x80;
            viewer.unk2C |= 0x100;
            e->unk1C.bytes[3] = 0;
            e->unk70 &= ~0x2000000;
        } else if (e->unk05 == 5) {
            game_progress.unk1440.word |= 0x20000000;
        }
    }
next:
    if (e->unk02 == 5) {
        if (e->unk1C.bytes[0] == 0 && game_progress.unk1448[r->unk09] == 1) {
            e->unk1C.bytes[0] = 1;
            creature_start_script(e, 6);
            viewer.unk218 = 5;
            c7->unk22 = 2;
        }
        if (e->unk1C.bytes[0] == 1) {
            if ((short)e->unk2A > 0xa00) {
                anim_set_node_vertex((StAnimSet *)e->unkA4, e->unk0B + 128, 7, 13, &v);
                c7->unk08.vx = e->unk74.vx + v.vx;
                c7->unk08.vy = e->unk74.vy + v.vy - 0x80;
                c7->unk08.vz = e->unk74.vz + v.vz;
            }
        } else if (e->unk1C.bytes[0] == 2) {
            viewer.unk218 = 5;
            map_step_leaving_creature(e);
            return;
        } else if (e->unk64 != 0) {
            c7->unk22 = 2;
            c7->unk2D += 0x20;
        }
    }
    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        if (e->unk02 == 10) {
            if (game_progress.unk1440.word >> 7 & 1) {
                creature_pick_move();
            } else if (game_progress.unk1440.word >> 6 & 1) {
                if (e->unk1C.bytes[0] == 0) {
                    creature_start_script(e, 9);
                    e->unk1C.bytes[0] = 1;
                } else if (e->unk1C.bytes[0] == 1) {
                    e->unk2A = 0xfff;
                    e->unkB0 = 0;
                }
            } else {
                creature_start_script(e, 0);
                e->unk1C.bytes[3] = 3;
            }
        } else if (e->unk02 == 5) {
            switch (e->unk1C.bytes[0]) {
            case 0:
                creature_start_script(e, 0);
                break;
            case 1:
                creature_start_script(e, 6);
                e->unk2A = 0xfff;
                ground_item_pick_up(c7, 1);
                creature_start_script(e, 7);
                e->unk1C.bytes[0] = 2;
                break;
            }
        } else {
            creature_pick_move();
        }
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}
