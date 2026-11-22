/*
 * ST.EXE globals that no reconstructed source owns yet and that several
 * modules share. Each declaration spells only the fields the reconstructed
 * code proves, at the offsets its instructions use.
 */
#ifndef ST_DATA_H
#define ST_DATA_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBCD.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/equip_slots.h>
#include <st/pad_buttons.h>
#include <st/player_ids.h>

/* PSX.EXE's next-program byte and the two mailbox bytes after it. */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
#define MAILBOX_BYTE_2 ((u8 *)0x800103f2)

/*
 * Words at the end of the game .rdata, before LIBCD's: 0x80013974 holds the
 * bss start, 0x80013978..0x80013980 the overlay area 0x801eb560, 0x80013988
 * its end. main stores the heap base at 0x8001398c. src/st/rdata_bss_bounds.c
 * defines them (const there); main addresses each with its own lui pair, as
 * separate scalars.
 */
extern u8 *overlay_area_start;
extern u8 *overlay_area_end;
extern u8 *heap_base;

/* src/st/loop.c: the game loop runs while it is 0; 2 leaves for TITLE.EXE,
 * 3 for END.EXE. */
extern s32 game_exit_request;

/* Block heap arena (0x56400 bytes). */
extern StBlock model_heap[];

/*
 * The viewer block at 0x80198f28: the vsync seconds clock at +0x33/+0x34,
 * the FN. slots at +0x38, the position at +0x248 and two angles (pitch,
 * yaw) at +0x268.
 */
/* A viewer model slot's position (+0xc) and rotation (+0x14). */
typedef struct {
    u8 unk0[2];
    u16 reach;           /* 0x02: reach (player_update_hand_action) */
    u16 action_step;     /* 0x04: per-frame step of the hand action timer, scaled by viewer.attributes[1] (player_update_hand_action) */
    u16 hit_time;        /* 0x06: player_start_hand_action copies it to viewer.unk134 */
    u16 sound_time;      /* 0x08: and this one to viewer.action_sound_time */
    u8 unkA[2];
    short pos_x;
    short pos_y;
    short pos_z;
    short shot_vertex;
    SVECTOR rotation;
} EquipmentModel;

/* The first two halfwords are the hand timers; the next two are their
 * hit-window values. Retail also tests the timers together as one word. */
typedef union {
    s32 word;
    s16 halves[4];
} ActionTimers;

typedef char ActionTimersSize[sizeof(ActionTimers) == 8 ? 1 : -1];
typedef char ActionTimersWordWidth[sizeof(((ActionTimers *)0)->word) == 4 ? 1 : -1];
typedef char ActionTimersWordAt0[offsetof(ActionTimers, word) == 0 ? 1 : -1];
typedef char ActionTimersHalvesAt0[offsetof(ActionTimers, halves) == 0 ? 1 : -1];

/* Pitch and yaw offsets also tested together by the per-frame update. */
typedef union {
    s32 word;
    s16 halves[2];
} ViewerAnglePair;

/* A warp destination (viewer.warp_return, viewer.save_area): area, cell and offset,
 * yaw (player_warp). */
typedef struct {
    u8 area;
    u8 x;
    u8 y;
    u8 z;
    s16 dx;
    s16 dy;
    s16 dz;
    u16 yaw;
} StWarp;

typedef char ViewerAnglePairSize[sizeof(ViewerAnglePair) == 4 ? 1 : -1];
typedef char ViewerAnglePairWordWidth[sizeof(((ViewerAnglePair *)0)->word) == 4 ? 1 : -1];
typedef char ViewerAnglePairHalfWidth[sizeof(((ViewerAnglePair *)0)->halves[0]) == 2 ? 1 : -1];
typedef char ViewerAnglePairWordAt0[offsetof(ViewerAnglePair, word) == 0 ? 1 : -1];
typedef char ViewerAnglePairHalvesAt0[offsetof(ViewerAnglePair, halves) == 0 ? 1 : -1];

