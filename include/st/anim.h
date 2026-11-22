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
    /* A skeletal file (unk6, its node count, nonzero; src/st/anim_set.c)
     * has its node id table at `frames`, 4-byte node records at `clips`
     * and this offset of its clip offset table. */
    u_long unk14;
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

/* The 0x28-byte and 0x50-byte records a node set links: a node's scale,
 * rotation and translation, and the local and composed matrices built
 * from them. */
typedef struct StAnimSubRecord {
    VECTOR scale;        /* 0x00 */
    SVECTOR rot;         /* 0x10 */
    VECTOR trans;        /* 0x18 */
} StAnimSubRecord;

typedef struct StAnimRecord {
    u16 unk0;            /* 0x00: set once unk24 is composed for this pose */
    u16 unk2;            /* 0x02: set when a relative transform changed unk4 */
    MATRIX unk4;         /* 0x04: the node's own transform */
    MATRIX unk24;        /* 0x24: unk4 composed with the parents' */
    StAnimSubRecord *unk44;
    struct StAnimRecord *unk48; /* 0x48: the parent node's record, or NULL */
    u8 unk4C[4];
} StAnimRecord;

/* A 16-byte list node and the list of them anim_list_init builds. */
typedef struct StAnimNode {
    u_long unk0;
    StAnimRecord *unk4;  /* 0x04 */
    u16 unk8;            /* 0x08: id anim_list_find_id searches */
    u8 unkA;
    u8 unkB;
    s32 unkC;            /* 0x0c: key anim_list_find_key searches, -1 when free */
} StAnimNode;

typedef struct StAnimList {
    StAnimNode *nodes;
    s32 count;
    s32 capacity;
} StAnimList;

/* One block of a skeletal clip: its length in words, its command count,
 * the frame it applies from, then the node commands. */
typedef struct StAnimBlock {
    u16 length;
    u16 count;
    u_long frame;
    u_long cmds[1];
} StAnimBlock;

/* The identity transform anim_set_node_vertex composes the nodes from
 * (.data; also used by band E's object code). */
extern MATRIX DAT_8008040c;

/* src/st/anim.c */
void anim_register_pack(u16 id, u8 *pack);
void anim_register(u16 id, StAnimFile *file);
void vertices_sub(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count);
void vertices_add(SVECTOR *dst, SVECTOR *a, SVECTOR *b, int count);
StAnimKey *anim_find_key(StAnimFile *file, int clip, int frame, int *key, int *frac);
void anim_get_file(u16 id, StAnimFile **out);
/* An old-style definition; this is its promoted prototype. */
int anim_get_vertex(int id, int part, int frame, int index, SVECTOR *out);
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
StAnimNode *anim_list_add(StAnimList *list, int key);
StAnimNode *anim_list_remove(StAnimList *list, int key);
u16 anim_id_index(u_long *set, u_long *ids, u_long id);
u_long *anim_node_command(u_long *cmd, StAnimList *list, u_long *ids, u8 *table, u_long *set);
StAnimBlock *anim_block_run(u_long frame, StAnimBlock *block, StAnimList *list, u_long *ids,
                            u8 *table, u_long *set);

/* src/st/anim_set.c */
void anim_set_register(u16 id, StAnimFile *file);
void anim_sets_init(void);
void anim_sets_retire(void);
void anim_set_free(StAnimSet *set);
void anim_sets_free_idle(void);
void anim_sets_free_all(void);
void anim_set_alloc(StAnimSet **out);
void anim_record_compose(StAnimRecord *rec);
void anim_list_compose(void);
int anim_set_pose(StAnimSet **setp, int id, int clip, u_long frame);
int anim_prepare_any(void **slotp, int id, int clip, int frame);
/* VS.EXE has its own port under this name, with another signature; VS units define VS_EXE. */
#ifndef VS_EXE
void anim_set_node_vertex(StAnimSet *set, int id, int node, int index, VECTOR *out);
#endif

#endif
