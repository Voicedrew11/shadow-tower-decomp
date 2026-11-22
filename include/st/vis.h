#ifndef ST_VIS_H
#define ST_VIS_H

/*
 * ST.EXE visible-set lists (band 0x8001c100..0x8001cb88, WIP). All objects
 * here are unresolved .bss globals; only the members these functions touch
 * are named, as unkXXX until their meaning is proven. The viewer and the
 * area state are shared with other modules (st/data.h); the scratchpad is
 * st/scratch.h.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/data.h>
#include <st/scratch.h>

/* 0x801b5948: the source id list vis_collect_sources fills and the visible id
 * list vis_build_visible builds from it; vis_record_is_visible/vis_id_is_visible search the
 * visible list. */
/* The request fields CollisionState starts with, as a stack request (src/st/item.c,
 * ground_items_update in src/st/objects_ground_items.c). */
typedef struct {
    s32 tests;
    VECTOR position;
    SVECTOR velocity;
    u16 radius;
    u16 height;
    s16 heading;
    s16 speed;
} CollisionRequest;

typedef struct {
    s32 tests;           /* 0x00: the kind of the request collision_query takes */
    VECTOR position;     /* 0x04: its position */
    SVECTOR velocity;    /* 0x14: its velocity */
    u16 radius;          /* 0x1c: reach; added to a table B record radius (object_find_in_reach) */
    u16 height;          /* 0x1e: height added while resolving */
    s16 heading;         /* 0x20: the velocity's heading */
    s16 speed;           /* 0x22: its horizontal length */
    u8 unk24[0x2c - 0x24];
    s32 next_floor_dist; /* 0x2c: floor height found; collision_test_map resolves it (ground_items_update) */
    s32 floor_dist;      /* 0x30: a height player_check_fall_death tests */
    s32 next_ceiling_dist; /* 0x34: collision_test_map's heights (next_floor_dist..acid_surface_y) */
    s32 ceiling_dist;
    s32 wall_dist;
    s32 reach_wall_dist;
    s32 low_wall_dist;
    s32 low_reach_wall_dist;
    s32 surface_y;       /* 0x4c: 0x80000 when unset (player_update_submersion) */
    s32 unk50;
    s32 unk54;
    s32 acid_surface_y;  /* 0x58: 0x80000 when unset */
    s32 creature_in_reach; /* 0x5c: an index into vis_table_a.entries */
    s32 creature_touching; /* 0x60: creature_find_touching's result */
    s32 object_in_reach; /* 0x64: an index into vis_table_b.entries */
    u8 unk68[0xc];
    s32 hits;            /* 0x74: the kind bits collision_query resolved */
    s32 hit_normal_heading; /* 0x78: heading of the wall slid along */
    SVECTOR ceiling_normal; /* 0x7c: collision_test_map's normals (ceiling_normal..low_reach_wall_normal) */
    SVECTOR floor_normal;
    s16 wall_normal_x;   /* 0x8c..0x90: wall normal (bit 1 hits) */
    u8 unk8E[2];
    s16 wall_normal_z;
    u8 unk92[2];
    SVECTOR reach_wall_normal;
    s16 low_wall_normal_x; /* 0x9c..0xa0: wall normal (bit 3 hits) */
    u8 unk9E[2];
    s16 low_wall_normal_z;
    u8 unkA2[2];
    SVECTOR low_reach_wall_normal;
    VECTOR hit_body_position; /* 0xac: the resolved source's position */
    s32 hit_body_dist;   /* 0xbc: its horizontal distance */
    VECTOR vertices[272]; /* 0xc0: collision vertices in world space (src/st/collision.c) */
    VECTOR rel_vertices[272]; /* 0x11c0..: the same relative to the three test points */
    VECTOR next_rel_vertices[272];
    VECTOR low_rel_vertices[272];
    u16 hit_body_radius; /* 0x44c0 */
    u16 hit_body_height; /* 0x44c2 */
    u8 unk44C4[2];
    s16 slide_speed;     /* 0x44c6: push along the wall */
    s16 hit_block;       /* 0x44c8: the visible record the viewer stands in, -1: none (player_update_motion) */
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
    u8 unk46CE[0x47ce - 0x46ce];
    s16 floor_push_x;    /* 0x47ce..0x47d2: cleared by player_update_status */
    s16 floor_push_y;
    s16 floor_push_z;
    u8 unk47D4[0x47d8 - 0x47d4];
    u32 next_floor_block; /* 0x47d8: record index of the floor found */
    s32 floor_kinds;     /* 0x47dc: cleared by player_reset_state */
    u32 floor_kinds_off; /* 0x47e0: flag bits area_reload_images tests */
    s32 floor_y;         /* 0x47e4: floor height (collision_test_map) */
    s32 ceiling_y;       /* 0x47e8: ceiling height */
    u8 unk47EC[4];
} CollisionState;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 part;             /* 0x00: the model part drawn for this record */
    u8 region;           /* 0x01: copied to a table A entry's region (creatures_load) */
    u8 unk02;
    u8 turn;             /* 0x03: bits 0-1: the block's quarter turn about y (quarter_turn_matrices) */
    u8 draw_flags;       /* bit 1: flagged by vis_cull_by_range */
    s8 heading_bias_x;   /* 0x05, 0x06: FDAT: an offset (x, z) its entries' headings bend by */
    s8 heading_bias_z;
    u8 zone;             /* 0x07: area id; 0xff none (creatures_update) */
    u8 cell_x;
    u8 cell_y;
    u8 cell_z;
    u8 y_offset;
} MapBlock;

