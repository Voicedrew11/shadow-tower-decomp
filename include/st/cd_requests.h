/*
 * StCdRequest.kind (st/cd.h): what cd_request_queue was asked for, as the
 * request ring's callbacks (src/st/cdreq.c, VS.EXE's ports) treat it.
 */
#ifndef ST_CD_REQUESTS_H
#define ST_CD_REQUESTS_H

#define CD_REQUEST_NONE 0      /* a free ring slot */
#define CD_REQUEST_READ 2      /* re-read until the checksum holds; done(dest), then retired */
#define CD_REQUEST_STREAM 3    /* cd_queue_stream_member: done(request) per run */
#define CD_REQUEST_READ_UNCHECKED 4 /* cd_queue_read_member_unchecked: done(request) */

#endif
