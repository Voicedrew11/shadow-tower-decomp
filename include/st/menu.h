/*
 * ST.EXE menu screens (src/st/ui.c): the scratchpad
 * layout they use, the message table they draw from and their functions. The screens share
 * one scratchpad block at 0x1f800000 that differs from the renderer's
 * (st/scratch.h): screen state words at the start, the list fields around
 * +0x1f4, the input and layout fields at +0x2dc and the drawing records
 * from +0x300. Field names stay address-derived until behaviour is proven.
 */
#ifndef ST_MENU_H
#define ST_MENU_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBCD.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/text.h>

/*
 * The message table: byte offsets of each message from text, then the
 * messages, halfword glyph codes ending at 0xffff (0 breaks the line).
 */
typedef struct {
    u16 offsets[0x600];
    u8 text[0x5400 - 0xc00];
} MenuMessages;

typedef char MenuMessagesSize[sizeof(MenuMessages) == 0x5400 ? 1 : -1];

/*
 * The id grid two screens share (func_80031378, func_80031674): two tables
 * of four ids per row, the first row each one shows, and the message table
 * the ids index.
 */
typedef struct {
    s16 row0;            /* 0x00: first row shown of ids0 */
    s16 row1;            /* 0x02: first row shown of ids1 */
    s16 *ids0;           /* 0x04 */
    s16 *ids1;           /* 0x08 */
    s16 count0;          /* 0x0c: ids in ids0 (src/st/idgrid.c, func_8005d25c) */
    s16 count1;          /* 0x0e: ids in ids1 */
    DVECTOR *tex;        /* 0x10: cell images, twenty per block (func_80030be4) */
    MenuMessages *messages; /* 0x14 */
} MenuGrid;

/*
 * The item list the shop and inventory screens share: its message table,
 * the listed ids and their count.
 */
typedef struct {
    MenuMessages *messages; /* 0x00 */
    s16 *ids;            /* 0x04 */
    s16 count;           /* 0x08 */
    u16 unkA;            /* 0x0a */
} MenuList;

/* A 44-byte stored-item kind record func_8003025c details. */
typedef struct {
    u8 kind;             /* 0x00: indexes its name table */
    u8 unk1;             /* 0x01 */
    u16 unk2;            /* 0x02 */
    s16 unk4;            /* 0x04: shown as unk4 * 10 / 3048 and its remainder / 254 */
    s16 unk6;            /* 0x06: shown as unk6 * 100 / 454 with tenths */
    s16 pairs[4][2];     /* 0x08: value, then the optional value before it */
    u32 traits;          /* 0x18: message nibbles at bits 0, 8 and 16 */
    s8 figures[14];      /* 0x1c */
    u8 unk2A[2];
} MenuStoredItem;

/*
 * The stored-item grid (func_8002fe4c): the kind records, the message
 * table, one kind per entry, the entry count, the last row and its length,
 * the VRAM image of each of the fifteen visible cells and the detail page's.
 */
typedef struct {
    MenuStoredItem *items; /* 0x00 */
    MenuMessages *messages; /* 0x04 */
    u8 *kinds;           /* 0x08 */
    s16 count;           /* 0x0c */
    s16 rows;            /* 0x0e */
    s16 last;            /* 0x10: cells on the last row */
    u8 unk12[2];
    DVECTOR tex[15];     /* 0x14 */
    DVECTOR image;       /* 0x50: func_8003025c's image */
} MenuStore;

/* The screen state at the start of the scratchpad, per screen. */
typedef union {
    s32 word;
    u32 *block;          /* func_800637c0's 21504-byte message copy */
    s16 half[2];
    MenuGrid grid;
    MenuList list;
    DVECTOR vram[2];     /* func_8002f1f8's texture and CLUT positions */
    MenuStore store;
    DVECTOR icons[6];    /* the pause list page's icon texture positions, used from [1] */
    struct {
        MenuMessages *messages; /* 0x00: the list's message table */
        DVECTOR icons[5]; /* 0x04: the rows' icon texture positions (MenuState.icons[1..5]) */
        s16 count;       /* 0x18: entries in the list */
        u8 unk1A[2];
        s16 *entries;    /* 0x1c: entry ids */
    } page_list;         /* the pause menu list page (func_8002a1d0) */
    struct {
        u8 unk00[0x20];
        s16 value;       /* 0x20 */
        s16 selected;    /* 0x22 */
        s16 base[2][7];  /* 0x24: left and right column base values */
        s16 bonus[2][7]; /* 0x40: and their bonuses */
    } stats;
    struct {
        s32 unk00;
        s32 current;     /* 0x04: the entry marked current, -1 for none */
        s32 ids[10];     /* 0x08 */
    } map;
    struct {
        s16 flags;       /* 0x00: tested against DAT_801cd180.unk22 */
        u8 level[12];    /* 0x02 */
    } levels;            /* func_8002b84c */
} MenuState;