/* 0x8017d6d0: 64-byte records, a 512-bit mask per source id. */
typedef struct {
    u_long bits[16];
} MapBlockMask;

/* One 40-byte collision part header in the area's collision file
 * (world_state.area_geometry[1]); offsets are from the file start + 12. */
typedef struct {
    s32 vertices;        /* 0x00: SVECTOR vertices */
    s32 n_vertices;      /* 0x04 */
    s32 normals;         /* 0x08: SVECTOR normals, by byte offset */
    s32 unkC;
    s32 faces;           /* 0x10: face blocks */
    s32 unk14;
    u16 n18;             /* 0x18: 20-byte records before the faces */
    u16 n1a;             /* 0x1a: 24-byte records */
    u16 n1c;             /* 0x1c: 28-byte records */
    u16 n1e;             /* 0x1e: 32-byte records */
    u16 n_tris;          /* 0x20: CollisionTri faces */
    u16 unk22;
    u16 n_quads;         /* 0x24: CollisionQuad faces after the triangles */
    u16 unk26;
} CollisionPart;

/* A collision triangle: a push direction, the face flags, its normal and
 * vertices (SVECTOR byte offsets). */
typedef struct {
    u8 push_x;
    u8 push_z;
    u16 flags;           /* 1 floor, 2 wall, 4 ceiling, 0x10..0x800 floor kinds */
    u16 normal;
    u16 v[3];
} CollisionTri;

typedef struct {
    u8 push_x;
    u8 push_z;
    u16 flags;
    u16 normal;
    u16 v[4];
    u16 pad;
} CollisionQuad;

/* The scratchpad while a request is tested (src/st/collision.c). */
typedef struct {
    u8 *base;            /* 0x000: the collision file */
    CollisionPart *part; /* 0x004 */
    u8 *normals;         /* 0x008 */
    u8 *vertices;        /* 0x00c */
    VECTOR *world;       /* 0x010: vis_lists.vertices cursor */
    VECTOR *rel[3];      /* 0x014: unk11C0..unk33C0 cursors */
    u16 turn;            /* 0x020: the record's quarter turn (quarter_turn_matrices) */
    s16 hit;             /* 0x022: the record hit, -1 none */
    VECTOR e[3][4];      /* 0x024: per point, the face edges' outer products */
    VECTOR pos[3];       /* 0x0e4: the test points */
    s32 floor_dist;      /* 0x114: nearest floor */
    s32 next_floor_dist;
    s32 wall_floor_dist;
    s32 ceiling_dist;    /* 0x120: nearest ceiling */
    s32 next_ceiling_dist;
    s32 wall_dist;       /* 0x128: nearest wall */
    s32 reach_wall_dist;
    s32 low_wall_dist;
    s32 low_reach_wall_dist;
    s32 surface_y;
    s32 h13c;
    s32 h140;
    s32 acid_surface_y;
    SVECTOR dir;         /* 0x148: the request's reach */
    SVECTOR ceiling_normal;
    SVECTOR floor_normal;
    SVECTOR next_floor_normal;
    SVECTOR wall_floor_normal;
    SVECTOR wall_normal;
    SVECTOR reach_wall_normal;
    SVECTOR low_wall_normal;
    SVECTOR low_reach_wall_normal;
    SVECTOR push;
    u32 flags;           /* 0x198: floor kind bits */
} CollisionScratch;

