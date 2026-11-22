/*
 * ST.EXE area loading (src/st/area_*.c): the area request in area_state
 * (st/data.h), its load state machine and the CD callbacks it queues.
 * Address-derived names stay until behaviour is proven.
 */
#ifndef ST_AREA_H
#define ST_AREA_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/cd.h>

/* 0x800881dc: set to the overlay area start next to DAT_800881d8; the area
 * module addresses it as an extern with lui, not $gp, and as an aggregate:
 * retail keeps its load in front of a store through a pointer, which the
 * scheduler only does for an in-struct read. */
extern u8 *DAT_800881dc[1];
/* Tables game_load_common_data fills from its pack; WIP extents. */
extern u8 DAT_8009b088[];

/* src/st/area_a.c */
extern u8 *DAT_800881d8;
void area_hook_none(void);
void image_stream_set_ready(StCdRequest *request);
void area_reload_images(void);
void game_load_common_data(void);
void player_init(void);
void item_model_slots_clear(void);

/* 0x800f2c88 (after the block heap) and 0x800fc488: the buffers
 * area_load_current_blocking puts in world_state.unk8308[1] and [0]; WIP extents. */
extern u_long DAT_800f2c88[];
extern u_long DAT_800fc488[];

/* src/st/area_wait.c */
void area_load_current_blocking(void);
void player_shift_position(int dx, int dy, int dz);

/* src/st/area_cb.c */
void func_80045568(void);
void area_load_records_done(void);
void area_load_overlay_done(void);
void area_load_tables_done(void);
void area_load_world_done(void);
void area_load_collision_done(void);
void area_load_images_done(StCdRequest *request);
void area_stream_set_ready(StCdRequest *request);
void func_80045664(void);
void area_request_commit(void);

/* src/st/area_cd.c */
void image_stream_next_run(StCdRequest *request);
void image_stream_update(void);

/* 0x8008038c: the handlers an area uses until its own are loaded (load
 * data, WIP owner; src/st/area_load.c). */
extern AreaHooks area_hooks_default;

/* src/st/area_request.c (also declared in st/player.h) */
#ifndef ST_AREA_REQUEST_LOAD_OLD_STYLE /* one caller has no prototype in scope */
KF_VALUELESS_S32 area_request_load(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5);
#endif

/* WIP prototypes of the rest of the area code */
void area_load_update(void);
void image_stream_queue(int archive, int member, void (*done)());

#endif
