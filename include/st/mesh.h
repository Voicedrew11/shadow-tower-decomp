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
    u_long *prim_end;    /* 0x14: end of the primitive buffer */
    u8 unk18[4];         /* 0x18 */
    void *faces;         /* 0x1c: face cursor the renderers hand back */
    MeshHeader *header;  /* 0x20 */
    u8 *normals;         /* 0x24 */
    u8 unk28[0x18];      /* 0x28 */
    u8 *vertices;        /* 0x40 */
    u8 unk44[4];         /* 0x44 */
    u8 *local_vertices;  /* 0x48: SVECTOR model-space vertices, same indices */
    u8 unk4c[0x14];      /* 0x4c */
    CVECTOR rgb;         /* 0x60: NCDS input colour; .cd carries the GPU code */
    u8 unk64[0xc];       /* 0x64 */
    long opz;            /* 0x70: NCLIP result */
    long opz2;           /* 0x74: NCLIP of a quad's second triangle */
    u8 unk78[8];         /* 0x78 */
    int near_z;          /* 0x80: faces at or before this OT depth are culled */
    u8 unk84[8];         /* 0x84 */
    int face_count;      /* 0x8c: faces submitted */
    int prim_count;      /* 0x90: primitives emitted */
    u8 unk94[4];         /* 0x94 */
    u_short clut_base;   /* 0x98: added to subdivided faces' CLUT */
} MeshScratch;

#define MESH_SCRATCH ((MeshScratch *)0x1f800000)

/* Links a primitive of `len` words into an OT entry, writing both words
 * whole. The halves are also used apart, with a GTE command between them to
 * cover its input delay. */
#define mesh_set_tag(ot, p, len) (*(u_long *)(p) = (*(ot) & 0xffffff) | ((len) << 24))
#define mesh_set_ot(ot, p) (*(ot) = (u_long)(p) & 0xffffff)
#define mesh_add_prim(ot, p, len) (mesh_set_tag(ot, p, len), mesh_set_ot(ot, p))

/* Unresolved ST.EXE global at 0x8015d090, a large .bss object of which the
 * renderers and func_8001c60c read only fields near its end (WIP view). */
typedef struct {
    u8 unk0[0x1edf8];
    int tint_level;      /* 0x1edf8 */
    u8 render_mode;      /* 0x1edfc */
    u8 unk1edfd[3];
    u_long unk1ee00;     /* 0x1ee00: func_8001c60c's near range (<< 11) */
    u_long unk1ee04;     /* 0x1ee04: its middle range (<< 11) */
    u8 *unk1ee08;        /* 0x1ee08: halfword at +0x6a read */
    u8 unk1ee0c[9];
    u8 unk1ee15;         /* 0x1ee15 */
    u8 unk1ee16[2];
} Dat8015d090;

extern Dat8015d090 DAT_8015d090;

/* Render-mode byte bits: a tint (and its callback), and its strength scale. */
#define MESH_MODE (DAT_8015d090.render_mode)
#define MESH_TINT_A 2
#define MESH_TINT_B 4
#define MESH_TINT_SLOW 0x10
#define MESH_TINT_LEVEL (DAT_8015d090.tint_level)

/* Render-mode bit: light with NCCS (no depth cueing). */
#define MESH_NO_DEPTH_CUE 0x10

/* RCpoly* work area; every renderer reserves room for the larger kind. */
typedef union {
    DIVPOLYGON3 d3;
    DIVPOLYGON4 d4;
} MeshDiv;

#endif
