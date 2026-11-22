/*
 * ST.EXE graphics state at 0x8015d090 (world_state) and the functions of
 * src/st/view.c, render.c, gfx.c, prim.c, quad.c and fade.c. Unmodeled ranges stay opaque;
 * address-derived names stay until behaviour is proven.
 */
#ifndef ST_GFX_H
#define ST_GFX_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* A transformed vertex: its screen XY, then the depth and depth-cue values
 * the face loops average. Faces index vertices by byte offset. */
typedef struct {
    DVECTOR sxy;
    short z;
    short p;
} MeshVertex;

/* One animated-texture slot (see src/st/render.c). */
typedef struct {
    u8 type;
    u8 period;
    u8 countdown;
    u8 step;
    s16 area;
    s16 phase;
    RECT dst;
    RECT src;
} StTexAnim;

#define ST_TEXANIM_SLOTS 16

/* A 3x3 matrix as gte_SetLightMatrix / gte_SetColorMatrix load it (the
 * first 18 bytes of a MATRIX) and its padding; the lighting code assigns
 * it whole. */
typedef struct {
    short m[3][3];
    short pad;
} StLightMatrix;

/*
 * A 0x6c-byte lighting environment: four light matrices
 * (lighting_build_matrices derives 1..3 from 0), a light colour matrix
 * (which gte_SetColorMatrix loads), the back colour and two halfwords
 * copied to the scratchpad.
 */
typedef struct {
    StLightMatrix light[4]; /* 0x00: the world renderers pick one per part */
    StLightMatrix color;
    CVECTOR back;
    s16 depth_cue_near;
    s16 depth_cue_far; /* also vis_cull_by_range's far range (/ 2048) */
} StLight;

/* One packet buffer: its start, its end and the next free word (primitives
 * are allocated from `cur` up to `end`). */
typedef struct {
    u_long *start;
    u_long *end;
    u_long *cur;
} StPacketBuffer;

/*
 * The graphics state: double-buffered drawing and display environments,
 * ordering tables and packet buffers, the animated-texture slots, the
 * camera matrices, the mesh renderers' render mode and the current
 * lighting environment.
 */
/*
 * The screen fade channels fade_add_color accumulates and fade_draw_additive /
 * fade_draw_subtractive draw and clear once per frame: index 0 for modes 1 and 3,
 * index 1 for mode 2. `mode` selects the semi-transparency rate.
 */
typedef struct {
    u8 active[2];
    u8 mode[2];
    u16 r[2];
    u16 g[2];
    u16 b[2];
} StFade;

/* One 40-byte model part: its primitive offset and per-kind counts (the
 * records st/mesh.h's MeshHeader views from 12 bytes earlier). */
typedef struct {
    u32 unk0;
    u32 offset;
    u32 unk8;
    u16 counts[8];
    u8 unk1C[0xc];
} ModelPart;

typedef struct {
    u8 unk0[8];
    u32 parts_count;
    u8 unkC[0xc];
    ModelPart parts[1];
} Model;

/* One vertex-animation slot (src/st/anim.c): the clip key it blends and
 * the per-vertex deltas it allocated. Tagged for its owner back-pointer. */
typedef struct StAnimSlot {
    s16 state;           /* 0x00: state; bit 2 is set on every slot */
    u16 model_id;        /* 0x02: model file id, 0xffff when unset */
    u16 clip;            /* 0x04: clip, 255 when unset */
    u16 key;             /* 0x06: key */
    SVECTOR *from_keyframe; /* 0x08: the key's start keyframe */
    SVECTOR *to_keyframe; /* 0x0c: its end keyframe */
    SVECTOR *deltas;     /* 0x10: end minus start, from mem_alloc */
    struct StAnimSlot **owner; /* 0x14: the owner's slot pointer */
} StAnimSlot;

/* One skeletal-animation slot (src/st/anim_set.c): its model file, the
 * clip block it has run up to and the node buffer it allocated (node,
 * record and sub-record arrays and the node list, types in st/anim.h). */
