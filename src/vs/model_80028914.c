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
u_long DAT_800632d8 = 0x808080;

/*
 * Draws the current fighter (DAT_80063300) unless it has no animation
 * frame, as func_80025204 does, but as one vertex-animated part with its
 * floor reflection (func_80028474).
 */
ADDRESS(0x80028914, 0x480)
void func_80028914(void)
{
    if (DAT_80063300->unk4DEEA != -1) {
    VsScratchPad *sp = VS_SCRATCHPAD;
    VsLights lights = {
        {{3000, 3700, -1500}, {-2000, 3700, -3000}, {-1300, -1000, 1024}}, 0,
        {{4096, 3000, 3000}, {4096, 3000, 3000}, {4096, 3000, 3000}}, 0,
        {100, 100, 100, 0}, {-1, -1}};
    MATRIX m = {{{ONE, 0, 0}, {0, ONE, 0}, {0, 0, ONE}}, {0, 0, 0}};
    VECTOR unused0;      /* unused: retail reserves their frame slots */
    SVECTOR unused1;
    int n;

    ScaleMatrix(&m, &DAT_8005b2a8);
    RotMatrixX(DAT_80063218, &m);
    RotMatrixY(DAT_8006321c, &m);
    MulMatrix((MATRIX *)&lights.l, &m);
    gte_SetColorMatrix(&lights.lc);
    gte_SetLightMatrix(&lights.l);
    sp->unk68 = lights.dq[0];
    sp->unk6C = lights.dq[1];
    gte_SetBackColor(lights.bk.r, lights.bk.g, lights.bk.b);
    sp->matrix.t[0] = DAT_80063300->unk4DEF0.vx - DAT_801624d0.unk2CA78.vx;
    sp->matrix.t[1] = DAT_80063300->unk4DEF0.vy - DAT_801624d0.unk2CA78.vy;
    sp->matrix.t[2] = DAT_80063300->unk4DEF0.vz - DAT_801624d0.unk2CA78.vz;
    RotMatrix((SVECTOR *)&DAT_80063300->unk4DF00, &sp->matrix);
    CompMatrix(&DAT_801624d0.unk2C998, &sp->matrix, &sp->matrix);
    gte_SetRotMatrix(&sp->matrix);
    gte_SetTransMatrix(&sp->matrix);
    n = DAT_80063300 - DAT_800633b0.fighter;
    sp->model = (u8 *)DAT_801624d0.unk834C[n] + ((ModelFile *)DAT_801624d0.unk834C[n])->unk8;
    sp->header = (MeshHeader *)(sp->model + 12);
    if (func_800281d4(&DAT_80063300->unk4DEE4, n, DAT_80063300->unk4DEE8,
                      DAT_80063300->unk4DEEA) == 0)
        return;
    sp->ot = DAT_801624d0.ot_cur;
    sp->far_z = 4096;
    if (DAT_80063300->unk4DF12 < 128) {
        sp->rgb.r = DAT_80063300->unk4DF12;
        sp->rgb.g = DAT_80063300->unk4DF12;
        sp->rgb.b = DAT_80063300->unk4DF12;
        sp->rgb.cd = 0;
        DAT_80063350 = 1;
    } else {
        *(u_long *)&sp->rgb = DAT_800632d8;
        DAT_80063350 = 0;
    }
    sp->near_z = 4;
    sp->face_count = DAT_801624d0.unk831C[1];
    sp->prim_count = DAT_801624d0.unk8324;
    sp->prim = DAT_801624d0.packet->cur;
    sp->prim_end = DAT_801624d0.packet->end;
    sp->vertices = DAT_801624d0.unkA7EC;
    sp->clut_base = 0;
    if (DAT_80063300->unk4DEEA != -1)
        func_80028474();
    DAT_801624d0.packet->cur = sp->prim;
    DAT_801624d0.packet->end = sp->prim_end;
    DAT_801624d0.unk831C[1] = sp->face_count;
    DAT_801624d0.unk8324 = sp->prim_count;
    }
}
