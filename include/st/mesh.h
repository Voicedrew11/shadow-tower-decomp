#ifndef ST_MESH_H
#define ST_MESH_H

/*
 * ST.EXE mesh renderer state (band 0x800184c4..0x8001d224, WIP).
 *
 * The renderers keep their working state in the scratchpad at 0x1f800000:
 * every access is an absolute `lui $s0, 0x1f80` base with no relocation, so
 * the source spells the fixed address (ScratchPad, st/scratch.h, which
 * includes this header for MeshHeader).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/gfx.h>

/* A transformed vertex: its screen XY, then the depth and depth-cue values
 * the face loops average. Faces index vertices by byte offset. */
typedef struct {
    DVECTOR sxy;
    short z;
    short p;
} MeshVertex;

/*
 * Face records. Vertex (i*) and normal (n*) fields are byte offsets into the
 * scratchpad's vertex and normal arrays; the texture words are copied into
 * the primitive (uv0 + CLUT, uv1 + tpage, uv2, uv3), and `code` is ORed into
 * the GPU code byte. The FT4 record's 28-byte stride shows the words are
 * word-aligned members.
 */
typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short normal;
    u_short i0, i1, i2;
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
    u_short i0, i1, i2, i3;
} MeshFaceFT4;

typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short n0, i0;
    u_short n1, i1;
    u_short n2, i2;
} MeshFaceGT3;

typedef struct {
    u_long uv0_clut;
    u_long uv1_tpage;
    u_short uv2;
    s8 depth_bias;
    u8 code;
    u_short uv3;
    u_short unk0e;
    u_short n0, i0;
    u_short n1, i1;
    u_short n2, i2;
    u_short n3, i3;
} MeshFaceGT4;

/* The high half of a face texture word: its CLUT or tpage. */
#define MESH_WORD_HI(word) (((u_short *)&(word))[1])

/* A model part's header: offsets of its normal and face arrays (from the
 * model base + 12) and its face counts per primitive kind. src/st/prim.c's
 * ModelPart views the same 40-byte records from 12 bytes later (its
 * retail code addresses them that way). */
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

/* Links a primitive of `len` words into an OT entry, writing both words
 * whole. The halves are also used apart, with a GTE command between them to
 * cover its input delay. */
#define mesh_set_tag(ot, p, len) (*(u_long *)(p) = (*(ot) & 0xffffff) | ((len) << 24))
#define mesh_set_ot(ot, p) (*(ot) = (u_long)(p) & 0xffffff)
#define mesh_add_prim(ot, p, len) (mesh_set_tag(ot, p, len), mesh_set_ot(ot, p))

/* Render-mode byte bits: a tint (and its callback), and its strength scale. */
#define MESH_MODE (world_state.render_mode)
#define MESH_TINT_A 2
#define MESH_TINT_B 4
#define MESH_TINT_SLOW 0x10
#define MESH_TINT_LEVEL (world_state.tint_level)

/* Render-mode bit: light with NCCS (no depth cueing). */
#define MESH_NO_DEPTH_CUE 0x10

/* RCpoly* work area; every renderer reserves room for the larger kind. */
typedef union {
    DIVPOLYGON3 d3;
    DIVPOLYGON4 d4;
} MeshDiv;

/* Tint callbacks (each renderer has its own pair; the FT3 pair is in src/st/prim.c). */
void mesh_ft3_tint_a(CVECTOR *rgb);
void mesh_ft3_tint_b(CVECTOR *rgb);

void mesh_draw_ft3(int count, MeshFaceFT3 *face, u_long *ot, int flags);
void mesh_draw_ft4(int count, MeshFaceFT4 *face, u_long *ot, int flags);
void mesh_draw_gt3(int count, MeshFaceGT3 *face, u_long *ot, int flags);
void mesh_draw_gt4(int count, MeshFaceGT4 *face, u_long *ot, int flags);
void mesh_draw_part(u_short part, u_int flags);
void mesh_draw_ft3_nccs(int count, MeshFaceFT3 *face, u_long *ot, int flags);
void mesh_draw_ft4_nccs(int count, MeshFaceFT4 *face, u_long *ot, int flags);
void mesh_draw_gt3_nccs(int count, MeshFaceGT3 *face, u_long *ot, int flags);
void mesh_draw_gt4_nccs(int count, MeshFaceGT4 *face, u_long *ot, int flags);
void mesh_draw_part_nccs(u_short part, u_int flags);
void mesh_div_draw_ft3(int count, MeshFaceFT3 *face, u_long *ot, int flags);
void mesh_div_draw_ft4(int count, MeshFaceFT4 *face, u_long *ot, int flags);
void mesh_div_draw_gt3(int count, MeshFaceGT3 *face, u_long *ot, int flags);
void mesh_div_draw_gt4(int count, MeshFaceGT4 *face, u_long *ot, int flags);
void mesh_div_draw_part(u_short part, u_int flags);

#endif
