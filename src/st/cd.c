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

/* CD request kinds queued by func_80015bcc. */
#define CD_REQUEST_READ 2
#define CD_REQUEST_STREAM 3
#define CD_REQUEST_READ_4 4

/* Root counter 3 interval: 60000 vertical-sync ticks with target 1. */
#define TICK_TARGET 60000

SDATA(0x80087ecc, 0xb)

ADDRESS(0x80015b78, 0x54)
int func_80015b78(u32 *data, int sectors)
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
StCdRequest *func_80015bcc(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)())
{
    StCdRequest *req = cd_state.unk9D4;

    func_80015b2c(req);
    req->unk0 = kind;
    req->unk1 = 0;
    req->unk18 = *loc;
    req->unk20 = dest;
    req->unk28 = (size + 2047) >> 11;
    req->unk2C = done;
    if (++cd_state.unk9D4 == &cd_state.unk14[ST_CD_REQUESTS])
        cd_state.unk9D4 = cd_state.unk14;
    if (cd_state.unk9D8 == req) {
        CdReadyCallback(0);
        CdSyncCallback(func_8001561c);
        while (!CdControl(CdlSeekL, (u_char *)loc, 0))
            ;
    }
    return req;
}

ADDRESS(0x80015cd0, 0x84)
int func_80015cd0(int archive, int member, CdlLOC *loc)
{
    StArchive *a = &cd_state.archives[archive];
    u16 start = a->sectors[member];

    if (loc)
        func_80015a94(&a->file.pos, start, loc);
    return (a->sectors[member + 1] - start) << 11;
}

ADDRESS(0x80015d54, 0x54)
void func_80015d54(int archive, int member, u8 *dest, void (*done)())
{
    CdlLOC loc;
    int size = func_80015cd0(archive, member, &loc);

    if (size)
        func_80015bcc(CD_REQUEST_READ, &loc, size, dest, done);
}

ADDRESS(0x80015da8, 0x54)
void func_80015da8(int archive, int member, u8 *dest, void (*done)())
{
    CdlLOC loc;
    int size = func_80015cd0(archive, member, &loc);

    if (size)
        func_80015bcc(CD_REQUEST_READ_4, &loc, size, dest, done);
}

/*
 * Not exact: the two field clears and the &loc argument are scheduled in a
 * different order around the func_80015cd0 call (retail puts &loc in the
 * delay slot).
 */
ADDRESS(0x80015dfc, 0xcc)
void func_80015dfc(int archive, int member, u8 *dest, void (*done)())
{
    StCdRequest *req = cd_state.unk9D4;
    CdlLOC loc;
    u32 size;

    func_80015b2c(req);
    req->unkA = 0;
    req->unk10 = 0;
    size = func_80015cd0(archive, member, &loc);
    req->unkC = req->unkE = size >> 11;
    if (req->unkE > 16) {
        req->unkE -= 16;
        func_80015bcc(CD_REQUEST_STREAM, &loc, 0x8000, dest, done);
    } else {
        req->unkE = 0;
        func_80015bcc(CD_REQUEST_STREAM, &loc, size, dest, done);
    }
}

/* Not exact: retail loads sectors[member] before sectors[member + 1]. */
ADDRESS(0x80015ec8, 0x44)
int func_80015ec8(u16 archive, u16 member)
{
    StArchive *a = &cd_state.archives[archive];

    return (a->sectors[member + 1] - a->sectors[member]) << 11;
}

ADDRESS(0x80015f0c, 0xe8)
void func_80015f0c(CdlLOC *loc, u_long *buf, int sectors)
{
    u_char result[8];
    int n;

    result[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, result, 0);
    VSync(4);
    func_80015ad4();
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
void func_80015ff4(u_long *buf, StFileExtent *file, u32 size)
{
    if (size == 0)
        size = file->size;
    func_80015f0c(&file->pos, buf, (size + 2047) >> 11);
}

ADDRESS(0x80016030, 0x98)
int func_80016030(u16 archive, u16 member, u_long *buf)
{
    StArchive *a = &cd_state.archives[archive];
    CdlLOC loc;
    int sectors = a->sectors[member + 1] - a->sectors[member];

    func_80015a94(&a->file.pos, a->sectors[member], &loc);
    do
        func_80015f0c(&loc, buf, sectors);
    while (func_80015b78(buf, sectors));
    return sectors;
}

ADDRESS(0x800160c8, 0xc4)
void *func_800160c8(char *name)
{
    char path[64];
    CdlFILE file;
    u32 size;
    void *buf;

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    func_80015ad4();
    while (!CdSearchFile(&file, path))
        CdControl(CdlPause, 0, 0);
    size = (file.size + 2047) & ~2047;
    buf = func_800152d0(size);
    func_80015f0c(&file.pos, buf, size >> 11);
    return buf;
}

ADDRESS(0x8001618c, 0xc0)
int func_8001618c(void *buf, char *name, u32 size)
{
    char path[64];
    CdlFILE file;
    CdlLOC loc;

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    func_80015ad4();
    while (!CdSearchFile(&file, path))
        CdControl(CdlPause, 0, 0);
    if (size == 0)
        size = file.size;
    func_80015f0c(&file.pos, buf, (size + 2047) >> 11);
    return 0;
}

ADDRESS(0x8001624c, 0x48)
void func_8001624c(void)
{
    u_char param[16];

    CdControl(CdlStop, 0, 0);
    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    VSync(4);
}

/*
 * Not exact: retail stores a->sectors after loading func_80014d2c's count
 * and source arguments; this build stores it first.
 */
ADDRESS(0x80016294, 0xe0)
void func_80016294(u16 archive, char *name)
{
    struct {
        u16 count;
        u16 sectors[1055];
    } header;
    CdlFILE file;
    char path[64];
    StArchive *a = &cd_state.archives[archive];

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    while (!CdSearchFile(&file, path))
        ;
    a->file.pos = file.pos;
    a->file.size = file.size;
    func_80015ff4((u_long *)&header, &a->file, 2048);
    a->sectors = func_800152d0((header.count + 1) * 2);
    func_80014d2c(a->sectors, header.sectors, header.count + 1);
}

ADDRESS(0x80016374, 0xe0)
void func_80016374(void)
{
    int i;
    StCdRequest *req;

    EnterCriticalSection();
    cd_state.unk0 = OpenEvent(RCntCNT3, EvSpINT, EvMdINTR, func_800153b4);
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
    func_8001624c();
}

ADDRESS(0x80016454, 0x60)
void func_80016454(void)
{
    EnterCriticalSection();
    CloseEvent(cd_state.unk0);
    ExitCriticalSection();
    func_800152f8(cd_state.archives[2].sectors);
    func_800152f8(cd_state.archives[1].sectors);
    func_800152f8(cd_state.archives[0].sectors);
}

ADDRESS(0x800164b4, 0x54)
void func_800164b4(void)
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
void func_80016508(void)
{
    world_state.unk1ECD8[0] = 1;
    world_state.unk1ECD8[1] = 1;
    world_state.unk1ECD6[0] = world_state.unk1ECDA[0] =
        world_state.unk1ECDE[0] = world_state.unk1ECE2[0] = 0;
    world_state.unk1ECD6[1] = world_state.unk1ECDA[1] =
        world_state.unk1ECDE[1] = world_state.unk1ECE2[1] = 0;
    world_state.unk1ECD2 = 255;
}
