/*
 * ST.EXE placement records and TIM upload (src/st/tim.c).
 */
#ifndef ST_TIM_H
#define ST_TIM_H

#include <kf/lib/types.h>
#include <sys/types.h>

/* One 16-byte placement record as placements_expand reads it. */
typedef struct {
    u16 id;              /* 0xffff: empty */
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 bx;               /* 0x05: coordinate blocks of 4096 */
    u8 by;
    u8 bz;
    u8 unk8;
    u8 unk9;
    s16 x;               /* 0x0a: offsets inside the blocks */
    s16 y;
    s16 z;
} StPlacement;

/* One 24-byte expanded placement (the table at 0x8015c48c). */
typedef struct {
    u16 id;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;             /* 0x05: random, below unk3 */
    u8 unk6[2];
    s32 x;               /* 0x08 */
    s32 y;
    s32 z;
    u8 unk14[4];
} StPlaced;

extern StPlaced DAT_8015c48c[128];

/* A TIM file header: its id word and its flags (8: a CLUT block follows). */
typedef struct {
    u_long id;
    u_long flag;
} TimHeader;

/* A TIM CLUT or pixel block header: its byte count (header included) and
 * its VRAM rectangle, whose size is read unsigned. */
typedef struct {
    u_long bnum;
    u_short x;
    u_short y;
    u_short w;
    u_short h;
} TimBlock;

void placements_expand(StPlacement *src);
int tim_load_images(u_long *buf, u_long size);

#endif
