/*
 * Model files and skeletal animation: the loaded
 * model's primitive fix-up, the animated model registry and animation
 * slots, the node lists and their command interpreter, record and list
 * composition, the fighter's model draw and the node list lookup. One
 * translation unit: retail lays out its constants (0x800112a8, the matrix
 * print strings, the draw's and the lookup's initializers) before its
 * switch tables (0x80011340, model_fixup_face_indices's and anim_node_command's), the
 * order cc1 writes one object's read-only data in.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/mesh.h>
#include <vs/mesh.h>
#include <kf/lib/null.h>
#include <MALLOC.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/scratch.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/funcs.h>
#include <INLINE_C.H>
#include <vs/scratch.h>
#include <GTEMAC.H>
#include <vs/buffer.h>

/* The module's constants, then its switch tables. */
RODATA(0x800112a8, 0x134)

/* The file's mesh model, part headers and part vertex arrays. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))
#define PART_VERTICES(model, part) ((SVECTOR *)((model) + (MODEL_PARTS(model)[part].vertices + 12)))

/*
 * A loaded model file's primitive fix-up: every
 * part's tagged face records get their vertex and normal indices turned
 * into byte offsets (8 bytes an entry).
 */

/*
 * Walks each part's face list: a tag word (GPU code in the top byte, the
 * record's length in words in the second) before each record. The code's
 * semi-transparency bit is ignored for the switch.
 */
