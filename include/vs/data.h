/*
 * VS.EXE (versus mode) globals that no reconstructed source owns yet. VS
 * links its own copies of many ST.EXE functions; their record types are
 * ST's (st/cd.h, st/gfx.h, ...), but the blocks holding them have VS sizes
 * and addresses, so each is declared here with a VS type and its
 * address-derived VS identity. Only the fields the reconstructed code
 * proves are spelled, at the offsets its instructions use.
 */
#ifndef VS_DATA_H
#define VS_DATA_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>
#include <st/cd.h>
#include <vs/archives.h>
#include <vs/fighter_ids.h>
#include <st/gfx.h>
#include <st/vis.h>

/* Ordering-table lengths, as in ST.EXE (src/st/gfx.c). */
#define VS_OT_LENGTH 4096
#define VS_OT2_LENGTH 64

/* The CD request ring holds 4 entries in VS (ST.EXE: 48). */
#define VS_CD_REQUESTS 4
/* VS.EXE's archive tables: cd_open_vs_archives opens numbers up to 11
 * (vs/archives.h) and the twelfth 80-byte record ends at the load cursor.
 * ST.EXE opens nine, ST_ARCHIVES. */
#define VS_ARCHIVES 12

/* An archive table: its start-sector table and file extent like ST.EXE's
 * StArchive, in an 80-byte record (the rest is unmodeled). */
typedef struct {
    u16 *sectors;
    StFileExtent file;
    u8 unkC[0x50 - 0xc];
} VsArchive;

/*
 * The CD state block at 0x801a4590 (bss): ST.EXE's StCdState without the
 * root counter and vsync words, with a 4-entry ring, then the load cursor
 * and the load buffer.
 */
typedef struct {
    u8 unk0[8];
    StCdRequest requests[VS_CD_REQUESTS];
    StCdRequest *next_free; /* 0xd8: the next free entry (ST: unk9D4) */
    StCdRequest *current; /* 0xdc: the entry being read (ST: unk9D8) */
    VsArchive archives[VS_ARCHIVES]; /* 0xe0: by archive number (cd_archive_open) */
    u_long *load_cursor; /* 0x4a0: load cursor in load_buffer (lineup_screen_show); the menu file buffer (card_screen_run) */
    u_long load_buffer[0x8000]; /* 0x4a4: the 0x20000-byte load buffer */
} VsCdState;

typedef char VsCdRequestRingStride[(sizeof(((VsCdState *)0)->requests[0]) == 0x34) ? 1 : -1];
typedef char VsCdRequestRingCount[(sizeof(((VsCdState *)0)->requests) == 4 * 0x34) ? 1 : -1];
typedef char VsCdRequestRingAt8[(offsetof(VsCdState, requests) == 8) ? 1 : -1];
typedef char VsCdFreeAt0xd8[(offsetof(VsCdState, next_free) == 0xd8) ? 1 : -1];
typedef char VsCdCurrentAt0xdc[(offsetof(VsCdState, current) == 0xdc) ? 1 : -1];


/* Layout check: the offset the code proves. */
typedef char VsCdStateArchivesAt0xE0[offsetof(VsCdState, archives) == 0xe0 ? 1 : -1];
typedef char VsArchiveSize[sizeof(VsArchive) == 0x50 ? 1 : -1];
typedef char VsCdStateLoadCursorAt0x4A0[offsetof(VsCdState, load_cursor) == 0x4a0 ? 1 : -1];
typedef char VsCdStateUnk4A4At0x4A4[offsetof(VsCdState, load_buffer) == 0x4a4 ? 1 : -1];

extern VsCdState vs_cd_state;

/*
 * The graphics state at 0x801624d0 (bss): laid out as ST.EXE's WorldState
 * through the per-frame counters at 0x8338; the frame index and the loading
 * flag sit at other offsets. WIP extent ends with the last field the code reads.
 */
typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
    u_long *packet_base;
    u_long ot[2][VS_OT_LENGTH];
    u_long *ot_cur;      /* 0x80e4 */
    u_long ot2[2][VS_OT2_LENGTH];
    u_long *ot2_cur;
    StPacketBuffer packets[2];
    StPacketBuffer *packet; /* 0x8304 */
    void *unk8308[4];
    void *unk8318;
    s32 vertex_face_counts[2];
    s32 prim_count;
    s32 prim_alloc_count;
    s32 unk832C;
    s32 unk8330;
    s32 unk8334;
    s32 unk8338[5];
    void *model_files[144]; /* 0x834c: model files by id, as in ST.EXE */
    u8 unk858C[0xa3fc - 0x858c];
    StAnimSlot anim_slots[12]; /* 0xa3fc: 12 vertex-animation slots (ST: 16 at 0xa5fc) */
    StAnimSet anim_sets[12]; /* 0xa51c: 12 skeletal-animation slots (ST: 16 at 0xa77c) */
    MeshVertex projected_vertices[1024]; /* 0xa7ec: the renderers' transformed vertices (VsScratchPad.vertices) */
    u8 unkC7EC[4];
    SVECTOR posed_vertices[0x2000]; /* 0xc7f0: the vertices anim_pose interpolates (ST: unkEB40) */
    u8 unk1C7F0[0x2c970 - 0x1c7f0];
    u8 drawn_frames;     /* 0x2c970: frames drawn since the last rate update (round_play) */
    u8 frame_rate;       /* 0x2c971: the frame rate round_play shows */
    u8 frame;            /* 0x2c972: index of the current DRAWENV and buffers (ST: 0x1ecc2) */
    u8 unk2C973;
    u8 loading_screen_active; /* 0x2c974: set while the loading display runs (ST: unk1ECC4) */
    u8 unk2C975[0x2c998 - 0x2c975];
    MATRIX view_matrix;  /* 0x2c998: the camera matrix */
    u8 unk2C9B8[0x2ca78 - 0x2c9b8];
    VECTOR camera_pos;   /* 0x2ca78: the camera position */
    SVECTOR camera_rot;  /* 0x2ca88: the camera angles (model_draw_list) */
} VsWorldState;

/* Layout checks: the offsets the code proves. */
typedef char VsWorldStateAnimSlotsAt0xA3FC[offsetof(VsWorldState, anim_slots) == 0xa3fc ? 1 : -1];
typedef char VsWorldStateVerticesAt0xA7EC[offsetof(VsWorldState, projected_vertices) == 0xa7ec ? 1 : -1];
typedef char VsWorldStatePosedAt0xC7F0[offsetof(VsWorldState, posed_vertices) == 0xc7f0 ? 1 : -1];
typedef char VsWorldStateFrameAt0x2C972[offsetof(VsWorldState, frame) == 0x2c972 ? 1 : -1];
typedef char VsWorldStateCameraAt0x2CA78[offsetof(VsWorldState, camera_pos) == 0x2ca78 ? 1 : -1];

extern VsWorldState vs_gfx_state;

/* 2000 20-byte particle records at 0x801c4a58 (bss): sparks_start_burst
 * starts them, sparks_clear_all clears their lives. */
typedef struct {
    SVECTOR position;        /* 0x00: position */
    SVECTOR velocity;        /* 0x08: velocity */
    u16 life;                /* 0x10: life, 0 when free */
    u8 unk12[2];
} VsSpark;

extern VsSpark sparks[2000];

/* The number of sets in a match, 1 or 3 (0x80063228, .sdata, initially 1);
 * the two-choice dialog menu_vs_setup_draw_choice compares it with its page. */
extern s32 round_count;

/* The VRAM position tim_load_member_async saves for its read's completion
 * callback tim_load_member_done (bss). */
extern s32 DAT_80063304;
extern s32 DAT_80063308;

/* A fighter's attack record: its damage per kind (32 bytes, the stride
 * fighter_apply_attack_hit indexes). */
typedef struct {
    u16 effect;              /* 0x00: effect kind (effect_spawn_attack) */
    s16 offset_x;            /* 0x02..0x06: the effect's offset from the fighter, turned by its facing */
    s16 offset_y;
    s16 offset_z;
    u8 unk8[2];
    s16 strike_frame;        /* 0x0a: the frame it strikes on (fighter_update) */
    u16 gauge_cost;          /* 0x0c: its cost to the second gauge */
    u16 damage[9];           /* 0x0e */
} VsAttack;

/* A hit fighter_apply_hit applies: three base damage amounts and an amount of
 * one further kind (3 + kind). */
typedef struct {
    u16 damage[3];
    s16 kind;
    u16 amount;
} VsHit;

/* A fighter's 70-byte hit record for one attack animation
 * (fighter_update): up to three hits, each landing on its frame within its
 * reach and arc, and up to three lunges that move the fighter between two
 * frames. */
typedef struct {
    u8 hit_count;
    u8 unk1;
    struct {
        s16 frame;
        s16 range;           /* added to the fighter's hit radius */
        s16 arc;
        VsHit hit;
    } hits[3];               /* 0x02 */
    u8 move_count;           /* 0x32 */
    u8 unk33;
    struct {
        s16 from;
        s16 to;
        s16 speed;
    } moves[3];              /* 0x34 */
} VsHitRecord;

typedef char VsHitRecordSize[sizeof(VsHitRecord) == 70 ? 1 : -1];

/* An animation a fighter plays: its id and its frame step (negated to
 * play it backwards). */
