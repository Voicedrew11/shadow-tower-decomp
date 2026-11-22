/*
 * ST.EXE: CD request ring and archive file helpers (WIP module).
 *
 * The \ST\ path literals live in .sdata: the unit is compiled with a small
 * data threshold, and ASPSX still forms their addresses with lui/addiu.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBETC.H>
#include <LIBCD.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/scratch.h>
#include <st/funcs.h>

/* CD request kinds queued by cd_request_queue. */
#define CD_REQUEST_READ 2
#define CD_REQUEST_STREAM 3
#define CD_REQUEST_READ_4 4

/* Root counter 3 interval: 60000 vertical-sync ticks with target 1. */
#define TICK_TARGET 60000

SDATA(0x80087ecc, 0xb)

ADDRESS(0x80015b78, 0x54)
int cd_checksum_mismatch(u32 *data, int sectors)
{
    u32 sum = 0x12345678;
    int last = (sectors << 9) - 1;
    int count = last;
    u32 *p = data;

    while (count--)
        sum += *p++;
    return sum != data[last];
}

ADDRESS(0x80015bcc, 0x104)
StCdRequest *cd_request_queue(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)())
{
    StCdRequest *req = cd_state.unk9D4;

    cd_request_wait_free(req);
    req->unk0 = kind;
    req->unk1 = 0;
    req->unk18 = *loc;
    req->unk20 = dest;
    req->unk28 = (size + 2047) >> 11;
    req->done = done;
    if (++cd_state.unk9D4 == &cd_state.unk14[ST_CD_REQUESTS])
        cd_state.unk9D4 = cd_state.unk14;
    if (cd_state.unk9D8 == req) {
        CdReadyCallback(0);
        CdSyncCallback(cd_request_sync_callback);
        while (!CdControl(CdlSeekL, (u_char *)loc, 0))
            ;
    }
    return req;
}

ADDRESS(0x80015cd0, 0x84)
int cd_archive_member_locate(int archive, int member, CdlLOC *loc)
{
    StArchive *a = &cd_state.archives[archive];
    u16 start = a->sectors[member];

    if (loc)
        cd_loc_add_sectors(&a->file.pos, start, loc);
    return (a->sectors[member + 1] - start) << 11;
}

ADDRESS(0x80015d54, 0x54)
void cd_queue_read_member(int archive, int member, u8 *dest, void (*done)())
{
    CdlLOC loc;
    int size = cd_archive_member_locate(archive, member, &loc);

    if (size)
        cd_request_queue(CD_REQUEST_READ, &loc, size, dest, done);
}

ADDRESS(0x80015da8, 0x54)
void cd_queue_read_member_unchecked(int archive, int member, u8 *dest, void (*done)())
{
    CdlLOC loc;
    int size = cd_archive_member_locate(archive, member, &loc);

    if (size)
        cd_request_queue(CD_REQUEST_READ_4, &loc, size, dest, done);
}

ADDRESS(0x80015dfc, 0xcc)
void cd_queue_stream_member(int archive, int member, u8 *dest, void (*done)())
{
    StCdRequest *req = cd_state.unk9D4;
    CdlLOC loc;
    u32 size;

    cd_request_wait_free(req);
    req->unk4.image.h = 0;
    req->unk10 = 0;
    size = cd_archive_member_locate(archive, member, &loc);
    req->unkC = req->unkE = size >> 11;
    if (req->unkE > 16) {
        req->unkE -= 16;
        cd_request_queue(CD_REQUEST_STREAM, &loc, 0x8000, dest, done);
    } else {
        req->unkE = 0;
        cd_request_queue(CD_REQUEST_STREAM, &loc, size, dest, done);
    }
}

ADDRESS(0x80015ec8, 0x44)
int cd_archive_member_size(u16 archive, u16 member)
{
    StArchive *a = &cd_state.archives[archive];
    int first = a->sectors[member];

    return (a->sectors[member + 1] - first) << 11;
}

ADDRESS(0x80015f0c, 0xe8)
void cd_read_sectors(CdlLOC *loc, u_long *buf, int sectors)
{
    u_char result[8];
    int n;

    result[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, result, 0);
    VSync(4);
    cd_wait_idle();
    for (;;) {
        while (!CdControl(CdlSetloc, (u_char *)loc, 0))
            ;
        if (!CdRead(sectors, buf, CdlModeSpeed)) {
            CdSync(0, result);
            continue;
        }
        while ((n = CdReadSync(1, result)) > 0)
            func_800214cc();
        CdControl(CdlPause, 0, 0);
        if (n == 0)
            break;
    }
}

ADDRESS(0x80015ff4, 0x3c)
void cd_read_file_extent(u_long *buf, StFileExtent *file, u32 size)
{
    if (size == 0)
        size = file->size;
    cd_read_sectors(&file->pos, buf, (size + 2047) >> 11);
}