typedef struct {
    u16 max_hp;
    u16 hp;
    u16 max_mp;
    u16 mp;
    u16 sp_total;        /* 0x8: shown by the status menu */
    u16 weapon_attack_min; /* 0xa: least of weapon_attack[0..2] (player_compute_derived_stats) */
    u16 weapon_attack_max; /* 0xc: greatest of weapon_attack[0..2] */
    u16 magic_attack_right; /* 0xe: shown by the stats menu */
    u16 magic_attack_left; /* 0x10 */
    u16 armour_defence;  /* 0x12..0x18: player_hit's defence terms */
    u16 magic_defence;
    u16 shield_defence;
    u16 shield_magic_defence;
    u16 attack_speed;    /* 0x1a: player_compute_derived_stats */
    u16 magic_speed;
    u16 impact_resistance; /* 0x1e: divides player_take_damage's shake time */
    u16 status_recovery; /* 0x20: scales player_hit's status-time limits */
    u16 equip_weight;    /* 0x22: compared with equip_weight_limit by the equipment screen; player_compute_derived_stats */
    u16 equip_weight_limit; /* 0x24 */
    u8 unk26;
    u8 cunes_found;      /* 0x27: shown by the map menu */
    u16 items_found;
    u8 card_screen_done;
    u8 save_warp_request; /* 0x2b: cleared after the warp player_warp_to_save_point runs */
    u32 status_flags;    /* 0x2c: flag bits the HUD shows (hud_update); status flags (src/st/player.c) */
    u16 creatures_killed; /* 0x30 */
    u8 unk32;
    u8 play_frames; /* incremented every vsync, wraps at 60 */
    s32 play_seconds; /* incremented when play_frames wraps */
    u16 equipment[21]; /* set from FN. ids 970..990 */
    /* Three parallel 14-entry figure arrays (player_reset_stats sets attributes and
     * base_attributes to 50 and clears attribute_bonuses). menu_equip_info_draw_attributes lists attributes; effect_play
     * reads six u16s from 0x78 (attributes[11]..attribute_bonuses[2]) by its level. */
    s16 attributes[14];  /* 0x62 */
    s16 attribute_bonuses[14]; /* 0x7e */
    u16 base_attributes[14]; /* 0x9a: figures creature_award_kill_bonuses adds bonuses to */
    u16 physical_base;   /* 0xb6: player_hit's resistance terms */
    u16 magic_base;      /* 0xb8 */
    u16 weapon_attack[3]; /* 0xba: shown by the stats menu; effect_play reads them as a row */
    u16 weapon_element_attack[6]; /* 0xc0: player_compute_derived_stats writes [0] only */
    u16 accessory_element_bonus[6]; /* 0xcc: player_compute_derived_stats */
    u16 element_defence[6]; /* 0xd8: player_hit's per-kind defence */
    u16 shield_element_defence; /* 0xe4: player_compute_derived_stats */
    u8 unkE6[2];
    EquipmentModel *slot_model_info[4]; /* 0xe8: player_draw_equipment_model's slot positions and rotations */
    void *slot_model_files[4]; /* 0xf8: its slot model blocks (ModelFile, st/mesh.h) */
    void *slot_anims[4]; /* 0x108: its slot animation state: a StAnimSlot (anim_prepare) or
                          * a node set (anim_prepare_any), by the model file's halfword +6 */
    void *magic_defs[2]; /* 0x118: player_load_magic's effects.defs record per slot */
    u8 *magic_buffers[2]; /* 0x120: its load buffers (magic_model_buffers) */
    s32 unk128[2];
    ActionTimers action_timers; /* 0x130..0x137: timers and hit windows */
    s16 action_sound_time[2]; /* 0x138 */
    s16 cast_request[2]; /* 0x13c: per hand, -1 when idle */
    u8 unk140[0x148 - 0x140];
    u8 slot_model_ids[4]; /* 0x148: player_load_equipment_model's item model per slot (255: none); per hand: held item kind */
    u8 magic_ids[2];     /* 0x14c: player_load_magic's item per slot (255: none); per hand: magic id */
    u8 action_recoil;
    u8 shield_lowering;
    u8 slot_anim_ids[4]; /* 0x150 */
    u8 action_started;
    u8 shot_magic[2];    /* 0x155: player_fire_magic_shot's kind per slot (255: none): the magic cast */
    u8 shots_left[2];
    u8 shot_interval[2]; /* 0x159: per hand: reload for shot_countdown */
    u8 shot_countdown[2]; /* 0x15b: per hand: countdown to the next player_fire_magic_shot */
    u8 unk15D;
    s8 quick_item_index; /* 0x15e: quick-item cursor; the HUD frame offset (world_render_frame) */
    u8 unk15F;
    SVECTOR shot_offsets[2]; /* 0x160: player_get_launch_vector's offsets per slot */
    void *prev_block_record; /* 0x170: block_record before the last move (player_update_motion) */
    void *block_record;  /* 0x174: the MapBlock the viewer stands in (st/vis.h) */
    s16 block;
    s16 death_pitch_step;
    s16 unk17C;
    u16 weapon_gauge;    /* 0x17e: HUD bar 0 value */
    u16 swing_gauge;
    u16 magic_gauge;     /* 0x182: HUD bar 1 value */
    u16 cast_gauge;
    u8 hud_redraw;       /* 0x186: 2 when the HUD numbers need redrawing */
    u8 weapon_gauge_delay;
    u8 magic_gauge_delay;
    u8 shield_raised;
    u16 hud_hp;          /* 0x18a: hp as last shown */
    u16 hud_mp;          /* 0x18c: mp as last shown */
    u8 unk18E[2];
    union {
        s16 halves[13][2]; /* level, peak (player_step_status_timer) */
        s32 words[13];     /* both at once: zero when the status is over; the HUD
                            * shows an icon while timer 5.. is nonzero */
    } timers;            /* 0x190: status timers */
/* The halfword view under its first name. */
#define unk190 timers.halves
    u8 hp_regen_interval;
    u8 mp_regen_interval;
    u8 double_damage_chance;
    s8 equipment_light;  /* 0x1c7: lighting_update picks lighting rows by its sign */
    u8 status_time_scale;
    u8 hp_drain_percent;
    u8 mp_drain_percent;
    u8 blocks_magic;
    s16 push_x;
    s16 push_y;
    s16 push_z;
    u8 unk1D2[2];
    u8 slot_sounds[4];   /* 0x1d4: per slot/hand: sound id */
    CdlLOC xa_start;     /* 0x1d8: XA stream start position (src/st/xa.c) */
    /* 0x1dc: its current position. Volatile: the CD ready callback
     * (xa_ready_callback) writes it, and retail keeps its three byte stores in
     * source order where the scheduler would otherwise reorder them. */
    volatile CdlLOC xa_pos;
    s32 xa_length;       /* 0x1e0: its length in sectors */
    u8 unk1E4;
    u8 unk1E5[3];
    u32 xa_start_tick;   /* 0x1e8: cd_state.vsync_count when the stream started */
    s32 damage_number_timers[5]; /* 0x1ec: frames left to show damage_numbers[i] */
    u8 unk200[0x20c - 0x200];
    s16 damage_numbers[5]; /* 0x20c: numbers hud_queue_damage_number queues */
    u8 unk216[2];
    u8 state;            /* 0x218 */
    u8 shield_wear;
    u8 armour_wear;
    u8 weapon_wear;
    u8 update_all_spawns;
    u8 free_anims_after_menu;
    u8 motion_stopped;
    u8 bow_released;
    union {
        u8 bytes[0xc];
        StWarp warp;
    } warp_return;       /* 0x220: the place a warp was entered from (player_warp_back) */
    u8 save_area;
    u8 save_x;
    u8 save_y;
    u8 save_z;
    s16 save_dx;
    s16 save_dy;
    s16 save_dz;
    s16 save_yaw;
    u8 fall_death;
    u8 unk239;
    u8 unk23A[2];
    u32 frame_count;
    u8 unk240[4];
    s32 footstep_timer;
    VECTOR position;     /* 0x248..0x257: includes the copied pad word */
    u8 unk258[8];
    SVECTOR velocity;    /* 0x260: motion (creature_aim_shot leads targets by it) */
    short pitch;         /* 0x268: pitch in view_point_in_near_cone */
    short yaw;           /* 0x26a: yaw in view_point_in_near_cone */
    short roll;          /* 0x26c: roll; model_draw_list turns kind-2 parts by all three */
    short unk26E;
    SVECTOR camera_rot;  /* 0x270 */
    SVECTOR view_rot;    /* 0x278: player_reset_state copies it to 0x268; view pitch (vx) and yaw (vy) player_update_look turns
                          * (band L reads the yaw, view_rot.vy) */
    ViewerAnglePair rot_offset; /* 0x280: view pitch/yaw offsets */
    s16 roll_offset;
    s16 death_timer;
    u16 shake_pitch;     /* 0x288, 0x28c: player_update_shake adds its offsets */
    u16 shake_yaw;
    u16 shake_roll;
    u8 unk28E[2];
    s16 fall_pitch;
    s16 unk292;
    s16 unk294;
    u8 unk296[2];
    MATRIX camera_matrix; /* 0x298: player_get_launch_vector's rotation */
    VECTOR walk_velocity; /* 0x2b8: walk velocity: vx strafes, vz moves ahead (player_update_walk) */
    s16 walk_speed;
    s16 yaw_speed;       /* 0x2ca: yaw speed */
    s16 pitch_speed;     /* 0x2cc: pitch speed */
    short bob_offset;    /* 0x2ce: added to y by world_transform_parts */
    short bob_phase;
    short unk2D2;        /* 0x2d2: added to y by world_transform_parts */
    s16 vertical_speed;
    s16 red_fade;        /* 0x2d6: red fade level, falls by red_fade_step */
    s16 red_fade_step;
    s16 yellow_fade;     /* 0x2da: yellow fade level, falls by yellow_fade_step */
    s16 yellow_fade_step;
    short hand_rot_x;    /* 0x2de..0x2e2: added to player_draw_equipment_model's slot rotations */
    short hand_rot_y;
    short hand_rot_z;
    s32 walk_speed_max;
    s32 yaw_speed_max;   /* 0x2e8: top yaw speed */
    s32 submerged;
    s32 submerged_acid;
    s32 shake_timer;     /* 0x2f4: player_update_shake's countdown, -1 when idle */
} Viewer;

