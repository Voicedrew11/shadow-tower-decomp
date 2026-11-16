/*
 * ST.EXE: the frame wait, the vsync counters, the CD read-request ring
 * callbacks and BCD/CD position helpers (WIP module). The module ends at
 * func_800159d4, which is not exact, so that the functions after it keep
 * their retail offsets in their own unit (src/st/cdpos.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <st/state.h>
#include <st/data.h>
#include <st/cdreq.h>
#include <st/funcs.h>

/* unk1 of the current request: the step its read is at. */
#define STEP_SEEK 0
#define STEP_READ 1
#define STEP_DONE 2

ADDRESS(0x8001531c, 0x40)
void func_8001531c(void)
{
    func_80058620();
    func_80045ffc();
    func_800214cc();
    DrawSync(0);
    VSync(0);
}

/* Waits for the next vsync tick, then consumes it. */
ADDRESS(0x8001535c, 0x58)
void func_8001535c(void)
{
    while (DAT_80088200.unkC == 0)
        VSync(0);
    DAT_80088200.unkC = 0;
}

ADDRESS(0x800153b4, 0x78)
void func_800153b4(void)
{
    DAT_80088200.unk4++;
    DAT_80088200.unkC++;
    if (++DAT_80198f28.unk33 == 60) {
        DAT_80198f28.unk33 = 0;
        DAT_80198f28.unk34++;
    }
    func_80057744();
}

ADDRESS(0x8001542c, 0xbc)
void func_8001542c(u_char status, u_char *result)
{
    StCdRequest *request = DAT_80088200.unk9D8;
    u_char param[8]; /* unused */

    if (status == CdlComplete) {
        switch (request->unk0) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->unk24 = 0;
            CdReadCallback(0);
            request->unk1 = STEP_DONE;
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
            CdReadCallback(0);
            request->unk1 = STEP_DONE;
            break;
        }
    }
}

/* CdReadyCallback of a read: copies each sector's data into the request. */
ADDRESS(0x800154e8, 0x134)
void func_800154e8(u_char status, u_char *result)
{
    StCdRequest *request = DAT_80088200.unk9D8;
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
                func_8001561c(CdlComplete, 0);
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
            func_8001561c(CdlComplete, 0);
            break;
        }
    }
}

/* CdSyncCallback of the request ring: seek, read, check, finish. */
ADDRESS(0x8001561c, 0x22c)
void func_8001561c(u_char status, u_char *result)
{
    StCdRequest *request = DAT_80088200.unk9D8;

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
                CdReadyCallback(func_800154e8);
                CdSyncCallback(func_8001561c);
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
                    } else if (func_80015b78(request->unk20, request->unk28) != 0) {
                        request->unk1 = STEP_SEEK;
                        while (CdControl(CdlSeekL, (u_char *)&request->unk18, 0) == 0)
                            ;
                    } else {
                        if (request->unk2C != NULL)
                            request->unk2C(request->unk20);
                        func_80015848(request);
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
ADDRESS(0x80015848, 0xac)
void func_80015848(StCdRequest *request)
{
    request->unk0 = 0;
    request->unk1 = STEP_SEEK;
    if (++request == &DAT_80088200.unk14[ST_CD_REQUESTS])
        request -= ST_CD_REQUESTS;
    DAT_80088200.unk9D8 = request;
    if (request->unk0 != 0) {
        CdReadyCallback(0);
        CdSyncCallback(func_8001561c);
        while (CdControl(CdlSeekL, (u_char *)&request->unk18, 0) == 0)
            ;
    } else {
        CdReadyCallback(0);
        CdSyncCallback(0);
        CdControl(CdlPause, 0, 0);
    }
}

ADDRESS(0x800158f4, 0x20)
int func_800158f4(u8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

ADDRESS(0x80015914, 0x3c)
int func_80015914(u8 value)
{
    return ((value / 10) << 4) | (value % 10);
}

/* Sector number of a CD position (no 150-sector lead-in offset). */
ADDRESS(0x80015950, 0x84)
u32 func_80015950(CdlLOC *pos)
{
    int minute = func_800158f4(pos->minute);
    int second = func_800158f4(pos->second);

    return minute * 60 * 75 + second * 75 + func_800158f4(pos->sector);
}

ADDRESS(0x800159d4, 0xc0)
void func_800159d4(CdlLOC *pos, u32 sector)
{
    pos->minute = func_80015914(sector / (60 * 75));
    sector %= 60 * 75;
    pos->second = func_80015914(sector / 75);
    sector %= 75;
    pos->sector = func_80015914(sector);
    pos->track = 0;
}