#define COLLISION_SCRATCH ((CollisionScratch *)0x1f800000)

/* 0x8008042c: four yaw matrices, 0, 90, 180 and 270 degrees, that turn a
 * map block a quarter at a time (load data, WIP owner). */
extern MATRIX quarter_turn_matrices[4];

extern CollisionState vis_lists;
extern MapBlock vis_records[];
extern MapBlockMask vis_source_masks[];
/* 0x801856d0: per axis (x, y, z) and 4096-unit cell, the sources that cover it. */
extern MapBlockMask vis_axis_masks[3][128];

/* 0x801ba138: 64 entries of 212 bytes, then the collected index list. */
/* A flags word whose byte +2 is also updated on its own (world_draw_creatures). */
typedef union {
    u32 word;
    u8 bytes[4];
} CreatureStatus;

/* A table A record's script: a command stream (WIP). */
typedef struct {
    u8 command;          /* 0x00: kind; 'P' and '@' are picked by creature_take_damage */
    u8 anim;             /* 0x01: the FDAT overlays copy it to Creature.anim */
    u8 unk02;
    u8 wake_range;       /* 0x03: an FDAT overlay squares it into a range */
    u8 weight;           /* 0x04: an FDAT overlay's pick weight */
    u8 sound;            /* 0x05: FDAT sound bank offset; 0xff none */
    u8 unk06[2];
    u32 actions;         /* 0x08: FDAT: the Creature.action states it may run in */
    u16 anim_step;       /* 0x0c */
    u16 sound_trigger;   /* 0x0e: FDAT sound trigger: kind (0xf000) and frame/period */
    /* 0x10: FDAT command arguments; signed or unsigned by command (WIP) */
    union {
        s16 s[16];
        u16 u[16];
        u8 b[32];
        s32 w[8];
        struct {             /* FDAT attack commands (creature_spawn_attack_hits) */
            u16 unk10[2];
            u16 frame[3];    /* 0x14: frame of each hit, 0xffff none */
            s16 effect;      /* 0x1a: passed to effect_spawn_attack */
            u16 vertex[3];   /* 0x1c: model vertex of each hit, 0xffff the origin */
            u16 node[3];     /* 0x22: skeleton node of each hit */
            s16 offset[3];   /* 0x28: offset added after the turn */
        } hit;
    } arg;
} CreatureScript;

