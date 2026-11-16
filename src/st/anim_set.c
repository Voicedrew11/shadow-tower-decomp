/*
 * ST.EXE: skeletal animation (WIP module): model files whose clips are
 * blocks of node commands (src/st/anim.c runs them), the 16 slots at
 * world_state.anim_sets that pose them, and composing the nodes' matrices
 * on the GTE. The slot functions mirror src/st/anim.c's vertex-animation
 * ones; this file repeats anim_register as anim_set_register.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <INLINE_C.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>

/* The file's mesh model, part headers and part vertex arrays. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))
#define PART_VERTICES(model, part) ((SVECTOR *)((model) + (MODEL_PARTS(model)[part].vertices + 12)))

/*
 * rec->unk24 = the GTE's current matrix (the parent's) times rec->unk4,
 * left loaded as the GTE rotation and translation.
 */
#define RECORD_COMPOSE(sp, rec)                                   \
    do {                                                          \
        (sp)->unk9C.vx = (rec)->unk4.t[0];                        \
        (sp)->unk9C.vy = (rec)->unk4.t[1];                        \
        (sp)->unk9C.vz = (rec)->unk4.t[2];                        \
        gte_ldv0(&(sp)->unk9C);                                   \
        gte_rt();                                                 \
        gte_stlvnl((rec)->unk24.t);                               \
        gte_SetTransMatrix(&(rec)->unk24);                        \
        gte_ldclmv(&(rec)->unk4);                                 \
        gte_rtir();                                               \
        gte_stclmv(&(rec)->unk24);                                \
        gte_ldclmv((char *)&(rec)->unk4 + 2);                     \
        gte_rtir();                                               \
        gte_stclmv((char *)&(rec)->unk24 + 2);                    \
        gte_ldclmv((char *)&(rec)->unk4 + 4);                     \
        gte_rtir();                                               \
        gte_stclmv((char *)&(rec)->unk24 + 4);                    \
        gte_SetRotMatrix(&(rec)->unk24);                          \
    } while (0)

