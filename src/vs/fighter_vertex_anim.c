/*
 * VS.EXE: drawing the current fighter's one-part model with its floor
 * reflection (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <GTEMAC.H>
#include <st/mesh.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/scratch.h>
#include <vs/funcs.h>

RODATA(0x800113dc, 0x50)
SDATA(0x800632d8, 0x4)

/* The renderers' default NCCS colour (128, 128, 128). */
u_long fighter_vertex_anim_rgb = 0x808080;

/*
 * Draws the current fighter (current_fighter) unless it has no animation
 * frame, as fighter_draw_skeletal does, but as one vertex-animated part with its
 * floor reflection (model_draw_mirrored).
 */
ADDRESS(0x80028914, 0x480)
void fighter_draw_vertex_anim(void)
{
    if (current_fighter->anim_frame != -1) {
    VsScratchPad *sp = VS_SCRATCHPAD;
    VsLights lights = {
        {{3000, 3700, -1500}, {-2000, 3700, -3000}, {-1300, -1000, 1024}}, 0,
        {{4096, 3000, 3000}, {4096, 3000, 3000}, {4096, 3000, 3000}}, 0,
        {100, 100, 100, 0}, {-1, -1}};
    MATRIX m = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    VECTOR unused0;      /* unused: retail reserves their frame slots */
    SVECTOR unused1;
    int n;

    ScaleMatrix(&m, &light_scale);
    RotMatrixX(light_rot_x, &m);
    RotMatrixY(light_rot_y, &m);
    MulMatrix((MATRIX *)&lights.l, &m);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    sp->depth_cue_near = lights.dq[0];
    sp->depth_cue_far = lights.dq[1];
    gte_SetBackColor(lights.bk.r, lights.bk.g, lights.bk.b);
    sp->matrix.t[0] = current_fighter->position.vx - vs_gfx_state.camera_pos.vx;
    sp->matrix.t[1] = current_fighter->position.vy - vs_gfx_state.camera_pos.vy;
    sp->matrix.t[2] = current_fighter->position.vz - vs_gfx_state.camera_pos.vz;
    RotMatrix((SVECTOR *)&current_fighter->rot_x, &sp->matrix);
    CompMatrix(&vs_gfx_state.view_matrix, &sp->matrix, &sp->matrix);
    gte_SetRotMatrix(&sp->matrix);
    gte_SetTransMatrix(&sp->matrix);
    n = current_fighter - shared_buffer.fighter;
    sp->model = (u8 *)vs_gfx_state.model_files[n] + ((ModelFile *)vs_gfx_state.model_files[n])->model;
    sp->header = (MeshHeader *)(sp->model + 12);
    if (anim_pose(&current_fighter->anim_slot, n, current_fighter->anim,
                      current_fighter->anim_frame) == 0)
        return;
    sp->ot = vs_gfx_state.ot_cur;
    sp->far_z = 4096;
    if (current_fighter->brightness < 128) {
        sp->rgb.r = current_fighter->brightness;
        sp->rgb.g = current_fighter->brightness;
        sp->rgb.b = current_fighter->brightness;
        sp->rgb.cd = 0;
        mesh_semi_trans = 1;
    } else {
        *(u_long *)&sp->rgb = fighter_vertex_anim_rgb;
        mesh_semi_trans = 0;
    }
    sp->near_z = 4;
    sp->face_count = vs_gfx_state.vertex_face_counts[1];
    sp->prim_count = vs_gfx_state.prim_count;
    sp->prim = vs_gfx_state.packet->cur;
    sp->prim_end = vs_gfx_state.packet->end;
    sp->vertices = vs_gfx_state.projected_vertices;
    sp->clut_base = 0;
    if (current_fighter->anim_frame != -1)
        model_draw_mirrored();
    vs_gfx_state.packet->cur = sp->prim;
    vs_gfx_state.packet->end = sp->prim_end;
    vs_gfx_state.vertex_face_counts[1] = sp->face_count;
    vs_gfx_state.prim_count = sp->prim_count;
    }
}
