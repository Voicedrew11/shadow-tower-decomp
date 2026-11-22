/*
 * FDAT_372.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through its
 * own move picker. One object, because the picker's switch table is
 * 8-aligned after creature_run_script_command's in the same read-only run.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>
#include <st/menu.h>
#include <LIBGPU.H>
#include <st/light.h>
#include <st/fdat_372.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();
void map_pick_creature_move();
void map_talk_on_player_death();

const FdatHeader fdat_header = {
    0x2a,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_talk_on_player_death, map_hook_none, map_hook_none,
        map_pick_creature_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x5d,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* The scene state: 0 pending, 0xff done (flag 20). */
DATA(0x801eda3c, 0x4, ".bss")
int fdat_801eda3c;

/* Sets up the area: the scene by flag 20, and record 5's flags 0x180 cleared
 * once its stream counter has started. */
ADDRESS(0x801ed4a0, 0x70)
void map_restore_progress(void)
{
    CreatureType *r = &vis_table_a.types[5];

    if (!game_progress.event_flags.bit.f20)
        fdat_801eda3c = 0;
    else
        fdat_801eda3c = 0xff;
    if (game_progress.talk_counts[r->talk_id] != 0)
        r->actions &= ~0x180;
}

/* Until record 5's stream counter starts, the viewer passing z 0x4c000
 * sends table A entry 32; beaten, it changes the lighting of four areas
 * (flag 20) and leaves. */
ADDRESS(0x801ed510, 0x178)
void map_update_events(void)
{
    Creature *e = &vis_table_a.entries[32];
    CreatureType *r = &vis_table_a.types[5];

    if (game_progress.talk_counts[r->talk_id] == 0 && viewer.position.vz > 0x4c000) {
        viewer.state = 5;
        creature_force_talk(e, 0, 0);
        e->hit = 1;
        viewer.state = 0;
        e->flags &= ~0x2000000;
        r->actions &= ~0x180;
    }
    if (fdat_801eda3c == 0 && e->hp == 0) {
        fdat_801eda3c = 1;
        lighting.area_mode[10] = 0x11;
        lighting.area_mode[21] = 0x11;
        lighting.area_mode[32] = 0x11;
        lighting.area_mode[37] = 0x11;
        game_progress.event_flags.word |= 0x100000;
    }
    if (fdat_801eda3c == 1 && e->state == 3) {
        viewer.state = 5;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 0);
        fdat_801eda3c = 0xff;
        viewer.state = 0;
    }
}

/*
 * The entry's move picker, creature_pick_move with one addition: kind 5 waits
 * (action 128) while its record's stream counter is 0.
 */
ADDRESS(0x801ed688, 0x27c)
void map_pick_creature_move(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->script_state == 0xf0 || e->script_state == 0)
        return;
    creature_choose_action();
    if (e->type == 5 && game_progress.talk_counts[r->talk_id] == 0)
        e->action = 128;
    creature_pick_weighted_script();
    switch (e->action) {
    case 8:
        wall = e->wall_heading;
        if ((short)wall == 0x7fff) {
            e->unk5A = e->target_heading = (e->player_heading + 2048) & 0xfff;
        } else {
            d = (((e->player_heading + 2048) & 0xfff) - wall + 1024) & 0xfff;
            if (d < 1024 || d > 3072)
                e->unk5A = e->target_heading = (wall + 3072) & 0xfff;
            else
                e->unk5A = e->target_heading = (wall + 1024) & 0xfff;
        }
        break;
    case 2:
    case 4:
    case 16:
    case 32:
        e->unk5A = e->target_heading = e->player_heading;
        break;
    default:
        e->unk5A = e->target_heading = e->facing;
        break;
    }
    if (e->action != 8)
        e->wall_heading = 0x7fff;
    if (vis_records[e->block].heading_bias_x != 0 || vis_records[e->block].heading_bias_z != 0) {
        x = rcos(e->target_heading - 1024) >> 5;
        z = rsin(e->target_heading - 1024) >> 5;
        dx = x + vis_records[e->block].heading_bias_x;
        dz = z + vis_records[e->block].heading_bias_z;
        e->target_heading = (ratan2(dz, dx) + 1024) & 0xfff;
        x = rcos(e->unk5A) >> 5;
        z = rsin(e->unk5A) >> 5;
        dx = x + vis_records[e->block].heading_bias_x;
        dz = z + vis_records[e->block].heading_bias_z;
        e->unk5A = ratan2(dz, dx);
    }
}

void map_pick_creature_move(void);
#define FDAT_PICK_MOVE map_pick_creature_move

#include "../lib/fdat_creature_step_script.inc"

/* In area 1, while flag 20 is clear, hands table A entry 32 to its exit once
 * its record's stream counter has started. */
ADDRESS(0x801ed9b4, 0x88)
void map_talk_on_player_death(void)
{
    Creature *e;
    CreatureType *r;

    e = &vis_table_a.entries[32];
    r = &vis_table_a.types[5];

    if (game_progress.talk_counts[r->talk_id] != 0
        && !(game_progress.event_flags.word >> 20 & 1) && vis_table_a.zone == 1) {
        game_progress.talk_counts[r->talk_id] = 2;
        e->flags |= 0x2000000;
        creature_force_talk(e, 1, 1);
    }
}
