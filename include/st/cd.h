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
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>

#define ST_CD_REQUESTS 48
#define ST_ARCHIVES 3

/* The request's eight-byte image/bank payload. */
struct StSoundBuf;
typedef struct StCdBankPayload {
    s16 slot;
    s16 member;
    struct StSoundBuf *buffer;
} StCdBankPayload;

typedef union StCdPayload {
    RECT image;
    StCdBankPayload bank;
} StCdPayload;

typedef char StCdRectSize[(sizeof(RECT) == 8) ? 1 : -1];
typedef char StCdBankPayloadSize[(sizeof(StCdBankPayload) == 8) ? 1 : -1];
typedef char StCdPayloadSize[(sizeof(StCdPayload) == 8) ? 1 : -1];
typedef char StCdPayloadImageAt0[(offsetof(StCdPayload, image) == 0) ? 1 : -1];
typedef char StCdPayloadBankAt0[(offsetof(StCdPayload, bank) == 0) ? 1 : -1];
typedef char StCdBankSlotAt0[(offsetof(StCdBankPayload, slot) == 0) ? 1 : -1];
typedef char StCdBankMemberAt2[(offsetof(StCdBankPayload, member) == 2) ? 1 : -1];
typedef char StCdBankBufferAt4[(offsetof(StCdBankPayload, buffer) == 4) ? 1 : -1];
typedef char StCdBankSlotWidth[(sizeof(((StCdBankPayload *)0)->slot) == 2) ? 1 : -1];
typedef char StCdBankMemberWidth[(sizeof(((StCdBankPayload *)0)->member) == 2) ? 1 : -1];
typedef char StCdBankBufferWidth[(sizeof(((StCdBankPayload *)0)->buffer) == 4) ? 1 : -1];

/* One 0x34-byte entry of the CD read-request ring. */
typedef struct {
    volatile u8 unk0; /* request kind, 0 when free; switch reads keep a separate zero-extension */
    volatile u8 unk1;
    u8 unk2[2];
    StCdPayload unk4; /* 0x04..0x0b: image rectangle or bank arguments */
    s16 unkC;
    s16 unkE;
    volatile u8 unk10; /* completion state the callbacks set; stores keep their own slot */
    volatile u8 unk11; /* bank requests (src/st/sound.c): load step; stores keep their own slot */
    u8 unk12[2];
    CdlLOC unk14; /* copied from unk18 as a read starts */
    CdlLOC unk18;
    u8 *unk1C; /* CdGetSector destination, advanced 2048 per sector */
    u8 *unk20; /* destination queued by cd_request_queue; copied to unk1C as a read starts */
    s32 unk24;
    s32 unk28; /* sector count */
    /* 0x2c: completion callback, set only by the request builder (ST
     * cd_request_queue, VS cd_request_queue) from its argument; called with the
     * destination (unk20) when a checked read (kind 2) completes, else
     * with the request. Its value sources: config/retail/indirect_pointers.tsv
     * (StCdRequest.done). */
    void (*done)();
    s32 unk30;
} StCdRequest;

