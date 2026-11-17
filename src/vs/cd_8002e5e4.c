/*
 * VS.EXE: the loading-display frame step, word/halfword copies, the
 * malloc wrappers, the CD read-request ring callbacks, BCD/CD position
 * helpers, the plain CD reader and a whole-file reader (WIP module).
 * VS's copies of ST.EXE functions from src/st/quad.c, memory.c, heap.c,
 * cdreq.c, cdpos.c and cd.c; VS keeps a 4-entry request ring.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <MALLOC.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <st/cd.h>
#include <vs/data.h>
#include <vs/funcs.h>

/* unk1 of the current request: the step its read is at. */
#define STEP_SEEK 0
#define STEP_READ 1
#define STEP_DONE 2

ADDRESS(0x8002e5e4, 0x4c)
void func_8002e5e4(void)
{
    if (DAT_801624d0.unk2C974) {
        func_8001d1b8();
        func_8001d374();
    }
}

/* Returns the end of the source. */
ADDRESS(0x8002e630, 0x30)
u32 *func_8002e630(u32 *dst, u32 *src, int count)
{
    int left;

    while ((left = count, count = count - 1, left))
        *dst++ = *src++;
    return src;
}

ADDRESS(0x8002e660, 0x30)
u16 *func_8002e660(u16 *dst, u16 *src, int count)
{
    int left;

    while ((left = count, count = count - 1, left))
        *dst++ = *src++;
    return src;
}

/* The C library's malloc, or NULL when the result is not in main RAM
 * (ST.EXE's copy calls malloc3). */
ADDRESS(0x8002e690, 0x44)
void *func_8002e690(u32 size)
{
    u8 *p = malloc(size);

    if ((u32)(p - (u8 *)0x80000000) > 0x1fffff)
        return NULL;
    return p;
}

ADDRESS(0x8002e6d4, 0x28)
void *func_8002e6d4(u32 size)
{
    return func_8002e690((size + 7) & ~7);
}

/* free3 of a non-NULL pointer (the shape of LIBSN's _OP_DELE, as in ST.EXE). */
ADDRESS(0x8002e6fc, 0x24)
void func_8002e6fc(void *p)
{
    if (p != NULL)
        free3(p);
}

/*
 * CdReadyCallback of a read: copies each sector's data into the request.
 * Unlike ST.EXE's copy it ends a read with CdlNop polling, not by calling
 * the sync callback.
 */
ADDRESS(0x8002e720, 0x148)
void func_8002e720(u_char status, u_char *result)
{
    StCdRequest *request = DAT_801a4590.unkDC;
    u_char param[8]; /* unused */
    u_long rest[70];

    if (status == CdlDataReady) {
        switch (request->unk0) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            while (CdGetSector(request->unk1C, 3) == 0)
                ;
            while (CdGetSector(request->unk1C, 512) == 0)
                ;
            request->unk1C += 2048;
            while (CdGetSector(rest, 70) == 0)
                ;
            if (--request->unk24 == 0) {
                CdReadyCallback(0);
                request->unk1 = STEP_DONE;
                while (CdControlF(CdlNop, 0) == 0)
                    ;
            }
            break;
        }
    } else if (status == CdlDiskError) {
        switch (request->unk0) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->unk24 = -1;
            CdReadyCallback(0);
            request->unk1 = STEP_DONE;
            while (CdControlF(CdlNop, 0) == 0)
                ;
            break;
        }
    }
}

/* CdSyncCallback of the request ring: seek, read, check, finish. */
ADDRESS(0x8002e868, 0x22c)
void func_8002e868(u_char status, u_char *result)
{
    StCdRequest *request = DAT_801a4590.unkDC;

    if (status == CdlComplete) {
        switch (request->unk0) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            switch (request->unk1) {
            case STEP_SEEK:
                request->unk14 = request->unk18;
                request->unk1 = STEP_READ;
                request->unk24 = request->unk28;
                request->unk1C = request->unk20;
                CdReadyCallback(func_8002e720);
                CdSyncCallback(func_8002e868);
                while (CdControlF(CdlReadN, 0) == 0)
                    ;
                break;
            case STEP_READ:
                break;
            case STEP_DONE:
                if (request->unk0 == 2) {
                    if (request->unk24 != 0) {
                        request->unk1 = STEP_SEEK;
                        while (CdControl(CdlSeekL, (u_char *)&request->unk18, 0) == 0)
                            ;
                    } else if (func_8002ed20((u32 *)request->unk20, request->unk28) != 0) {
                        request->unk1 = STEP_SEEK;
                        while (CdControl(CdlSeekL, (u_char *)&request->unk18, 0) == 0)
                            ;
                    } else {
                        if (request->unk2C != NULL)
                            request->unk2C(request->unk20);
                        func_8002ea94(request);
                    }
                } else if (request->unk2C != NULL) {
                    request->unk2C(request);
                }
                break;
            }
            break;
        }
    } else if (status == CdlAcknowledge) {
        /* Reads the volatile state (retail keeps the load) and acts on no value. */
        switch (request->unk0) {
        }
    } else if (status == CdlDiskError) {
        switch (request->unk0) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->unk1 = STEP_SEEK;
            while (CdControl(CdlSeekL, (u_char *)&request->unk18, 0) == 0)
                ;
            break;
        }
    }
}