typedef struct {
    u8 spawn_chance;     /* 0x00: 0xff when free (creatures_clear) */
    u8 kind;
    u8 type;             /* 0x02: index of its 192-byte CreatureTable record */
    u8 visibility;
    u8 draw_state;
    u8 state;            /* 0x05: state (creature_update_spawn); 1 active, 5 dying (creature_take_damage) */
    u8 region;
    u8 zone;             /* 0x07: area id, 0xfd: the previous area (creatures_bind_previous_area) */
    u8 anim;             /* 0x08 */
    u8 unk09;
    u8 unk0A;
    u8 model_slot;       /* 0x0b: model slot - 128 */
    u8 vram_tag;         /* 0x0c: block tag */
    u8 spawn_cost;       /* 0x0d: added to vis_table_a.spawn_cost_total while live */
    u8 move_mode;
    u8 command;
    u16 drop_item0;      /* 0x10..0x14: the three items it drops: table C ids (creatures_load) */
    u16 drop_item1;
    u16 drop_item2;
    s16 vertical_speed;  /* 0x16: vertical speed */
    u8 script_index;     /* 0x18: index of the running script in its record */
    u8 prev_command;
    u8 script_state;
    u8 unk1B;
    CreatureStatus status; /* 0x1c: byte 2 bits 6/7 near and far range */
    u8 unk20;
    s8 depth_bias;       /* 0x21: OT depth bias */
    s8 voice;            /* 0x22: voice, -1 none */
    u8 unk23;
    s16 unk24;           /* 0x24 */
    u16 block;
    u16 home_block;      /* 0x28: record index (creature_return_home) */
    u16 anim_frame;      /* 0x2a */
    u16 hp;              /* 0x2c: hit points (creature_take_damage) */
    u16 radius;          /* 0x2e */
    u16 height;          /* 0x30: height */
    u16 shadow_size;     /* 0x32: shadow size */
    s16 hurt_timer;      /* 0x34 */
    s16 fade;            /* 0x36 */
    u8 unk38;
    u8 touched_player;
    u16 sound_id;        /* 0x3a: the FDAT overlays' last sound id */
    u32 player_dist_sq;  /* 0x3c: squared distance */
    s32 player_dist_xz;  /* 0x40: horizontal distance */
    s32 action;
    u8 unk48;
    u8 spawned_object;   /* 0x49: 106 + the table B entry creature_alloc_object spawned */
    u8 unk4A[2];
    s32 height_above_floor; /* 0x4c: height above the floor */
    s32 collision_tests; /* 0x50: request kind bits */
    u16 aggressive;
    u16 player_heading;  /* 0x56: facing the viewer is compared with; heading to the viewer */
    u16 player_bearing;  /* 0x58: its difference from the facing */
    s16 unk5A;           /* 0x5a: FDAT: a second heading, set with target_heading */
    s16 target_heading;  /* 0x5c: FDAT: the heading an entry turns to */
    u16 wall_heading;    /* 0x5e: heading of the wall slid along */
    u16 range_bits;
    u16 attack_miss;     /* 0x62: FDAT: 1 or 2 masks a range bit of range_bits for a frame */
    u8 hit;              /* 0x64: set when hit */
    u8 drop_chance0;     /* 0x65..0x67: percent chances of drop_item0..drop_item2 */
    u8 drop_chance1;
    u8 drop_chance2;
    u16 home_facing;     /* 0x68: facing, 0xffff random (creature_reset_facing) */
    s16 home_offset_x;   /* 0x6a..0x6e: offset in its cell (creature_return_home) */
    s16 home_offset_y;
    s16 home_offset_z;
    u32 flags;           /* 0x70: bit 11 set by effect_emit */
    VECTOR position;     /* 0x74: position */
    s32 spawn_y;
    u8 unk88[4];
    u16 rot_x;           /* 0x8c: rotation */
    s16 facing;          /* 0x8e: facing */
    u16 rot_z;
    u8 unk92[2];
    SVECTOR scale;       /* 0x94: scale */
    SVECTOR velocity;    /* 0x9c: velocity: this frame's movement */
    u32 anim_slot;       /* 0xa4: from here the anim block objects.c hands model_draw (&anim_slot) */
    CreatureScript *script; /* 0xa8: the running script, an entry of its record's scripts list */
    CreatureScript *chain_script; /* 0xac: FDAT: the script a 17/20 command returns to */
    s16 anim_step;       /* 0xb0: magnitude of the last anim_frame step; set from the script's anim_step */
    u16 unkB2;
    u8 unkB4[8];
    s16 contact_hit;     /* 0xbc: cleared by the FDAT overlays */
    s16 unkBE;
    u8 unkC0[0x14];
} Creature;

/* An entry of a CreatureType record's scripts list (creature_type_find_script). */
typedef struct {
    u8 command;          /* 0x0: id */
    u8 anim;             /* 0x1: animation (Creature.anim) */
    u8 unk2[3];
    u8 sound;            /* 0x5: sound */
    u8 unk6[6];
    s16 anim_step;       /* 0xc: step of Creature.anim_frame per frame */
} CreatureTalkScript;

