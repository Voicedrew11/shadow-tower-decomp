/*
 * VS.EXE: scratchpad vertex access and the three-point NCLIP (WIP module).
 * VS's copies of ST.EXE's func_80017cb0 and mesh_nclip_triangle (src/st/prim.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <INLINE_C.H>
#include <vs/scratch.h>

ADDRESS(0x80025878, 0x14)
u_char *func_80025878(u16 offset)
{
    return VS_SCRATCHPAD->vertices + offset;
}

/* Loads three screen points from the scratchpad buffer and stores NCLIP. */
ADDRESS(0x8002588c, 0x64)
void mesh_nclip_triangle(u16 i0, u16 i1, u16 i2)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    u_char *base = sp->vertices;

    sp->unk30 = (u_long *)(base + i0);
    sp->unk34 = (u_long *)(base + i1);
    sp->unk38 = (u_long *)(base + i2);
    gte_ldsxy3(*sp->unk30, *sp->unk34, *sp->unk38);
    gte_nclip();
    gte_stopz(&VS_SCRATCHPAD->opz);
}
