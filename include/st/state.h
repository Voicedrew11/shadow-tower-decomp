/*
 * ST.EXE's CD state block at 0x80088200 (bss): the CD read-request ring,
 * the root-counter event and tick words, and the three archive tables.
 * Field names stay address-derived until their behaviour is proven.
 */
#ifndef ST_STATE_H
#define ST_STATE_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>

#define ST_CD_REQUESTS 48
#define ST_ARCHIVES 3

/* One 0x34-byte entry of the CD read-request ring. */
typedef struct {
    volatile u8 unk0; /* band A: switch reads keep a separate zero-extension */
    volatile u8 unk1;
    u8 unk2[8];
    s16 unkA;
    s16 unkC;
    s16 unkE;
    u8 unk10;
    u8 unk11[3];
    CdlLOC unk14; /* band A: copied from unk18 as a read starts */
    CdlLOC unk18;
    u8 *unk1C; /* band A: CdGetSector destination, advanced 2048 per sector */
    u8 *unk20; /* band A: copied to unk1C as a read starts */
    s32 unk24;
    s32 unk28;
    void (*unk2C)(); /* band A: completion callback, called unprototyped */
    s32 unk30;
} StCdRequest;

/* A file's CD position and byte size (CdlFILE's first two fields). */
typedef struct {
    CdlLOC pos;
    u_long size;
} StFileExtent;

/*
 * One 12-byte archive table: a start-sector table (one u16 per member plus
 * the end sector) and the archive file's extent.
 */
typedef struct {
    u16 *sectors;
    StFileExtent file;
} StArchive;

typedef struct {
    s32 unk0;
    u32 unk4;
    u32 unk8;
    s32 unkC;
    s32 unk10;
    StCdRequest unk14[ST_CD_REQUESTS];
    StCdRequest *unk9D4;
    StCdRequest *unk9D8;
    StArchive archives[ST_ARCHIVES];
} State80088200;

extern State80088200 DAT_80088200;


/* One animated-texture slot (see src/st/texanim.c). */
typedef struct {
    u8 type;
    u8 period;
    u8 countdown;
    u8 step;
    s16 area;
    s16 phase;
    RECT dst;
    RECT src;
} StTexAnim;

#define ST_TEXANIM_SLOTS 16

/*
 * A 0x6c-byte lighting environment: a light colour matrix (the first 18
 * bytes of a MATRIX, which gte_SetColorMatrix loads), the back colour and
 * two words copied to the scratchpad.
 */
typedef struct {
    u8 unk0[0x50];
    short color[3][3];
    u8 unk62[2];
    u8 back[3];
    u8 unk67;
    s16 unk68;
    s16 unk6A;
} StLight;

/* One packet buffer: its start, its end and the next free byte. */
typedef struct {
    u_long *start;
    u_long *end;
    u_long *cur;
} StPacketBuffer;

/*
 * The graphics state at 0x8015d090: double-buffered drawing and display
 * environments, ordering tables and packet buffers, the animated-texture
 * slots and the camera. Unmodeled ranges stay opaque.
 */
typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
    u_long *unkE0;
    u_long ot[2][4096];
    u_long *ot_cur;
    u_long ot2[2][64];
    u_long *ot2_cur;
    StPacketBuffer packets[2];
    StPacketBuffer *packet;
    void *unk8308[4];
    void *unk8318;
    s32 unk831C[4];
    s32 unk832C;
    s32 unk8330;
    s32 unk8334;
    s32 unk8338[5];
    u8 unk834C[0x1eb40 - 0x834c];
    StTexAnim texanim[ST_TEXANIM_SLOTS];
    u8 unk1ECC0;
    u8 unk1ECC1;
    u8 id;
    u8 unk1ECC3;
    u8 unk1ECC4;
    u8 unk1ECC5[8];
    u8 unk1ECCD;
    u8 unk1ECCE;
    u8 unk1ECCF;
    u8 unk1ECD0[2];
    u8 unk1ECD2;
    u8 unk1ECD3[3];
    u8 unk1ECD6[2];
    u8 unk1ECD8[2];
    s16 unk1ECDA[2];
    s16 unk1ECDE[2];
    s16 unk1ECE2[2];
    u8 unk1ECE6[2];
    MATRIX unk1ECE8;
    u8 unk1ED08[0x20];
    MATRIX unk1ED28;
    u8 unk1ED48[0x60];
    MATRIX unk1EDA8;
    VECTOR unk1EDC8;
    SVECTOR unk1EDD8;
    u8 unk1EDE0[0x10];
    s32 unk1EDF0;
    s32 unk1EDF4;
    u8 unk1EDF8[4];
    u8 unk1EDFC;
    u8 unk1EDFD;
    u8 unk1EDFE;
    u8 unk1EDFF[9];
    StLight *unk1EE08;
    u8 unk1EE0C[9];
    u8 unk1EE15;
    u8 unk1EE16;
} Dat8015d090;

extern Dat8015d090 DAT_8015d090;

/* The scratchpad (fixed at 0x1f800000); only the words band B uses. */
typedef struct {
    u8 unk0[0x2c];
    u_long *unk2C;
    u_long *unk30;
    u_long *unk34;
    u8 unk38[8];
    u_char *unk40;
    u8 unk44[0x20];
    s32 unk64;
    s32 unk68;
    u8 unk6C[4];
    long unk70;
    u8 unk74[0x6c];
    s32 unkE0;
    s32 unkE4;
    s32 unkE8;
    u8 unkEC[0x2a8];
    StLight *unk394;
} ScratchPad;

#define SCRATCHPAD ((ScratchPad *)0x1f800000)

/* The bss object at 0x80198f28: a position and two angles at +0x248. */
typedef struct {
    u8 unk0[0x248];
    s32 unk248;
    s32 unk24C;
    s32 unk250;
    u8 unk254[0x14];
    s16 unk268;
    s16 unk26A;
} Dat80198f28;

extern Dat80198f28 DAT_80198f28;

/* The two 0x20000-byte GPU packet buffers. */
extern u_long DAT_8011c488[];
/* Word cleared by func_80016fb8. */
extern s32 DAT_8015c488;
/* The CLUT a type-49 texture slot cycles (entries 1-14 rotate). */
extern u16 DAT_8009ac88[16];
/* The current area, compared with each texture slot's area. */
extern u8 DAT_801b5929;
/* The three lighting blend targets of func_80017d28. */
extern StLight DAT_8018ca38[3];

#endif
