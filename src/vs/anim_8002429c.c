/*
 * VS.EXE: animation record and list composition (WIP module), ST.EXE's
 * (src/lib/anim_compose.inc, shared with src/st/anim_set.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/memory.h>
#include <st/gfx.h>
#include <st/mesh.h>
#include <st/scratch.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/scratch.h>
#include <vs/funcs.h>

/* The file's mesh model and part headers. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))

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
int func_800246d8(StAnimSet **setp, int id, int clip, u_long frame)
{
    StAnimFile *file = vs_gfx_state.unk834C[id];
    StAnimSet *set = *setp;
    VsScratchPad *sp = VS_SCRATCHPAD;
    StAnimFile *still;
    u_long t;
    u_long *data;
    u8 *model;

    if (file->unk4 == 0) {
        if (set != NULL)
            func_80023820(set);
        still = vs_gfx_state.unk834C[id];
        model = ANIM_MODEL(still);
        sp->model = model;
        sp->header = MODEL_PARTS(model);
        sp->local_vertices = model + (MODEL_PARTS(model)->vertices + 12);
        return 1;
    }
    if (set == NULL) {
        func_800241b4(&set);
        if (set == NULL)
            return 0;
    claim:
        set->unk2 = id;
        set->unk38 = setp;
    retry:
        set->unk28 = func_8002398c(file->unk6 * sizeof(StAnimNode));
        set->unk2C = func_8002398c(file->unk6 * sizeof(StAnimRecord));
        set->unk30 = func_8002398c(file->unk6 * sizeof(StAnimSubRecord));
        set->unk34 = func_8002398c(file->unk6 * sizeof(StAnimList));
        if (set->unk28 == NULL || set->unk2C == NULL || set->unk30 == NULL ||
            set->unk34 == NULL) {
            func_8002392c();
            goto retry;
        }
        *setp = set;
        anim_list_init(set, file->unk6);
        set->unk8 = (u_long *)((u8 *)file + file->frames);
        set->unkC = (u8 *)file + file->clips;
    } else if (set->unk2 != id) {
        func_80023820(set);
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
    sp->anims = set->unk34;
    if (set->unk20 != t) {
        while (t >= set->unk24->frame) {
            set->unk24 = anim_block_run(t, set->unk24, set->unk34, set->unk8, set->unkC,
                                        (u_long *)(ANIM_MODEL(file) + 4));
            anim_list_compose();
            set->unk14++;
        }
    }
    set->unk20 = t;
    set->unk0 = 2;
    return (int)set;
}