typedef char ViewerActionTimersAt0x130[offsetof(Viewer, action_timers) == 0x130 ? 1 : -1];
typedef char ViewerActionTimersEndAt0x138[offsetof(Viewer, action_sound_time) == 0x138 ? 1 : -1];

typedef char ViewerTimersAt0x190[offsetof(Viewer, timers) == 0x190 ? 1 : -1];
typedef char ViewerTimersEndAt0x1C4[offsetof(Viewer, hp_regen_interval) == 0x1c4 ? 1 : -1];

typedef char ViewerAnglePairAt0x280[offsetof(Viewer, rot_offset) == 0x280 ? 1 : -1];
typedef char ViewerAnglePairSecondAt0x282[offsetof(Viewer, rot_offset) + sizeof(s16) == 0x282 ? 1 : -1];
typedef char ViewerAnglePairEndsAt0x284[offsetof(Viewer, roll_offset) == 0x284 ? 1 : -1];

typedef char ViewerPositionAt0x248[offsetof(Viewer, position) == 0x248 ? 1 : -1];
typedef char ViewerPositionEndsAt0x258[offsetof(Viewer, unk258) == 0x258 ? 1 : -1];

extern Viewer viewer;

/*
 * The state block at 0x801b5920: game_load_start_file sets bytes 4..9 from FN.;
 * area_draw_entry_fade times the screen fade from the tick word at +0x18;
 * byte 9 is the current area the texture slots and table C entries are
 * compared with.
 */
