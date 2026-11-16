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

/* dst = a + b, per vertex. */
ADDRESS(0x80033b00, 0x7c)
void vertices_add(SVECTOR *dst, SVECTOR *a, SVECTOR *b, int count)
{
    while (count--) {
        dst->vx = a->vx + b->vx;
        dst->vy = a->vy + b->vy;
        dst->vz = a->vz + b->vz;
        dst++;
        a++;
        b++;
    }
}

/* Copies a count-prefixed vertex list (the count halfword, a pad
 * halfword, then SVECTORs). */
ADDRESS(0x80033b7c, 0x60)
void vertex_list_copy(short *dst, short *src)
{
    int count;

    count = *src;
    src += 2;
    *dst = count;
    dst += 2;
    while (count--) {
        dst[0] = *src++;
        dst[1] = *src++;
        dst[2] = *src++;
        src++;
        dst += 4;
    }
}

/*
 * dst = a + the vertices of the count-prefixed list b.
 *
 * Not exact (96.7%, size-exact): retail sets up b's second pointer after
 * dst's and loads b's halfword before a's in each sum; this build sets it
 * up first and loads a's first.
 */
ADDRESS(0x80033bdc, 0x84)
void vertex_list_add(SVECTOR *dst, SVECTOR *a, short *b)
{
    int count = *b;

    b += 2;
    while (count--) {
        dst->vx = a->vx + b[0];
        dst->vy = a->vy + b[1];
        dst->vz = a->vz + b[2];
        dst++;
        a++;
        b += 4;
    }
}

/*
 * dst += the vertices of the count-prefixed list src.
 *
 * Not exact (99.3%, size-exact): retail sets up dst's second pointer
 * before src's; this build the other way round.
 */
ADDRESS(0x80033c60, 0x74)
void vertex_list_accumulate(SVECTOR *dst, short *src)
{
    int count = *src;

    src += 2;
    while (count--) {
        dst->vx += src[0];
        dst->vy += src[1];
        dst->vz += src[2];
        dst++;
        src += 4;
    }
}

/* dst += the list's vertices scaled by p / 4096 (LIBGTE gteMIMefunc). */
ADDRESS(0x80033cd4, 0x28)
void vertex_list_scale_add(SVECTOR *dst, short *src, long p)
{
    long count = *src;

    src += 2;
    gteMIMefunc(dst, (SVECTOR *)src, count, p);
}

/*
 * Finds the key of `clip` that `frame` falls in: stores its index and the
 * position in it (4096ths) and returns it. Past the clip's end it returns
 * the last key at 4096.
 */
ADDRESS(0x80033cfc, 0xbc)
StAnimKey *anim_find_key(StAnimFile *file, int clip, int frame, int *key, int *frac)
{
    StAnimKey *k = NULL;
    int i = 0;
    int end = 0;
    int start;
    StAnimClip *c;
    u_long *keys;
    int count;
    u_int length;
    int f;

    c = (StAnimClip *)((u8 *)file + ((u_long *)((u8 *)file + file->clips))[clip]);
    keys = c->keys;
    count = c->count;
    start = end;
    while (count--) {
        k = (StAnimKey *)((u8 *)file + *keys++);
        length = k->length;
        end += length;
        if (frame < end) {
            f = ((frame - start) << 12) / length;
            goto found;
        }
        start = end;
        i++;
    }
    i--;
    f = 4096;
found:
    *key = i;
    *frac = f;
    return k;
}

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

/* Links `count` nodes to their 0x50-byte records (and those to their
 * 0x28-byte ones) and starts the set's list empty. */
ADDRESS(0x800346dc, 0x84)
void anim_list_init(StAnimSet *set, int count)
{
    StAnimNode *node = set->unk28;
    StAnimRecord *rec = set->unk2C;
    StAnimSubRecord *sub = set->unk30;
    int i;

    set->unk34->count = 0;
    set->unk34->capacity = count;
    set->unk34->nodes = set->unk28;
    for (i = 0; i < count; i++) {
        node->unk0 = 0x80000000;
        node->unkC = -1;
        node->unk4 = rec;
        rec->unk44 = sub;
        node->unk8 = 0xffff;
        node->unkA = 0;
        node->unkB = 0;
        node++;
        rec++;
        sub++;
    }
}

ADDRESS(0x80034760, 0x4c)
StAnimNode *anim_list_find_id(StAnimList *list, int id)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (id == node->unk8)
            break;
    }
    if (i == list->count)
        return NULL;
    return node;
}

ADDRESS(0x800347ac, 0x4c)
StAnimNode *anim_list_find_key(StAnimList *list, int key)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (key == node->unkC)
            break;
    }
    if (i == list->count)
        return NULL;
    return node;
}

/* Claims a free node for `key` (reusing a released one first, else
 * appending while below capacity) and resets its record to the identity
 * transform; NULL when the list is full. */
ADDRESS(0x800347f8, 0x130)
StAnimNode *anim_list_add(StAnimList *list, int key)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (node->unkC == -1)
            break;
    }
    if (i >= list->count) {
        if (i >= list->capacity)
            return NULL;
        node = &list->nodes[list->count++];
    }
    node->unkC = key;
    node->unk0 = 0;
    node->unk4->unk48 = NULL;
    node->unk4->unk4.m[0][0] = 4096;
    node->unk4->unk4.m[0][1] = 0;
    node->unk4->unk4.m[0][2] = 0;
    node->unk4->unk4.m[1][0] = 0;
    node->unk4->unk4.m[1][1] = 4096;
    node->unk4->unk4.m[1][2] = 0;
    node->unk4->unk4.m[2][0] = 0;
    node->unk4->unk4.m[2][1] = 0;
    node->unk4->unk4.m[2][2] = 4096;
    node->unk4->unk4.t[0] = 0;
    node->unk4->unk4.t[1] = 0;
    node->unk4->unk4.t[2] = 0;
    node->unk8 = 0xffff;
    node->unkA = 0;
    node->unkB = 0;
    return node;
}

