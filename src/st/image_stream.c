/*
 * ST.EXE: reading an area file into the small-file buffer, and the CD
 * completion callback that streams a read buffer of image blocks into VRAM
 * (WIP module). It is its own object, apart from st.area_load: retail places
 * its .sdata vector 0x8008809c right after st.area_load's SDATA, and it can be
 * neither st.area_load's own nor st.math's, which reads it with a lui pair (a
 * -G8 unit reaches its own small data through $gp).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/area.h>
#include <st/funcs.h>

/* The unit z vector vector_from_angles_zyx turns (src/st/math.c). */
DATA(0x8008809c, 0x8, ".sdata")
SVECTOR unit_vector_z = {0, 0, 4096, 0};

ADDRESS(0x80045f90, 0x28)
void image_stream_queue(int archive, int member, void (*done)())
{
    cd_queue_stream_member(archive, member, (u8 *)cd_file_buffer, done);
}

/* Takes the next run of at most 16 sectors from the request's count. */
ADDRESS(0x80045fb8, 0x44)
void image_stream_next_run(StCdRequest *request)
{
    int n = request->stream_left;

    if (n > 16) {
        request->sectors_left = 16;
        request->sectors = 16;
        request->stream_left -= 16;
    } else {
        request->sectors_left = n;
        request->sectors = n;
        request->stream_left = 0;
    }
}

/*
 * The current request's 32 KiB buffer holds image blocks: an 8-halfword
 * header (the RECT twice) and its pixels; a header of -1 ends the file.
 * Loads what fits, keeps a partial image for the next buffer, then seeks
 * the next run.
 */
ADDRESS(0x80045ffc, 0x274)
void image_stream_update(void)
{
    StCdRequest *request = cd_state.current;
    u16 *p;
    int used;
    int left;
    int n;
    u16 h;
    s16 rows;

    if (request->kind != CD_REQUEST_STREAM || request->status != 1)
        return;
    used = 0;
    p = (u16 *)request->dest;
    request->status = 0;
    for (;;) {
        left = 0x8000 - used;
        h = request->payload.image.h;
        if (request->payload.image.h != 0) {
            n = request->payload.image.h * request->payload.image.w;
            if (left < n * 2) {
                rows = left / request->payload.image.w >> 1;
                if (rows != 0) {
                    request->payload.image.h = rows;
                    LoadImage(&request->payload.image, (u_long *)p);
                    request->payload.image.h = h - rows;
                    request->payload.image.y += rows;
                }
                break;
            }
            request->payload.image.h = h;
            LoadImage(&request->payload.image, (u_long *)p);
            used += n * 2;
            p += n;
            request->payload.image.h = 0;
            continue;
        }
        if (left < 16)
            break;
        if (p[0] != p[4] || p[1] != p[5] || p[2] != p[6] || p[3] != p[7] || p[2] == 0 ||
            p[3] == 0) {
            request->payload.image.h = 0;
            request->status = 0;
            request->stream_left = request->stream_sectors;
            image_stream_next_run(request);
            request->step = 0;
            while (!CdControl(CdlSeekL, (u_char *)&request->loc, 0))
                ;
            return;
        }
        used += 16;
        if (p[0] == 0xffff) {
            DrawSync(0);
            request->status = 2;
            request->sectors_left = 0;
            request->done(request);
            cd_request_retire(request);
            return;
        }
        request->payload.image.x = p[0];
        request->payload.image.y = p[1];
        request->payload.image.w = p[2];
        request->payload.image.h = p[3];
        p += 8;
    }
    cd_loc_add_sectors(&request->loc, request->sectors, &request->loc);
    image_stream_next_run(request);
    request->step = 0;
    DrawSync(0);
    while (!CdControl(CdlSeekL, (u_char *)&request->loc, 0))
        ;
}
