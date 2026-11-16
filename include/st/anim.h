/*
 * ST.EXE animated model files and their vertex animation (src/st/anim.c).
 * A file starts with its size; offsets in it are from its start. Its mesh
 * model (st/mesh.h) holds part 0's base vertices; keyframes are vertex
 * arrays the clips' keys index. Address-derived names stay until behaviour
 * is proven.
 */
#ifndef ST_ANIM_H
#define ST_ANIM_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/gfx.h>

typedef struct {
    u_long size;         /* 0x00: bytes, header included */
    u16 unk4;            /* 0x04: 0 for a model without animation */
    u16 unk6;
    u_long model;        /* 0x08: offset of the mesh model */
    u_long frames;       /* 0x0c: offset of the keyframe offset table */
    u_long clips;        /* 0x10: offset of the clip offset table */
} StAnimFile;

/* One key: its length in frames and the keyframes it blends between. */
typedef struct {
    u16 length;
    u8 to;
    u8 from;
} StAnimKey;

/* One clip: its key count and the keys' offsets. */
typedef struct {
    u8 count;
    u8 unk1[3];
    u_long keys[1];
} StAnimClip;

/* The 0x28-byte and 0x50-byte records a node set links. */
typedef struct {
    u8 unk0[0x28];
} StAnimSubRecord;

typedef struct {
    u8 unk0[0x44];
    StAnimSubRecord *unk44;
    u8 unk48[8];
} StAnimRecord;

/* A 16-byte list node and the list of them anim_list_init builds. */
typedef struct {
    u_long unk0;
    StAnimRecord *unk4;  /* 0x04 */
    u16 unk8;            /* 0x08: id anim_list_find_id searches */
    u8 unkA;
    u8 unkB;
    s32 unkC;            /* 0x0c: key anim_list_find_key searches */
} StAnimNode;

typedef struct {
    StAnimNode *nodes;
    int count;
    int capacity;
} StAnimList;

/* The owner of the node, 0x50-byte and 0x28-byte record arrays. */
typedef struct {
    u8 unk0[0x28];
    StAnimNode *unk28;
    StAnimRecord *unk2C;
    StAnimSubRecord *unk30;
    StAnimList *unk34;
} StAnimSet;

/* Not reconstructed yet (0x80035008..0x8003569c). */
void func_80035008(StAnimSlot *slot);
int func_8003569c(StAnimSlot **slotp, int id, int clip, int frame);

/* src/st/anim.c */
void anim_register_pack(u16 id, u8 *pack);
void anim_register(u16 id, StAnimFile *file);
void vertices_sub(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count);
void vertices_add(SVECTOR *dst, SVECTOR *a, SVECTOR *b, int count);
StAnimKey *anim_find_key(StAnimFile *file, int clip, int frame, int *key, int *frac);
void anim_get_file(u16 id, StAnimFile **out);
int anim_get_vertex(u16 id, int part, int frame, int index, SVECTOR *out);
void anim_slots_init(void);
void anim_slots_retire(void);
void anim_slot_free(StAnimSlot *slot);
void anim_slots_free_idle(void);
void anim_slots_free_all(void);
void anim_slot_alloc(StAnimSlot **out);
int anim_pose(StAnimSlot **slotp, int id, int clip, int frame);
int anim_prepare(StAnimSlot **slotp, int id, int clip, int frame);
void anim_list_init(StAnimSet *set, int count);
StAnimNode *anim_list_find_id(StAnimList *list, int id);
StAnimNode *anim_list_find_key(StAnimList *list, int key);

#endif
