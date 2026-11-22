/*
 * VS.EXE's memory card screens: the card state they keep at the head of
 * the menu scratchpad block (vs/menu.h), the card file header and the two
 * players' save records. ST.EXE links the same routines (unreconstructed
 * there). Only proven fields are spelled.
 */
#ifndef VS_CARD_H
#define VS_CARD_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>

/* One save file listed on a card: its name and three header values. */
typedef struct {
    char name[21];       /* 0x00: the directory entry's name */
    u8 unk15;            /* 0x15: header byte 0x3d */
    u16 unk16;           /* 0x16: header halfword 0x3e */
    u16 unk18;           /* 0x18: header halfword 0x40 */
    u8 unk1A[2];
} VsCardFile;

/* One card slot: up to 7 listed save files and the scan results. */
typedef struct {
    VsCardFile files[7]; /* 0x00 */
    u16 status;          /* 0xc4: 0 ready, 1 new card, 2 error, 3, 4 no file, 5 full */
    s16 count;           /* 0xc6: save files found */
    s16 unkC8;           /* 0xc8: set while the card has room */
} VsCardPort;

/* The card screens' state at the head of the menu scratchpad block. */
typedef struct {
    u8 unk0[4];
    long sw_events[4];   /* 0x04: SwCARD IOE, ERROR, TIMOUT, NEW */
    long hw_events[4];   /* 0x14: HwCARD IOE, ERROR, TIMOUT, NEW */
    VsCardPort ports[2]; /* 0x24 */
} VsCardState;

/* Layout checks: the offsets and strides the code proves. */
typedef char VsCardPortSize[sizeof(VsCardPort) == 0xca ? 1 : -1];
typedef char VsCardStateSize[sizeof(VsCardState) == 0x1b8 ? 1 : -1];

/* The 512-byte card file header; only the values the code reads. */
typedef struct {
    u8 unk0[0x34];
    u8 unk34[6];         /* 0x34: the title's three SJIS digits (match_award_requested_item) */
    u8 unk3A[0x3d - 0x3a];
    u8 unk3D;
    u16 unk3E;
    u16 unk40;
    u8 unk42;            /* 0x42: card_write_blocks's sum of the save record */
    u8 unk43[0x200 - 0x43];
} VsCardHeader;

/* Inventory portion of a player's save record; only proven fields. */
/* Per-item marks item_request_any_available compares between the two saves (VS band 6):
 * item ids 0..265 index the byte arrays. */
typedef struct {
    u32 unk0[5];         /* 0x000: one bit per fighter id owned (menu_creature_select_run) */
    u8 unk14[266];       /* 0x014: a second mark, needed when the item's flag bit is set */
    u8 unk11E[0x150 - 0x11e];
    u8 unk150[266];      /* 0x150: the item is held */
    u8 unk25A[0x2b4 - 0x25a];
    u16 unk2B4[21];      /* 0x2b4: listed item ids */
    u8 unk2DE[0x2e0 - 0x2de];
    s32 unk2E0[3];       /* 0x2e0: the player's team (VsTeam.slot, card_screen_run) */
    u8 unk2EC[314];      /* 0x2ec: per-item durability, shared with ST's save format */
} VsSaveSub;

typedef char VsSaveSubSize[sizeof(VsSaveSub) == 0x428 ? 1 : -1];

typedef struct {
    s32 unk0;            /* 0x0000: compared with the card file's first word */
    u8 unk4[0x14cc - 4];
    u32 unk14CC[9];      /* 0x14cc: 266 flag bits, counted into the header */
    u8 unk14F0[0x1802 - 0x14f0];
    u8 unk1802;          /* 0x1802: the count of items won (match_award_requested_item) */
    u8 unk1803[0x18bc - 0x1803];
    VsSaveSub unk18BC;   /* 0x18bc (VS band 6) */
    u8 unk1CE4[0x1d88 - 0x1ce4]; /* remaining save payload */
} VsSave;

typedef char VsSaveSize[sizeof(VsSave) == 0x1d88 ? 1 : -1];
typedef char VsSaveUnk18BCAt0x18BC[offsetof(VsSave, unk18BC) == 0x18bc ? 1 : -1];
typedef char VsSaveSubCountsAt0x2EC[offsetof(VsSaveSub, unk2EC) == 0x2ec ? 1 : -1];
typedef char VsSaveCountsAt0x1BA8[offsetof(VsSave, unk18BC) + offsetof(VsSaveSub, unk2EC) == 0x1ba8 ? 1 : -1];
typedef char VsSaveSuffixAt0x1CE4[offsetof(VsSave, unk1CE4) == 0x1ce4 ? 1 : -1];
typedef char VsSaveSubCountsEndAt0x426[offsetof(VsSaveSub, unk2EC) + sizeof(((VsSaveSub *)0)->unk2EC) == 0x426 ? 1 : -1];
typedef char VsSaveCountEndAt0x1CE2[offsetof(VsSave, unk18BC) + offsetof(VsSaveSub, unk2EC) + sizeof(((VsSaveSub *)0)->unk2EC) == 0x1ce2 ? 1 : -1];
typedef char VsSaveInventoryBoundary[offsetof(VsSave, unk18BC) + sizeof(VsSaveSub) == offsetof(VsSave, unk1CE4) ? 1 : -1];
typedef char VsSavePairSize[sizeof(VsSave[2]) == 0x3b10 ? 1 : -1];

/* The two players' card file names (0x801a0a30, bss). */
extern char DAT_801a0a30[2][40];
extern VsSave DAT_801a0a80[2];

/* The file list rows the two players' saves were loaded from
 * (card_screen_run; bss). */
extern u16 DAT_80063328[2];

/* The card file headers of the two players' saves (0x8015ffc0, bss). */
extern VsCardHeader DAT_8015ffc0[2];

/* LIBC's itoa (Psy-Q's headers do not declare it): the decimal text of
 * value in a static buffer. */
extern char *itoa(int value);

/* src/vs/menu_80016f10.c */
void card_check_player_save(int port);
int card_write_blocks(int fd, int offset, u8 *buf, int size);

#endif
