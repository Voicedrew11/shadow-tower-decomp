#ifndef ST_FDAT_H
#define ST_FDAT_H

/*
 * The FDAT.T map code overlays: ST.EXE loads one entry at 0x801eb560 (the
 * end of its bss) and calls through the handler table at its first byte.
 * Every overlay starts with the same 21 functions (src/lib/fdat_common.inc),
 * spelled with their FDAT_002.BIN addresses; its own handlers follow.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>

/* The table at the overlay's first byte (WIP layout). */
typedef struct {
    s32 id;              /* 0x00: the overlay's id (0x37 in FDAT_002.BIN) */
    void (*hooks[32])(); /* 0x04: handlers */
    s32 unk84;           /* 0x84: id + 0x33 in every overlay */
} FdatHeader;

/* ST.EXE, no source yet. */
void effect_spawn_attack(Creature *e, int arg, VECTOR *pos);
void object_play_sound(AreaObject *e, int sound);

/* ST.EXE functions declared in st/funcs.h, which the overlay units do not
 * include (see below). */
void ground_items_update(void);
void player_update_hand_action(u8 hand);
void player_load_equipment_model(u16 id, u8 slot, u8 reload);
void player_compute_derived_stats(int arg);
void player_update_durability_warning(void);

/*
 * ST.EXE's positional sound starters and player_hit (src/st/sound.c,
 * src/st/player_damage.c). The overlays keep the voice the starters return
 * (Creature.voice, stored from $v0 after the call), which ST.EXE's own
 * st/funcs.h declares void; the overlay units therefore declare these here
 * and do not include st/funcs.h.
 */
int sound_play_at_range(int id, VECTOR *pos, short volume, int range, int fade, int note);
int sound_play_at(int id, VECTOR *pos, short volume, int note);
void player_hit(u16 *damage, u16 scale, u16 effect, long *from);

/* src/lib/fdat_common.inc */
void creature_clear_contact_hit(Creature *e);
void creature_start_script(Creature *e, int id);
void creature_turn_toward(short angle, int step);
void creature_set_velocity(short angle, int speed);
void creature_steer_height(int y, int step);
int creature_random_note(Creature *e);
void creature_emit_script_sound(Creature *e);
void creature_update_script_sound(Creature *e);
void creature_hit_player(Creature *e, CreatureScript *s);
void creature_spawn_attack_hits(Creature *e, CreatureScript *s);
void creature_run_script_command(Creature *e);
void creature_choose_action(void);
int creature_pick_weighted_script(void);
int creature_step_hurt_timer(Creature *e);
void creature_fade_out_despawn(Creature *e);
void player_glide_to_pose(VECTOR *pos, SVECTOR *rot);
void player_turn_to_creature(Creature *e);
void creature_finish_fade_in(Creature *e);
void creature_force_talk(Creature *e, int fixed, int arg2);
void creature_spawn_now(Creature *e);
void map_hook_none(void);

/* The map handlers several overlays share (src/lib/fdat_creature_pick_move.inc
 * and fdat_creature_step_script.inc), claimed at FDAT_002.BIN's addresses. */
void creature_pick_move(void);
void creature_step_script(void);

/*
 * Overlay words one map unit defines and a later unit of the same overlay
 * reads (each name exists in its own overlay only). A name that is a
 * static in another overlay goes in that overlay's own st/fdat_nnn.h.
 */
extern int fdat_801edfac; /* FDAT_012.BIN: src/fdat_012/main.c */
extern int fdat_801edb3c; /* FDAT_332.BIN: src/fdat_332/main.c */
extern int fdat_801ed96c; /* FDAT_042.BIN: src/fdat_042/main.c */
extern int fdat_801ed970; /* FDAT_042.BIN: src/fdat_042/map_rising_creature.c */
extern int fdat_801ed938; /* FDAT_122.BIN: src/fdat_122/main.c */
extern int fdat_801edc50; /* FDAT_182.BIN: src/fdat_182/main.c */
extern int fdat_801edc54; /* FDAT_182.BIN: src/fdat_182/map_warp_player.c */
extern int fdat_801ee0b4; /* FDAT_242.BIN: src/fdat_242/main.c */
extern int fdat_801ee0b8; /* FDAT_242.BIN: src/fdat_242/main.c */
extern int fdat_801ee0bc; /* FDAT_242.BIN: src/fdat_242/main.c */

#endif
