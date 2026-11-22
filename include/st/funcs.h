/*
 * Prototypes of ST.EXE game functions that no source reconstructs yet. Each
 * signature is the WIP one its callers' argument registers support; the
 * function moves to its module's header once that module exists.
 */
#ifndef ST_FUNCS_H
#define ST_FUNCS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/sound_ids.h>

struct Creature; /* st/vis.h */

void effect_update(void); /* runs effects.cur one frame by its kind (a per-kind switch with a step counter) */
void player_reload_equipment(void);
void player_update_status(int apply);
void player_recalc_stats(void);
void player_hit(u16 *damage, u16 scale, u16 effect, long *from);
void player_get_camera(VECTOR *a, SVECTOR *b);
void player_reset_motion(void);
int player_distance_in_arc(VECTOR *pos, int yaw, int range, int arg3);
void tim_load_images_at(void *data, DVECTOR *a, DVECTOR *b, int arg3);
void player_load_magic(u16 id, u8 slot);
void player_load_equipment_model(u16 id, u8 slot, u8 reload);
void player_enter_normal_state(void); /* src/st/player.c */
void player_update(void);
void game_load_common_data(void);
void area_load_current_blocking(void);
void area_load_update(void);
void image_stream_update(void);
int angle_step_toward(int angle, int to, int step);
void matrix_quarter_turns_y(MATRIX *m, MATRIX *out);
void matrix_rot_x(short angle, MATRIX *out);
void matrix_rot_yxz(SVECTOR *rot, MATRIX *out);
void vector_from_angles(SVECTOR *rot, SVECTOR *out);
void vector_rotate(SVECTOR *rot, SVECTOR *v, VECTOR *out);
void vector_scale(int scale, SVECTOR *v);
int angle_within(int angle, int heading, int range);
int angle_from_xz(int dx, int dz);
int vector_length_xyz(int x, int y, int z);
void vector_to_angles(int dx, int dy, int dz, SVECTOR *out);
int math_step_toward(int v, int to, int step);
int angle_turn_speed(int angle, int heading, int speed, int accel, int decay);
void matrix_rot_y(short angle, MATRIX *out);
void matrix_rot_z(short angle, MATRIX *out);
void matrix_rot_xzy(SVECTOR *rot, MATRIX *out);
void vector_from_angles_zyx(SVECTOR *rot, SVECTOR *out);
void vector_scale_xy(short scale, long *v);
int angle_is_after(int a, int b);
int angle_signed_greater(int a, int b);
int vector_length_xz(int dx, int dz);
void vector_to_angles_ratan2(int dx, int dy, int dz, SVECTOR *out);
int vector_distance_xz_within(VECTOR *p, int x, int y, int z, int range, int above, int below);
int vector_distance_xz_within_rev(VECTOR *p, int x, int y, int z, int range, int above, int below);
int vector_sphere_distance(VECTOR *a, int ra, VECTOR *b, int rb, int height);
int math_random(int n);
int math_random_signed(int n);
int angle_lerp(int from, short to, int t);
void vector_add_scaled(VECTOR *p, SVECTOR *v, int s, VECTOR *out);
int svector_distance_to_segment(SVECTOR *p, SVECTOR *a, SVECTOR *b);
int vector_distance_to_segment(VECTOR *p, VECTOR *a, VECTOR *b);
void creatures_clear(void);
struct Creature *creature_find_in_cone(VECTOR *pos, short yaw, short pitch, int range, int arg4, int arg5, int *dist,
                   int arg7);
void creatures_update(void);
void objects_clear(void);
int player_in_cell_rect(int x, int z, int w, int d, int arg4);
int player_in_centred_rect(int x, int z, int w, int d, int arg4);
void fade_run_frames(int arg0, int arg1, int arg2, int arg3);
void objects_update(void);
void ground_items_clear(void);
void ground_items_update(void);
void save_reset_new_game(void);
void area_run_frame_hook(void);
void pad_init(void);
void pad_read(void);
void pad_enable(void);
void pad_clear_input(void);
void play_warp_sound(void);
void pad_update_vibration(void);
void sound_init(void);
void sound_shutdown(void);
void sound_play_at_range(int id, VECTOR *pos, short volume, int range, int fade, int note);
void sound_play_at(int id, VECTOR *pos, short volume, int note);
int item_is_uncollected(int id);
void player_compute_derived_stats(int arg);
void sound_set_listener(VECTOR *a, SVECTOR *b);
s16 sound_play(int sound, int volume);
void sound_stop(s16 voice, int sound);
void sound_bank_transfer_body(void);
void sound_bank_load(int archive, int index, int slot);
void card_screen_open(int arg0, int arg1);
void card_screen_close(void);
int card_screen_run(int arg0);
void menu_save_prompt_run(void);
void anim_free_all_for_menu(void);
void play_confirm_sound(void);
void play_unlock_sound(void);
void play_locked_sound(void);

/* Called by the band Q screens (src/st/menu_equip_arrange.c, src/st/idgrid.c). */

/* 0x8008809c: the unit z vector vector_from_angles_zyx turns (load data, owner unknown). */
extern SVECTOR unit_vector_z;

#endif