typedef char StCdRequestSize[(sizeof(StCdRequest) == 0x34) ? 1 : -1];
typedef char StCdRequestunk0Offset[(offsetof(StCdRequest, unk0) == 0x0) ? 1 : -1];
typedef char StCdRequestunk1Offset[(offsetof(StCdRequest, unk1) == 0x1) ? 1 : -1];
typedef char StCdRequestunk2Offset[(offsetof(StCdRequest, unk2) == 0x2) ? 1 : -1];
typedef char StCdRequestunk4Offset[(offsetof(StCdRequest, unk4) == 0x4) ? 1 : -1];
typedef char StCdRequestunkCOffset[(offsetof(StCdRequest, unkC) == 0xc) ? 1 : -1];
typedef char StCdRequestunkEOffset[(offsetof(StCdRequest, unkE) == 0xe) ? 1 : -1];
typedef char StCdRequestunk10Offset[(offsetof(StCdRequest, unk10) == 0x10) ? 1 : -1];
typedef char StCdRequestunk11Offset[(offsetof(StCdRequest, unk11) == 0x11) ? 1 : -1];
typedef char StCdRequestunk12Offset[(offsetof(StCdRequest, unk12) == 0x12) ? 1 : -1];
typedef char StCdRequestunk14Offset[(offsetof(StCdRequest, unk14) == 0x14) ? 1 : -1];
typedef char StCdRequestunk18Offset[(offsetof(StCdRequest, unk18) == 0x18) ? 1 : -1];
typedef char StCdRequestunk1COffset[(offsetof(StCdRequest, unk1C) == 0x1c) ? 1 : -1];
typedef char StCdRequestunk20Offset[(offsetof(StCdRequest, unk20) == 0x20) ? 1 : -1];
typedef char StCdRequestunk24Offset[(offsetof(StCdRequest, unk24) == 0x24) ? 1 : -1];
typedef char StCdRequestunk28Offset[(offsetof(StCdRequest, unk28) == 0x28) ? 1 : -1];
typedef char StCdRequestDoneOffset[(offsetof(StCdRequest, done) == 0x2c) ? 1 : -1];
typedef char StCdRequestunk30Offset[(offsetof(StCdRequest, unk30) == 0x30) ? 1 : -1];


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
    StArchive unkA00;    /* 0xa00: a fourth archive table (menu_equip_info_run seeks to member 1) */
    u8 unkA0C[0xa6c - 0xa0c];
    CdlFILE unkA6C; /* \LOAD02.TIM;1, searched by loading_screen_find_file, read by loading_screen_show */
    u_long *unkA84;      /* 0xa84: load cursor into DAT_80088c88, which follows this block (src/st/menu.c) */
} StCdState;

typedef char StCdRequestRingStride[(sizeof(((StCdState *)0)->unk14[0]) == 0x34) ? 1 : -1];
typedef char StCdRequestRingCount[(sizeof(((StCdState *)0)->unk14) == 48 * 0x34) ? 1 : -1];
typedef char StCdRequestRingAt0x14[(offsetof(StCdState, unk14) == 0x14) ? 1 : -1];
typedef char StCdFreeAt0x9d4[(offsetof(StCdState, unk9D4) == 0x9d4) ? 1 : -1];
typedef char StCdCurrentAt0x9d8[(offsetof(StCdState, unk9D8) == 0x9d8) ? 1 : -1];
typedef char StCdArchivesAt0x9dc[(offsetof(StCdState, archives) == 0x9dc) ? 1 : -1];


extern StCdState cd_state;

/* src/st/cdreq.c */
void cd_wait_frame(void);
void vsync_wait_tick(void);
void vsync_handler(void);
void cd_request_read_callback(u_char status, u_char *result);
void cd_request_ready_callback(u_char status, u_char *result);
void cd_request_sync_callback(u_char status, u_char *result);
void cd_request_retire(StCdRequest *request);
int bcd_to_int(u8 bcd);
int int_to_bcd(u8 value);
u32 cd_loc_to_sector(CdlLOC *pos);
void cd_sector_to_loc(volatile CdlLOC *pos, u32 sector);

/* src/st/cdpos.c */
void cd_loc_add_sectors(CdlLOC *pos, int offset, CdlLOC *out);
void cd_wait_idle(void);
void cd_request_wait_free(StCdRequest *request);

/* src/st/cd.c */
int cd_checksum_mismatch(u32 *data, int sectors);
StCdRequest *cd_request_queue(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)());
int cd_archive_member_locate(int archive, int member, CdlLOC *loc);
void cd_queue_read_member(int archive, int member, u8 *dest, void (*done)());
void cd_queue_read_member_unchecked(int archive, int member, u8 *dest, void (*done)());
void cd_queue_stream_member(int archive, int member, u8 *dest, void (*done)());
int cd_archive_member_size(u16 archive, u16 member);
void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors);
void cd_read_file_extent(u_long *buf, StFileExtent *file, u32 size);
/* VS.EXE's callers declare their own cd_read_member with int or no
 * parameters; VS units define VS_EXE. */
#ifndef VS_EXE
int cd_read_member(u16 archive, u16 member, u_long *buf);
#endif
void *load_st_file_alloc(char *name);
int load_st_file(void *buf, char *name, u32 size);
void cd_stop_drive(void);
void cd_archive_open(u16 archive, char *name);
void cd_init(void);
void cd_shutdown(void);
void frame_rate_update(void);
void fade_reset(void);

#endif
