/*
 * VS.EXE: clip key lookup, ST.EXE's (src/lib/anim_find_key.inc, shared
 * with src/st/anim.c), VS's second copy of anim_register and its
 * vertex-animation slot release (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/scratch.h>
#include <vs/funcs.h>
#include <MALLOC.H>

/* The file's mesh model and part headers. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))

#include "../lib/anim_find_key.inc"

ADDRESS(0x80027fec, 0x3c)
void anim_register(u16 id, StAnimFile *file)
{
    vs_gfx_state.model_files[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

/* Frees a slot: clears its state and its owner's pointer and frees its
 * delta buffer (ST.EXE's anim_slot_free checks the slot first and uses
 * mem_free). */
ADDRESS(0x80028028, 0x4c)
void anim_slot_free(StAnimSlot *slot)
{
    slot->state = 0;
    *slot->owner = NULL;
    if (slot->deltas != NULL) {
        free(slot->deltas);
        slot->deltas = NULL;
    }
}

/* Frees every slot in state 1. */
ADDRESS(0x80028074, 0x6c)
void anim_slots_free_idle(void)
{
    StAnimSlot *slot = vs_gfx_state.anim_slots;
    u16 i;

    i = 12;
    do {
        if (slot->state == 1)
            anim_slot_free(slot);
        slot++;
    } while (--i);
}

/* Puts every used slot in state 1. */
ADDRESS(0x800280e0, 0x3c)
void anim_slots_retire(void)
{
    StAnimSlot *slot = vs_gfx_state.anim_slots;
    u16 i;

    i = 12;
    do {
        if (slot->state)
            slot->state = 1;
        slot++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x8002811c, 0x60)
void anim_slots_free_all(void)
{
    StAnimSlot *slot = vs_gfx_state.anim_slots;
    s16 i;

    i = 12;
    do {
        if (slot->state)
            anim_slot_free(slot);
        slot++;
    } while (--i);
}

/* Finds a free slot, or NULL (ST.EXE's anim_slot_alloc frees the state-1
 * slots and searches again; VS gives up). */
ADDRESS(0x8002817c, 0x58)
void anim_slot_alloc(StAnimSlot **out)
{
    u16 i;

    *out = vs_gfx_state.anim_slots;
    i = 12;
    do {
        if (!(*out)->state) {
            (*out)->clip = 255;
            return;
        }
        (*out)++;
    } while (--i);
    *out = NULL;
}

/*
 * ST.EXE's anim_prepare and anim_pose in one: for a model without
 * animation frees the slot at *slotp and points the scratchpad at the
 * model (returns 1); otherwise poses model `id` at `frame` of `clip`
 * through the slot at *slotp, claiming one and its delta buffer first
 * (freeing every slot until the buffer fits). The posed vertices go to
 * vs_gfx_state.posed_vertices and the scratchpad's model-space vertex pointer.
 * Returns the slot, or 0 when none is free.
 */
ADDRESS(0x800281d4, 0x2a0)
int anim_pose(StAnimSlot **slotp, int id, int clip, int frame)
{
    StAnimFile *file = vs_gfx_state.model_files[id];
    StAnimSlot *slot = *slotp;
    VsScratchPad *sp = VS_SCRATCHPAD;
    StAnimFile *still;
    StAnimKey *k;
    int key;
    int frac;
    u_long *frames;

    if (file->animated == 0) {
        u8 *model;

        if (slot != NULL)
            anim_slot_free(slot);
        still = vs_gfx_state.model_files[id];
        model = ANIM_MODEL(still);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (slot == NULL) {
        anim_slot_alloc(&slot);
        if (slot == NULL)
            return 0;
    claim:
        slot->model_id = id;
        slot->owner = slotp;
    retry:
        slot->deltas = mem_alloc_checked(sp->header->n_vertices * sizeof(SVECTOR));
        if (slot->deltas == NULL) {
            anim_slots_free_all();
            goto retry;
        }
        *slotp = slot;
    } else if (slot->model_id != id) {
        anim_slot_free(slot);
        slot->clip = 255;
        goto claim;
    }
    k = anim_find_key(file, clip, frame, &key, &frac);
    if (slot->clip != clip || slot->key != key) {
        u8 *model = sp->model;

        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        frames = (u_long *)((u8 *)file + file->frames);
        slot->from_keyframe = (SVECTOR *)((u8 *)file + frames[k->from]);
        slot->to_keyframe = (SVECTOR *)((u8 *)file + frames[k->to]);
        vertices_sub(slot->deltas, slot->from_keyframe, slot->to_keyframe, sp->header->n_vertices);
        slot->clip = clip;
        slot->key = key;
    }
    vertices_add(vs_gfx_state.posed_vertices, (SVECTOR *)(sp->model + (sp->header->vertices + 12)),
                 slot->from_keyframe, sp->header->n_vertices);
    gteMIMefunc(vs_gfx_state.posed_vertices, slot->deltas, sp->header->n_vertices, frac);
    sp->local_vertices = (u8 *)vs_gfx_state.posed_vertices;
    slot->state = 2;
    return (int)slot;
}
