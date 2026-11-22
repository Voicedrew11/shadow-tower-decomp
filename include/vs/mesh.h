/*
 * VS.EXE's untextured mesh faces. VS's renderers (src/vs/mesh_*.c) draw
 * flat and Gouraud faces besides ST.EXE's textured ones (st/mesh.h): a face
 * carries its colour word (the GPU code in .cd) and byte offsets into the
 * scratchpad's normal (n*) and vertex (i*) arrays.
 */
#ifndef VS_MESH_H
#define VS_MESH_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

typedef struct {
    CVECTOR rgb;
    u_short normal;
    u_short i0, i1, i2;
} VsFaceF3;

typedef struct {
    CVECTOR rgb;
    u_short normal;
    u_short i0, i1, i2, i3;
    u_short unk0e;
} VsFaceF4;

typedef struct {
    CVECTOR rgb;
    u_short n0, i0;
    u_short n1, i1;
    u_short n2, i2;
} VsFaceG3;

typedef struct {
    CVECTOR rgb;
    u_short n0, i0;
    u_short n1, i1;
    u_short n2, i2;
    u_short n3, i3;
} VsFaceG4;

/* VS band 6: a model part's face list as model_fixup_face_indices walks it (the
 * part headers are 40 bytes, as MeshHeader): the list's offset from the
 * parts and its record count. */
typedef struct {
    u8 unk0[0x10];
    u_long faces;        /* 0x10 */
    u_long n_faces;      /* 0x14 */
    u8 unk18[0x10];
} VsModelPart;

typedef char VsModelPartSize[sizeof(VsModelPart) == 0x28 ? 1 : -1];

/* The tag word before each face record: the record's length in words
 * and its GPU code. */
typedef struct {
    u_long unk0 : 8;
    u_long len : 8;
    u_long unk16 : 8;
    u_long code : 8;
} VsFaceTag;

#endif
