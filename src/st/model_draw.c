/*
 * ST.EXE model renderers (WIP module): model_draw places one model part
 * (or, through model_draw_list, a model's list of object parts) with its
 * own matrix and runs the band C part renderers on it; func_80021ac4 does
 * the same for a viewer model slot. Each brackets the part renderers with
 * the frame's packet cursor and the four per-frame counters
 * (world_state.unk831C..unk8328 in ScratchPad.z_ot..unk94).
 *
 * In this phase the scratchpad's 0xa8 VECTOR (part_pos for the world
 * renderers) is the start of the model's MATRIX, and 0x50..0x58 holds a
 * camera-space vertex. The unit is -G8: func_80021ac4 reads its colour word
 * $gp-relative.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <GTEMAC.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>

RODATA(0x80011010, 0x20)

/* The model matrix (rotation and translation) the part renderers draw with. */
/* A camera-space vertex (RT result). */
#define MODEL_POS(sp) ((VECTOR *)&(sp)->unk50)

/* A model block's part data. */
#define MODEL_DATA(block) ((u8 *)(block) + ((ModelFile *)(block))->unk8)

/*
 * The NCDS input colour func_80021ac4 draws with (128, 128, 128). A
 * tentative definition: the linker merges it into the initialized .sdata
 * word another unit defines; its size lets ASPSX use the $gp-relative load
 * (an extern declaration gets lui+lw).
 */
u_long DAT_80087ee8;

/*
 * Projects the current part's model-space vertices (RTPS) into the
 * transformed-vertex buffer, with a depth-cue value from the lighting's
 * near/far depths (ScratchPad.unk64/unk68).
 */
#define MODEL_PROJECT(sp, out, in, n)                                          \
    while (n--) {                                                              \
        gte_ldv0(in);                                                          \
        gte_rtps();                                                            \
        gte_stsxy(&(out)->sxy);                                                \
        gte_stsz(&(out)->z);                                                   \
        if ((sp)->unk64 != (sp)->unk68) {                                      \
            (out)->p = (((out)->z - (sp)->unk64) << 12) / ((sp)->unk68 - (sp)->unk64); \
        } else {                                                               \
            (out)->p = 0;                                                      \
        }                                                                      \
        if ((out)->p > 4065) {                                                 \
            (out)->p = 4065;                                                   \
        }                                                                      \
        if ((out)->p < 0) {                                                    \
            (out)->p = 0;                                                      \
        }                                                                      \
        out++;                                                                 \
        in++;                                                                  \
    }

/*
 * Transforms the current part's vertices to camera space (RT) without
 * projecting them: x and y become the screen word, the caller's z the
 * depth and the camera-space z the depth-cue slot.
 */
#define MODEL_TRANSFORM(sp, depth)                                             \
    {                                                                          \
        MeshVertex *tout;                                                      \
        SVECTOR *tin;                                                          \
        int tn;                                                                \
                                                                               \
        (sp)->z_ot += (sp)->header->n_vertices;                                \
        tout = (MeshVertex *)(sp)->vertices;                                   \
        tin = (SVECTOR *)(sp)->local_vertices;                                 \
        tn = (sp)->header->n_vertices;                                         \
        while (tn--) {                                                         \
            gte_ldv0(tin);                                                     \
            gte_rt();                                                          \
            gte_stlvnl(MODEL_POS(sp));                                         \
            tin++;                                                         \
            tout->sxy.vx = MODEL_POS(sp)->vx;                              \
            tout->sxy.vy = MODEL_POS(sp)->vy;                              \
            tout->p = MODEL_POS(sp)->vz;                                   \
            tout->z = depth;                                               \
            tout++;                                                            \
        }                                                                      \
    }