typedef struct StAnimSet {
    s16 state;           /* 0x00: state; bit 3 is set on every slot */
    u16 model_id;        /* 0x02: model file id, 0xffff when unset */
    u16 clip;            /* 0x04: clip, 255 when unset */
    u16 unk6;
    u_long *node_ids;    /* 0x08: the file's id table */
    u8 *node_table;      /* 0x0c: the file's 4-byte record table */
    struct StAnimBlock *first_block; /* 0x10: the clip's first block */
    u_long blocks_run;   /* 0x14: blocks run */
    u_long block_count;  /* 0x18: the clip's block count */
    u_long length;       /* 0x1c: the clip's length in frames */
    u_long posed_frame;  /* 0x20: the frame last posed, -1 when unset */
    struct StAnimBlock *next_block; /* 0x24: the next block to run */
    struct StAnimNode *nodes; /* 0x28: the node buffer, from mem_alloc */
    struct StAnimRecord *records;
    struct StAnimSubRecord *sub_records;
    struct StAnimList *list;
    struct StAnimSet **owner; /* 0x38: the owner's slot pointer */
} StAnimSet;

/* An owner's animation state (Creature, AreaObject, GroundItem, StEffect,
 * the viewer's slots): a vertex-animation slot or a skeletal node set, by
 * the model file (anim_prepare / anim_prepare_any). The slot's owner field
 * points back at it; model_draw takes its address. */
typedef union {
    StAnimSlot *slot;
    StAnimSet *set;
} StAnimState;

typedef char StAnimStateSize[sizeof(StAnimState) == 4 ? 1 : -1];

typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
    u_long *packet_base; /* 0xe0: CD load buffer, also a primitive work area */
    u_long ot[2][4096];
    u_long *ot_cur;      /* 0x80e4 */
    u_long ot2[2][64];
    u_long *ot2_cur;
    StPacketBuffer packets[2];
    StPacketBuffer *packet; /* 0x8304 */
    void *area_geometry[4]; /* 0x8308: primitive-pointer slots (func_80017c5c) */
    void *unk8318;
    s32 vertex_face_counts[2];
    s32 prim_count;      /* 0x8324: primitives added by quad_add_at_depth */
    s32 prim_alloc_count; /* 0x8328: primitives allocated by quad_add_at_depth */
    s32 div_prim_count;
    s32 unk8330;
    s32 unk8334;
    s32 unk8338[5];
    /* 0x834c: heap block owners and model files by id: creature_model_load's bank
     * slots (by slot), creature_second_model_load and src/st/slots.c (by slot - 16), the
     * viewer models at 32.. (player_draw_equipment_model), model_draw's model blocks, the
     * animation model files (src/st/anim.c); area_load_update frees the creature
     * models 128..143 and the item models 588..715 on an area change; objects
     * start at 256. */
    void *model_files[716];
    u8 unk8E7C[0xa5fc - 0x8e7c];
    StAnimSlot anim_slots[16]; /* 0xa5fc */
    StAnimSet anim_sets[16]; /* 0xa77c */
    MeshVertex world_vertices[2048]; /* 0xab3c: the renderers' transformed vertices (ScratchPad.vertices) */
    u8 unkEB3C[4];
    SVECTOR posed_vertices[0x2000]; /* 0xeb40: the vertices anim_pose interpolates */
    StTexAnim texanim[ST_TEXANIM_SLOTS];
    u8 drawn_frames;
    u8 frame_rate;
    u8 frame;            /* 0x1ecc2: index of the current DRAWENV and buffers */
    u8 unk1ECC3;
    u8 loading_screen_active; /* 0x1ecc4: set while the loading display runs */
    u8 unk1ECC5[8];
    u8 unk1ECCD;
    u8 unk1ECCE;
    u8 unk1ECCF;
    u8 unk1ECD0[2];
    u8 unk1ECD2;
    u8 unk1ECD3[3];
    StFade fade;         /* 0x1ecd6 */
    u8 unk1ECE6[2];
    MATRIX view_matrix;
    u8 unk1ED08[0x20];
    MATRIX view_quarter_turns[4]; /* 0x1ed28: the view matrix turned 0..3 quarter turns (ScratchPad.turn) */
    MATRIX pitch_matrix;
    VECTOR camera_pos;
    SVECTOR camera_rot;
    u8 unk1EDE0[0x10];
    s32 unk1EDF0;
    s32 draw_collision;
    int tint_level;      /* 0x1edf8: the mesh renderers' tint strength */
    u8 render_mode;      /* 0x1edfc: MESH_* bits (st/mesh.h) */
    u8 selected_light;
    u8 light_blend;
    u8 unk1EDFF;
    u_long range_near;   /* 0x1ee00: vis_cull_by_range's near range (<< 11) */
    u_long range_mid;    /* 0x1ee04: its middle range (<< 11) */
    StLight *light;      /* 0x1ee08: the current lighting environment */
    StLight *light_from; /* 0x1ee0c: the environment a blend starts from */
    StLight *light_to;   /* 0x1ee10: the environment a blend ends at */
    u8 next_render_mode; /* 0x1ee14: the light-select flags last requested */
    u8 no_plane_test;
    u8 talk_overlay_state;
    /* 0x1ee16..0x1ee19 (src/st/render.c): talk picture overlay state, mode, fade length, frame counter */
    u8 talk_overlay_mode;
    u8 talk_fade_length;
    u8 talk_frame;
} WorldState;

