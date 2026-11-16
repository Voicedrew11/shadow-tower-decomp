#ifndef ST_MESH_H
#define ST_MESH_H

/*
 * ST.EXE mesh renderer state (band 0x800184c4..0x8001d224, WIP).
 *
 * The renderers keep their working state in the scratchpad at 0x1f800000:
 * every access is an absolute `lui $s0, 0x1f80` base with no relocation, so
 * the source spells the fixed address.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* A transformed vertex: its screen XY word, then the depth and depth-cue
 * values the face loops average. Faces index vertices by byte offset. */
typedef struct {
    u_long sxy;
    short z;
    short p;
} MeshVertex;

/*
 * Face records. Vertex and normal fields are byte offsets into the
 * scratchpad's vertex and normal arrays; uv/clut/tpage words are copied
 * into the primitive, and `code` is ORed into the GPU code byte.
 */
typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short normal;
    u_short v0, v1, v2;
} MeshFaceFT3;

typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short uv3;
    u_short unk0e;
    u_short normal;
    u_short v0, v1, v2, v3;
} MeshFaceFT4;

typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short n0, v0;
    u_short n1, v1;
    u_short n2, v2;
} MeshFaceGT3;

typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short uv3;
    u_short unk0e;
    u_short n0, v0;
    u_short n1, v1;
    u_short n2, v2;
    u_short n3, v3;
} MeshFaceGT4;

/* A model part's header: offsets of its normal and face arrays (from the
 * model base + 12) and its face counts per primitive kind. */
typedef struct {
    u8 unk00[8];
    u_long normals;      /* 0x08 */
    u8 unk0c[4];
    u_long faces;        /* 0x10 */
    u8 unk14[4];
    u_short n_ft3;       /* 0x18 */
    u_short n_gt3;       /* 0x1a */
    u_short n_ft4;       /* 0x1c */
    u_short n_gt4;       /* 0x1e */
    u8 unk20[8];
} MeshHeader;

typedef struct {
    u_long *ot;          /* 0x00 */
    int far_z;           /* 0x04: faces at or beyond this OT depth are culled */
    u8 unk8[4];          /* 0x08 */
    u8 *model;           /* 0x0c */
    u_long *prim;        /* 0x10: next free primitive word */
    u8 unk14[8];         /* 0x14 */
    void *faces;         /* 0x1c: face cursor the renderers hand back */
    MeshHeader *header;  /* 0x20 */
    u8 *normals;         /* 0x24 */
    u8 unk28[0x18];      /* 0x28 */
    u8 *vertices;        /* 0x40 */
    u8 unk44[0x1c];      /* 0x44 */
    CVECTOR rgb;         /* 0x60: NCDS input colour; .cd carries the GPU code */
    u8 unk64[0xc];       /* 0x64 */
    long opz;            /* 0x70: NCLIP result */
    u8 unk74[0xc];       /* 0x74 */
    int near_z;          /* 0x80: faces at or before this OT depth are culled */
    u8 unk84[8];         /* 0x84 */
    int face_count;      /* 0x8c: faces submitted */
    int prim_count;      /* 0x90: primitives emitted */
} MeshScratch;

#define MESH_SCRATCH ((MeshScratch *)0x1f800000)

/* Links a primitive of `len` words into an OT entry, writing both words whole. */
#define mesh_add_prim(ot, p, len)                                              \
    (*(u_long *)(p) = (*(ot) & 0xffffff) | ((len) << 24),                      \
     *(ot) = (u_long)(p) & 0xffffff)

/* Unresolved ST.EXE global at 0x8015d090, a large .bss object of which the
 * renderers read only a word and the render-mode byte at its end (WIP view). */
typedef struct {
    u8 unk0[0x1edf8];
    int tint_level;      /* 0x1edf8 */
    u8 render_mode;      /* 0x1edfc */
} Dat8015d090;

extern Dat8015d090 DAT_8015d090;

#endif