ADDRESS(0x80034f0c, 0x3c)
void anim_set_register(u16 id, StAnimFile *file)
{
    world_state.unk834C[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

ADDRESS(0x80034f48, 0x7c)
void anim_sets_init(void)
{
    StAnimSet *set = world_state.anim_sets;
    u16 i;

    i = 16;
    do {
        set->unk0 = 8;
        set->unk2 = 0xffff;
        set->unk4 = 255;
        set->unk8 = NULL;
        set->unkC = NULL;
        set->unk10 = NULL;
        set->unk14 = 0;
        set->unk18 = 0;
        set->unk1C = 0;
        set->unk20 = 0xffff;
        set->unk24 = NULL;
        set->unk28 = NULL;
        set->unk2C = NULL;
        set->unk30 = NULL;
        set->unk34 = NULL;
        set->unk38 = NULL;
        set++;
    } while (--i);
}

ADDRESS(0x80034fc4, 0x44)
void anim_sets_retire(void)
{
    StAnimSet *set = world_state.anim_sets;
    u16 i;

    i = 16;
    do {
        if (set->unk0 & ~8)
            set->unk0 = 9;
        set++;
    } while (--i);
}

/* Frees a slot: clears its owner's pointer and its node buffer. */
ADDRESS(0x80035008, 0x94)
void anim_set_free(StAnimSet *set)
{
    if (set != NULL && (set->unk0 & 8)) {
        set->unk0 = 8;
        *set->unk38 = NULL;
        set->unk2 = 0xffff;
        set->unk4 = 255;
        set->unk8 = NULL;
        set->unkC = NULL;
        set->unk10 = NULL;
        set->unk14 = 0;
        set->unk18 = 0;
        set->unk1C = 0;
        set->unk20 = 0xffff;
        set->unk24 = NULL;
        if (set->unk28 != NULL) {
            func_800152f8(set->unk28);
            set->unk28 = NULL;
        }
        set->unk38 = NULL;
    }
}

/* Frees the slots in state 1. */
ADDRESS(0x8003509c, 0x84)
void anim_sets_free_idle(void)
{
    StAnimSet *set = world_state.anim_sets;
    u16 i;

    i = 16;
    do {
        if ((set->unk0 & ~8) == 1)
            anim_set_free(set);
        set++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x80035120, 0x70)
void anim_sets_free_all(void)
{
    StAnimSet *set = world_state.anim_sets;
    s16 i;

    for (i = 16; i != 0; i--) {
        if (set->unk0 & ~8)
            anim_set_free(set);
        set++;
    }
}

/* Finds a free slot (freeing the state-1 slots when none is), or NULL. */
ADDRESS(0x80035190, 0xd0)
void anim_set_alloc(StAnimSet **out)
{
    u16 i;

    *out = world_state.anim_sets;
    i = 16;
    do {
        if (!((*out)->unk0 & ~8)) {
            (*out)->unk4 = 255;
            return;
        }
        (*out)++;
    } while (--i);
    anim_sets_free_idle();
    *out = world_state.anim_sets;
    i = 16;
    do {
        if (!((*out)->unk0 & ~8)) {
            (*out)->unk4 = 255;
            return;
        }
        (*out)++;
    } while (--i);
    *out = NULL;
}

/*
 * Composes the record's matrix with its parents' (composing them first
 * unless already done for this pose; a root one with the scratchpad's
 * root transform) and leaves it loaded on the GTE.
 */
ADDRESS(0x80035260, 0x3a4)
void anim_record_compose(StAnimRecord *rec)
{
    ScratchPad *sp = SCRATCHPAD;

    if (rec->unk48 == NULL) {
        gte_SetRotMatrix(SCRATCH_MATRIX(SCRATCHPAD));
        gte_SetTransMatrix(SCRATCH_MATRIX(SCRATCHPAD));
        RECORD_COMPOSE(sp, rec);
    } else if (rec->unk0) {
        gte_SetTransMatrix(&rec->unk24);
        gte_SetRotMatrix(&rec->unk24);
        return;
    } else {
        anim_record_compose(rec->unk48);
        RECORD_COMPOSE(sp, rec);
    }
    rec->unk0 = 1;
}

/* Composes the matrices of every node of the scratchpad's node list that
 * is in use and has an id. */
ADDRESS(0x80035604, 0x98)
void anim_list_compose(void)
{
    StAnimList *list = SCRATCHPAD->list.anims;
    int i = list->count;
    StAnimNode *node = list->nodes;

    while (i--) {
        if (node->unkC != -1 && node->unk8 != 0xffff)
            anim_record_compose(node->unk4);
        node++;
    }
}

/*
 * Poses skeletal model `id` at `frame` (4096ths of the clip) of `clip`
 * through the slot at *setp (claiming one first): runs the clip's blocks
 * up to the frame (from the start again when going back) and composes the
 * node matrices. Returns 0 when no slot or node buffer is available.
 */
ADDRESS(0x8003569c, 0x40c)
int anim_set_pose(StAnimSet **setp, int id, int clip, u_long frame)
{
    StAnimFile *file = world_state.unk834C[id];
    StAnimSet *set = *setp;
    /* Per node: a list node, a record, a sub-record and a 12-byte list
     * (one would do; the buffer holds one per node). */
    u_long size = sizeof(StAnimNode) + sizeof(StAnimRecord) + sizeof(StAnimSubRecord) +
                  sizeof(StAnimList);
    u_long *data;
    u_long t;

    if (set == NULL) {
        anim_set_alloc(&set);
        if (set == NULL)
            return 0;
    claim:
        set->unk2 = id;
        set->unk38 = setp;
        if (set->unk28 != NULL) {
            func_800152f8(set->unk28);
            set->unk28 = NULL;
        }
        set->unk28 = func_800152d0(file->unk6 * size);
        set->unk2C = (StAnimRecord *)(set->unk28 + file->unk6);
        set->unk30 = (StAnimSubRecord *)(set->unk2C + file->unk6);
        set->unk34 = (StAnimList *)(set->unk30 + file->unk6);
        if (set->unk28 == NULL) {
            anim_slots_free_idle();
            anim_sets_free_idle();
            anim_slots_free_all();
            anim_sets_free_all();
            set->unk38 = NULL;
            set->unk2 = 0xffff;
            return 0;
        }
        *setp = set;
        anim_list_init(set, file->unk6);
        set->unk8 = (u_long *)((u8 *)file + file->frames);
        set->unkC = (u8 *)file + file->clips;
    } else if (set->unk2 != id) {
        anim_set_free(set);
        set->unk4 = 255;
        goto claim;
    }
    if (set->unk4 != clip) {
        data = (u_long *)((u8 *)file + ((u_long *)((u8 *)file + file->unk14))[clip]);
        set->unk10 = (StAnimBlock *)(data + 1);
        set->unk18 = *(u_long *)set->unk10;
        set->unk10 = (StAnimBlock *)(data + 2);
        set->unk24 = set->unk10;
        set->unk14 = set->unk18 - 1;
        while (set->unk14--)
            set->unk24 = (StAnimBlock *)((u_long *)set->unk24 + set->unk24->length);
        set->unk1C = set->unk24->frame;
        set->unk24 = set->unk10;
        set->unk14 = 0;
        set->unk20 = -1;
    }
    t = (set->unk1C * frame) >> 12;
    set->unk4 = clip;
    if (t < set->unk20 && set->unk20 != -1) {
        while (set->unk14++ < set->unk18) {
            set->unk24 = anim_block_run(set->unk24->frame, set->unk24, set->unk34, set->unk8,
                                        set->unkC, (u_long *)(ANIM_MODEL(file) + 4));
        }
        set->unk14 = 0;
        set->unk24 = set->unk10;
    }
    SCRATCHPAD->list.anims = set->unk34;
    while (t >= set->unk24->frame) {
        set->unk24 = anim_block_run(t, set->unk24, set->unk34, set->unk8, set->unkC,
                                    (u_long *)(ANIM_MODEL(file) + 4));
        anim_list_compose();
        set->unk14++;
    }
    set->unk20 = t;
    set->unk0 = 10;
    return (int)set;
}

/*
 * Prepares model `id` for drawing like anim_prepare, but frees the slot at
 * *slotp as both kinds for a model without animation and poses skeletal
 * models through anim_set_pose.
 */
ADDRESS(0x80035aa8, 0xd8)
int anim_prepare_any(void **slotp, int id, int clip, int frame)
{
    ScratchPad *sp = SCRATCHPAD;
    void *slot = *slotp;
    StAnimFile *file;
    u8 *model;

    if (((StAnimFile *)world_state.unk834C[id])->unk4 == 0) {
        if (slot != NULL) {
            anim_set_free(slot);
            anim_slot_free(slot);
        }
        file = world_state.unk834C[id];
        model = ANIM_MODEL(file);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (((StAnimFile *)world_state.unk834C[id])->unk6 == 0)
        return anim_pose((StAnimSlot **)slotp, id, clip, frame);
    return anim_set_pose((StAnimSet **)slotp, id, clip, frame);
}

/*
 * The position of vertex `index` of part `node` of model `id`, moved by
 * that node's composed matrix in the slot `set`; a zero vector when the
 * model or slot is missing.
 */
ADDRESS(0x80035b80, 0x148)
void anim_set_node_vertex(StAnimSet *set, int id, int node, int index, VECTOR *out)
{
    MATRIX unused;
    StAnimFile *file = world_state.unk834C[id];
    StAnimNode *n;
    StAnimRecord *rec;
    SVECTOR *v;

    if (file == NULL || set == NULL) {
        out->vz = 0;
        out->vy = 0;
        out->vx = 0;
        return;
    }
    v = &PART_VERTICES(ANIM_MODEL(file), node)[index];
    n = anim_list_find_id(set->unk34, node);
    if (n == NULL)
        return;
    for (rec = n->unk4; rec != NULL; rec = rec->unk48)
        rec->unk0 = 0;
    *SCRATCH_MATRIX(SCRATCHPAD) = DAT_8008040c;
    anim_record_compose(n->unk4);
    gte_ldv0(v);
    gte_rt();
    gte_stlvnl(out);
}
