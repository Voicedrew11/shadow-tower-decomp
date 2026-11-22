/*
 * ST.EXE: the frame wait, the vsync counters, the CD read-request ring
 * callbacks and BCD/CD position helpers (WIP module). src/st/cdpos.c
 * continues it: the two were split while cd_sector_to_loc was not exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/funcs.h>

/* unk1 of the current request: the step its read is at. */
#define STEP_SEEK 0
#define STEP_READ 1
#define STEP_DONE 2

ADDRESS(0x8001531c, 0x40)
void cd_wait_frame(void)
{
    sound_bank_transfer_body();
    image_stream_update();
    loading_screen_step();
    DrawSync(0);
    VSync(0);
}

/* Waits for the next vsync tick, then consumes it. */
ADDRESS(0x8001535c, 0x58)
void vsync_wait_tick(void)
{
    while (cd_state.pending_ticks == 0)
        VSync(0);
    cd_state.pending_ticks = 0;
}

ADDRESS(0x800153b4, 0x78)
void vsync_handler(void)
{
    cd_state.vsync_count++;
    cd_state.pending_ticks++;
    if (++viewer.play_frames == 60) {
        viewer.play_frames = 0;
        viewer.play_seconds++;
    }
    pad_read();
}

ADDRESS(0x8001542c, 0xbc)
void cd_request_read_callback(u_char status, u_char *result)
{
    StCdRequest *request = cd_state.current;
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

/* CdReadyCallback of a read: copies each sector's data into the request. */
ADDRESS(0x800154e8, 0x134)
void cd_request_ready_callback(u_char status, u_char *result)
{
    StCdRequest *request = cd_state.current;
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
                cd_request_sync_callback(CdlComplete, 0);
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
            cd_request_sync_callback(CdlComplete, 0);
            break;
        }
    }
}

/* CdSyncCallback of the request ring: seek, read, check, finish. */
ADDRESS(0x8001561c, 0x22c)
void cd_request_sync_callback(u_char status, u_char *result)
{
    StCdRequest *request = cd_state.current;

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
                CdReadyCallback(cd_request_ready_callback);
                CdSyncCallback(cd_request_sync_callback);
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

/* Retires a request and starts the next one in the ring, or pauses. */
ADDRESS(0x80015848, 0xac)
void cd_request_retire(StCdRequest *request)
{
    request->kind = 0;
    request->step = STEP_SEEK;
    if (++request == &cd_state.requests[ST_CD_REQUESTS])
        request -= ST_CD_REQUESTS;
    cd_state.current = request;
    if (request->kind != 0) {
        CdReadyCallback(0);
        CdSyncCallback(cd_request_sync_callback);
        while (CdControl(CdlSeekL, (u_char *)&request->loc, 0) == 0)
            ;
    } else {
        CdReadyCallback(0);
        CdSyncCallback(0);
        CdControl(CdlPause, 0, 0);
    }
}

ADDRESS(0x800158f4, 0x20)
int bcd_to_int(u8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

ADDRESS(0x80015914, 0x3c)
int int_to_bcd(u8 value)
{
    return ((value / 10) << 4) | (value % 10);
}

/* Sector number of a CD position (no 150-sector lead-in offset). */
ADDRESS(0x80015950, 0x84)
u32 cd_loc_to_sector(CdlLOC *pos)
{
    int minute = bcd_to_int(pos->minute);
    int second = bcd_to_int(pos->second);

    return minute * 60 * 75 + second * 75 + bcd_to_int(pos->sector);
}

/* Writes a sector count as a BCD CD position without a lead-in offset.
 * The position is volatile (the CD callbacks read the ones this fills):
 * retail leaves the last call's delay slot empty behind the store of the
 * second, which reorg only does for a volatile store. */
ADDRESS(0x800159d4, 0xc0)
void cd_sector_to_loc(volatile CdlLOC *pos, u32 sector)
{
    pos->minute = int_to_bcd(sector / (60 * 75));
    sector %= 60 * 75;
    pos->second = int_to_bcd(sector / 75);
    sector %= 75;
    pos->sector = int_to_bcd(sector);
    pos->track = 0;
}
