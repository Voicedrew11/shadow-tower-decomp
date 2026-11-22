/*
 * FDAT_182.BIN: the gate scene (WIP module; a unit of its own until its neighbours in
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

#include <st/gfx.h>
#include <st/sound.h>
#include <st/pad.h>
#include <st/bank.h>
#include <st/cd.h>

/* 1 while the area 3 exit scene is armed, 0xff once played; defined here
 * only because each map unit places one COMMON word (src/fdat_182/
 * main.c uses it). */
DATA(0x801edc54, 0x4, ".bss")
int fdat_801edc54;

void fade_run_frames(int arg0, int arg1, int arg2, int arg3);
void creatures_update(void);
void player_enter_normal_state(void);

/*
 * The gate scene of map_use_item_request_warp's event: fades out, puts the viewer at
 * gate 0 or 1 facing in, fades in; at gate 0 table A entry 32 wakes
 * (loading its model if needed) and, unless record 12's stream counter has
 * started, is handed to its exit; otherwise it is set to run.
 */
ADDRESS(0x801ed9cc, 0x284)
void map_warp_player(int gate)
{
    Creature *e = &vis_table_a.entries[32];
    CreatureType *r = &vis_table_a.types[12];
    int index;
    int slot;

    play_warp_sound();
    fade_run_frames(2, 0, 0x1000, 0x100);
    creatures_bind_previous_area();
    if (gate == 0) {
        viewer.position.vx = 0x50800;
        viewer.position.vy = 0x425c0;
        viewer.position.vz = 0x3bc00;
        viewer.view_rot.vy = 0x800;
    } else {
        viewer.position.vx = 0x41800;
        viewer.position.vy = 0x401c0;
        viewer.position.vz = 0x37000;
        viewer.view_rot.vy = 0;
    }
    viewer.state = 0;
    viewer.shake_roll = 0;
    viewer.shake_yaw = 0;
    viewer.shake_pitch = 0;
    viewer.view_rot.vx = 0;
    *(SVECTOR *)&viewer.pitch = viewer.view_rot;
    viewer.state = 5;
    fade_run_frames(2, 0x1000, 0x1000, 0);
    creatures_update();
    if (gate == 0 && e->state != 3 && e->state != 1) {
        index = e->type;
        slot = e->model_slot + 128;
        creature_spawn_now(e);
        if (world_state.unk834C[slot] == NULL)
            creature_model_load(2, index * 5, slot, e->vram_tag & 0xf);
        e->fade = -1;
        e->status.bytes[2] &= 0xf2;
        creature_start_script(e, 9);
        vis_table_a.zone = 3;
    }
    cd_wait_idle();
    play_warp_sound();
    fade_run_frames(2, 0x1000, 0, -0x100);
    pad_clear_input();
    if (gate == 0 && game_progress.unk1448[r->talk_id] == 0) {
        creature_force_talk(e, 0, 0);
        e->flags &= ~0x2000000;
    } else {
        e->action = 0x80;
    }
    vis_table_a.current = e;
    vis_table_a.current_type = r;
    creature_pick_move();
    player_enter_normal_state();
    fdat_801edc50 = 0;
}
