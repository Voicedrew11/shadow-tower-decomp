/*
 * ST.EXE: animated model files: registering them by id, keyframe vertex
 * arithmetic, clip key lookup, vertex positions, the vertex-animation
 * slots and the node lists (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/funcs.h>
#include <st/anim.h>

JUMPTABLES(0x800114a8, 0x24)

/* The file's mesh model, part headers and part vertex arrays. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))
#define PART_VERTICES(model, part) ((SVECTOR *)((model) + (MODEL_PARTS(model)[part].vertices + 12)))

/*
 * Registers the files of a pack (a count, then the files back to back)
 * under consecutive ids from `id`; files of 4 bytes or less are skipped.
 */
ADDRESS(0x800339c4, 0x84)
void anim_register_pack(u16 id, u8 *pack)
{
    ScratchPad *sp = SCRATCHPAD;
    u16 count = *(u16 *)pack;
    StAnimFile *file;
    u_long size;

    pack += 4;
    while (count--) {
        file = (StAnimFile *)pack;
        size = file->size;
        if (size >= 5) {
            world_state.unk834C[id] = file;
            sp->model = ANIM_MODEL(file);
        }
        pack += size;
        id++;
    }
}

ADDRESS(0x80033a48, 0x3c)
void anim_register(u16 id, StAnimFile *file)
{
    world_state.unk834C[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

/* dst = to - from, per vertex. */
ADDRESS(0x80033a84, 0x7c)
void vertices_sub(SVECTOR *dst, SVECTOR *from, SVECTOR *to, int count)
{
    while (count--) {
        dst->vx = to->vx - from->vx;
        dst->vy = to->vy - from->vy;
        dst->vz = to->vz - from->vz;
        dst++;
        from++;
        to++;
    }
}

#include "../lib/anim_vertices.inc"

#include "../lib/anim_vertex_list.inc"

/* dst += the list's vertices scaled by p / 4096 (LIBGTE gteMIMefunc). */
ADDRESS(0x80033cd4, 0x28)
void vertex_list_scale_add(SVECTOR *dst, short *src, long p)
{
    long count = *src;

    src += 2;
    gteMIMefunc(dst, (SVECTOR *)src, count, p);
}

#include "../lib/anim_find_key.inc"

/* The file registered under `id`; a heap-loaded one (id 112 and up) only
 * while its block is releasable or in use. */
ADDRESS(0x80033db8, 0x78)
void anim_get_file(u16 id, StAnimFile **out)
{
    StAnimFile *file = world_state.unk834C[id];
    u8 state;

    *out = file;
    if (id >= 112) {
        if (file == NULL) {
            *out = NULL;
        } else {
            state = func_80015258(file);
            if (state != ST_BLOCK_RELEASABLE && state != ST_BLOCK_USED)
                *out = NULL;
        }
    }
}

/*
 * Vertex `index` of model `id`: of part `part` & 0x7f when `part` is 128
 * or more, else of part 0 posed at `frame` of clip `part`. Returns 1 (and
 * a zero vector) when the model is missing. An old-style definition:
 * callers pass `id` unconverted.
 */
ADDRESS(0x80033e30, 0x22c)
int anim_get_vertex(id, part, frame, index, out)
    u16 id;
    int part;
    int frame;
    int index;
    SVECTOR *out;
{
    StAnimFile *found;
    StAnimFile *file;
    u8 *model;
    SVECTOR *verts;

    anim_get_file(id, &found);
    file = found;
    if (file == NULL) {
        out->vz = 0;
        out->vy = 0;
        out->vx = 0;
        return 1;
    }
    model = ANIM_MODEL(file);
    if (part >= 128) {
    part_vertex:
        verts = PART_VERTICES(model, part & 0x7f);
        *out = verts[index];
    } else if (file->unk4 == 0) {
        part = 0;
        goto part_vertex;
    } else {
        SVECTOR *from;
        SVECTOR *to;
        StAnimKey *k;
        SVECTOR a;
        SVECTOR d;
        int key;
        int frac;
        u_long *frames;

        verts = PART_VERTICES(model, 0);
        k = anim_find_key(file, part, frame, &key, &frac);
        frames = (u_long *)((u8 *)file + file->frames);
        from = (SVECTOR *)((u8 *)file + frames[k->from]);
        to = (SVECTOR *)((u8 *)file + frames[k->to]);
        a.vx = verts[index].vx + from[index].vx;
        a.vy = verts[index].vy + from[index].vy;
        a.vz = verts[index].vz + from[index].vz;
        d.vx = to[index].vx - from[index].vx;
        d.vy = to[index].vy - from[index].vy;
        d.vz = to[index].vz - from[index].vz;
        out->vx = a.vx + ((d.vx * frac) >> 12);
        out->vy = a.vy + ((d.vy * frac) >> 12);
        out->vz = a.vz + ((d.vz * frac) >> 12);
    }
    return 0;
}

/* The vertex count of model `id`'s part 0 (or of part `part` & 0x7f when
 * `part` is 128 or more); 0 when the model is missing. */
ADDRESS(0x8003405c, 0x68)
u_long anim_vertex_count(int id, int part)
{
    StAnimFile *file = world_state.unk834C[id];
    u8 *model;
    u_long count;

    if (file == NULL)
        return 0;
    model = ANIM_MODEL(file);
    if (part < 128)
        count = MODEL_PARTS(model)[0].n_vertices;
    else
        count = MODEL_PARTS(model)[part & 0x7f].n_vertices;
    return count;
}

ADDRESS(0x800340c4, 0x34)
void anim_slots_init(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    u16 i;

    i = 16;
    do {
        slot->unk0 = 4;
        slot->unk10 = NULL;
        slot++;
    } while (--i);
}

ADDRESS(0x800340f8, 0x44)
void anim_slots_retire(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    u16 i;

    i = 16;
    do {
        if (slot->unk0 & ~4)
            slot->unk0 = 5;
        slot++;
    } while (--i);
}

/* Frees a slot: clears its owner's pointer and its delta buffer. */
ADDRESS(0x8003413c, 0x64)
void anim_slot_free(StAnimSlot *slot)
{
    if (slot != NULL && (slot->unk0 & 4)) {
        slot->unk0 = 4;
        *slot->unk14 = NULL;
        if (slot->unk10 != NULL) {
            func_800152f8(slot->unk10);
            slot->unk10 = NULL;
        }
    }
}

/* Frees the slots in state 1. */
ADDRESS(0x800341a0, 0x84)
void anim_slots_free_idle(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    u16 i;

    i = 16;
    do {
        if ((slot->unk0 & ~4) == 1)
            anim_slot_free(slot);
        slot++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x80034224, 0x70)
void anim_slots_free_all(void)
{
    StAnimSlot *slot = world_state.anim_slots;
    s16 i;

    for (i = 16; i != 0; i--) {
        if (slot->unk0 & ~4)
            anim_slot_free(slot);
        slot++;
    }
}

/* Finds a free slot (freeing the state-1 slots when none is), or NULL. */
ADDRESS(0x80034294, 0xd0)
void anim_slot_alloc(StAnimSlot **out)
{
    u16 i;

    *out = world_state.anim_slots;
    i = 16;
    do {
        if (!((*out)->unk0 & ~4)) {
            (*out)->unk4 = 255;
            return;
        }
        (*out)++;
    } while (--i);
    anim_slots_free_idle();
    *out = world_state.anim_slots;
    i = 16;
    do {
        if (!((*out)->unk0 & ~4)) {
            (*out)->unk4 = 255;
            return;
        }
        (*out)++;
    } while (--i);
    *out = NULL;
}

/*
 * Poses model `id` at `frame` of `clip` through the slot at *slotp
 * (claiming one first): the posed vertices go to world_state.unkEB40 and
 * the scratchpad's model-space vertex pointer. Returns 0 when no slot or
 * delta buffer is available.
 */
ADDRESS(0x80034364, 0x2a0)
int anim_pose(StAnimSlot **slotp, int id, int clip, int frame)
{
    ScratchPad *sp = SCRATCHPAD;
    StAnimFile *file = world_state.unk834C[id];
    StAnimSlot *slot = *slotp;
    StAnimKey *k;
    int key;
    int frac;
    u8 *model;
    u_long *frames;

    if (slot == NULL) {
        anim_slot_alloc(&slot);
        if (slot == NULL)
            return 0;
    claim:
        slot->unk2 = id;
        slot->unk14 = slotp;
        if (slot->unk10 != NULL) {
            func_800152f8(slot->unk10);
            slot->unk10 = NULL;
        }
        slot->unk10 = func_800152d0(sp->header->n_vertices * sizeof(SVECTOR));
        if (slot->unk10 == NULL) {
            anim_slots_free_idle();
            anim_sets_free_idle();
            anim_slots_free_all();
            anim_sets_free_all();
            slot->unk14 = NULL;
            slot->unk2 = 0xffff;
            return 0;
        }
        *slotp = slot;
    } else if (slot->unk2 != id) {
        anim_slot_free(slot);
        slot->unk4 = 255;
        goto claim;
    }
    k = anim_find_key(file, clip, frame, &key, &frac);
    if (slot->unk4 != clip || slot->unk6 != key) {
        model = sp->model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        frames = (u_long *)((u8 *)file + file->frames);
        slot->unk8 = (SVECTOR *)((u8 *)file + frames[k->from]);
        slot->unkC = (SVECTOR *)((u8 *)file + frames[k->to]);
        vertices_sub(slot->unk10, slot->unk8, slot->unkC, sp->header->n_vertices);
        slot->unk4 = clip;
        slot->unk6 = key;
    }
    frames = (u_long *)((u8 *)file + file->frames);
    slot->unk8 = (SVECTOR *)((u8 *)file + frames[k->from]);
    vertices_add(world_state.unkEB40, (SVECTOR *)(sp->model + 12 + sp->header->vertices),
                 slot->unk8, sp->header->n_vertices);
    gteMIMefunc(world_state.unkEB40, slot->unk10, sp->header->n_vertices, frac);
    sp->local_vertices = (u8 *)world_state.unkEB40;
    slot->unk0 = 6;
    return (int)slot;
}

/*
 * Prepares model `id` for drawing: a model without animation is set up
 * directly (freeing the slot at *slotp) and gives 1; otherwise the result
 * of the posing function for its kind.
 */
ADDRESS(0x80034604, 0xd8)
int anim_prepare(StAnimSlot **slotp, int id, int clip, int frame)
{
    ScratchPad *sp = SCRATCHPAD;
    StAnimSlot *slot = *slotp;
    StAnimFile *file;
    u8 *model;

    if (((StAnimFile *)world_state.unk834C[id])->unk4 == 0) {
        if (slot != NULL) {
            anim_slot_free(slot);
            anim_set_free((StAnimSet *)slot);
        }
        file = world_state.unk834C[id];
        model = ANIM_MODEL(file);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (((StAnimFile *)world_state.unk834C[id])->unk6 == 0)
        return anim_pose(slotp, id, clip, frame);
    return anim_set_pose((StAnimSet **)slotp, id, clip, frame);
}

#include "../lib/anim_list_init.inc"

#include "../lib/anim_list_find_id.inc"

#include "../lib/anim_list.inc"
