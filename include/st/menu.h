/*
 * ST.EXE menu screens (src/st/menu.c, src/st/menu_*.c): the scratchpad
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
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/text.h>

/*
 * The message table: byte offsets of each message from text, then the
 * messages, halfword glyph codes ending at 0xffff (0 breaks the line).
 */
typedef struct {
    u16 offsets[0x600];
    u8 text[1];
} MenuMessages;

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
    u8 unkC[8];          /* 0x0c */
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

/*
 * The stored-item grid (func_8002fe4c): the message table, one kind per
 * entry, the entry count, the last row and its length, and the VRAM image
 * of each of the fifteen visible cells.
 */
typedef struct {
    u8 unk0[4];
    MenuMessages *messages; /* 0x04 */
    u8 *kinds;           /* 0x08 */
    s16 count;           /* 0x0c */
    s16 rows;            /* 0x0e */
    s16 last;            /* 0x10: cells on the last row */
    u8 unk12[2];
    DVECTOR tex[15];     /* 0x14 */
} MenuStore;

/* The screen state at the start of the scratchpad, per screen. */
typedef union {
    s32 word;
    s16 half[2];
    MenuGrid grid;
    MenuList list;
    DVECTOR vram[2];     /* func_8002f1f8's texture and CLUT positions */
    MenuStore store;
    DVECTOR icons[6];    /* the pause list page's icon texture positions, used from [1] */
    struct {
        MenuMessages *messages; /* 0x00: the list's message table */
        u8 unk04[0x14];
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
} MenuState;

/* A 32-byte list record func_8002c2b8 lists. */
typedef struct {
    u8 unk0[0x19];
    u8 kind;             /* 0x19: indexes the record title table */
    u16 unk1A;           /* 0x1a */
    u8 unk1C[4];
} MenuRecord;

typedef struct {
    MenuState unk0;      /* 0x000 */
    u8 unk5C[0x158];     /* 0x05c */
    s16 unk1B4[14];      /* 0x1b4: attribute values */
    u8 unk1D0[0x24];     /* 0x1d0 */
    MenuRecord *unk1F4;  /* 0x1f4: the listed records */
    u16 unk1F8;          /* 0x1f8 */
    s16 unk1FA;          /* 0x1fa: list length */
    s16 unk1FC;          /* 0x1fc */
    s16 unk1FE;          /* 0x1fe: cursor row */
    s16 unk200;          /* 0x200: first visible list entry */
    u16 unk202;          /* 0x202 */
    u8 unk204[2];        /* 0x204 */
    s16 unk206;          /* 0x206 */
    u8 unk208[0x54];     /* 0x208 */
    s16 unk25C[14];      /* 0x25c: attribute changes */
    u8 unk278[0x46];     /* 0x278 */
    s16 unk2BE;          /* 0x2be */
    s16 unk2C0;          /* 0x2c0 */
    u8 unk2C2[2];        /* 0x2c2 */
    s16 unk2C4;          /* 0x2c4: attribute bar scale */
    u8 unk2C6;           /* 0x2c6 */
    u8 unk2C7;           /* 0x2c7 */
    u8 unk2C8[4];        /* 0x2c8 */
    MenuMessages *unk2CC; /* 0x2cc */
    u8 unk2D0[4];        /* 0x2d0 */
    u32 *unk2D4;         /* 0x2d4: one bit per grid cell */
    u8 unk2D8[4];        /* 0x2d8 */
    u16 unk2DC;          /* 0x2dc */
    u16 unk2DE;          /* 0x2de: selected row of the item and stats menus */
    s16 unk2E0;          /* 0x2e0 */
    s16 unk2E2;          /* 0x2e2 */
    u8 unk2E4;           /* 0x2e4: cursor blink phase */
    u8 unk2E5;           /* 0x2e5: text font mode (0: 6x12 half-width cells) */
    u8 unk2E6[0x1a];     /* 0x2e6 */
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

#define MENU_SCRATCH ((MenuScratch *)0x1f800000)

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

/* src/st/menu.c */
void func_80029b74(void);
void func_8002a1d0(void);
void func_8002a5a4(void);
void func_8002a954(void);
void func_8002b2e8(void);

/* src/st/menu_pages.c */
void func_8002c050(void);
void func_8002c090(void);
void func_8002c1a4(void);

/* src/st/menu_dialogs.c */
void func_8002c8bc(void);
void func_8002c958(void);
void func_8002c998(void);
void func_8002c9e8(void);
void func_8002ca84(void);
void func_8002cd98(void);
void func_8002ce84(void);
void func_8002cef8(void);
void func_8002d0b0(void);

/* src/st/menu_shop.c */
void func_8002d574(void);
void func_8002d60c(void);
void func_8002d8e8(void);
void func_8002daa0(void);
void func_8002dd70(void);
void func_8002e310(void);

/* src/st/menu_status.c */
void func_8002e490(void);
void func_8002e4d0(void);
void func_8002e94c(void);

/* src/st/menu_status_pages.c */
void func_8002ed98(void);
void func_8002f1f8(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4);
void func_8002f498(void);
void func_8002f97c(void);

/* src/st/menu_status_frame.c */
void func_8002fc4c(void);

/* src/st/menu_store.c */
void func_8002fe4c(void);

/* src/st/menu_store_frame.c */
void func_80030ab8(void);

/* src/st/menu_grid.c */
void func_80031378(void);
void func_80031674(void);

/* src/st/menu_misc.c */
void func_80031aec(void);
void func_80031cfc(void);
void func_80031d50(void);

/*
 * Not reconstructed yet: they share merged constants or $gp-relative
 * .sbss with band E functions of the same translation unit.
 */
void func_8002c2b8(void);
void func_8002d1dc(void);
void func_80030be4(void);
void func_80030fa0(void);
void func_800319a8(void);
/* Not reconstructed yet (the stored-item detail page). */
void func_8003025c(void);

#endif