/*
 * A 32-byte list record func_8002c2b8 lists: one memory card save file
 * (src/st/menu.c): its directory name, the number in its name and bytes
 * from its header.
 */
typedef struct {
    char name[0x14];     /* 0x00: the file name firstfile/nextfile found */
    s32 unk14;           /* 0x14: the two digits at name[17..18] */
    u8 unk18;
    u8 kind;             /* 0x19: indexes the record title table */
    u16 unk1A;           /* 0x1a */
    u16 unk1C;           /* 0x1c */
    u8 unk1E;            /* 0x1e */
    u8 unk1F;
} MenuRecord;

/* One memory card port's directory as the save screens keep it. */
typedef struct {
    MenuRecord files[7]; /* 0x00 */
    u16 status;          /* 0xe0: 0 ready, 1 new card, 2 error, 3 unformatted;
                          * 4/5 are set by func_8006103c */
    s16 count;           /* 0xe2: files found */
    s16 unkE4;           /* 0xe4: 1 when at least two blocks are free */
    u8 unkE6[2];
} MenuCard;

/*
 * The memory card screens' view of the menu scratchpad block's first 0x1f4
 * bytes (src/st/menu.c): the screen state halfwords (MenuState.half), the
 * SwCARD and HwCARD event descriptors (EvSpIOE, EvSpERROR, EvSpTIMOUT,
 * EvSpNEW each) and both ports' directories. Later fields are MenuScratch's.
 */
typedef struct {
    s16 unk0;            /* 0x00: message id */
    s16 unk2;            /* 0x02 */
    long sw_events[4];   /* 0x04 */
    long hw_events[4];   /* 0x14 */
    MenuCard cards[2];   /* 0x24 */
} MenuCardScratch;

#define MENU_CARD_SCRATCH ((MenuCardScratch *)0x1f800000)

typedef struct {
    MenuState unk0;      /* 0x000 */
    u8 unk5C[0x174 - 0x5c]; /* 0x05c */
    s16 *unk174[10];     /* 0x174: the equipment page's id lists, by kind (func_8005ad00) */
    s16 unk19C[10];      /* 0x19c: and their lengths */
    s16 *unk1B0;         /* 0x1b0: the ids the open list shows */
    s16 unk1B4[14];      /* 0x1b4: attribute values */
    u8 unk1D0[0x24];     /* 0x1d0 */
    MenuCard *unk1F4;    /* 0x1f4: the listed records: the current port's directory */
    s16 unk1F8;          /* 0x1f8: the memory card port (src/st/menu.c) */
    s16 unk1FA;          /* 0x1fa: list length */
    s16 unk1FC;          /* 0x1fc */
    s16 unk1FE;          /* 0x1fe: cursor row */
    s16 unk200;          /* 0x200: first visible list entry */
    s16 unk202;          /* 0x202: dialog choice (func_800615d8 returns it) */
    s16 unk204;          /* 0x204: lowest unused save file number (func_8006103c) */
    s16 unk206;          /* 0x206 */
    s32 unk208;          /* 0x208: func_80062c28 hands it to func_800562d4 */
    u8 unk20C[0x50];     /* 0x20c */
    s16 unk25C[14];      /* 0x25c: attribute changes */
    u8 unk278[0x292 - 0x278]; /* 0x278 */
    u16 unk292[21];      /* 0x292: copy of viewer.unk38 (func_8005975c, func_80059cec) */
    s16 unk2BC;          /* 0x2bc */
    s16 unk2BE;          /* 0x2be */
    s16 unk2C0;          /* 0x2c0 */
    u16 unk2C2;          /* 0x2c2: unk1B0's length */
    s16 unk2C4;          /* 0x2c4: attribute bar scale */
    u8 unk2C6;           /* 0x2c6 */
    u8 unk2C7;           /* 0x2c7 */
    u8 unk2C8[4];        /* 0x2c8 */
    MenuMessages *unk2CC; /* 0x2cc */
    u32 *unk2D0;         /* 0x2d0: 36 words of grid cell bits (func_8005ad00) */
    u32 *unk2D4;         /* 0x2d4: one bit per grid cell */
    s16 unk2D8;          /* 0x2d8: viewer.unk2 when the equipment page opened (src/st/equip_menu.c, src/st/menu.c) */
    s16 unk2DA;          /* 0x2da: viewer.unk6 */
    u16 unk2DC;          /* 0x2dc */
    u16 unk2DE;          /* 0x2de: selected row of the item and stats menus */
    s16 unk2E0;          /* 0x2e0 */
    s16 unk2E2;          /* 0x2e2 */
    u8 unk2E4;           /* 0x2e4: cursor blink phase */
    u8 unk2E5;           /* 0x2e5: text font mode (0: 6x12 half-width cells) */
    DVECTOR unk2E6[5];   /* 0x2e6: VRAM positions func_80058dc4 sets (image, CLUT pairs);
                          * [0]/[1] func_80063260's texture and CLUT, [2]/[3] func_800637c0's;
                          * src/st/idgrid.c uploads its images to all five */
    u8 unk2FA[6];        /* 0x2fa */
    CVECTOR style[8];    /* 0x300: text colours; .cd carries the glyph size */
    CVECTOR *unk320;     /* 0x320: figure colours of the status pages */
    CVECTOR *unk324;     /* 0x324 */
    CVECTOR *unk328;     /* 0x328 */
    CVECTOR unk32C;      /* 0x32c: highlight colour */
    RECT unk330;         /* 0x330: highlight bar */
    CVECTOR unk338;      /* 0x338 */
    RECT unk33C;         /* 0x33c */
    CVECTOR unk344;      /* 0x344 */
    RECT unk348;         /* 0x348 */
    RECT unk350;         /* 0x350 */
    DVECTOR unk358;      /* 0x358: texture position in VRAM */
    DVECTOR unk35C;      /* 0x35c: CLUT position in VRAM */
    u16 unk360;          /* 0x360: blink offset */
} MenuScratch;

