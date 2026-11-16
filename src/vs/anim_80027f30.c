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
void func_80027fec(u16 id, StAnimFile *file)
{
    DAT_801624d0.unk834C[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

/* Frees a slot: clears its state and its owner's pointer and frees its
 * delta buffer (ST.EXE's anim_slot_free checks the slot first and uses
 * func_800152f8). */
ADDRESS(0x80028028, 0x4c)
void func_80028028(StAnimSlot *slot)
{
    slot->unk0 = 0;
    *slot->unk14 = NULL;
    if (slot->unk10 != NULL) {
        free(slot->unk10);
        slot->unk10 = NULL;
    }
}

/* Frees every slot in state 1. */
ADDRESS(0x80028074, 0x6c)
void func_80028074(void)
{
    StAnimSlot *slot = DAT_801624d0.anim_slots;
    u16 i;

    i = 12;
    do {
        if (slot->unk0 == 1)
            func_80028028(slot);
        slot++;
    } while (--i);
}

/* Puts every used slot in state 1. */
ADDRESS(0x800280e0, 0x3c)
void func_800280e0(void)
{
    StAnimSlot *slot = DAT_801624d0.anim_slots;
    u16 i;

    i = 12;
    do {
        if (slot->unk0)
            slot->unk0 = 1;
        slot++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x8002811c, 0x60)
void func_8002811c(void)
{
    StAnimSlot *slot = DAT_801624d0.anim_slots;
    s16 i;

    i = 12;
    do {
        if (slot->unk0)
            func_80028028(slot);
        slot++;
    } while (--i);
}

/* Finds a free slot, or NULL (ST.EXE's anim_slot_alloc frees the state-1
 * slots and searches again; VS gives up). */
ADDRESS(0x8002817c, 0x58)
void func_8002817c(StAnimSlot **out)
{
    u16 i;

    *out = DAT_801624d0.anim_slots;
    i = 12;
    do {
        if (!(*out)->unk0) {
            (*out)->unk4 = 255;
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
 * DAT_801624d0.unkC7F0 and the scratchpad's model-space vertex pointer.
 * Returns the slot, or 0 when none is free.
 */
ADDRESS(0x800281d4, 0x2a0)
int func_800281d4(StAnimSlot **slotp, int id, int clip, int frame)
{
    StAnimFile *file = DAT_801624d0.unk834C[id];
    StAnimSlot *slot = *slotp;
    VsScratchPad *sp = VS_SCRATCHPAD;
    StAnimFile *still;
    StAnimKey *k;
    int key;
    int frac;
    u_long *frames;

    if (file->unk4 == 0) {
        u8 *model;

        if (slot != NULL)
            func_80028028(slot);
        still = DAT_801624d0.unk834C[id];
        model = ANIM_MODEL(still);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (slot == NULL) {
        func_8002817c(&slot);
        if (slot == NULL)
            return 0;
    claim:
        slot->unk2 = id;
        slot->unk14 = slotp;
    retry:
        slot->unk10 = func_8002e690(sp->header->n_vertices * sizeof(SVECTOR));
        if (slot->unk10 == NULL) {
            func_8002811c();
            goto retry;
        }
        *slotp = slot;
    } else if (slot->unk2 != id) {
        func_80028028(slot);
        slot->unk4 = 255;
        goto claim;
    }
    k = anim_find_key(file, clip, frame, &key, &frac);
    if (slot->unk4 != clip || slot->unk6 != key) {
        u8 *model = sp->model;

        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        frames = (u_long *)((u8 *)file + file->frames);
        slot->unk8 = (SVECTOR *)((u8 *)file + frames[k->from]);
        slot->unkC = (SVECTOR *)((u8 *)file + frames[k->to]);
        func_80027c64(slot->unk10, slot->unk8, slot->unkC, sp->header->n_vertices);
        slot->unk4 = clip;
        slot->unk6 = key;
    }
    vertices_add(DAT_801624d0.unkC7F0, (SVECTOR *)(sp->model + (sp->header->vertices + 12)),
                 slot->unk8, sp->header->n_vertices);
    gteMIMefunc(DAT_801624d0.unkC7F0, slot->unk10, sp->header->n_vertices, frac);
    sp->local_vertices = (u8 *)DAT_801624d0.unkC7F0;
    slot->unk0 = 2;
    return (int)slot;
}