typedef struct {
    u8 anim;                 /* animation id */
    u8 unk1;
    u16 anim_step;           /* frame step */
} VsAnimRef;

/* A fighter's animation sound effect (fighters_play_anim_sounds): the tone keyed on
 * and a trigger word (kind in the top nibble: 1 at a frame, 2 at a frame on
 * 11 bits, 8 at a frame by chance, 4 when the frame count / 20 is a
 * multiple of the period; the frame or period below). Tone 255 with
 * trigger 0: no sound. */
typedef struct {
    u16 tone;
    u16 trigger;
} VsFighterSound;

/*
 * A fighter's data block at +0x4df18 to the record's end: the movement
 * code addresses its animations and walking speed through one pointer to
 * it, so it is one object. Offsets in the field names are block-relative.
 */
typedef struct {
    VsAnimRef anims[15];     /* 0x000: animations (13 and 14: id 255 in every record) */
    VsFighterSound sounds[15]; /* 0x03c: the sound of each animation (fighters_play_anim_sounds plays 0-9; 2-7 go with anims[2..7]) */
    VsHitRecord combo_hits[3]; /* 0x078: hit records of the three combo animations (fighter_update) */
    VsAttack attacks[3];     /* 0x14a: attacks (count WIP) */
    s16 move_kind;           /* 0x1aa: movement kind */
    s16 walk_speed;          /* 0x1ac: walking speed */
    s16 turn_speed;          /* 0x1ae: turning speed */
    u8 unk1B0[0x1b2 - 0x1b0];
    s16 origin_height;       /* 0x1b2: the start height, negated (fighters_load_round) */
    s16 radius;              /* 0x1b4: hit radius */
    s16 height;              /* 0x1b6: height */
    s16 max_hp;              /* 0x1b8: full hit points */
    s16 max_gauge;           /* 0x1ba: full second gauge (0: none) */
    u16 resistances[9];      /* 0x1bc: resistance per damage kind */
} VsFighterData;

/* fighters_load_round copies a whole record (0x1ce bytes, halfword aligned). */
typedef char VsFighterDataSize[sizeof(VsFighterData) == 0x1ce ? 1 : -1];

/* The fighters' data blocks by character, copied into VsFighter.data
 * when a round loads (.data, src/vs/data_fighters.c). */
extern VsFighterData fighter_data[151];

/*
 * The two fighters at 0x800633b0 (bss), 0x4e0e8 bytes each (the code
 * indexes them by fighter number and takes the opponent as 1 - number).
 * A fighter's animated model file (st/anim.h StAnimFile) is loaded at its
 * start; only the state fields the code proves are spelled.
 */
typedef struct {
    u8 file[0x4b000];        /* 0x00000: the model file, then unmodeled */
    u_long effect_model[(0x4dee0 - 0x4b000) / 4]; /* 0x4b000: a second mesh model, absent when its first word is 0 */
    StAnimSet *anim_set;     /* 0x4dee0: the fighter's skeletal-animation slot */
    StAnimSlot *anim_slot;   /* 0x4dee4: the fighter's vertex-animation slot (fighter_draw_vertex_anim) */
    u8 anim;                 /* 0x4dee8: animation playing (VsAnimRef.anim); fighters_play_anim_sounds matches it against the sound keys */
    u8 unk4DEE9;
    s16 anim_frame;          /* 0x4deea: animation frame */
    s16 anim_step;           /* 0x4deec */
    u8 unk4DEEE[0x4def0 - 0x4deee];
    VECTOR position;         /* 0x4def0: position */
    s16 rot_x;               /* 0x4df00: rotation (an SVECTOR over 0x4df00..0x4df06): x */
    s16 facing;              /* 0x4df02: facing angle */
    s16 rot_z;               /* 0x4df04: z */
    u8 unk4DF06[0x4df08 - 0x4df06];
    s16 hp;                  /* 0x4df08: hit points */
    s16 gauge;               /* 0x4df0a: second gauge */
    s16 hp_target;           /* 0x4df0c: hit points left by the last hit */
    s16 state;               /* 0x4df0e: state; 3 hit, 4 down; 3 and 4 cannot be hit */
    s16 model_kind;          /* 0x4df10: model kind: 1 skeletal, 2 vertex-animated */
    s16 brightness;          /* 0x4df12: model brightness; 128 and up: the default colour (fighter_draw_skeletal) */
    s16 guard_flash;         /* 0x4df14: state when an attack was blocked */
    s16 hits_left;           /* 0x4df16: hits left in the current attack (fighter_update) */
    VsFighterData data;      /* 0x4df18: the fighter's data block */
} VsFighter;

