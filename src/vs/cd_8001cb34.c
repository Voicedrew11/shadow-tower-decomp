/*
 * VS.EXE: the frame wait, the CD read-request ring callbacks, the request
 * queue and archive member lookup, the frame buffer swap and the VRAM
 * clear (WIP module). VS's copies of ST.EXE functions from src/st/cdreq.c,
 * cdpos.c, cd.c, gfx.c and ui.c; VS keeps a second copy of the ring
 * callbacks in src/vs/cd_8002e5e4.c.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <st/cd.h>
#include <vs/data.h>
#include <vs/funcs.h>

/* unk1 of the current request: the step its read is at. */
#define STEP_SEEK 0
#define STEP_READ 1
#define STEP_DONE 2

/* The VRAM clear's RECT initializer: an 8-byte constant, so .sdata under -G8. */
SDATA(0x80063258, 0x8)

/* The frame wait (ST.EXE's func_8001531c also steps sound, images and the
 * loading display). */
ADDRESS(0x8001cb34, 0x28)
void func_8001cb34(void)
{
    DrawSync(0);
    VSync(0);
}

ADDRESS(0x8001cb5c, 0xbc)
void func_8001cb5c(u_char status, u_char *result)
{
    StCdRequest *request = DAT_801a4590.unkDC;
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

/* CdReadyCallback of a read: copies each sector's data into the request,
 * then ends the read with CdlNop polling. */
ADDRESS(0x8001cc18, 0x148)
void func_8001cc18(u_char status, u_char *result)
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
ADDRESS(0x8001cd60, 0x22c)
void func_8001cd60(u_char status, u_char *result)
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
                CdReadyCallback(func_8001cc18);
                CdSyncCallback(func_8001cd60);
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
                        if (request->done != NULL)
                            request->done(request->unk20);
                        func_8002ea94(request);
                    }
                } else if (request->done != NULL) {
                    request->done(request);
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

/* Waits until the request ring is empty. */
ADDRESS(0x8001cf8c, 0x58)
void func_8001cf8c(void)
{
    while (DAT_801a4590.unkDC != DAT_801a4590.unkD8 || DAT_801a4590.unkDC->unk0 != 0)
        func_8001cb34();
}

ADDRESS(0x8001cfe4, 0x4c)
void func_8001cfe4(StCdRequest *request)
{
    while (request->unk0)
        func_8001cb34();
}

ADDRESS(0x8001d030, 0x104)
StCdRequest *func_8001d030(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)())
{
    StCdRequest *req = DAT_801a4590.unkD8;

    func_8001cfe4(req);
    req->unk0 = kind;
    req->unk1 = 0;
    req->unk18 = *loc;
    req->unk20 = dest;
    req->unk28 = (size + 2047) >> 11;
    req->done = done;
    if (++DAT_801a4590.unkD8 == &DAT_801a4590.unk8[VS_CD_REQUESTS])
        DAT_801a4590.unkD8 = DAT_801a4590.unk8;
    if (DAT_801a4590.unkDC == req) {
        CdReadyCallback(0);
        CdSyncCallback(func_8001cd60);
        while (!CdControl(CdlSeekL, (u_char *)loc, 0))
            ;
    }
    return req;
}

ADDRESS(0x8001d134, 0x84)
int func_8001d134(int archive, int member, CdlLOC *loc)
{
    VsArchive *a = &DAT_801a4590.archives[archive];
    u16 start = a->sectors[member];

    if (loc)
        func_8002ece0(&a->file.pos, start, loc);
    return (a->sectors[member + 1] - start) << 11;
}

ADDRESS(0x8001d1b8, 0xe0)
void func_8001d1b8(void)
{
    vs_gfx_state.frame = !vs_gfx_state.frame;
    vs_gfx_state.packet = &vs_gfx_state.packets[vs_gfx_state.frame];
    vs_gfx_state.packet->cur = vs_gfx_state.packet->start;
    vs_gfx_state.ot_cur = vs_gfx_state.ot[vs_gfx_state.frame];
    ClearOTagR(vs_gfx_state.ot_cur, VS_OT_LENGTH);
    vs_gfx_state.ot2_cur = vs_gfx_state.ot2[vs_gfx_state.frame];
    ClearOTagR(vs_gfx_state.ot2_cur, VS_OT2_LENGTH);
    vs_gfx_state.unk831C[0] = vs_gfx_state.unk831C[1] =
        vs_gfx_state.unk8324 = vs_gfx_state.unk8328 = 0;
    vs_gfx_state.unk832C = 0;
    vs_gfx_state.unk8330 = 0;
    vs_gfx_state.unk8338[0] = 0;
    vs_gfx_state.unk8338[1] = 0;
    vs_gfx_state.unk8338[2] = 0;
    vs_gfx_state.unk8338[3] = 0;
    vs_gfx_state.unk8338[4] = 0;
}

ADDRESS(0x8001d298, 0x90)
void func_8001d298(void)
{
    DrawSync(0);
    VSync(3);
    PutDispEnv(&vs_gfx_state.disp[vs_gfx_state.frame]);
    PutDrawEnv(&vs_gfx_state.draw[vs_gfx_state.frame]);
}

ADDRESS(0x8001d328, 0x4c)
void func_8001d328(void)
{
    AddPrims(vs_gfx_state.ot_cur, &vs_gfx_state.ot2_cur[VS_OT2_LENGTH - 1],
             vs_gfx_state.ot2_cur);
    DrawOTag(&vs_gfx_state.ot_cur[VS_OT_LENGTH - 1]);
}

ADDRESS(0x8001d374, 0x28)
void func_8001d374(void)
{
    func_8001d298();
    func_8001d328();
}

/* Clears the 640x480 VRAM area from the origin to black. */
ADDRESS(0x8001d39c, 0x54)
void func_8001d39c(void)
{
    RECT r = {0, 0, 640, 480};

    ClearImage(&r, 0, 0, 0);
}
