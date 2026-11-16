/*
 * VS.EXE: the sound system start-up and a sound effect key-on (WIP
 * module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBSPU.H>
#include <LIBSND.H>
#include <vs/sound.h>

/* Starts libsnd: the SEQ attribute table, 60 Hz ticks, studio reverb at
 * depth 40, CD audio mixed in, full master volume. */
ADDRESS(0x80013aec, 0x88)
void func_80013aec(void)
{
    SsInit();
    SsSetMVol(0, 0);
    SsSetTableSize(DAT_801175a8, 2, 1);
    SsSetTickMode(SS_TICK60);
    SsStart2();
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(40, 40);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetMVol(127, 127);
}

/* Keys on `tone` of sound bank `bank` at `note` (clamped to 0-127) when a
 * voice is free; returns the voice SsUtKeyOn picked, or -1. The scan reads
 * the states one entry low (status[22] down to status[-1]), as retail does. */
ADDRESS(0x80013b74, 0xe8)
short func_80013b74(u_char tone, int note, int bank)
{
    int i;

    if (note < 0)
        note = 0;
    else if (note >= 128)
        note = 127;
    SpuGetAllKeysStatus(DAT_80117580);
    for (i = 23; i != -1; i--) {
        if (DAT_80117580[i - 1] == SPU_OFF)
            return SsUtKeyOn(DAT_801604b0[bank].vab, 0, tone, note, 0, 32, 32);
    }
    return -1;
}