/* One 32-byte record func_80059dc0 writes from scratch +0x8 per shown
 * equipment slot (MenuAttrScratch.recs). */
typedef struct {
    u8 unk0;
    u8 unk1;             /* 0x1: kind - 9, indexes unk27A/unk286 */
    u16 unk2;            /* 0x2 */
    s16 unk4[14];        /* 0x4 */
} MenuAttrRecord;

/*
 * The status screens' phase of the same block: seven attribute rows of 14
 * from +0x1b4 (MenuScratch's unk1B4 and unk25C are the first and last),
 * over the list fields the other screens keep there (func_80059dc0).
 */
typedef struct {
    u8 unk0[8];
    MenuAttrRecord recs[11]; /* 0x008: one per shown equipment slot */
    MenuAttrRecord *unk168; /* 0x168: next record func_80059dc0 fills */
    u16 unk16C;          /* 0x16c: viewer.unk38[6] / 3, 0xffff: none */
    u16 unk16E;          /* 0x16e */
    u16 unk170;          /* 0x170: viewer.unk38[12] / 3, 0xffff: none */
    u16 unk172;          /* 0x172 */
    u8 unk174[0x1b4 - 0x174];
    s16 unk1B4[14];      /* 0x1b4: shown values, clamped at 0 */
    s16 unk1D0[14];      /* 0x1d0: equipment sums, clamped at 0 */
    s16 unk1EC[14];      /* 0x1ec: totals, at most 999 */
    s16 unk208[14];      /* 0x208: equipment sums */
    s16 unk224[14];      /* 0x224: viewer.unk7E */
    s16 unk240[14];      /* 0x240: viewer.unk9A */
    s16 unk25C[14];      /* 0x25c: unk208 - unk224 */
    u8 unk278[2];
    u16 unk27A[6];       /* 0x27a: per kind sums of the records' unk2 */
    u16 unk286[6];       /* 0x286 */
    u16 unk292[21];      /* 0x292: MenuScratch.unk292 */
    s16 unk2BC;          /* 0x2bc: sum of the shown records' unk24 */
} MenuAttrScratch;

#define MENU_ATTR ((MenuAttrScratch *)0x1f800000)

/* The viewer's 21 equipment slots (viewer.unk38, st/data.h) as one block:
 * the status screens copy it whole and index it through this view. */
typedef struct {
    u16 ids[21];
} MenuSlots;

#define MENU_SLOTS ((MenuSlots *)viewer.unk38)

#define MENU_SCRATCH ((MenuScratch *)0x1f800000)

/*
 * A word of story flags: ST.EXE masks it whole; the FDAT overlays read single
 * bits as 1-bit fields (a byte load for bit 7, a compare with 1).
 */
typedef union {
    u32 word;
    struct {
        u32 f0 : 1, f1 : 1, f2 : 1, f3 : 1, f4 : 1, f5 : 1, f6 : 1, f7 : 1;
        u32 f8 : 1, f9 : 1, f10 : 1, f11 : 1, f12 : 1, f13 : 1, f14 : 1, f15 : 1;
        u32 f16 : 1, f17 : 1, f18 : 1, f19 : 1, f20 : 1, f21 : 1, f22 : 1, f23 : 1;
        u32 f24 : 1, f25 : 1, f26 : 1, f27 : 1, f28 : 1, f29 : 1, f30 : 1, f31 : 1;
    } bit;
    struct {
        u32 : 10;
        u32 gate : 3; /* bits 10-12: FDAT_272.BIN's gate stage (func_801ed4d0) */
    } field;
} Flags801cba08;

