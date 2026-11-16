/*
 * ST.EXE CD state block at 0x80088200 (bss) and the CD functions that use
 * it: the frame wait, vsync counters and read-request ring callbacks
 * (src/st/cdreq.c), CD position helpers (src/st/cdpos.c), and the request
 * queue and archive file readers (src/st/cd.c). Field names stay
 * address-derived until their behaviour is proven.
 */
#ifndef ST_CD_H
#define ST_CD_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBCD.H>

#define ST_CD_REQUESTS 48
#define ST_ARCHIVES 3

/* One 0x34-byte entry of the CD read-request ring. */
typedef struct {
    volatile u8 unk0; /* request kind, 0 when free; switch reads keep a separate zero-extension */
    volatile u8 unk1;
    u8 unk2[8];
    s16 unkA;
    s16 unkC;
    s16 unkE;
    u8 unk10;
    u8 unk11[3];
    CdlLOC unk14; /* copied from unk18 as a read starts */
    CdlLOC unk18;
    u8 *unk1C; /* CdGetSector destination, advanced 2048 per sector */
    u8 *unk20; /* destination queued by func_80015bcc; copied to unk1C as a read starts */
    s32 unk24;
    s32 unk28; /* sector count */
    void (*unk2C)(); /* completion callback, called unprototyped */
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

/*
 * The CD state block: the root counter 3 event (+0), vsync tick words
 * (+0x4..+0x10), the request ring and its two pointers, and the archive
 * tables, and the loading screen file at +0xa6c. WIP extent ends with it.
 */
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
    u8 unkA00[0xa6c - 0xa00];
    CdlFILE unkA6C; /* \LOAD02.TIM;1, searched by func_800210f4, read by func_80021158 */
} StCdState;

extern StCdState cd_state;

/* src/st/cdreq.c */
void func_8001531c(void);
void func_8001535c(void);
void func_800153b4(void);
void func_8001542c(u_char status, u_char *result);
void func_800154e8(u_char status, u_char *result);
void func_8001561c(u_char status, u_char *result);
void func_80015848(StCdRequest *request);
int func_800158f4(u8 bcd);
int func_80015914(u8 value);
u32 func_80015950(CdlLOC *pos);
void func_800159d4(CdlLOC *pos, u32 sector);

/* src/st/cdpos.c */
void func_80015a94(CdlLOC *pos, int offset, CdlLOC *out);
void func_80015ad4(void);
void func_80015b2c(StCdRequest *request);

/* src/st/cd.c */
int func_80015b78(u32 *data, int sectors);
StCdRequest *func_80015bcc(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)());
int func_80015cd0(int archive, int member, CdlLOC *loc);
void func_80015d54(int archive, int member, u8 *dest, void (*done)());
void func_80015da8(int archive, int member, u8 *dest, void (*done)());
void func_80015dfc(int archive, int member, u8 *dest, void (*done)());
int func_80015ec8(u16 archive, u16 member);
void func_80015f0c(CdlLOC *loc, u_long *buf, int sectors);
void func_80015ff4(u_long *buf, StFileExtent *file, u32 size);
int func_80016030(u16 archive, u16 member, u_long *buf);
void *func_800160c8(char *name);
int func_8001618c(void *buf, char *name, u32 size);
void func_8001624c(void);
void func_80016294(u16 archive, char *name);
void func_80016374(void);
void func_80016454(void);
void func_800164b4(void);
void func_80016508(void);

#endif
