/*
 * ST.EXE progress state: new-game defaults, the per-area snapshots of the
 * entry tables and the save-data image (src/st/save.c). Field names stay
 * address-derived.
 */
#ifndef ST_SAVE_H
#define ST_SAVE_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/equip_slots.h>

/* One table C slot (entries[32..63]) as saved. */
typedef struct {
    u16 item;
    u16 block;
    s32 position[3];     /* 0x04: position */
    u16 rotation[3];     /* 0x10: rotation */
    u8 state;
    u8 area;
} StSaveGroundItem;

/* Five bytes copied as one block (area_state.loaded_area..unk8). */
typedef struct {
    u8 unk[5];
} StBytes5;

/* area_state as the save code addresses it: bytes 4..8 as one block. */
typedef struct {
    u8 unk0[4];
    StBytes5 loaded_ids;
} StAreaBytes;

/* Fourteen bytes copied as one halfword-aligned block. */
typedef struct {
    u16 unk[7];
} StHalves7;

/* The save-data image save_pack_data fills and save_unpack_data reads. */
typedef struct {
    u32 save_id;         /* 0x0000: game_progress.save_id */
    u8 loaded_ids[5];    /* 0x0004: area_state.loaded_area..unk8 */
    u8 unk9[3];
    u32 creatures_gone[0x60]; /* 0x000c: game_progress.creatures_gone */
    u32 ground_items_gone[0x30];
    u32 objects_removed[0x180];
    u32 objects_unlocked[0x180];
    u32 objects_finished[0x180];
    u32 event_flags[2];
    u32 talk_counts[12];
    u32 areas_visited[12];
    u32 creature_seen_bits[5];
    u8 ground_item_counts[4]; /* 0x14c8: vis_table_c.ground_item_counts[2..5] */
    u32 collected_bits[9]; /* 0x14cc: vis_table_c.collected_bits */
    StSaveGroundItem ground_items[32]; /* 0x14f0: table C entries 32..63 */
    u32 status_flags;    /* 0x17f0: viewer fields */
    s32 play_seconds;
    u16 hp;
    u16 mp;
    u16 sp_total;
    u16 creatures_killed;
    u8 unk1800;
    u8 cunes_found;
    u8 items_found;
    u8 unk1803;
    s16 attributes[14];
    s16 attribute_bonuses[14];
    u16 base_attributes[14];
    VECTOR position;     /* 0x1858: viewer position */
    SVECTOR view_rot;    /* 0x1868: viewer rotation (viewer.view_rot) */
    u32 save_point[3];
    s16 status_timers[13][2];
    u8 wear[3];
    u8 unk18B3;
    u8 settings[6];      /* 0x18b4: settings (game_settings) */
    u8 unk18BA[2];
    u32 creature_killed_bits[5];
    u32 vs_items_lost[0x4f];
    u32 inventory_counts[0x4f]; /* 0x1a0c: inventory */
    u32 placed_bits[10];
    u16 equipment[21];
    u8 unk1B9A[2];
    u32 vs_lineup[3];
    u8 item_durability[314]; /* 0x1ba8: each ItemType's durability */
    u8 unk1CE2[2];
    u32 area_light_modes[24]; /* 0x1ce4: lighting.area_mode */
    u16 shop_ids[14];
    u8 shop_stock[14];
    u16 buttons[12];     /* 0x1d6e: the button masks (pad_state.buttons) */
} StSaveData;

void item_counts_clear(void);
void save_reset_new_game(void);
void area_save_snapshot(int area);
void area_restore_snapshot(int area);
void save_pack_data(StSaveData *save, int fresh);
void save_unpack_data(StSaveData *save);
void area_run_frame_hook(void);

#endif
