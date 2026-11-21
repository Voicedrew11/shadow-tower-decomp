/*
 * ST.EXE memory card access (src/st/menu.c). The save screens keep their
 * card state in the menu scratchpad block (MenuCardScratch, st/menu.h).
 */
#ifndef ST_CARD_H
#define ST_CARD_H

#include <kf/lib/types.h>

/*
 * The 512-byte header of a save file (the memory card block format): the
 * "SC" magic, the icon type and block count, the Shift-JIS title, the
 * game's own bytes in the title's tail (copied into the file's MenuRecord),
 * then the icon CLUT and frames.
 */
/* The header's 64-byte title: Shift-JIS text, then the game's own bytes,
 * which func_8006184c copies into the file's MenuRecord. */
typedef struct {
    u8 text[0x39];       /* 0x00 */
    u8 kind;             /* 0x39: the area saved in */
    u16 unk3A;           /* 0x3a: viewer.unk8 */
    u16 unk3C;           /* 0x3c: viewer.unk28 */
    u8 unk3E;            /* 0x3e: the sum of the data frames' sums */
    u8 unk3F;
} CardTitle;

typedef struct {
    char magic[2];       /* 0x00: "SC" */
    u8 icon;             /* 0x02: 0x12: two icon frames */
    u8 blocks;           /* 0x03 */
    CardTitle title;     /* 0x04 */
    u8 unk44[0x1c];      /* 0x44 */
    u16 clut[16];        /* 0x60: the icon CLUT */
    u8 icons[0x100];     /* 0x80: two 16x16 4-bit icon frames */
    u8 unk180[0x80];     /* 0x180 */
} CardHeader;

void func_80060bac(void);
void func_80060c00(void);
void func_80060c54(void);
void func_80060de4(void);
int func_80060ed4(void);
int func_80060f88(void);
void func_8006103c(int port);
void func_800613e8(int port, int mode);
int func_80061578(int port);
int func_800615d8(void);
int func_800616f8(void);
int func_8006184c(void);
int func_80061a08(void);
int func_80061d34(int fd, int offset, u8 *data, int size);
int func_80061ee8(int fd, int offset, u8 *data, int size);
int func_8006205c(u8 *data);
int func_80062228(u8 *data);
long func_80062bc0(int port);
int func_80062c28(int arg);
int func_80063078(void);
void func_800631ac(void);
int func_80063260(void);

/* src/st/menu.c */
int func_8005e924(int points);

/* src/st/menu.c */
void func_8005f38c(void);
void func_8005f4fc(void);

/* src/st/menu.c */
void func_80060334(void);

/* src/st/menu_vram.c */
void func_80063638(void);
void func_800636f8(void);
void func_800637c0(void);
void func_80063954(void);

#endif