/* The current area's handlers (area_state.hooks): 32 slots, the handler
 * table an overlay starts with after its id (FdatHeader.hooks, st/fdat.h);
 * called without arguments unless noted. area_state.hooks is set only by
 * the area loader area_load_update: to area_hooks_default (every slot
 * area_hook_none) while an area loads, then to the loaded overlay's table
 * (DAT_800881d8). The value sources of each called slot are listed in
 * config/retail/indirect_pointers.tsv. */
typedef struct {
    void (*examine)();   /* 0x00: the examine action, (position, facing) (player_examine) */
    void (*frame)(void); /* 0x04: per frame (area_run_frame_hook) */
    void (*unk08)();
    void (*unk0C)();
    void (*unk10)();
    void (*loaded)(void); /* 0x14: called once the area is loaded (area_load_update) */
    void (*item_added)(int); /* 0x18: item id rose by one (item_add) */
    void (*unk1C)();
    void (*load_object)(void *entry, void *record); /* 0x20: a table B entry kind objects_load does not know */
    void (*update_object)(void); /* 0x24: a table B entry kind objects_update does not handle */
    void (*unk28)();
    void (*unk2C)();
    void (*item_used)(); /* 0x30: item use (item id, or 0x4000 | the table B index when key
                          * item 0x114 unlocks that object) (item_use) */
    void (*player_died)(void); /* 0x34: the player died (player_update) */
    void (*unk38)();
    void (*unk3C)();
    void (*creature_pick_move)(void); /* 0x40: per table A entry, every fourth frame (creatures_update) */
    void (*creature_step_script)(void); /* 0x44: per table A entry, every frame */
    void (*unk48[14])();
} AreaHooks;