/* Draws viewer model slot `slot` (skipped while its unk130 is -1). */
ADDRESS(0x80021ac4, 0x54c)
void func_80021ac4(u8 slot)
{
    ScratchPad *sp;
    MATRIX light;
    SVECTOR rot;
    MeshVertex *out;
    SVECTOR *in;
    int n;

    if (viewer.unk130[slot] == -1) {
        return;
    }
    func_80017d28(0, 0);
    sp = SCRATCHPAD;
    gte_MulMatrix0((MATRIX *)&sp->unk394->light[3], &world_state.unk1ECE8, &light);
    gte_SetLightMatrix(&light);
    sp->model_t[0] = viewer.unkE8[slot]->unkC;
    sp->model_t[1] = viewer.unkE8[slot]->unkE;
    sp->model_t[2] = viewer.unkE8[slot]->unk10;
    if (viewer.unk38[16] != 0xffff && slot != 0) {
        rot.vx = viewer.unkE8[slot]->unk14.vx + viewer.unk2DE;
        rot.vy = viewer.unkE8[slot]->unk14.vy + viewer.unk2E0;
        rot.vz = viewer.unkE8[slot]->unk14.vz + viewer.unk2E2;
        RotMatrix(&rot, SCRATCH_MATRIX(sp));
    } else {
        RotMatrix(&viewer.unkE8[slot]->unk14, SCRATCH_MATRIX(sp));
    }
    sp->far_z = 64;
    sp->near_z = 4;
    sp->ot = world_state.ot2_cur;
    sp->z_ot = world_state.unk831C[0];
    sp->face_count = world_state.unk831C[1];
    sp->prim_count = world_state.unk8324;
    sp->unk94 = world_state.unk8328;
    sp->prim = world_state.packet->cur;
    sp->prim_end = world_state.packet->end;
    sp->vertices = world_state.world_vertices;
    sp->clut_base = 0;
    *(u_long *)&sp->rgb = DAT_80087ee8;
    if (((ModelFile *)viewer.unkF8[slot])->unk6 == 0) {
        gte_SetRotMatrix(SCRATCH_MATRIX(sp));
        gte_SetTransMatrix(SCRATCH_MATRIX(sp));
        sp->model = MODEL_DATA(world_state.unk834C[slot + 32]);
        sp->header = (MeshHeader *)(sp->model + 12);
        if (anim_prepare((StAnimSlot **)&viewer.unk108[slot], slot + 32, viewer.unk150[slot],
                          viewer.unk130[slot])) {
            sp->z_ot += sp->header->n_vertices;
            out = (MeshVertex *)sp->vertices;
            in = (SVECTOR *)sp->local_vertices;
            n = sp->header->n_vertices;
            MODEL_PROJECT(sp, out, in, n);
            mesh_draw_part_nccs(0, 0x80);
        }
    } else {
        sp->model = MODEL_DATA(world_state.unk834C[slot + 32]);
        if (anim_prepare_any(&viewer.unk108[slot], slot + 32, viewer.unk150[slot],
                          viewer.unk130[slot])) {
            model_draw_list(0x80, 0);
        }
    }
    world_state.packet->cur = sp->prim;
    world_state.packet->end = sp->prim_end;
    world_state.unk831C[0] = sp->z_ot;
    world_state.unk831C[1] = sp->face_count;
    world_state.unk8328 = sp->unk94;
    world_state.unk8324 = sp->prim_count;
}

/*
 * Draws each entry of the scratchpad's model list: kind 2 turns its joint
 * matrix to face the viewer first, kinds 1, 3 and 4 draw scaled and only on
 * the entries whose index matches the frame counter (every 6th, 4th, 8th).
 */
