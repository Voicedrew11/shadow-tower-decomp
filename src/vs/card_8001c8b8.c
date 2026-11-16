/*
 * VS.EXE: the menu sound effects and their bank (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBSND.H>
#include <vs/sound.h>

ADDRESS(0x8001c8b8, 0x40)
void func_8001c8b8(void)
{
    SsUtKeyOn(DAT_8006335a, 0, 11, 60, 0, 128, 128);
}

ADDRESS(0x8001c8f8, 0x40)
void func_8001c8f8(void)
{
    SsUtKeyOn(DAT_8006335a, 0, 13, 60, 0, 128, 128);
}

ADDRESS(0x8001c938, 0x40)
void func_8001c938(void)
{
    SsUtKeyOn(DAT_8006335a, 0, 14, 60, 0, 128, 128);
}

ADDRESS(0x8001c978, 0x40)
void func_8001c978(void)
{
    SsUtKeyOn(DAT_8006335a, 0, 12, 60, 0, 128, 128);
}

/* Opens the menu sound bank and transfers its body to SPU memory. */
ADDRESS(0x8001c9b8, 0x58)
void func_8001c9b8(void)
{
    SsVabTransBody(DAT_800ff580, DAT_8006335a = SsVabOpenHead(DAT_80116580, -1));
    SsVabTransCompleted(SS_WAIT_COMPLETED);
}
