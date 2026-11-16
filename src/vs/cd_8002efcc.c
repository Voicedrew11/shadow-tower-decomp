/*
 * VS.EXE: stopping the drive (WIP module). VS's copy of ST.EXE's
 * func_8001624c (src/st/cd.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBETC.H>
#include <LIBCD.H>

ADDRESS(0x8002efcc, 0x48)
void func_8002efcc(void)
{
    u_char param[16];

    CdControl(CdlStop, 0, 0);
    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    VSync(4);
}