/* Releases the node of `key` (its key stays until it is reused). */
ADDRESS(0x80034928, 0x64)
StAnimNode *anim_list_remove(StAnimList *list, int key)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (node->unkC == key)
            break;
    }
    if (i < list->count) {
        node->unk8 = 0xffff;
        node->unkA = 0;
        node->unkB = 0;
        return node;
    }
    return NULL;
}

/* The index of `id` among the first set[1] words of `ids`, or 0xffff. */
ADDRESS(0x8003498c, 0x44)
u16 anim_id_index(u_long *set, u_long *ids, u_long id)
{
    int count = set[1];
    int i = 0;

    while (count > 0) {
        if (id == *ids)
            break;
        i++;
        count--;
        ids++;
    }
    if (count == 0)
        return 0xffff;
    return i;
}

/*
 * Runs one node command of a skeletal clip block on the node of its key
 * (a scratch node when the key has none) and returns the next command.
 * The command word holds the key (bits 0-15), the opcode (16-19), its
 * flags (20-23) and its length in words (24-31).
 */
ADDRESS(0x800349d0, 0x498)
u_long *anim_node_command(u_long *cmd, StAnimList *list, u_long *ids, u8 *table, u_long *set)
{
    u_long unused[6];
    StAnimNode scratch_node;
    MATRIX scratch_matrix;
    StAnimSubRecord scratch_sub;
    u_long word = cmd[0];
    long *p = (long *)(cmd + 1);
    int key = word & 0xffff;
    int op = (word >> 16) & 0xf;
    int flags = (word >> 20) & 0xf;
    int length = word >> 24;
    StAnimNode *node;
    MATRIX *m;
    StAnimSubRecord *sub;
    StAnimRecord *rec;

    node = anim_list_find_key(list, key);
    if (node == NULL) {
        node = &scratch_node;
        m = &scratch_matrix;
        sub = &scratch_sub;
    } else {
        rec = node->unk4;
        sub = rec->unk44;
        m = &rec->unk4;
        rec->unk0 = 0;
    }
    switch (op) {
    case 0:
        node->unk0 = (node->unk0 & p[0]) | p[1];
        break;
    case 1:
        if (flags & 1) {
            /* relative */
            if (flags & 2) {
                sub->rot.vx += p[0] / 360;
                sub->rot.vy += p[1] / 360;
                sub->rot.vz += p[2] / 360;
                p += 3;
            }
            if (flags & 4) {
                sub->scale.vx = sub->scale.vx * ((short *)p)[0] / 4096;
                sub->scale.vy = sub->scale.vy * ((short *)p)[1] / 4096;
                sub->scale.vz = sub->scale.vz * ((short *)p)[2] / 4096;
                p += 2;
            }
            if (flags & 8) {
                sub->trans.vx += p[0];
                sub->trans.vy += p[1];
                sub->trans.vz += p[2];
            }
            RotMatrix(&sub->rot, m);
            ScaleMatrix(m, &sub->scale);
            TransMatrix(m, &sub->trans);
            node->unk4->unk2 = 1;
        } else {
            if (flags & 2) {
                sub->rot.vx = p[0] / 360;
                sub->rot.vy = p[1] / 360;
                sub->rot.vz = p[2] / 360;
                p += 3;
                RotMatrix(&sub->rot, m);
            }
            if (flags & 4) {
                sub->scale.vx = ((short *)p)[0];
                sub->scale.vy = ((short *)p)[1];
                sub->scale.vz = ((short *)p)[2];
                p += 2;
                if (!(flags & 2))
                    RotMatrix(&sub->rot, m);
                ScaleMatrix(m, &sub->scale);
            }
            if (flags & 8) {
                sub->trans.vx = p[0];
                sub->trans.vy = p[1];
                sub->trans.vz = p[2];
                TransMatrix(m, &sub->trans);
            }
        }
        break;
    case 2:
        if (set != NULL) {
            node->unk8 = anim_id_index(set, ids, *(u16 *)p);
            node->unkA = table[node->unk8 * 4];
        }
        break;
    case 3:
        if (p[0] == 0 || p[0] == 0xffff)
            node->unk4->unk48 = NULL;
        else
            node->unk4->unk48 = anim_list_find_key(list, p[0])->unk4;
        break;
    case 8:
        if (list != NULL) {
            switch (flags) {
            case 0:
                if (anim_list_find_key(list, key) == NULL)
                    anim_list_add(list, key);
                break;
            case 1:
                anim_list_remove(list, key);
                break;
            }
        }
        break;
    }
    return cmd + length;
}

/* Runs the block's commands once `frame` has reached it and returns the
 * next block (the same block before then). */
ADDRESS(0x80034e68, 0xa4)
StAnimBlock *anim_block_run(u_long frame, StAnimBlock *block, StAnimList *list, u_long *ids,
                            u8 *table, u_long *set)
{
    u_long count = block->count;
    u_long *cmd = (u_long *)block;
    u_long i;

    if (frame >= block->frame) {
        cmd = block->cmds;
        for (i = 0; i < count; i++)
            cmd = anim_node_command(cmd, list, ids, table, set);
    }
    return (StAnimBlock *)cmd;
}
