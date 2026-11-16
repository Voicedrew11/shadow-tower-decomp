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
int func_8001ca18(void)
{
    u8 busy;

    EnterCriticalSection();
    busy = 1;
    if (DAT_801a4590.unkDC == DAT_801a4590.unkD8)
        busy = DAT_801a4590.unkDC->unk0 != 0;
    ExitCriticalSection();
    return busy;
}

/* The picture read's completion callback: uploads the TIM images from the
 * read buffer to the saved position. */
ADDRESS(0x8001ca74, 0x4c)
void func_8001ca74(void)
{
    DVECTOR pos;

    pos.vx = DAT_80063304;
    pos.vy = DAT_80063308;
    func_80012c2c((u_long *)&DAT_800633b0, &pos, NULL);
    DrawSync(0);
}

/* Queues the read of archive member `member` into the read buffer, to be
 * uploaded at (x, y) when it completes. */
ADDRESS(0x8001cac0, 0x74)
void func_8001cac0(u16 archive, u16 member, s32 x, s32 y)
{
    CdlLOC loc;
    int size;

    size = func_8001d134(archive, member, &loc);
    DAT_80063304 = x;
    DAT_80063308 = y;
    func_8001d030(2, &loc, size, (u8 *)&DAT_800633b0, func_8001ca74);
}
