/*
 * VS.EXE: the CD mixing volume (WIP module). The XA channel selection
 * that follows (func_800129e8) shares its translation unit's constant
 * pool with func_80014070 (strings from 0x80011000), so it waits for that
 * run to be claimed.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBSND.H>

/* Sets the CD (serial A) mixing volume, both channels. */
ADDRESS(0x800129bc, 0x2c)
void func_800129bc(short volume)
{
    SsSetSerialVol(SS_SERIAL_A, volume, volume);
}