typedef char AreaHooksSize[sizeof(AreaHooks) == 0x80 ? 1 : -1];

typedef struct {
    s16 loading;
    volatile u16 load_step; /* area_draw_entry_fade reads it zero-extended, then as a short */
    u8 loaded_area;
    u8 loaded_world;
    u8 loaded_images;
    u8 loaded_sound_bank;
    u8 unk8;
    u8 area;             /* 0x09 */
    u8 unkA[2];
    AreaHooks *hooks;    /* 0x0c: the area's handlers */
    u8 request_area;     /* 0x10: 0xff when unset */
    u8 request_world;
    u8 request_images;   /* 0x12 */
    u8 request_sound_bank; /* 0x13 */
    u8 unk14;            /* 0x14 */
    u8 unk15[3];
    s32 entry_fade_start; /* 0x18: tick area_draw_entry_fade times its fade from, -1 when idle */
    u8 shift_applied;
    u8 loading_world;
    u8 tables_loaded;
    u8 skip_model_sync;  /* 0x1f: nonzero skips the block refresh (band E object passes) */
    u8 shift_x;          /* 0x20 */
    u8 shift_z;
    u8 shift_y;
    u8 shift_x_fine;
    u8 shift_y_fine;
    u8 shift_z_fine;
    u16 unk26;           /* 0x26 */
} AreaState;

extern AreaState area_state;

/* One count per id: a 7-bit count under a flag bit. */
typedef struct {
    u8 count : 7;
    u8 in_bag : 1;
} ItemCount;

/* The counts of the 314 table C ids, then a halfword id table. */
typedef struct {
    ItemCount counts[314];
    u16 shop_ids[14];    /* 0x13a: the shop's item ids */
    u8 shop_stock[14];   /* 0x156: their stock (src/st/save.c) */
    u8 unk164[4];
} Inventory;
extern Inventory inventory;


typedef struct {
    u16 prev_held;       /* 0x00: held of the previous frame (src/st/pad.c) */
    u16 held;            /* 0x02: held bits */
    u16 pressed;         /* 0x04: page bits menu_equip_info_draw consumes; request bits player_start_hand_action clears */
    u16 repeat_held;     /* 0x06..0x0c: src/st/pad.c's repeat and combination words */
    u16 combo_first;
    u16 combo_second;
    u16 pressed_repeat;  /* 0x0c: bits the menu loop menu_main_run consumes; the memory card
                          * screens' port-select bits (src/st/menu.c); the id grid screen's
                          * pressed bits (src/st/idgrid.c) */
    u8 no_input;         /* 0x0e: 1 while the controller is disconnected */
    u8 unkF;
    u8 disabled;
    u8 repeat_timer;
    u8 combo_timer;      /* 0x12 */
    u8 held_changed;
    u8 combo;            /* 0x14: request player_update_hands consumes */
    /* 0x15..0x17: written by the vsync-time pad update; every access reloads them */
    volatile u8 port_state; /* 0x15: last PadGetState(0) */
    volatile u8 disconnected; /* 0x16: disconnected since the last stable state */
    volatile u8 align_pending; /* 0x17: PadStateFindPad seen; actuator alignment pending */
    u8 actuator_count;   /* 0x18: PadInfoAct(0, -1, 0) */
    u8 vibration_frames; /* 0x19: frames of vibration left */
    u8 act_align[6];     /* 0x1a: PadSetActAlign data */
    u8 unk20[2];
    /* 0x22: twelve button masks tested against held / pressed,
     * set by pad_set_default_buttons: the settings page lists them in two columns of
     * six (menu_button_config_draw). [2]/[3] and [6]/[7] turn the view (player_update_look);
     * [8] is slot 0's request bit, [9] the other slots' (player_start_hand_action);
     * [10] the examine and quick-item use button (player_handle_examine). */
    u16 buttons[12];
    u16 action_mask;     /* 0x3a: buttons[8] | buttons[9] | buttons[10] | buttons[11] */
} PadState;
extern PadState pad_state;