/* Retires a request and starts the next one in the ring, or pauses. */
ADDRESS(0x8002ea94, 0xac)
void func_8002ea94(StCdRequest *request)
{
    request->unk0 = 0;
    request->unk1 = STEP_SEEK;
    if (++request == &DAT_801a4590.unk8[VS_CD_REQUESTS])
        request -= VS_CD_REQUESTS;
    DAT_801a4590.unkDC = request;
    if (request->unk0 != 0) {
        CdReadyCallback(0);
        CdSyncCallback(func_8002e868);
        while (CdControl(CdlSeekL, (u_char *)&request->unk18, 0) == 0)
            ;
    } else {
        CdReadyCallback(0);
        CdSyncCallback(0);
        CdControl(CdlPause, 0, 0);
    }
}

ADDRESS(0x8002eb40, 0x20)
int func_8002eb40(u8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

ADDRESS(0x8002eb60, 0x3c)
int func_8002eb60(u8 value)
{
    return ((value / 10) << 4) | (value % 10);
}

/* Sector number of a CD position (no 150-sector lead-in offset). */
ADDRESS(0x8002eb9c, 0x84)
u32 func_8002eb9c(CdlLOC *pos)
{
    int minute = func_8002eb40(pos->minute);
    int second = func_8002eb40(pos->second);

    return minute * 60 * 75 + second * 75 + func_8002eb40(pos->sector);
}

/*
 * Writes a sector number as a BCD CD position. The stores go through a
 * volatile view: retail keeps the second-field store out of the last
 * call's delay slot (the ST.EXE twin func_800159d4 shows the same nop).
 */
ADDRESS(0x8002ec20, 0xc0)
void func_8002ec20(volatile CdlLOC *pos, u32 sector)
{
    pos->minute = func_8002eb60(sector / (60 * 75));
    sector %= 60 * 75;
    pos->second = func_8002eb60(sector / 75);
    pos->sector = func_8002eb60(sector % 75);
    pos->track = 0;
}

ADDRESS(0x8002ece0, 0x40)
void func_8002ece0(CdlLOC *pos, int offset, CdlLOC *out)
{
    func_8002ec20(out, func_8002eb9c(pos) + offset);
}

ADDRESS(0x8002ed20, 0x54)
int func_8002ed20(u32 *data, int sectors)
{
    u32 sum = 0x12345678;
    int last = (sectors << 9) - 1;
    int count = last;
    u32 *p = data;

    while (count--)
        sum += *p++;
    return sum != data[last];
}

/* Reads sectors at double speed until a read completes (no wait for the
 * request ring, unlike ST.EXE's copy). */
ADDRESS(0x8002ed74, 0xe0)
void func_8002ed74(CdlLOC *loc, u_long *buf, int sectors)
{
    u_char result[8];
    int n;

    result[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, result, 0);
    VSync(3);
    for (;;) {
        while (!CdControl(CdlSetloc, (u_char *)loc, 0))
            ;
        if (!CdRead(sectors, buf, CdlModeSpeed)) {
            CdSync(0, result);
            continue;
        }
        while ((n = CdReadSync(1, result)) > 0)
            func_8002e5e4();
        CdControl(CdlPause, 0, 0);
        if (n == 0)
            break;
    }
}

ADDRESS(0x8002ee54, 0x3c)
void func_8002ee54(u_long *buf, StFileExtent *file, u32 size)
{
    if (size == 0)
        size = file->size;
    func_8002ed74(&file->pos, buf, (size + 2047) >> 11);
}

/* Reads an archive member whole, rereading until its checksum holds;
 * returns its sector count (0 for an empty member, which VS skips). */
ADDRESS(0x8002ee90, 0xac)
int func_8002ee90(u16 archive, u16 member, u_long *buf)
{
    VsArchive *a = &DAT_801a4590.archives[archive];
    u16 start = a->sectors[member];
    CdlLOC loc;
    int sectors = -start + a->sectors[member + 1];

    if (sectors == 0)
        return 0;
    func_8002ece0(&a->file.pos, start, &loc);
    do
        func_8002ed74(&loc, buf, sectors);
    while (func_8002ed20(buf, sectors));
    return sectors;
}

/*
 * Reads file `name` whole (or its first `size` bytes) into buf, pausing
 * the drive until the search succeeds; returns 0.
 */
ADDRESS(0x8002ef3c, 0x90)
int func_8002ef3c(u_long *buf, char *name, u32 size)
{
    char path[64]; /* unused */
    CdlFILE file;
    u_char param[8]; /* unused */
    CdlFILE *entry = &file;

retry:
    if (!CdSearchFile(entry, name)) {
        CdControl(CdlPause, 0, 0);
        goto retry;
    }
    if (size == 0)
        size = file.size;
    func_8002ed74(&entry->pos, buf, (size + 2047) >> 11);
    return 0;
}