/* 0x3500 of CreatureTable: 192-byte records. */
typedef struct {
    u8 unk00;            /* 0x00: 0xff ends the records (creature_types_link_scripts) */
    u8 alert_range;
    u8 aggression;
    u8 depth_bias;
    u8 vram_tag_count;
    u8 move_mode;
    u8 unk06[2];
    u8 unk08;
    u8 talk_id;          /* 0x09: its XA stream file (src/st/xa.c) */
    u8 unk0A[4];
    s16 gravity;         /* 0x0e: gravity */
    u16 kind;            /* 0x10: map bit, 0xffff none */
    u16 unk12;           /* 0x12: added to vis_table_a.unk612C while live */
    u16 near_range;      /* 0x14: FDAT: near range (squared << 16 against Creature.player_dist_sq) */
    u16 radius;
    u16 height;
    u16 shadow_size;
    u32 actions;
    u32 flags;
    s8 kill_bonus[14];   /* 0x24: bonuses to the viewer's figures */
    u16 max_hp;
    u8 unk34[0x16];
    u16 resistances[9];  /* 0x4a: level weights (creature_calc_damage) */
    u8 unk5C[4];
    CreatureScript *scripts[24]; /* 0x60: null-terminated (creature_type_find_script); offsets from vis_table_a.types[16] until creature_types_link_scripts */
} CreatureType;

typedef struct {
    Creature entries[64];
    CreatureType types[58];
    u8 unk6080[0x80];
    u8 count;            /* 0x6100 */
    u8 skip_flags;
    u8 unk6102;
    u8 prev_zone;        /* 0x6103: the previous area id */
    u16 list[12];        /* 0x6104 */
    u8 unk611C[0xc];
    s32 spawn_cost_total; /* 0x6128: sum of the live entries' spawn_cost */
    s32 unk612C;         /* 0x612c: sum of their records' unk12 */
    s32 unk6130;
    CreatureType *current_type; /* 0x6134: the current record */
    Creature *current;   /* 0x6138: the current entry */
    CreatureType *leader_type; /* 0x613c: the record of leader */
    Creature *leader;    /* 0x6140: the entry the current one follows */
    s32 current_index;   /* 0x6144: loop index player_update_hand_action keeps in memory */
    s32 current_type_index; /* 0x6148: the current entry's record index */
    s32 active_count;    /* 0x614c: active entries counted this frame */
    s32 frame;           /* 0x6150: frame counter */
    u8 unk6154[4];
    u16 zone_history[4]; /* 0x6158: the last four area ids */
    s16 push_x;          /* 0x6160..0x6164: cleared by player_update_status */
    s16 push_y;
    s16 push_z;
    u8 unk6166[2];
    u8 zone;             /* 0x6168: the current area id */
    u8 object_sequence_state; /* 0x6169: cleared by area_request_commit */
    u8 unk616A[0x61d8 - 0x616a];
} CreatureTable;

/* 0x801c0310: 240 entries of 88 bytes at +0x1f20, then the index list. */
typedef struct {
    u8 visibility;       /* 0x00: 2 skips the entry (world_draw_objects) */
    u8 unk01;
    u8 anim;             /* 0x02 */
    u8 draw_flags;       /* 0x03: bits 6/7 near and far range */
    u8 flags;
    u8 kind;
    u8 unk06;
    u8 drawn;
    u16 block;
    u16 type;            /* 0x0a: index of its 24-byte AreaObjectTable record */
    u16 state;           /* 0x0c: state of a moving entry (object_move_child); 0xff once opened (src/st/save.c) */
    u16 examined;
    u16 anim_frame;      /* 0x10 */
    u16 height;          /* 0x12: height */
    s16 depth_bias;      /* 0x14: OT depth bias */
    s16 light;           /* 0x16 */
    u32 player_dist_sq;  /* 0x18: squared distance */
    s32 player_dist_xz;  /* 0x1c: distance (sound type 236) */
    u16 player_heading;  /* 0x20: facing */
    u16 player_bearing;  /* 0x22: a door's opening angle (src/st/item.c) */
    VECTOR position;     /* 0x24: position */
    u16 rot_x;           /* 0x34: rotation */
    s16 facing;          /* 0x36: yaw */
    u16 rot_z;
    u8 unk3A[2];
    SVECTOR scale;       /* 0x3c: scale */
    u8 anim_slot[4];     /* 0x44 */
    u8 unk48;            /* 0x48: sound area width (2048 units) */
    u8 unk49;            /* 0x49: sound area depth */
    /* 0x4a: by kind: a sound source's sound (src/st/objects.c) or the record
     * swap and open state of a door (src/st/save.c) */
    union {
        struct {
            u8 sound;    /* 0x4a: sound id */
            u8 volume;   /* 0x4b: volume */
            u8 fade_range; /* 0x4c: fade range */
            u8 sound_flags; /* 0x4d: sound flags */
            u16 period;  /* 0x4e: repeat period */
        } snd;
        struct {
            u16 block_a; /* 0x4a: first record of one swap range */
            u16 block_b; /* 0x4c: first record of the other */
            u8 n_blocks; /* 0x4e: records to swap */
            u8 lock;     /* 0x4f: state, 0xff idle */
        } door;
    } params;
    s32 unk50;           /* 0x50: next tick to play; other kinds keep halfwords here */
    s16 unk54;           /* 0x54: playing voice, -1 none */
    s16 unk56;
} AreaObject;