typedef char WorldStateModelFilesAt0x834C[offsetof(WorldState, model_files) == 0x834c ? 1 : -1];
typedef char WorldStateUnk8E7CAt0x8E7C[offsetof(WorldState, unk8E7C) == 0x8e7c ? 1 : -1];
typedef char WorldStateAnimSlotsAt0xA5FC[offsetof(WorldState, anim_slots) == 0xa5fc ? 1 : -1];
typedef char WorldStateWorldVerticesAt0xAB3C[offsetof(WorldState, world_vertices) == 0xab3c ? 1 : -1];
typedef char WorldStatePosedVerticesAt0xEB40[offsetof(WorldState, posed_vertices) == 0xeb40 ? 1 : -1];
typedef char MeshVertexSize[sizeof(MeshVertex) == 8 ? 1 : -1];

extern WorldState world_state;

/* The two 0x20000-byte GPU packet buffers. */
extern u_long packet_buffers[];
/* 128 animated objects of 24 bytes; id 0xffff marks a free one. */
typedef struct {
    u16 model;           /* 0x00: model id - 40; bit 15 hides it */
    u8 kind;             /* 0x02: bits 0-5 kind, bits 6-7 a rotation */
    u8 frame_count;      /* 0x03: frame count */
    u8 frame_period;     /* 0x04: frame period, 0 still */
    u8 frame;            /* 0x05: frame (placements_expand starts it at random below frame_count) */
    u8 unk6[2];
    VECTOR position;     /* 0x08: position */
} PlacedObject;

/*
 * The frame counter gfx_init clears and world_draw_effects_placements advances, then
 * the animated objects it steps. -G8 code reads the counter with lui, so it
 * is not a small object of its own.
 */
typedef struct {
    s32 frame;
    PlacedObject objects[128];
} PlacementObjects;
extern PlacementObjects placement_objects;
/* The CLUT a type-49 texture slot cycles (entries 1-14 rotate). */
extern u16 texanim_clut_buffer[16];

/* src/st/view.c */
int view_point_in_near_cone(void);

/* src/st/render.c */
void texanim_clear_all(void);
StTexAnim *texanim_find_free(void);
int texanim_start(RECT *rect, int period, int step, int type, short area);
void texanim_update(void);

/* src/st/gfx.c */
void gfx_init(void);
void gfx_setup_640x480(void);
void gfx_setup_640x240(void);
void gfx_setup_320x240(void);
void gfx_begin_frame(void);
void gfx_flip(void);
void gfx_draw_ot(void);
void gfx_end_frame(void);
void gfx_set_camera(VECTOR *pos, SVECTOR *rot);

/* src/st/prim.c (the FT3 tint pair is declared with the other renderers'
 * in st/mesh.h) */
void model_offset_textures(Model *model, DVECTOR *uv, DVECTOR *clut, u8 count);
void func_80017c5c(u16 index, void *p);
void func_80017c8c(u16 index, void *p);
u_char *func_80017cb0(u16 offset);
void mesh_nclip_triangle(u16 i0, u16 i1, u16 i2);
void lighting_load_gte(u32 t, int flags);

/* src/st/quad.c */
void quad_draw_now(int x0, int y0, int x1, int y1, u8 u, u8 v, u8 w, u8 h, u8 semi,
                   u16 tpage, u16 clut, u8 r, u8 g, u8 b);
void quad_add(int x0, int y0, int x1, int y1, int u, int v, int w, int h, u8 semi,
                   int tpage, int clut, int r, int g, int b);
void loading_screen_find_file(void);
void loading_screen_show(void);
void loading_screen_end(void);
void loading_screen_step(void);
/* An old-style definition; this is its promoted prototype. */
void quad_add_at_depth(int x0, int y0, int x1, int y1, int u, int v, int w, int h, int semi,
                   int tpage, int clut, int r, int g, int b, int depth);
void area_draw_entry_fade(void);

/* src/st/fade.c */
void fade_draw_additive(void);
void fade_draw_subtractive(void);
void fade_add_color(int r, int g, int b, int scale, int mode);

#endif