ADDRESS(0x80016030, 0x98)
int cd_read_member(u16 archive, u16 member, u_long *buf)
{
    StArchive *a = &cd_state.archives[archive];
    CdlLOC loc;
    int sectors = a->sectors[member + 1] - a->sectors[member];

    cd_loc_add_sectors(&a->file.pos, a->sectors[member], &loc);
    do
        cd_read_sectors(&loc, buf, sectors);
    while (cd_checksum_mismatch(buf, sectors));
    return sectors;
}

ADDRESS(0x800160c8, 0xc4)
void *load_st_file_alloc(char *name)
{
    char path[64];
    CdlFILE file;
    u32 size;
    void *buf;

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    cd_wait_idle();
    while (!CdSearchFile(&file, path))
        CdControl(CdlPause, 0, 0);
    size = (file.size + 2047) & ~2047;
    buf = mem_alloc(size);
    cd_read_sectors(&file.pos, buf, size >> 11);
    return buf;
}

ADDRESS(0x8001618c, 0xc0)
int load_st_file(void *buf, char *name, u32 size)
{
    char path[64];
    CdlFILE file;
    CdlLOC loc;

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    cd_wait_idle();
    while (!CdSearchFile(&file, path))
        CdControl(CdlPause, 0, 0);
    if (size == 0)
        size = file.size;
    cd_read_sectors(&file.pos, buf, (size + 2047) >> 11);
    return 0;
}

ADDRESS(0x8001624c, 0x48)
void cd_stop_drive(void)
{
    u_char param[16];

    CdControl(CdlStop, 0, 0);
    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    VSync(4);
}

ADDRESS(0x80016294, 0xe0)
void cd_archive_open(u16 archive, char *name)
{
    struct {
        u16 count;
        u16 sectors[1055];
    } header;
    CdlFILE file;
    char path[64];
    StArchive *a = &cd_state.archives[archive];
    u16 *sectors;
    int count;
    u16 *source;

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    while (!CdSearchFile(&file, path))
        ;
    a->file.pos = file.pos;
    a->file.size = file.size;
    cd_read_file_extent((u_long *)&header, &a->file, 2048);
    sectors = mem_alloc((header.count + 1) * 2);
    count = header.count + 1;
    source = header.sectors;
    a->sectors = sectors;
    copy_halfwords(sectors, source, count);
}

ADDRESS(0x80016374, 0xe0)
void cd_init(void)
{
    int i;
    StCdRequest *req;

    EnterCriticalSection();
    cd_state.unk0 = OpenEvent(RCntCNT3, EvSpINT, EvMdINTR, vsync_handler);
    EnableEvent(cd_state.unk0);
    SetRCnt(RCntCNT3, TICK_TARGET, RCntMdINTR);
    StartRCnt(RCntCNT3);
    cd_state.unk10 = 0;
    cd_state.unkC = 0;
    cd_state.unk8 = 0;
    cd_state.unk4 = 0;
    req = cd_state.unk14;
    for (i = 0; i < ST_CD_REQUESTS; i++, req++) {
        req->unk0 = 0;
        req->unk1 = 0;
    }
    cd_state.unk9D8 = cd_state.unk9D4 = cd_state.unk14;
    CdSyncCallback(0);
    CdReadyCallback(0);
    CdReadCallback(0);
    ExitCriticalSection();
    cd_stop_drive();
}

ADDRESS(0x80016454, 0x60)
void cd_shutdown(void)
{
    EnterCriticalSection();
    CloseEvent(cd_state.unk0);
    ExitCriticalSection();
    mem_free(cd_state.archives[2].sectors);
    mem_free(cd_state.archives[1].sectors);
    mem_free(cd_state.archives[0].sectors);
}

ADDRESS(0x800164b4, 0x54)
void frame_rate_update(void)
{
    if (cd_state.unk8 + 59 < cd_state.unk4) {
        u8 value = world_state.unk1ECC0;

        world_state.unk1ECC0 = 0;
        cd_state.unk8 = cd_state.unk4;
        world_state.unk1ECC1 = value;
        cd_state.unk10++;
    }
}

ADDRESS(0x80016508, 0x4c)
void fade_reset(void)
{
    world_state.fade.mode[0] = 1;
    world_state.fade.mode[1] = 1;
    world_state.fade.active[0] = world_state.fade.r[0] =
        world_state.fade.g[0] = world_state.fade.b[0] = 0;
    world_state.fade.active[1] = world_state.fade.r[1] =
        world_state.fade.g[1] = world_state.fade.b[1] = 0;
    world_state.unk1ECD2 = 255;
}
