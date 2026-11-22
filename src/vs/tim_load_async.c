/*
 * VS.EXE: an empty function, the CD queue's busy test and a still picture
 * loaded by a CD read whose completion callback uploads it (WIP module).
 * -G8: the picture position words at 0x80063304/0x80063308 are stored
 * through ASPSX's $at expansion of a symbolic store.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>
#include <vs/data.h>
#include <vs/funcs.h>
#include <vs/buffer.h>

ADDRESS(0x8001ca10, 0x8)
void func_8001ca10(void)
{
}

/* Nonzero while the CD request queue holds or runs a request. */
ADDRESS(0x8001ca18, 0x5c)
int cd_queue_busy(void)
{
    u8 busy;

    EnterCriticalSection();
    busy = 1;
    if (vs_cd_state.current == vs_cd_state.next_free)
        busy = vs_cd_state.current->kind != 0;
    ExitCriticalSection();
    return busy;
}

/* The picture read's completion callback: uploads the TIM images from the
 * read buffer to the saved position. */
ADDRESS(0x8001ca74, 0x4c)
void tim_load_member_done(void)
{
    DVECTOR pos;

    pos.vx = DAT_80063304;
    pos.vy = DAT_80063308;
    tim_load_images_at((u_long *)&shared_buffer, &pos, NULL);
    DrawSync(0);
}

/* Queues the read of archive member `member` into the read buffer, to be
 * uploaded at (x, y) when it completes. */
ADDRESS(0x8001cac0, 0x74)
void tim_load_member_async(u16 archive, u16 member, s32 x, s32 y)
{
    CdlLOC loc;
    int size;

    size = cd_archive_member_locate(archive, member, &loc);
    DAT_80063304 = x;
    DAT_80063308 = y;
    cd_request_queue(2, &loc, size, (u8 *)&shared_buffer, tim_load_member_done);
}