/* AreaObjectTable+0: 24-byte records. */
typedef struct {
    u8 unk00[3];
    u8 flags;            /* 0x03 */
    u16 radius;          /* 0x04 */
    u16 use_radius;      /* 0x06: trigger radius (object_find_usable) */
    u16 height;          /* 0x08 */
    u16 depth_bias;      /* 0x0a: the entry's OT depth bias (objects_load) */
    u16 shadow_x;        /* 0x0c */
    u16 shadow_z;        /* 0x0e */
    u8 unk10[2];
    u16 push_side;       /* 0x12: which side a door swings the viewer from (objects_update) */
    u8 unk14[4];
} AreaObjectType;

typedef struct {
    AreaObjectType types[330];
    u8 unk1EF0[0x30];
    AreaObject entries[240];
    u8 unk71a0[4];
    s16 push_x;          /* 0x71a4..0x71a8: cleared by player_update_status */
    s16 push_y;
    s16 push_z;
    u8 unk71AA[2];
    AreaObjectType *current_type; /* 0x71ac: the record of the entry objects_update is updating */
    AreaObject *current; /* 0x71b0: an entry object_find_in_reach skips */
    u16 list[96];        /* 0x71b4 */
    u8 count;            /* 0x7274 */
    u8 unk7275;
    s16 unk7276;         /* 0x7276..0x727a: cleared by objects_clear */
    s16 unk7278;
    s16 unk727A;
} AreaObjectTable;

/*
 * 0x801c76f8: the index list, a bit per id at +0x70, 314 records of 44
 * bytes from +0x118 (game_load_start_file reads byte +4 of each), then 64 entries
 * of 48 bytes at +0x3710.
 */
typedef struct {
    u8 unk0[2];
    u8 price;            /* 0x02: carry limit or price base (src/st/menu_shop.c) */
    u8 repair_price;     /* 0x03 */
    u8 model;            /* 0xff: none (game_load_start_file); player_load_equipment_model's model */
    u8 unk5[0xb];
    u8 attributes[14];   /* 0x10: counts menu_arrange_icons_draw_details lists */
    u16 flags;           /* 0x1e */
    /* 0x20: three bonuses (kind, amount) player_sum_equipment_bonuses sums */
    u32 kind0 : 4;
    u32 amount0 : 4;
    u32 kind1 : 4;
    u32 amount1 : 4;
    u32 kind2 : 8;
    u32 amount2 : 8;
    u16 weight;          /* 0x24: menu_arrange_icons_draw_caption shows it / 4.54 */
    u8 category;
    u8 max_durability;   /* 0x27 */
    u8 durability;       /* 0x28: durability, 0 when broken (player_wear_item) */
    u8 unk29[3];
} ItemType;

