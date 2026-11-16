/*
 * ST.EXE mesh renderers without depth cueing (WIP module): NCCS-lit
 * variants of the st.mesh_draw renderers.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <INLINE_C.H>
#include <st/mesh.h>

ADDRESS(0x800194c4, 0x1fc)
void func_800194c4(int count, MeshFaceFT3 *face, u_long *ot, int flags)
{
    MeshScratch *sp = MESH_SCRATCH;
    u_long *start;
    MeshVertex *v0;
    MeshVertex *v1;
    MeshVertex *v2;
    POLY_FT3 *p;
    MeshFaceFT3 *next;
    int t;
    short z;

    start = sp->prim;
    sp->face_count += count;
    while ((t = count, count = count - 1, t)) {
        v0 = (MeshVertex *)(sp->vertices + face->v0);
        v1 = (MeshVertex *)(sp->vertices + face->v1);
        v2 = (MeshVertex *)(sp->vertices + face->v2);
        gte_ldsxy3(v0->sxy, v1->sxy, v2->sxy);
        next = face + 1;
        gte_nclip();
        sp->rgb.cd = face->code | ((flags & 2) | 0x24);
        gte_stopz(&sp->opz);
        if (sp->opz > 0) {
            z = ((v0->z + v1->z + v2->z) >> 5) / 3 + face->depth_bias;
            if (z < sp->far_z && z > sp->near_z) {
                p = (POLY_FT3 *)sp->prim;
                sp->prim = (u_long *)(p + 1);
                gte_stsxy3_ft3(p);
                gte_ldv0(sp->normals + face->normal);
                gte_ldrgb(&sp->rgb);
                *(u_long *)&p->u0 = face->uv0_clut;
                gte_nccs_b();
                *(u_long *)&p->u1 = face->uv1_tpage | (flags & 0x600000);
                *(u_short *)&p->u2 = face->uv2;
                mesh_add_prim(ot + z, p, 7);
                gte_strgb(&p->r0);
            }
        }
        face = next;
    }
    sp->faces = face;
    sp->prim_count += (sp->prim - start) / 8;
}