ADDRESS(0x80022010, 0x668)
void model_draw_list(u8 flags, SVECTOR *rot)
{
    ScratchPad *sp = SCRATCHPAD;
    ModelEntry *e = sp->list.models->entries;
    int i = sp->list.models->count;
    MATRIX work;         /* unused: retail reserves its frame slot */
    MATRIX scale = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    MeshVertex *out;
    SVECTOR *in;
    int n;

    while (i--) {
        e->obj->unk0 = 0;
        e++;
    }
    e = sp->list.models->entries;
    i = sp->list.models->count;
    while (i--) {
        e->obj->unk0 = 0;
        if (e->unk0C == -1 || e->part == 0xffff) {
            e++;
            continue;
        }
        if (e->kind == 2) {
            MATRIX face = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
            SVECTOR angle;

            angle.vx = -viewer.unk268;
            angle.vy = -viewer.unk26A;
            angle.vz = -viewer.unk26C;
            RotMatrixX(angle.vx, &face);
            RotMatrixY(angle.vy + rot->vy, &face);
            RotMatrixZ(angle.vz, &face);
            if (e->obj->unk48->unk2 == 0) {
                RotMatrix(&e->obj->unk48->unk44->rot, &e->obj->unk48->unk4);
            }
            gte_MulMatrix0(&e->obj->unk48->unk4, &face, &e->obj->unk48->unk4);
            e->obj->unk48->unk2 = 0;
        }
        anim_record_compose(e->obj);
        if (e->kind == 1) {
            if (i % 6 != world_state.unk1ECC0 % 6) {
                e++;
                continue;
            }
            ScaleMatrix(&scale, &e->obj->unk44->scale);
            gte_SetRotMatrix(&scale);
        } else if (e->kind == 3) {
            if (i % 4 != world_state.unk1ECC0 % 4) {
                e++;
                continue;
            }
            ScaleMatrix(&scale, &e->obj->unk44->scale);
            gte_SetRotMatrix(&scale);
        } else if (e->kind == 4) {
            if (i % 8 != world_state.unk1ECC0 % 8) {
                e++;
                continue;
            }
            ScaleMatrix(&scale, &e->obj->unk44->scale);
            gte_SetRotMatrix(&scale);
        }
        sp->header = (MeshHeader *)(sp->model + 12) + e->part;
        sp->local_vertices = sp->model + 12 + sp->header->vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        if (flags & 0x40) {
            mesh_div_draw_part(e->part, flags);
        } else if (flags & 0x80) {
            mesh_draw_part(e->part, flags);
        } else {
            mesh_draw_part_nccs(e->part, flags);
        }
        e++;
    }
}

/*
 * Draws part `part` of model block `model` at `pos` with rotation `rot`
 * and an optional scale. With a view matrix, `pos` is a world position
 * (made viewer-relative and turned by the camera) and the vertices are
 * projected; without one, `pos` is already in camera space and the
 * vertices are only transformed, at depth `z`. Flag 0x40 selects the
 * subdividing renderers (dropped when the packet buffer runs low), 0x80
 * the depth-cued ones; part numbers from 128 select a part directly.
 */