/* Layout check: the record size the index arithmetic proves. */
typedef char VsFighterSize[sizeof(VsFighter) == 0x4e0e8 ? 1 : -1];
typedef char VsFighterDataAt0x4DF18[offsetof(VsFighter, data) == 0x4df18 ? 1 : -1];

/* The fighters live in shared_buffer (vs/buffer.h VsBuffer.fighter). */

/* The frame's length in fields, which movement and animation multiply by
 * (the debug text's "frm/draw"; .sdata, unowned; read absolutely by the
 * -G8 moves). */
extern s32 frames_per_draw;

/* The round phase (round_play): the fighters can hit each other only in
 * phase 1, the fight; the other phases are the opening, pause, draw, time
 * up, ring-out, win and end screens (bss, unowned). */
extern s32 round_phase;

/* Renderer switches (unowned): a fixed OT depth for every face (.sdata),
 * semi-transparent faces and mirrored winding (bss). */
extern s32 mesh_fixed_depth;
extern s32 mesh_semi_trans;
extern s32 mesh_mirror;

/* frames_per_draw (declared above) is also the animation step the sound
 * triggers scale a fighter's frame step by. */


/* VS's copy of ST.EXE's vis_table_c (bss, 0x80191d00): the 44-byte item
 * records from +0x118; only those are used so far. */
extern ItemTable vis_table_c;

/* A switch menu_item_request_draw_details sets around its count list and ui_draw_number
 * reads (bss). */
extern u16 DAT_8006337a;

/* A light matrix, a light colour matrix, a back colour and the depth-cue
 * near and far depths, as the model renderers load them (fighter_draw_skeletal,
 * effect_draw_model_part). Halfword-aligned: retail copies its initializers with
 * an alignment test. */
typedef struct {
    short l[3][3];
    short pad0;
    short lc[3][3];
    short pad1;
    CVECTOR bk;
    short dq[2];
} VsLights;

/* The fighter being drawn (bss, unowned). */
extern VsFighter *current_fighter;

/* Rotation offsets added to the drawn fighter's x and y angles (.sdata,
 * src/vs/main.c; read absolutely elsewhere). */
extern s32 light_rot_x;
extern s32 light_rot_y;

/* The model renderers' scale (4096, 4096, 4096; .data, src/vs/main.c). */
extern VECTOR light_scale;

/* The lineup column hud_draw_fighter_names shows (bss, unowned). */
extern s32 round_index;

/* The ids hud_draw_fighter_names names come from both players' lineups
 * (vs/menu.h VsLineup.slot). */

/* A 5000-byte file buffer (archive 5 member 3, match_load_resources): the guard
 * flash's mesh model, a centre vertex and three rings of 16 (bss). */
extern u8 guard_flash_model[5000];

/* Rounds won per fighter (bss, unowned): src/vs/main.c proves the first
 * word its own symbol (the second, round_wins_2p, is read absolutely where
 * the first is $gp-relative); hud_draw_round_wins still walks both from it. */
extern s32 round_wins[1];

/* VS band 6: the text scroll switch and the two scroll offsets
 * ui_draw_glyph picks between (ST.EXE's text_scroll_mode/a0/a2; bss, unowned). */
extern u16 text_scroll_mode;
extern u16 DAT_8006333e;
extern u16 text_scroll;

/* VS band 6: the two 0x20000-byte GPU packet buffers gfx_init hands
 * the graphics state (bss; ST.EXE's packet_buffers). */
extern u_long packet_buffers[];

/* VS band 6: seven words and seven halfwords card_draw_port_choice clears, and the
 * two players' requested item ids, -1 for none (bss, unowned). */
extern s32 file_row_clipped[7];
extern u16 DAT_80117598[7];
/* The two players' requested item ids (0..265), -1 for none. Retail's translation units
 * see two sizes: the team, pick and setup screens read both words
 * $gp-relative through an 8-byte array (define VS_PICKED_PAIR before
 * including this header), while the main module and the card screen see
 * only the first word here and the second as requested_item_2p (vs/main.h). */
#ifdef VS_PICKED_PAIR
extern s32 requested_items[2];
#else
extern s32 requested_items[1];
#endif

/* Read-only words the first-pass round sequence and effect code read
 * directly (WIP; 0x80011934 is the start of vs.effect_spawn_run's
 * JUMPTABLES claim, 0x8001150c of vs.round_hud's). */
extern s32 DAT_80011934;
extern s32 DAT_8001147c;
extern s32 DAT_80011480;
extern s32 DAT_80011484;
extern s32 DAT_80011488;
extern s32 DAT_8001148c;
extern s32 DAT_800114cc;
extern s32 DAT_8001150c;

/* A halfword the team screen and the setup page read (bss, unowned). */
extern u16 DAT_8006335c;
#endif
