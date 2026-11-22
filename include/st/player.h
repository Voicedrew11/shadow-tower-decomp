/*
 * ST.EXE player state functions (src/st/player.c, WIP module): the
 * viewer block's hit points, status timers, equipment bonuses and
 * durability, damage and its screen shake, the equipment model and item
 * slots, and the equipment effects. The viewer block itself is st/data.h's
 * Viewer. Address-derived names stay until behaviour is proven; the band's
 * functions that other modules already call keep their st/funcs.h
 * prototypes (player_distance_in_reach, player_update_status, player_hit,
 * player_distance_in_arc, player_load_magic, player_load_equipment_model),
 * as do st.math's (st/funcs.h), st.light's (st/light.h) and st.anim's
 * (st/anim.h). The held items and magic follow in the same unit.
 */
#ifndef ST_PLAYER_H
#define ST_PLAYER_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* Clamps the hit points to their maximum. */
#define PLAYER_CLAMP_HP()                                                      \
    if (viewer.max_hp < viewer.hp) {                                           \
        viewer.hp = viewer.max_hp;                                             \
    }

/* Not reconstructed yet. */
void play_death_sound(void);
void pad_vibrate(int arg0);
void item_counts_clear(void);
void equip_load_item_slot_pictures(void);
struct StEffect *effect_spawn(u8 type, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...); /* src/st/effect_spawn.c: the effect spawned (st/effect.h) or NULL */
#ifndef ST_AREA_REQUEST_LOAD_OLD_STYLE /* one caller has no prototype in scope */
KF_VALUELESS_S32 area_request_load(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5);           /* src/st/area_request.c */
#endif

/* src/st/player.c: state, stats, wear, damage, models */
void player_kill(int unused);
void player_check_fall_death(void);
void player_update_submersion(void);
int func_8003c000(int x, int y, int z, int range, int kind);
void player_reset_state(void);
short player_step_status_timer(short *level, short *peak, int ramp, int max, int apply);
void area_load_set_blocking(int r, int g, int b, int arg3, int arg4, int arg5);
void player_reset_stats(void);
void player_sum_equipment_bonuses(void);
void player_add_hp(int delta);
void player_add_mp(int delta);
void player_check_item_durability(int id);
void player_update_durability_warning(void);
void player_wear_item(int amount, u16 *slot);
void player_wear_armour(int damage, u16 flags);
void player_update_shake(void);
void player_take_damage(int damage, int flags, long *from);
void player_hit_in_radius(u16 *damage, u16 effect, long *pos, int min, int range, int kind);
void player_recover_all(void);
void hud_clear_damage_numbers(void);
void player_reset_new_game(void);
void player_init(void);
void player_stop_motion(void);
int player_distance_within(int x, int y, int z, int range, int kind);
void func_8003ec4c(volatile u8 *p);
void player_start_hand_action(u8 value, u8 slot);
void player_start_cast(u8 unused, u8 slot);
int player_get_launch_vector(int length, VECTOR *pos, SVECTOR *dir, SVECTOR *rot, int slot);

/* Not reconstructed exactly yet (0x8003f910); called with two arguments. */
void player_fire_magic_shot(int kind, int slot, ...);

/* src/st/player.c: held items, magic, view, hand swaps */
void player_update_cast(u8 hand);
void player_cast_magic(u8 magic, u8 hand);
void player_swap_hand_items(u8 a, u8 b);
void player_enter_hurt_state(SVECTOR *a, SVECTOR *b, s16 c, int d);

/* Not reconstructed yet. */
void player_examine(long *pos, SVECTOR *rot);

#endif
