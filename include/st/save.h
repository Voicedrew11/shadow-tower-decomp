/*
 * ST.EXE progress state: new-game defaults, the per-area snapshots of the
 * entry tables and the save-data image (src/st/save.c). Field names stay
 * address-derived.
 */
#ifndef ST_SAVE_H
#define ST_SAVE_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* One table C slot (entries[32..63]) as saved. */
typedef struct {
    u16 unk00;
    u16 unk02;
    s32 unk04[3];        /* 0x04: position */
    u16 unk10[3];        /* 0x10: rotation */
    u8 unk16;
    u8 unk17;
} StSaveSlot;

/* Five bytes copied as one block (area_state.loaded_area..unk8). */
typedef struct {
    u8 unk[5];
} StBytes5;

/* area_state as the save code addresses it: bytes 4..8 as one block. */
typedef struct {
    u8 unk0[4];
    StBytes5 unk4;
} StAreaBytes;

/* Fourteen bytes copied as one halfword-aligned block. */
typedef struct {
    u16 unk[7];
} StHalves7;

/* The save-data image save_pack_data fills and save_unpack_data reads. */
typedef struct {
    u32 unk0;            /* 0x0000: game_progress.unk16E0 */
    u8 unk4[5];          /* 0x0004: area_state.loaded_area..unk8 */
    u8 unk9[3];
    u32 unkC[0x60];      /* 0x000c: game_progress.unk0 */
    u32 unk18C[0x30];
    u32 unk24C[0x180];
    u32 unk84C[0x180];
    u32 unkE4C[0x180];
    u32 unk144C[2];
    u32 unk1454[12];
    u32 unk1484[12];
    u32 unk14B4[5];
    u8 unk14C8[4];       /* 0x14c8: vis_table_c.ground_item_counts[2..5] */
    u32 unk14CC[9];      /* 0x14cc: vis_table_c.collected_bits */
    StSaveSlot unk14F0[32]; /* 0x14f0: table C entries 32..63 */
    u32 unk17F0;         /* 0x17f0: viewer fields */
    s32 unk17F4;
    u16 unk17F8;
    u16 unk17FA;
    u16 unk17FC;
    u16 unk17FE;
    u8 unk1800;
    u8 unk1801;
    u8 unk1802;
    u8 unk1803;
    s16 unk1804[14];
    s16 unk1820[14];
    u16 unk183C[14];
    VECTOR unk1858;      /* 0x1858: viewer position */
    SVECTOR unk1868;     /* 0x1868: viewer rotation (viewer.view_rot) */
    u32 unk1870[3];
    s16 unk187C[13][2];
    u8 unk18B0[3];
    u8 unk18B3;
    u8 unk18B4[6];       /* 0x18b4: settings (game_settings) */
    u8 unk18BA[2];
    u32 unk18BC[5];
    u32 unk18D0[0x4f];
    u32 unk1A0C[0x4f];   /* 0x1a0c: inventory */
    u32 unk1B48[10];
    u16 unk1B70[21];
    u8 unk1B9A[2];
    u32 unk1B9C[3];
    u8 unk1BA8[314];     /* 0x1ba8: each ItemType's durability */
    u8 unk1CE2[2];
    u32 unk1CE4[24];     /* 0x1ce4: lighting.area_mode */
    u16 unk1D44[14];
    u8 unk1D60[14];
    u16 unk1D6E[12];     /* 0x1d6e: the button masks (pad_state.buttons) */
} StSaveData;

void item_counts_clear(void);
void save_reset_new_game(void);
void area_save_snapshot(int area);
void area_restore_snapshot(int area);
void save_pack_data(StSaveData *save, int fresh);
void save_unpack_data(StSaveData *save);
void area_run_frame_hook(void);

#endif