ADDRESS(0x80022678, 0xb90)
void model_draw(int unused0, u16 model, VECTOR *pos, SVECTOR *rot, SVECTOR *scale, void *anim,
                MATRIX *view, u16 part, u16 arg8, short light, short unused10, u8 flags,
                int z)
{
    ScratchPad *sp = SCRATCHPAD;
    SVECTOR rel;
    VECTOR rel_l;        /* stored but never read */
    VECTOR s;
    MATRIX work0;        /* unused, like work1: retail reserves their frame slots */
    MATRIX lm;
    MATRIX work1;
    long flag;
    MeshVertex *out;
    SVECTOR *in;
    u_int i;
    int n;

    sp->vertices = world_state.world_vertices;
    /* 0x44 holds a second vertex cursor in this phase. */
    (&sp->vertices)[1] = world_state.world_vertices;
    sp->z_ot = world_state.unk831C[0];
    sp->face_count = world_state.unk831C[1];
    sp->prim_count = world_state.unk8324;
    sp->unk94 = world_state.unk8328;
    sp->prim = world_state.packet->cur;
    sp->prim_end = world_state.packet->end;
    if ((u8 *)sp->prim_end - 0x7800 < (u8 *)sp->prim) {
        flags &= ~0x40;
    }
    if (sp->prim_end < sp->prim) {
        func_8003f12c(0xffff, 0, 0);
        func_8003f12c(0xffff, 1, 0);
    }
    gte_SetRotMatrix(&world_state.unk1ECE8);
    gte_SetTransMatrix(&world_state.unk1ECE8);
    if (view) {
        rel_l.vx = pos->vx - viewer.position.vx;
        rel_l.vy = pos->vy - (viewer.position.vy + viewer.unk2CE + viewer.unk2D2);
        rel_l.vz = pos->vz - viewer.position.vz;
        rel.vx = pos->vx - viewer.position.vx;
        rel.vy = pos->vy - (viewer.position.vy + viewer.unk2CE + viewer.unk2D2);
        rel.vz = pos->vz - viewer.position.vz;
        gte_RotTrans(&rel, sp->model_t, &flag);
    } else {
        sp->model_t[0] = pos->vx;
        sp->model_t[1] = pos->vy;
        sp->model_t[2] = pos->vz;
    }
    func_80049e30(rot, SCRATCH_MATRIX(sp));
    func_80017d28(light, flags);
    gte_MulMatrix0((MATRIX *)sp->unk394, SCRATCH_MATRIX(sp), &lm);
    gte_SetLightMatrix(&lm);
    if (scale) {
        s.vx = scale->vx;
        s.vy = scale->vy;
        s.vz = scale->vz;
        ScaleMatrix(SCRATCH_MATRIX(sp), &s);
    }
    if (view) {
        gte_MulMatrix0(view, SCRATCH_MATRIX(sp), SCRATCH_MATRIX(sp));
    }
    gte_SetRotMatrix(SCRATCH_MATRIX(sp));
    gte_SetTransMatrix(SCRATCH_MATRIX(sp));
    if (((ModelFile *)world_state.unk834C[model])->unk6 == 0) {
        sp->model = MODEL_DATA(world_state.unk834C[model]);
        if (part < 128) {
            i = 0;
            sp->header = (MeshHeader *)(sp->model + 12);
            if (anim_prepare(anim, model, part, arg8) == 0) {
                sp->header = (MeshHeader *)(sp->model + 12);
                sp->local_vertices = sp->model + 12 + sp->header->vertices;
            }
        } else {
            i = part & 0x7f;
            sp->header = (MeshHeader *)(sp->model + 12) + i;
            sp->local_vertices = sp->model + 12 + sp->header->vertices;
        }
    } else {
        sp->model = MODEL_DATA(world_state.unk834C[model]);
        if (part < 128) {
            if (anim_prepare_any(anim, model, part, arg8) == 0) {
                sp->header = (MeshHeader *)(sp->model + 12);
                sp->local_vertices = sp->model + 12 + sp->header->vertices;
            } else {
                model_draw_list(flags, rot);
            }
            goto done;
        } else {
            i = part & 0x7f;
            sp->header = (MeshHeader *)(sp->model + 12) + i;
            sp->local_vertices = sp->model + 12 + sp->header->vertices;
        }
    }
    if (flags & 0x40) {
        sp->z_ot += sp->header->n_vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        if (view) {
            mesh_div_draw_part(i, flags);
        } else {
            MODEL_TRANSFORM(sp, z);
            mesh_draw_part_nccs(i, flags);
        }
    } else if (flags & 0x80) {
        sp->z_ot += sp->header->n_vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        if (view) {
            mesh_draw_part(i, flags);
        } else {
            MODEL_TRANSFORM(sp, z);
            mesh_draw_part_nccs(i, flags);
        }
    } else if (view) {
        sp->z_ot += sp->header->n_vertices;
        out = (MeshVertex *)sp->vertices;
        in = (SVECTOR *)sp->local_vertices;
        n = sp->header->n_vertices;
        MODEL_PROJECT(sp, out, in, n);
        mesh_draw_part_nccs(i, flags);
    } else {
        MODEL_TRANSFORM(sp, z);
        mesh_draw_part_nccs(i, flags);
    }
done:
    world_state.packet->cur = sp->prim;
    world_state.packet->end = sp->prim_end;
    world_state.unk831C[0] = sp->z_ot;
    world_state.unk831C[1] = sp->face_count;
    world_state.unk8324 = sp->prim_count;
    world_state.unk8328 = sp->unk94;
}
