/*
 * VS.EXE: node list lookup by id, ST.EXE's (src/lib/anim_list_find_id.inc,
 * shared with src/st/anim.c), and a fighter's posed node vertex (WIP
 * module).
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
#include <vs/funcs.h>
#include <vs/buffer.h>

/* func_800256bc's identity matrix. */
RODATA(0x80011320, 0x20)

/* The file's mesh model, part headers and part vertex arrays. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))
#define PART_VERTICES(model, part) ((SVECTOR *)((model) + (MODEL_PARTS(model)[part].vertices + 12)))

#include "../lib/anim_list_find_id.inc"

/*
 * The position of vertex `index` of part `node` of fighter `id`'s model,
 * moved by that node's composed matrix in the fighter's skeletal slot; a
 * zero vector when the model or slot is missing (ST.EXE's
 * anim_set_node_vertex composes from DAT_8008040c, VS from a local
 * identity).
 */
ADDRESS(0x800256bc, 0x1bc)
void func_800256bc(int id, int node, int index, VECTOR *out)
{
    MATRIX unused;
    StAnimSet *set = DAT_800633b0.fighter[id].unk4DEE0;
    StAnimFile *file = (StAnimFile *)DAT_800633b0.fighter[id].file;
    MATRIX identity = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};
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
    *SCRATCH_MATRIX(SCRATCHPAD) = identity;
    anim_record_compose(n->unk4);
    gte_ldv0(v);
    gte_rt();
    gte_stlvnl(out);
}
