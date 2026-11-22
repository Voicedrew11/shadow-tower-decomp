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

ADDRESS(0x80034f0c, 0x3c)
void anim_set_register(u16 id, StAnimFile *file)
{
    world_state.model_files[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

ADDRESS(0x80034f48, 0x7c)
void anim_sets_init(void)
{
    StAnimSet *set = world_state.anim_sets;
    u16 i;

    i = 16;
    do {
        set->state = 8;
        set->model_id = 0xffff;
        set->clip = 255;
        set->node_ids = NULL;
        set->node_table = NULL;
        set->first_block = NULL;
        set->blocks_run = 0;
        set->block_count = 0;
        set->length = 0;
        set->posed_frame = 0xffff;
        set->next_block = NULL;
        set->nodes = NULL;
        set->records = NULL;
        set->sub_records = NULL;
        set->list = NULL;
        set->owner = NULL;
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
        if (set->state & ~8)
            set->state = 9;
        set++;
    } while (--i);
}

/* Frees a slot: clears its owner's pointer and its node buffer. */
ADDRESS(0x80035008, 0x94)
void anim_set_free(StAnimSet *set)
{
    if (set != NULL && (set->state & 8)) {
        set->state = 8;
        *set->owner = NULL;
        set->model_id = 0xffff;
        set->clip = 255;
        set->node_ids = NULL;
        set->node_table = NULL;
        set->first_block = NULL;
        set->blocks_run = 0;
        set->block_count = 0;
        set->length = 0;
        set->posed_frame = 0xffff;
        set->next_block = NULL;
        if (set->nodes != NULL) {
            mem_free(set->nodes);
            set->nodes = NULL;
        }
        set->owner = NULL;
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
        if ((set->state & ~8) == 1)
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
        if (set->state & ~8)
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
        if (!((*out)->state & ~8)) {
            (*out)->clip = 255;
            return;
        }
        (*out)++;
    } while (--i);
    anim_sets_free_idle();
    *out = world_state.anim_sets;
    i = 16;
    do {
        if (!((*out)->state & ~8)) {
            (*out)->clip = 255;
            return;
        }
        (*out)++;
    } while (--i);
    *out = NULL;
}

#include "../lib/anim_compose.inc"

/*
 * Poses skeletal model `id` at `frame` (4096ths of the clip) of `clip`
 * through the slot at *setp (claiming one first): runs the clip's blocks
 * up to the frame (from the start again when going back) and composes the
 * node matrices. Returns 0 when no slot or node buffer is available.
 */
ADDRESS(0x8003569c, 0x40c)
int anim_set_pose(StAnimSet **setp, int id, int clip, u_long frame)
{
    StAnimFile *file = world_state.model_files[id];
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
        set->model_id = id;
        set->owner = setp;
        if (set->nodes != NULL) {
            mem_free(set->nodes);
            set->nodes = NULL;
        }
        set->nodes = mem_alloc(file->node_count * size);
        set->records = (StAnimRecord *)(set->nodes + file->node_count);
        set->sub_records = (StAnimSubRecord *)(set->records + file->node_count);
        set->list = (StAnimList *)(set->sub_records + file->node_count);
        if (set->nodes == NULL) {
            anim_slots_free_idle();
            anim_sets_free_idle();
            anim_slots_free_all();
            anim_sets_free_all();
            set->owner = NULL;
            set->model_id = 0xffff;
            return 0;
        }
        *setp = set;
        anim_list_init(set, file->node_count);
        set->node_ids = (u_long *)((u8 *)file + file->frames);
        set->node_table = (u8 *)file + file->clips;
    } else if (set->model_id != id) {
        anim_set_free(set);
        set->clip = 255;
        goto claim;
    }
    if (set->clip != clip) {
        data = (u_long *)((u8 *)file + ((u_long *)((u8 *)file + file->skeletal_clips))[clip]);
        set->first_block = (StAnimBlock *)(data + 1);
        set->block_count = *(u_long *)set->first_block;
        set->first_block = (StAnimBlock *)(data + 2);
        set->next_block = set->first_block;
        set->blocks_run = set->block_count - 1;
        while (set->blocks_run--)
            set->next_block = (StAnimBlock *)((u_long *)set->next_block + set->next_block->length);
        set->length = set->next_block->frame;
        set->next_block = set->first_block;
        set->blocks_run = 0;
        set->posed_frame = -1;
    }
    t = (set->length * frame) >> 12;
    set->clip = clip;
    if (t < set->posed_frame && set->posed_frame != -1) {
        while (set->blocks_run++ < set->block_count) {
            set->next_block = anim_block_run(set->next_block->frame, set->next_block, set->list, set->node_ids,
                                        set->node_table, (u_long *)(ANIM_MODEL(file) + 4));
        }
        set->blocks_run = 0;
        set->next_block = set->first_block;
    }
    SCRATCHPAD->list.anims = set->list;
    while (t >= set->next_block->frame) {
        set->next_block = anim_block_run(t, set->next_block, set->list, set->node_ids, set->node_table,
                                    (u_long *)(ANIM_MODEL(file) + 4));
        anim_list_compose();
        set->blocks_run++;
    }
    set->posed_frame = t;
    set->state = 10;
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

    if (((StAnimFile *)world_state.model_files[id])->animated == 0) {
        if (slot != NULL) {
            anim_set_free(slot);
            anim_slot_free(slot);
        }
        file = world_state.model_files[id];
        model = ANIM_MODEL(file);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (((StAnimFile *)world_state.model_files[id])->node_count == 0)
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
    StAnimFile *file = world_state.model_files[id];
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
    n = anim_list_find_id(set->list, node);
    if (n == NULL)
        return;
    for (rec = n->record; rec != NULL; rec = rec->parent)
        rec->composed = 0;
    *SCRATCH_MATRIX(SCRATCHPAD) = identity_matrix;
    anim_record_compose(n->record);
    gte_ldv0(v);
    gte_rt();
    gte_stlvnl(out);
}
