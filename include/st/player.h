/*
 * ST.EXE player items and magic (src/st/player.c). Address-derived names
 * stay until behaviour is proven.
 */
#ifndef ST_PLAYER_H
#define ST_PLAYER_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* 0x8018d298: 24-byte magic records indexed by magic id. */
typedef struct {
    u8 unk0[2];
    u16 unk2;            /* 0x02: cost, taken from viewer.unk6 */
    u8 unk4[0x14];
} StMagic;

extern StMagic DAT_8018d298[];

/* src/st/player.c */
void func_80040024(u8 hand);
void func_80040f04(u8 magic, u8 hand);
void func_80042620(u8 a, u8 b);

/* Band I (0x8003bd14..0x80040024) and later bands; WIP signatures. */
void func_8003d030(void);
void func_8003f670(int arg0, int arg1);
void func_8003f910(u8 magic, u8 hand);
void func_8003f4ec(int arg0, int arg1);
void func_80055374(long *pos, SVECTOR *rot);
void func_8003d104(int arg0, u16 *slot);
int func_8004a194(int a, int b);

#endif
