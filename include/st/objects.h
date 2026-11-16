/*
 * ST.EXE world object passes (src/st/objects.c): each frame they walk the
 * visible entries of the three object tables (st/vis.h) and two free-standing
 * object arrays, flag the model slots they need, and draw the models through
 * func_80022678.
 */
#ifndef ST_OBJECTS_H
#define ST_OBJECTS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* 0x8018e318: 128 records of 88 bytes; flags byte 0xff marks a free one. */
typedef struct {
    u8 unk0;             /* 0x00: 0xff free */
    u8 unk1;
    u8 unk2;             /* 0x02 */
    u8 unk3[3];
    u8 unk6;             /* 0x06: bits 0-1 cull mode, bits 2-3 kind */
    u8 unk7;             /* 0x07 */
    u8 unk8[2];
    u16 unkA;            /* 0x0a: model id - 96 */
    u8 unkC[2];
    u16 unkE;            /* 0x0e: CLUT base */
    u8 unk10[2];
    s16 unk12;           /* 0x12 */
    u16 unk14;           /* 0x14 */
    u8 unk16[0xa];
    VECTOR unk20;        /* 0x20: position */
    SVECTOR unk30;       /* 0x30: rotation */
    SVECTOR unk38;       /* 0x38: scale */
    u8 unk40[8];
    u8 unk48[0x10];      /* 0x48 */
} StObject88;

extern StObject88 DAT_8018e318[128];
/* An identity MATRIX in .data. */
extern MATRIX DAT_8008040c;
/* The SpuGetAllKeysStatus buffer. */
extern char DAT_801ddbf8[24];

/* src/st/objects.c */
void func_800241d0(void);
void func_800246e4(void);
void func_800251c8(void);
void func_800255f0(void);

/* src/st/slots.c */
void func_80023bc4(int archive, int index, int slot);
void func_80023c6c(int archive, int first, int slot, int count);
void func_80023ed8(int archive, int first, int slot, int count);
void func_80024004(int archive, int first, int slot, int count);

#endif