/*
 * One sound bank slot: the loaded bank's data (its state halfword at +4)
 * and the VAB id SsVabClose takes. creature_model_load uses slots 8..23 as four
 * rows of four; creature_models_sync addresses slots 8.. and object_models_sync slots
 * 24.. (src/st/slots.c). WIP extent.
 */
typedef struct {
    s16 *block;
    s16 vab;
    s16 unk6;
} StVabSlot;

/* One 0x1000-byte sound bank buffer: its data and load state (3: loaded). */
typedef struct StSoundBuf {
    u8 *data;
    s16 state;
    s16 unk6;
} StSoundBuf;

typedef char StSoundBufSize[(sizeof(StSoundBuf) == 8) ? 1 : -1];
typedef char StSoundBufDataAt0[(offsetof(StSoundBuf, data) == 0) ? 1 : -1];
typedef char StSoundBufStateAt4[(offsetof(StSoundBuf, state) == 4) ? 1 : -1];
typedef char StSoundBufUnk6At6[(offsetof(StSoundBuf, unk6) == 6) ? 1 : -1];


/* One 10-byte sound table entry: the bank slot, program, tone and a note
 * offset. */
typedef struct {
    s16 slot;            /* 0x0: index into the bank slots, -1: none */
    s16 prog;            /* 0x2 */
    s16 tone;            /* 0x4 */
    s16 note;            /* 0x6: added to the requested note */
    s16 unk8;
} StSound;

/*
 * The sound state block at 0x801dd1c0 (bss): the listener's position and
 * rotation, the 322 VAB bank slots, the SPU key status bytes and the sound
 * playing on each voice, the 1232-entry sound table (loaded from an area
 * pack, src/st/area_load.c) and the 16 bank buffers. WIP extent up to
 * equip_menu_state (band Q).
 */
typedef struct {
    u8 unk0;
    u8 unk1[0xc - 0x1];
    s32 unkC;            /* 0x00c: cleared when the area loader's read completes */
    VECTOR listener;     /* 0x010 */
    SVECTOR listener_rot; /* 0x020: vy is the listener's yaw */
    StVabSlot slots[322]; /* 0x028 */
    char keys[24];       /* 0xa38: SpuGetAllKeysStatus buffer */
    s16 voices[24];      /* 0xa50: the sound id keyed on per voice, -1: none */
    u8 unkA80[4];
    StSound sounds[1232]; /* 0xa84 */
    StSoundBuf bufs[16]; /* 0x3aa4 */
    u8 unk3B24[0x3b28 - 0x3b24];
} SoundState;
extern SoundState sound_state;

/* The 16 sound bank buffers (StSoundBuf.data): two, one, four, four, one
 * and four 0x1000-byte blocks below the sound state block. */
extern u8 sound_buffers_magic[2][0x1000];
extern u8 sound_buffer_area[0x1000];
extern u8 sound_buffers_object[4][0x1000];
extern u8 sound_buffers_creature[4][0x1000];
extern u8 sound_buffer_common[0x1000];
extern u8 sound_buffers_equipment[4][0x1000];

/* The CD file buffer cd_read_member loads TIMs and models into (WIP extent:
 * up to texanim_clut_buffer). */
extern u_long cd_file_buffer[];

/* Two 0x4000-byte load buffers (viewer.magic_buffers). */
extern u8 magic_model_buffers[2][0x4000];
/* Four 0x7000-byte model blocks (viewer.slot_model_files). */
extern u8 equipment_model_buffers[4][0x7000];
/* The 64 equipment model placements viewer.slot_model_info points into. */
extern EquipmentModel equipment_model_placements[64];

/* Settings bytes player_init initializes (WIP extent). */
typedef struct {
    u8 bgm_volume;
    u8 sfx_volume;       /* option page level, shown as a slider and a number (menu.c) */
    u8 walk_bob;         /* 0x02..0x05: option page on/off bytes */
    u8 show_status;      /* nonzero while the HUD is drawn (hud_queue_damage_number, world_render_frame) */
    u8 subtitles;        /* 1: the talk picture (TALKMSG.T) is drawn whatever the picture mode (text.c) */
    u8 vibration;
    u8 repeat_interval;
    u8 repeat_delay;
    u8 combo_window;
} GameSettings;
extern GameSettings game_settings;

#endif
