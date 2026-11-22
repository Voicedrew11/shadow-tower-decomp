/*
 * VS.EXE: the frame wait, the CD read-request ring callbacks, the request
 * queue and archive member lookup, the frame buffer swap and the VRAM
 * clear (WIP module). VS's copies of ST.EXE functions from src/st/cdreq.c,
 * cdpos.c, cd.c, gfx.c and ui.c; VS keeps a second copy of the ring
 * callbacks in src/vs/cd_read.c.
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

/* The frame wait (ST.EXE's cd_wait_frame also steps sound, images and the
 * loading display). */
ADDRESS(0x8001cb34, 0x28)
void cd_wait_frame(void)
{
    DrawSync(0);
    VSync(0);
}

ADDRESS(0x8001cb5c, 0xbc)
void cd_request_read_callback(u_char status, u_char *result)
{
    StCdRequest *request = vs_cd_state.unkDC;
    u_char param[8]; /* unused */

    if (status == CdlComplete) {
        switch (request->kind) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->sectors_left = 0;
            CdReadCallback(0);
            request->step = STEP_DONE;
            break;
        }
    } else if (status == CdlDiskError) {
        switch (request->kind) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->sectors_left = -1;
            CdReadCallback(0);
            request->step = STEP_DONE;
            break;
        }
    }
}

/* CdReadyCallback of a read: copies each sector's data into the request,
 * then ends the read with CdlNop polling. */
ADDRESS(0x8001cc18, 0x148)
void cd_queue_ready_callback(u_char status, u_char *result)
{
    StCdRequest *request = vs_cd_state.unkDC;
    u_char param[8]; /* unused */
    u_long rest[70];

    if (status == CdlDataReady) {
        switch (request->kind) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            while (CdGetSector(request->read_cursor, 3) == 0)
                ;
            while (CdGetSector(request->read_cursor, 512) == 0)
                ;
            request->read_cursor += 2048;
            while (CdGetSector(rest, 70) == 0)
                ;
            if (--request->sectors_left == 0) {
                CdReadyCallback(0);
                request->step = STEP_DONE;
                while (CdControlF(CdlNop, 0) == 0)
                    ;
            }
            break;
        }
    } else if (status == CdlDiskError) {
        switch (request->kind) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->sectors_left = -1;
            CdReadyCallback(0);
            request->step = STEP_DONE;
            while (CdControlF(CdlNop, 0) == 0)
                ;
            break;
        }
    }
}

/* CdSyncCallback of the request ring: seek, read, check, finish. */
ADDRESS(0x8001cd60, 0x22c)
void cd_queue_sync_callback(u_char status, u_char *result)
{
    StCdRequest *request = vs_cd_state.unkDC;

    if (status == CdlComplete) {
        switch (request->kind) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            switch (request->step) {
            case STEP_SEEK:
                request->read_loc = request->loc;
                request->step = STEP_READ;
                request->sectors_left = request->sectors;
                request->read_cursor = request->dest;
                CdReadyCallback(cd_queue_ready_callback);
                CdSyncCallback(cd_queue_sync_callback);
                while (CdControlF(CdlReadN, 0) == 0)
                    ;
                break;
            case STEP_READ:
                break;
            case STEP_DONE:
                if (request->kind == 2) {
                    if (request->sectors_left != 0) {
                        request->step = STEP_SEEK;
                        while (CdControl(CdlSeekL, (u_char *)&request->loc, 0) == 0)
                            ;
                    } else if (cd_checksum_mismatch((u32 *)request->dest, request->sectors) != 0) {
                        request->step = STEP_SEEK;
                        while (CdControl(CdlSeekL, (u_char *)&request->loc, 0) == 0)
                            ;
                    } else {
                        if (request->done != NULL)
                            request->done(request->dest);
                        cd_request_retire(request);
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
        switch (request->kind) {
        }
    } else if (status == CdlDiskError) {
        switch (request->kind) {
        case 0:
            break;
        case 2:
        case 3:
        case 4:
            request->step = STEP_SEEK;
            while (CdControl(CdlSeekL, (u_char *)&request->loc, 0) == 0)
                ;
            break;
        }
    }
}

/* Waits until the request ring is empty. */
ADDRESS(0x8001cf8c, 0x58)
void cd_wait_idle(void)
{
    while (vs_cd_state.unkDC != vs_cd_state.unkD8 || vs_cd_state.unkDC->kind != 0)
        cd_wait_frame();
}

ADDRESS(0x8001cfe4, 0x4c)
void cd_request_wait_free(StCdRequest *request)
{
    while (request->kind)
        cd_wait_frame();
}

ADDRESS(0x8001d030, 0x104)
StCdRequest *cd_request_queue(int kind, CdlLOC *loc, u32 size, u8 *dest, void (*done)())
{
    StCdRequest *req = vs_cd_state.unkD8;

    cd_request_wait_free(req);
    req->kind = kind;
    req->step = 0;
    req->loc = *loc;
    req->dest = dest;
    req->sectors = (size + 2047) >> 11;
    req->done = done;
    if (++vs_cd_state.unkD8 == &vs_cd_state.unk8[VS_CD_REQUESTS])
        vs_cd_state.unkD8 = vs_cd_state.unk8;
    if (vs_cd_state.unkDC == req) {
        CdReadyCallback(0);
        CdSyncCallback(cd_queue_sync_callback);
        while (!CdControl(CdlSeekL, (u_char *)loc, 0))
            ;
    }
    return req;
}

ADDRESS(0x8001d134, 0x84)
int cd_archive_member_locate(int archive, int member, CdlLOC *loc)
{
    VsArchive *a = &vs_cd_state.archives[archive];
    u16 start = a->sectors[member];

    if (loc)
        cd_loc_add_sectors(&a->file.pos, start, loc);
    return (a->sectors[member + 1] - start) << 11;
}

ADDRESS(0x8001d1b8, 0xe0)
void gfx_begin_frame(void)
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
void gfx_flip(void)
{
    DrawSync(0);
    VSync(3);
    PutDispEnv(&vs_gfx_state.disp[vs_gfx_state.frame]);
    PutDrawEnv(&vs_gfx_state.draw[vs_gfx_state.frame]);
}

ADDRESS(0x8001d328, 0x4c)
void gfx_draw_ot(void)
{
    AddPrims(vs_gfx_state.ot_cur, &vs_gfx_state.ot2_cur[VS_OT2_LENGTH - 1],
             vs_gfx_state.ot2_cur);
    DrawOTag(&vs_gfx_state.ot_cur[VS_OT_LENGTH - 1]);
}

ADDRESS(0x8001d374, 0x28)
void gfx_end_frame(void)
{
    gfx_flip();
    gfx_draw_ot();
}

/* Clears the 640x480 VRAM area from the origin to black. */
ADDRESS(0x8001d39c, 0x54)
void clear_vram(void)
{
    RECT r = {0, 0, 640, 480};

    ClearImage(&r, 0, 0, 0);
}
