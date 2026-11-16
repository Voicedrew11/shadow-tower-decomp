/*
 * ST.EXE's CD state block at 0x80088200 (bss): the CD read-request ring,
 * the root-counter event and tick words, and the three archive tables.
 * Field names stay address-derived until their behaviour is proven.
 */
#ifndef ST_STATE_H
#define ST_STATE_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBCD.H>

#define ST_CD_REQUESTS 48
#define ST_ARCHIVES 3

/* One 0x34-byte entry of the CD read-request ring. */
typedef struct {
    volatile u8 unk0; /* band A: switch reads keep a separate zero-extension */
    volatile u8 unk1;
    u8 unk2[8];
    s16 unkA;
    s16 unkC;
    s16 unkE;
    u8 unk10;
    u8 unk11[3];
    CdlLOC unk14; /* band A: copied from unk18 as a read starts */
    CdlLOC unk18;
    u8 *unk1C; /* band A: CdGetSector destination, advanced 2048 per sector */
    u8 *unk20; /* band A: copied to unk1C as a read starts */
    s32 unk24;
    s32 unk28;
    void (*unk2C)(); /* band A: completion callback, called unprototyped */
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
    u8 unkA00[0xa6c - 0xa00]; /* band D */
    CdlFILE unkA6C; /* band D: \LOAD02.TIM;1, read by func_80021158 */
} State80088200;

extern State80088200 DAT_80088200;

#endif