/* The save-game state block at 0x801cba08 (0x1714 bytes). */
typedef struct {
    /* 0x0..0x1440: per area (48), one bit per table A entry, table C slot 32..63
     * and three bits per table B entry (src/st/save.c) */
    u32 unk0[48][2];
    u32 unk180[48][1];
    u32 unk240[48][8];
    u32 unk840[48][8];
    u32 unkE40[48][8];
    Flags801cba08 unk1440; /* 0x1440: flag bits; 13 and 17 gate func_80044404's images */
    Flags801cba08 unk1444;
    u8 unk1448[48];      /* 0x1448: per XA stream file, the next channel to play */
    u8 unk1478[48];
    u32 unk14A8[0x5c];   /* 0x14a8: one bit per map id (func_800241d0) */
    /* 0x1618: per XA stream file, its position (func_8005408c); func_80044674
     * searches the first 40 */
    CdlLOC unk1618[48];
    s32 unk16D8;         /* 0x16d8: the examine/item-use result */
    u8 unk16DC;          /* 0x16dc: the warp request byte */
    u8 unk16DD[3];
    s32 unk16E0;         /* 0x16e0: save id, -1 until the first save (src/st/save.c) */
    u8 unk16E4[48];      /* by area */
} Data801cba08;
extern Data801cba08 DAT_801cba08;
typedef char Data801cba08Size[sizeof(Data801cba08) == 0x1714 ? 1 : -1];
/* Members of DAT_801cba08 under the names they had as separate identities:
 * +0x14a8, two 5-word rows cleared to -1 (src/st/main.c); +0x16d8, the
 * examine/item-use result (0 none, 1 done, 2 nothing to use; src/st/item.c);
 * +0x16dc, nonzero runs the warp func_80052c5c after the table A pass
 * (func_8004e324; func_80052c5c clears it). */
#define DAT_801cceb0 ((u32 (*)[5])DAT_801cba08.unk14A8)
#define DAT_801cd0e0 DAT_801cba08.unk16D8
#define DAT_801cd0e4 DAT_801cba08.unk16DC


/* .sbss words the map page reads with lui; their owner is unknown. */
extern s32 DAT_8008818c;
extern u16 DAT_800881a8;

/* Option page state bytes; owner unknown. */


extern MenuMessages menu_messages;

#define MESSAGE(i) ((u16 *)(menu_messages.text + menu_messages.offsets[i]))
/* A message of another table (a list's own). */
#define MESSAGE_IN(table, i) ((u16 *)((table)->text + (table)->offsets[i]))

/* Item stat figures that func_80031378 formats (whole and tenths). */
extern u16 DAT_8008819e;
extern u16 DAT_80088188;

/* The status pages' toggle between the attribute and the figure page. */
extern u16 DAT_800881aa;
extern u16 DAT_80088192;


/* Seven list slots cleared together by func_8002c090. */
extern s32 DAT_8017beb0[7];
extern s16 DAT_800881f0[7];

/* src/st/ui.c: the pause menu pages */
void func_80029b74(void);
void func_8002a1d0(void);
void func_8002a5a4(void);
void func_8002a954(void);
void func_8002b2e8(void);
void func_8002b84c(void);

/* the first menu page bodies */
void func_8002c050(void);
void func_8002c090(void);
void func_8002c1a4(void);
void func_8002c2b8(void);

/* the page frame and dialogs */
void func_8002c8bc(void);
void func_8002c958(void);
void func_8002c998(void);
void func_8002c9e8(void);
void func_8002ca84(void);
void func_8002cd98(void);
void func_8002ce84(void);
void func_8002cef8(void);
void func_8002d0b0(void);
void func_8002d1dc(void);

/* the item list screens */
void func_8002d574(void);
void func_8002d60c(void);
void func_8002d8e8(void);
void func_8002daa0(void);
void func_8002dd70(void);
void func_8002e310(void);

/* the status screens */
void func_8002e490(void);
void func_8002e4d0(void);
void func_8002e94c(void);

void func_8002ed98(void);
void func_8002f1f8(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4);
void func_8002f498(void);
void func_8002f97c(void);

void func_8002fc4c(void);

/* the stored-item screens */
void func_8002fe4c(void);
void func_8003025c(void);

void func_80030ab8(void);
void func_80030be4(void);
void func_80030fa0(void);
/* the id grid screens */
void func_80031378(void);
void func_80031674(void);
void func_800319a8(void);
void func_80031aec(void);
void func_80031cfc(void);
void func_80031d50(void);


#endif