typedef struct {
    u16 item;
    u16 block;
    u8 anim_slot[4];     /* 0x04 */
    VECTOR position;     /* 0x08: position */
    s16 rot_x;           /* 0x18: rotation */
    s16 facing;
    s16 rot_z;
    u8 unk1E[2];
    u16 unk20;           /* 0x20 */
    u8 state;            /* 0x22: kind; 0 and 4 are not drawn */
    u8 prev_state;
    u8 draw_flags;       /* 0x24 */
    s8 bounces;          /* 0x25: bounces (ground_items_update) */
    s16 vertical_speed;  /* 0x26: fall speed */
    s32 player_dist_xz;  /* 0x28: distance */
    u8 area;
    s8 depth_bias;       /* 0x2d: OT depth bias */
    u8 unk2E[2];
} GroundItem;

typedef struct {
    u8 ground_item_counts[6]; /* [0]: drops made; [2], [3]: slots used of entries[32..47], [48..63] */
    u8 count;            /* 0x0006 */
    u8 list[64];         /* 0x0007 */
    u8 unk0047;
    u32 placed_bits[10]; /* 0x0048: one bit per loaded id (ground_items_load) */
    u32 collected_bits[9]; /* 0x0070: one bit per id (266 ids) */
    u8 model_slots[0x80]; /* 0x0094: a slot per ItemType model, 0xff unassigned (item_model_slots_clear) */
    s32 next_model_slot; /* 0x0114: the next slot to assign */
    ItemType types[314]; /* 0x0118 */
    GroundItem entries[64]; /* 0x3710 */
} ItemTable;

extern CreatureTable vis_table_a;
extern AreaObjectTable vis_table_b;
extern ItemTable vis_table_c;

void vis_collect_sources(VECTOR *a, VECTOR *b, VECTOR *c);
void collision_source_positions(VECTOR *out);
void vis_build_visible(u8 *arg);
void vis_build_planes(void);
int vis_measure_position(void);
void vis_cull_by_range(void);
int vis_record_is_visible(int a, int b, int c);
int vis_id_is_visible(u16 id);
void vis_collect_creatures(void);
void vis_collect_objects(void);
void vis_collect_ground_items(void);

/* src/st/collision_query.c */
void collision_locate_hit_body(VECTOR *pos);
int collision_query(CollisionState *req);
/* src/st/collision.c */
void collision_transform_vertices(VECTOR *pos);
void collision_test_triangles(VECTOR *pos, int kind);
void collision_test_quads(VECTOR *pos, int kind);
int collision_test_map(CollisionState *req);
int creature_find_in_reach(CollisionState *req, int arg1);
int creature_find_touching(CollisionState *req);
int object_find_in_reach(CollisionState *req);

/* src/st/creature.c */
void creature_return_home(Creature *e);
void creature_reset_facing(Creature *e);
int creature_vertex_offset(Creature *e, int index, VECTOR *out);
int creature_aim_shot(Creature *e, VECTOR *pos, int speed, VECTOR *target, SVECTOR *dir, SVECTOR *out,
                  int pitch, int turn, int n);
void creature_copy_type_stats(Creature *e);
void creature_reset_for_spawn(Creature *e);
void creature_despawn(Creature *e);
void creature_spawn(Creature *e, CreatureType *r);
void creature_spawn_by_index(u16 i);
void creature_set_script(Creature *e, u8 *p);
int creature_score_script(u8 *p, int arg);
void creature_pick_script(int arg);
void creature_pick_script_by_distance(void);
u8 *creature_type_find_script(CreatureType *r, u8 id);
void creature_set_script_by_id(Creature *e, u8 id);
void creature_end_script(void);
void creature_set_anim(u8 v);
void creature_update_spawn(void);
int func_8004b5d8(int a, int b, int c);
int vector_point_segment_distance(int x, int y, int z, int r, VECTOR *a, VECTOR *b, int extra);
void creature_set_current(Creature *e);
void creature_anim_step_wrap(Creature *e, short step);
void creature_anim_step_clamp(Creature *e, short step);
int creature_anim_passed_frame(Creature *e, u16 v);
void creature_move(Creature *e);
void creature_move_vertical(void);
VECTOR *creature_get_position(Creature *e, VECTOR *out);
/* VS.EXE has its own port under this name, with another signature; VS units define VS_EXE. */
#ifndef VS_EXE
void effect_random_offset(SVECTOR *off, VECTOR *out, VECTOR *base, int arg3, short height, short up);
#endif
void effect_jitter_offset(SVECTOR *off, VECTOR *out, VECTOR *base);

