/*
 * ST.EXE: CD request ring and archive file helpers (WIP module).
 *
 * The \ST\ path literals live in .sdata: the unit is compiled with a small
 * data threshold, and ASPSX still forms their addresses with lui/addiu.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBAPI.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <psyq/libc.h>
#include <st/state.h>
#include <st/game.h>

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
StCdRequest *func_80015bcc(int kind, CdlLOC *loc, u32 size, int dest, int arg4)
{
    StCdRequest *req = DAT_80088200.unk9D4;

    func_80015b2c(req);
    req->unk0 = kind;
    req->unk1 = 0;
    req->unk18 = *loc;
    req->unk20 = dest;
    req->unk28 = (size + 2047) >> 11;
    req->unk2C = arg4;
    if (++DAT_80088200.unk9D4 == &DAT_80088200.unk14[ST_CD_REQUESTS])
        DAT_80088200.unk9D4 = DAT_80088200.unk14;
    if (DAT_80088200.unk9D8 == req) {
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
    StArchive *a = &DAT_80088200.archives[archive];
    u16 start = a->sectors[member];

    if (loc)
        func_80015a94(&a->file.pos, start, loc);
    return (a->sectors[member + 1] - start) << 11;
}

ADDRESS(0x80015d54, 0x54)
void func_80015d54(int archive, int member, int dest, int arg3)
{
    CdlLOC loc;
    int size = func_80015cd0(archive, member, &loc);

    if (size)
        func_80015bcc(CD_REQUEST_READ, &loc, size, dest, arg3);
}

ADDRESS(0x80015da8, 0x54)
void func_80015da8(int archive, int member, int dest, int arg3)
{
    CdlLOC loc;
    int size = func_80015cd0(archive, member, &loc);

    if (size)
        func_80015bcc(CD_REQUEST_READ_4, &loc, size, dest, arg3);
}

/*
 * Not exact: the two field clears and the &loc argument are scheduled in a
 * different order around the func_80015cd0 call (retail puts &loc in the
 * delay slot).
 */
ADDRESS(0x80015dfc, 0xcc)
void func_80015dfc(int archive, int member, int dest, int arg3)
{
    StCdRequest *req = DAT_80088200.unk9D4;
    CdlLOC loc;
    u32 size;

    func_80015b2c(req);
    req->unkA = 0;
    req->unk10 = 0;
    size = func_80015cd0(archive, member, &loc);
    req->unkC = req->unkE = size >> 11;
    if (req->unkE > 16) {
        req->unkE -= 16;
        func_80015bcc(CD_REQUEST_STREAM, &loc, 0x8000, dest, arg3);
    } else {
        req->unkE = 0;
        func_80015bcc(CD_REQUEST_STREAM, &loc, size, dest, arg3);
    }
}

/* Not exact: retail loads sectors[member] before sectors[member + 1]. */
ADDRESS(0x80015ec8, 0x44)
int func_80015ec8(u16 archive, u16 member)
{
    StArchive *a = &DAT_80088200.archives[archive];

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
    StArchive *a = &DAT_80088200.archives[archive];
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
int func_8001618c(u_long *buf, char *name, u32 size)
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
    StArchive *a = &DAT_80088200.archives[archive];

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
    DAT_80088200.unk0 = OpenEvent(RCntCNT3, EvSpINT, EvMdINTR, func_800153b4);
    EnableEvent(DAT_80088200.unk0);
    SetRCnt(RCntCNT3, TICK_TARGET, RCntMdINTR);
    StartRCnt(RCntCNT3);
    DAT_80088200.unk10 = 0;
    DAT_80088200.unkC = 0;
    DAT_80088200.unk8 = 0;
    DAT_80088200.unk4 = 0;
    req = DAT_80088200.unk14;
    for (i = 0; i < ST_CD_REQUESTS; i++, req++) {
        req->unk0 = 0;
        req->unk1 = 0;
    }
    DAT_80088200.unk9D8 = DAT_80088200.unk9D4 = DAT_80088200.unk14;
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
    CloseEvent(DAT_80088200.unk0);
    ExitCriticalSection();
    func_800152f8(DAT_80088200.archives[2].sectors);
    func_800152f8(DAT_80088200.archives[1].sectors);
    func_800152f8(DAT_80088200.archives[0].sectors);
}

ADDRESS(0x800164b4, 0x54)
void func_800164b4(void)
{
    if (DAT_80088200.unk8 + 59 < DAT_80088200.unk4) {
        u8 value = DAT_8015d090.unk1ECC0;

        DAT_8015d090.unk1ECC0 = 0;
        DAT_80088200.unk8 = DAT_80088200.unk4;
        DAT_8015d090.unk1ECC1 = value;
        DAT_80088200.unk10++;
    }
}

ADDRESS(0x80016508, 0x4c)
void func_80016508(void)
{
    DAT_8015d090.unk1ECD8[0] = 1;
    DAT_8015d090.unk1ECD8[1] = 1;
    DAT_8015d090.unk1ECD6[0] = DAT_8015d090.unk1ECDA[0] =
        DAT_8015d090.unk1ECDE[0] = DAT_8015d090.unk1ECE2[0] = 0;
    DAT_8015d090.unk1ECD6[1] = DAT_8015d090.unk1ECDA[1] =
        DAT_8015d090.unk1ECDE[1] = DAT_8015d090.unk1ECE2[1] = 0;
    DAT_8015d090.unk1ECD2 = 255;
}