ADDRESS(0x8002345c, 0x308)
void model_fixup_face_indices(ModelFile *file)
{
    VsModelPart *part = (VsModelPart *)((u8 *)file + 12);
    int n = file->model;
    int count;
    u32 *p;
    VsFaceTag tag;

    while (n--) {
        p = (u32 *)((u8 *)file + (part->faces + 12));
        count = part->n_faces;
        while (count--) {
            tag = *(VsFaceTag *)p++;
            tag.code &= ~2;
            tag.code |= 2;
            switch (tag.code & ~2) {
            case 0x24: {
                MeshFaceFT3 *f = (MeshFaceFT3 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->normal <<= 3;
                break;
            }
            case 0x2c: {
                MeshFaceFT4 *f = (MeshFaceFT4 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->i3 <<= 3;
                f->normal <<= 3;
                break;
            }
            case 0x34: {
                MeshFaceGT3 *f = (MeshFaceGT3 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->n0 <<= 3;
                f->n1 <<= 3;
                f->n2 <<= 3;
                break;
            }
            case 0x3c: {
                MeshFaceGT4 *f = (MeshFaceGT4 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->i3 <<= 3;
                f->n0 <<= 3;
                f->n1 <<= 3;
                f->n2 <<= 3;
                f->n3 <<= 3;
                break;
            }
            case 0x20: {
                VsFaceF3 *f = (VsFaceF3 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->normal <<= 3;
                break;
            }
            case 0x28: {
                VsFaceF4 *f = (VsFaceF4 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->i3 <<= 3;
                f->normal <<= 3;
                break;
            }
            case 0x30: {
                VsFaceG3 *f = (VsFaceG3 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->n0 <<= 3;
                f->n1 <<= 3;
                f->n2 <<= 3;
                break;
            }
            case 0x38: {
                VsFaceG4 *f = (VsFaceG4 *)p;

                f->i0 <<= 3;
                f->i1 <<= 3;
                f->i2 <<= 3;
                f->i3 <<= 3;
                f->n0 <<= 3;
                f->n1 <<= 3;
                f->n2 <<= 3;
                f->n3 <<= 3;
                break;
            }
            }
            p += tag.len;
        }
        part++;
    }
}

/*
 * Registering an animated model file by id, the 12 skeletal
 * animation slots (initialise, retire, free), a malloc wrapper and the
 * animation node lists: initialisation, key lookup, insertion, removal,
 * the node command interpreter and the block runner. The
 * list functions are ST.EXE's (src/lib/anim_list*.inc, shared with
 * src/st/render.c); the rest are VS's copies of ST.EXE's src/st/render.c
 * and anim_set.c functions. VS keeps anim_list_find_id further on in this
 * file.
 */

/* VS's first copy of anim_register; the second is anim_register
 * (src/vs/anim_slots.c). */
ADDRESS(0x80023764, 0x3c)
void anim_set_register(u16 id, StAnimFile *file)
{
    vs_gfx_state.unk834C[id] = file;
    SCRATCHPAD->model = ANIM_MODEL(file);
}

ADDRESS(0x800237a0, 0x44)
void anim_sets_init(void)
{
    StAnimSet *set = vs_gfx_state.anim_sets;
    u16 i;

    i = 12;
    do {
        set->state = 0;
        set->nodes = NULL;
        set->records = NULL;
        set->sub_records = NULL;
        set->list = NULL;
        set++;
    } while (--i);
}

/* Puts every used slot in state 1. */
ADDRESS(0x800237e4, 0x3c)
void anim_sets_retire(void)
{
    StAnimSet *set = vs_gfx_state.anim_sets;
    u16 i;

    i = 12;
    do {
        if (set->state)
            set->state = 1;
        set++;
    } while (--i);
}

/* Frees a slot: clears its state and its owner's pointer and frees its four
 * buffers (ST.EXE's anim_set_free frees one node buffer). */
ADDRESS(0x80023820, 0xa0)
void anim_set_free(StAnimSet *set)
{
    set->state = 0;
    *set->owner = NULL;
    if (set->nodes != NULL) {
        free(set->nodes);
        set->nodes = NULL;
    }
    if (set->records != NULL) {
        free(set->records);
        set->records = NULL;
    }
    if (set->sub_records != NULL) {
        free(set->sub_records);
        set->sub_records = NULL;
    }
    if (set->list != NULL) {
        free(set->list);
        set->list = NULL;
    }
}

/* Frees every slot in state 1. */
ADDRESS(0x800238c0, 0x6c)
void anim_sets_free_idle(void)
{
    StAnimSet *set = vs_gfx_state.anim_sets;
    u16 i;

    i = 12;
    do {
        if (set->state == 1)
            anim_set_free(set);
        set++;
    } while (--i);
}

/* Frees every slot in use. */
ADDRESS(0x8002392c, 0x60)
void anim_sets_free_all(void)
{
    StAnimSet *set = vs_gfx_state.anim_sets;
    s16 i;

    i = 12;
    do {
        if (set->state)
            anim_set_free(set);
        set++;
    } while (--i);
}

/* The C library's malloc, or NULL when the result is not in main RAM: a
 * second copy of mem_alloc_checked (src/vs/cd_read.c). */
ADDRESS(0x8002398c, 0x44)
void *mem_alloc_main_ram(u32 size)
{
    u8 *p = malloc(size);

    if ((u32)(p - (u8 *)0x80000000) > 0x1fffff)
        return NULL;
    return p;
}

#include "../lib/anim_list_init.inc"

#include "../lib/anim_list.inc"

/* Finds a free slot, or NULL (ST.EXE's anim_set_alloc frees the state-1
 * slots and searches again; VS gives up). */
ADDRESS(0x800241b4, 0x58)
void anim_set_alloc(StAnimSet **out)
{
    u16 i;

    *out = vs_gfx_state.anim_sets;
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

/* Debug print of a matrix (unused). */
ADDRESS(0x8002420c, 0x90)
void debug_print_matrix(MATRIX *m)
{
    FntPrint("==== Matrixprint ====\n");
    FntPrint("%5d %5d %5d\n", m->m[0][0], m->m[0][1], m->m[0][2]);
    FntPrint("%5d %5d %5d\n", m->m[1][0], m->m[1][1], m->m[1][2]);
    FntPrint("%5d %5d %5d\n", m->m[2][0], m->m[2][1], m->m[2][2]);
    FntPrint("%5d %5d %5d\n", m->t[0], m->t[1], m->t[2]);
}

/*
 * Animation record and list composition, ST.EXE's
 * (src/lib/anim_compose.inc, shared with src/st/anim_set.c).
 */

#include "../lib/anim_compose.inc"

/*
 * ST.EXE's anim_prepare_any and anim_set_pose in one: for a model without
 * animation frees the slot at *setp and points the scratchpad at the model
 * (returns 1); otherwise poses skeletal model `id` at `frame` (4096ths of
 * the clip) of `clip` through the slot at *setp, claiming one and its four
 * buffers first (freeing every slot until the buffers fit). Returns the
 * slot, or 0 when none is free.
 */
ADDRESS(0x800246d8, 0x484)
int anim_set_pose(StAnimSet **setp, int id, int clip, u_long frame)
{
    StAnimFile *file = vs_gfx_state.unk834C[id];
    StAnimSet *set = *setp;
    VsScratchPad *sp = VS_SCRATCHPAD;
    StAnimFile *still;
    u_long t;
    u_long *data;
    u8 *model;

    if (file->animated == 0) {
        if (set != NULL)
            anim_set_free(set);
        still = vs_gfx_state.unk834C[id];
        model = ANIM_MODEL(still);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (set == NULL) {
        anim_set_alloc(&set);
        if (set == NULL)
            return 0;
    claim:
        set->model_id = id;
        set->owner = setp;
    retry:
        set->nodes = mem_alloc_main_ram(file->node_count * sizeof(StAnimNode));
        set->records = mem_alloc_main_ram(file->node_count * sizeof(StAnimRecord));
        set->sub_records = mem_alloc_main_ram(file->node_count * sizeof(StAnimSubRecord));
        set->list = mem_alloc_main_ram(file->node_count * sizeof(StAnimList));
        if (set->nodes == NULL || set->records == NULL || set->sub_records == NULL ||
            set->list == NULL) {
            anim_sets_free_all();
            goto retry;
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
    sp->anims = set->list;
    if (set->posed_frame != t) {
        while (t >= set->next_block->frame) {
            set->next_block = anim_block_run(t, set->next_block, set->list, set->node_ids, set->node_table,
                                        (u_long *)(ANIM_MODEL(file) + 4));
            anim_list_compose();
            set->blocks_run++;
        }
    }
    set->posed_frame = t;
    set->state = 2;
    return (int)set;
}

/*
 * Drawing the current fighter's model: its object
 * parts through the model list (VS's copy of ST.EXE's model_draw_list,
 * src/st/model_draw.c) and the fighter itself, lit and posed.
 */
/* The renderers' default NCCS colour (128, 128, 128). */
SDATA(0x800632d0, 0x4)
u_long fighter_skeletal_rgb = 0x808080;

/* The frame slot of the model list's every-nth parts. */
DATA(0x800632d4, 0x4, ".sdata")
static s32 model_list_slot = 0;

/* Projects the current part's vertices: RTPT in threes, the first group
 * ahead of the loop so each pass stores one group and starts the next, then
 * RTPS for the rest. */
#define PROJECT(sp, out, in, n)                                                \
    {                                                                          \
        out = (MeshVertex *)(sp)->vertices;                                    \
        in = (SVECTOR *)(sp)->local_vertices;                                  \
        n = (sp)->header->n_vertices / 3;                                      \
        if (n--) {                                                             \
            gte_ldv3c(in);                                                     \
            gte_rtpt();                                                        \
            in += 3;                                                           \
            for (;;) {                                                         \
                gte_stsxy3(&out[0].sxy, &out[1].sxy, &out[2].sxy);             \
                gte_stsz3(&out[0].z, &out[1].z, &out[2].z);                    \
                out += 3;                                                      \
                if (!n--)                                                      \
                    break;                                                     \
                gte_ldv3c(in);                                                 \
                in += 3;                                                       \
                gte_rtpt();                                                    \
            }                                                                  \
        }                                                                      \
        n = (sp)->header->n_vertices % 3;                                      \
        while (n--) {                                                          \
            gte_ldv0(in);                                                      \
            gte_rtps();                                                        \
            in++;                                                              \
            gte_stsxy(&out->sxy);                                              \
            gte_stsz(&out->z);                                                 \
            out++;                                                             \
        }                                                                      \
    }

/*
 * Draws each entry of the scratchpad's model list: kind 2 turns its parent
 * matrix to face the camera first, kinds 1, 3 and 4 draw scaled and only
 * on the entry whose index matches model_list_slot's slot (every 6th, 4th,
 * 8th), stepping it.
 */
ADDRESS(0x80024b5c, 0x6a8)
void model_draw_list(void)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    ModelEntry *e = ((ModelList *)sp->anims)->entries;
    int i = ((ModelList *)sp->anims)->count;
    MATRIX scale = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    MeshVertex *out;
    SVECTOR *in;
    int n;

    while (i--) {
        e->obj->composed = 0;
        e++;
    }
    i = ((ModelList *)sp->anims)->count;
    e = ((ModelList *)sp->anims)->entries;
    while (i--) {
        e->obj->composed = 0;
        if (e->key != -1 && e->part != 0xffff) {
            if (e->kind == 2) {
                MATRIX face = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
                SVECTOR angle;

                angle.vx = -current_fighter->unk4DF00 + vs_gfx_state.unk2CA88.vx;
                angle.vy = -current_fighter->unk4DF02 + vs_gfx_state.unk2CA88.vy;
                angle.vz = -current_fighter->unk4DF04 + vs_gfx_state.unk2CA88.vz;
                RotMatrixX(angle.vx, &face);
                RotMatrixY(angle.vy, &face);
                RotMatrixZ(angle.vz, &face);
                if (e->obj->parent->relative_set == 0) {
                    RotMatrix(&e->obj->parent->transform->rot, &e->obj->parent->local_matrix);
                }
                gte_MulMatrix0(&e->obj->parent->local_matrix, &face, &e->obj->parent->local_matrix);
                e->obj->parent->relative_set = 0;
            }
            anim_record_compose(e->obj);
            if (e->kind == 1) {
                if (model_list_slot >= 6)
                    model_list_slot = 0;
                ScaleMatrix(&scale, &e->obj->transform->scale);
                gte_SetRotMatrix(&scale);
                if (i % 6 != model_list_slot) {
                    e++;
                    continue;
                }
                model_list_slot = i % 6 + 1;
            } else if (e->kind == 3) {
                if (model_list_slot >= 4)
                    model_list_slot = 0;
                ScaleMatrix(&scale, &e->obj->transform->scale);
                gte_SetRotMatrix(&scale);
                if (i % 4 != model_list_slot) {
                    e++;
                    continue;
                }
                model_list_slot = i % 4 + 1;
            } else if (e->kind == 4) {
                if (model_list_slot >= 8)
                    model_list_slot = 0;
                ScaleMatrix(&scale, &e->obj->transform->scale);
                gte_SetRotMatrix(&scale);
                if (i % 8 != model_list_slot) {
                    e++;
                    continue;
                }
                model_list_slot = i % 8 + 1;
            }
            sp->header = (MeshHeader *)(sp->model + 12) + e->part;
            sp->local_vertices = sp->model + 12 + sp->header->vertices;
            PROJECT(sp, out, in, n);
            mesh_mirror = 0;
            mesh_draw_part_nccs(e->part);
        }
        e++;
    }
}

/*
 * Draws the current fighter (current_fighter) unless it has no animation
 * frame: lights it, places it at its position turned by its rotation and
 * the camera, poses its model and draws it (dimmed by its brightness
 * below 128) with its object parts.
 */
ADDRESS(0x80025204, 0x46c)
void fighter_draw_skeletal(void)
{
    if (current_fighter->unk4DEEA != -1) {
    VsScratchPad *sp = VS_SCRATCHPAD;
    VsLights lights = {
        {{3000, 3700, -1500}, {-2000, 3700, -3000}, {-1300, -1000, 1024}}, 0,
        {{4096, 3000, 3000}, {4096, 3000, 3000}, {4096, 3000, 3000}}, 0,
        {100, 100, 100, 0}, {-1, -1}};
    MATRIX m = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    VECTOR unused0;      /* unused: retail reserves their frame slots */
    SVECTOR unused1;
    int rx;
    int ry;
    int n;

    rx = light_rot_x + current_fighter->unk4DF00;
    ry = light_rot_y + current_fighter->unk4DF02;
    ScaleMatrix(&m, &light_scale);
    RotMatrixX(rx, &m);
    RotMatrixY(ry, &m);
    MulMatrix((MATRIX *)&lights.l, &m);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    sp->unk68 = lights.dq[0];
    sp->unk6C = lights.dq[1];
    gte_SetBackColor(lights.bk.r, lights.bk.g, lights.bk.b);
    sp->matrix.t[0] = current_fighter->unk4DEF0.vx - vs_gfx_state.unk2CA78.vx;
    sp->matrix.t[1] = current_fighter->unk4DEF0.vy - vs_gfx_state.unk2CA78.vy;
    sp->matrix.t[2] = current_fighter->unk4DEF0.vz - vs_gfx_state.unk2CA78.vz;
    RotMatrix((SVECTOR *)&current_fighter->unk4DF00, &sp->matrix);
    CompMatrix(&vs_gfx_state.unk2C998, &sp->matrix, &sp->matrix);
    n = current_fighter - shared_buffer.fighter;
    sp->model = (u8 *)vs_gfx_state.unk834C[n] + ((ModelFile *)vs_gfx_state.unk834C[n])->model;
    if (anim_set_pose(&current_fighter->unk4DEE0, n, current_fighter->unk4DEE8,
                      current_fighter->unk4DEEA) == 0)
        return;
    sp->ot = vs_gfx_state.ot_cur;
    sp->far_z = 4096;
    if (current_fighter->unk4DF12 < 128) {
        sp->rgb.r = current_fighter->unk4DF12;
        sp->rgb.g = current_fighter->unk4DF12;
        sp->rgb.b = current_fighter->unk4DF12;
        sp->rgb.cd = 0;
        mesh_semi_trans = 1;
    } else {
        *(u_long *)&sp->rgb = fighter_skeletal_rgb;
        mesh_semi_trans = 0;
    }
    sp->face_count = vs_gfx_state.unk831C[1];
    sp->prim_count = vs_gfx_state.unk8324;
    sp->unk94 = vs_gfx_state.unk8328;
    sp->prim = vs_gfx_state.packet->cur;
    sp->prim_end = vs_gfx_state.packet->end;
    sp->vertices = vs_gfx_state.unkA7EC;
    sp->clut_base = 0;
    if (current_fighter->unk4DEEA != -1)
        model_draw_list();
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.unk831C[1] = sp->face_count;
    vs_gfx_state.unk8328 = sp->unk94;
    vs_gfx_state.unk8324 = sp->prim_count;
    }
}

/*
 * Node list lookup by id, ST.EXE's (src/lib/anim_list_find_id.inc,
 * shared with src/st/anim.c), and a fighter's posed node vertex.
 */

#include "../lib/anim_list_find_id.inc"

/*
 * The position of vertex `index` of part `node` of fighter `id`'s model,
 * moved by that node's composed matrix in the fighter's skeletal slot; a
 * zero vector when the model or slot is missing (ST.EXE's
 * anim_set_node_vertex composes from identity_matrix, VS from a local
 * identity).
 */
ADDRESS(0x800256bc, 0x1bc)
void anim_set_node_vertex(int id, int node, int index, VECTOR *out)
{
    MATRIX unused;
    StAnimSet *set = shared_buffer.fighter[id].unk4DEE0;
    StAnimFile *file = (StAnimFile *)shared_buffer.fighter[id].file;
    MATRIX identity = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}};
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
    *SCRATCH_MATRIX(SCRATCHPAD) = identity;
    anim_record_compose(n->record);
    gte_ldv0(v);
    gte_rt();
    gte_stlvnl(out);
}