/* src/st/creature_damage.c */
int creature_sound_note(Creature *e);
void creature_play_script_sound(Creature *e);
void func_8004d7fc(void);
void creature_drop_item(Creature *e, int id);
void creature_roll_drops(Creature *e);
void creature_award_kill_bonuses(Creature *e);
int creature_test_player_aim(Creature *e, short reach);
int creature_calc_damage(Creature *e, int arg, u16 *levels);

/* One 24-byte table A entry of an area file (creatures_load). */
typedef struct {
    u8 spawn_chance;     /* 0x00: 0xff: unused */
    u8 type;             /* 0x01: record index */
    u16 home_block;      /* 0x02: MapBlock index */
    u16 drop_items[3];   /* 0x04: table C ids, 0xffff none */
    u8 spawn_cost;
    u8 drop_chance0;
    u8 drop_chance1;
    u8 drop_chance2;
    u8 unk0E[2];
    u16 home_facing;     /* 0x10: facing */
    s16 home_offset_x;   /* 0x12..0x16: offset in its cell */
    s16 home_offset_y;
    s16 home_offset_z;
} CreatureFileEntry;

/* src/st/creatures.c */
int creature_take_damage(Creature *e, int kind, int power, u16 *levels, int crit);
void creatures_bind_previous_area(void);
void creature_types_link_scripts(void);
void creatures_load(CreatureFileEntry *src);

/* src/st/object_init.c (objects_clear: st/funcs.h) */
void object_reset(AreaObject *e);
void ground_item_set_hidden(int index, int mode);

/* src/st/object_load.c */
void objects_load(u8 *src);

/* src/st/objects_ground_items.c (player_in_cell_rect, player_in_centred_rect, fade_run_frames: st/funcs.h) */
int object_find_usable(int i, CollisionState *req);
void object_start_kind(AreaObject *e, int kind);
AreaObject *object_alloc(int start, int n, int now);
void objects_toggle_by_link_id(int key);
int object_try_key(AreaObject *e, int id);
void objects_try_key(int key);
void func_8004fc84(void);
int object_move_child(AreaObject *src, AreaObject *e, SVECTOR *from, SVECTOR *to, int fade, int steps);
void world_shift_position(int dx, int dy, int dz);
AreaObject *object_spawn_part(AreaObject *src, int kind, int state, int arg3, int size, int angle, int ox,
                     int oy, int oz);
void object_spawn_side_parts(AreaObject *e, int arg);
void object_spawn_forward_parts(AreaObject *e, int arg);
void object_spawn_side_parts_alt(AreaObject *e, int arg);

/* One 24-byte table C entry of an area file. */
typedef struct {
    u16 item;            /* 0x00: ItemType index, 0xffff none */
    u16 unk02;
    u8 cell_x;           /* 0x04..0x06: cell */
    u8 cell_y;
    u8 cell_z;
    u8 unk07;
    s16 offset_x;        /* 0x08..0x0c: offset in the cell */
    s16 offset_y;
    s16 offset_z;
    u16 rot_x;           /* 0x0e..0x12: rotation */
    u16 facing;
    u16 rot_z;
    u8 unk14[2];
    u16 block;
} GroundItemFileEntry;

/* src/st/objects_ground_items.c (ground_items_clear: st/funcs.h; item_model_slots_clear: st/area.h) */
void ground_items_load(GroundItemFileEntry *src);
void player_warp(StWarp *to, int keep);
void player_warp_to_save_point(void);
void player_warp_back(void);
int item_is_uncollected(int id);
void item_record_pickup(u16 *id);
void ground_item_remove(GroundItem *c);
int ground_item_find_in_reach(CollisionState *req, int unused);

/* 0x8005335c: no source yet. */
void ground_item_pick_up(GroundItem *c, int arg1);


#endif
