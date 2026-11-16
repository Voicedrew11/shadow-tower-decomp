/*
 * ST.EXE subdividing mesh renderers' part setup (WIP module): func_8001bf10
 * prepares a model part and runs the st.mesh_draw_div renderers. It is its
 * own unit only because the inexact func_8001b5dc before it is one
 * instruction short.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>

extern void func_8001a038(int count, MeshFaceFT3 *face, u_long *ot, int flags);
extern void func_8001a6ec(int count, MeshFaceFT4 *face, u_long *ot, int flags);
extern void func_8001ae7c(int count, MeshFaceGT3 *face, u_long *ot, int flags);
extern void func_8001b5dc(int count, MeshFaceGT4 *face, u_long *ot, int flags);

ADDRESS(0x8001bf10, 0x1f0)
void func_8001bf10(u_short part, u_int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    int mode;

    mode = (flags >> 1) & 2;
    mode |= (flags & 3) << 21;
    sp->header = (MeshHeader *)(sp->model + 12) + part;
    sp->normals = sp->model + 12 + sp->header->normals;
    if (MESH_MODE & MESH_TINT_A) {
        if (MESH_MODE & MESH_TINT_SLOW) {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -120 - (MESH_TINT_LEVEL >> 13);
        } else {
            sp->rgb.r = sp->rgb.g = sp->rgb.b = -96 - (MESH_TINT_LEVEL >> 11);
        }
    } else {
        sp->rgb.r = sp->rgb.g = sp->rgb.b = 128;
    }
    if (MESH_MODE & MESH_TINT_A) {
        gte_SetFarColor(6, 4, 2);
    } else if (MESH_MODE & MESH_TINT_B) {
        gte_SetFarColor(2, 6, 6);
    } else {
        gte_SetFarColor(5, 5, 5);
    }
    sp->faces = sp->model + 12 + sp->header->faces;
    func_8001a038(sp->header->n_ft3, sp->faces, sp->ot, mode);
    func_8001a6ec(sp->header->n_ft4, sp->faces, sp->ot, mode);
    func_8001ae7c(sp->header->n_gt3, sp->faces, sp->ot, mode);
    func_8001b5dc(sp->header->n_gt4, sp->faces, sp->ot, mode);
}
