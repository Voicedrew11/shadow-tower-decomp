/*
 * VS.EXE: a loaded model file's primitive fix-up (WIP module): every
 * part's tagged face records get their vertex and normal indices turned
 * into byte offsets (8 bytes an entry).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/mesh.h>
#include <vs/mesh.h>

/* The module's switch table. */
JUMPTABLES(0x80011340, 0x74)

/*
 * Walks each part's face list: a tag word (GPU code in the top byte, the
 * record's length in words in the second) before each record. The code's
 * semi-transparency bit is ignored for the switch.
 */
ADDRESS(0x8002345c, 0x308)
void func_8002345c(ModelFile *file)
{
    VsModelPart *part = (VsModelPart *)((u8 *)file + 12);
    int n = file->unk8;
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
