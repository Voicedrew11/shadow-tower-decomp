/*
 * ST.EXE menu screens (src/st/menu.c, WIP): the status, list, stats, map
 * and option pages drawn while the game is paused.
 */
#ifndef ST_MENU_H
#define ST_MENU_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/*
 * A message bank: 0x600 byte offsets, then the text they index from +0xc00
 * (two-byte glyph codes, as func_8002732c draws them).
 */
typedef struct {
    u16 offsets[0x600];
    u8 text[1];          /* variable length */
} StMessages;

#define MESSAGE(bank, i) ((u16 *)((bank)->text + (bank)->offsets[i]))

/* The resident message bank. */
extern StMessages DAT_801e0d50;

/*
 * The menus reuse the start of the scratchpad as their work area; each page
 * lays it out its own way (st/scratch.h names the fields other phases use).
 */
typedef union {
    DVECTOR icons[6];        /* list page icon texture positions, used from [1] */
    struct {
        StMessages *messages; /* 0x00: the list's message bank */
        u8 unk04[0x14];
        s16 count;           /* 0x18: entries in the list */
        u8 unk1A[2];
        s16 *entries;        /* 0x1c: entry ids */
    } list;
    struct {
        u8 unk00[0x20];
        s16 value;           /* 0x20 */
        s16 selected;        /* 0x22 */
        s16 base[2][7];      /* 0x24: left and right column base values */
        s16 bonus[2][7];     /* 0x40: and their bonuses */
    } stats;
    struct {
        s32 unk00;
        s32 current;         /* 0x04: the entry marked current, -1 for none */
        s32 ids[10];         /* 0x08 */
    } map;
} StMenuWork;

#define MENU_WORK ((StMenuWork *)0x1f800000)
/* The work area seen through a ScratchPad pointer. */
#define MENU(sp) ((StMenuWork *)(sp))

/* Per-area counts read by the map page (byte +0x16e4 onwards); WIP extent. */
typedef struct {
    u8 unk0[0x14a8];
    u32 unk14A8[0x8f];   /* 0x14a8: one bit per map id (func_800241d0) */
    u8 unk16E4[44];      /* by area */
} Data801cba08;
extern Data801cba08 DAT_801cba08;

/* .sbss words the map page reads with lui; their owner is unknown. */
extern s32 DAT_8008818c;
extern u16 DAT_800881a8;

/* Option page state bytes; owner unknown. */
extern u8 DAT_801cd121;
extern u8 DAT_801cd122[]; /* four bytes; 0x801cd123 and 0x801cd124 have their own identities */

void func_80029b74(void);
void func_8002a1d0(void);
void func_8002a5a4(void);
void func_8002a954(void);
void func_8002b2e8(void);

#endif
