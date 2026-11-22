/*
 * ST.EXE: CD position offset and the request-ring waits (WIP module). Split
 * from src/st/cdreq.c while cd_sector_to_loc there was not exact.
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
#include <st/funcs.h>

ADDRESS(0x80015a94, 0x40)
void cd_loc_add_sectors(CdlLOC *pos, int offset, CdlLOC *out)
{
    cd_sector_to_loc(out, cd_loc_to_sector(pos) + offset);
}

/* Waits until the request ring is empty. */
ADDRESS(0x80015ad4, 0x58)
void cd_wait_idle(void)
{
    while (cd_state.current != cd_state.next_free || cd_state.current->kind != 0)
        cd_wait_frame();
}

ADDRESS(0x80015b2c, 0x4c)
void cd_request_wait_free(StCdRequest *request)
{
    while (request->kind)
        cd_wait_frame();
}
