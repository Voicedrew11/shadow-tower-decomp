/*
 * ST.EXE: the sound module (WIP, 0x80057b48..0x8005850c): the fixed sound
 * effects, the sound system's start and stop, the positional and plain
 * sound players over the sound state block (DAT_801dd1c0). Its key-off and
 * VAB bank loads continue in src/st/sound_bank.c.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>
#include <LIBSPU.H>
#include <LIBSND.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/sound.h>

/* Plays the hit sound for damage above 256 and shakes the pad by it. */
ADDRESS(0x80057b48, 0x64)
void func_80057b48(int damage)
{
    if (damage >= 256) {
        damage -= 256;
        if (damage > 512)
            damage = 512;
        func_800582f0(20, (damage >> 3) + 64);
        func_80057a68(damage / 4);
    }
}

/* Plays the footstep sound of the floor the viewer stands on. */
ADDRESS(0x80057bac, 0x60)
void func_80057bac(int volume)
{
    int flags = vis_lists.unk47DC;
    int id;

    if (flags & 0x34)
        id = 19;
    else if ((flags & 3) == 1)
        id = 17;
    else if ((flags & 3) == 2)
        id = 18;
    else
        id = 17;
    func_800582f0(id, (volume >> 2) + 64);
}

ADDRESS(0x80057c0c, 0x24)
void func_80057c0c(void)
{
    func_800582f0(3, 128);
}

ADDRESS(0x80057c30, 0x24)
void func_80057c30(void)
{
    func_800582f0(2, 128);
}

ADDRESS(0x80057c54, 0x24)
void func_80057c54(void)
{
    func_800582f0(10, 128);
}

ADDRESS(0x80057c78, 0x2c)
void func_80057c78(void)
{
    func_80058318(10, 128, 128, 50);
}

ADDRESS(0x80057ca4, 0x24)
void func_80057ca4(void)
{
    func_800582f0(21, 128);
}

ADDRESS(0x80057cc8, 0x24)
void func_80057cc8(void)
{
    func_800582f0(12, 128);
}

ADDRESS(0x80057cec, 0x24)
void func_80057cec(void)
{
    func_800582f0(13, 128);
}

ADDRESS(0x80057d10, 0x24)
void func_80057d10(void)
{
    func_800582f0(11, 128);
}

ADDRESS(0x80057d34, 0x24)
void func_80057d34(void)
{
    func_800582f0(14, 128);
}

ADDRESS(0x80057d58, 0x24)
void func_80057d58(void)
{
    func_800582f0(16, 128);
}

ADDRESS(0x80057d7c, 0x24)
void func_80057d7c(void)
{
    func_800582f0(15, 128);
}

/* Starts the sound system and resets the bank slots, voices and buffers. */
ADDRESS(0x80057da0, 0x19c)
void func_80057da0(void)
{
    StVabSlot *slot;
    StSoundBuf *buf;
    char *key;
    s16 *voice;
    int i;

    SsInit();
    SsSetMVol(0, 0);
    SsSetTickMode(1);
    SsStart2();
    SsUtSetReverbType(4);
    SsUtReverbOn();
    SsUtSetReverbDepth(20, 20);
    SsSetSerialAttr(0, 0, 1);
    DAT_801dd1c0.unk0 = 0;
    slot = DAT_801dd1c0.slots;
    i = 322;
    while (i--) {
        slot->vab = -1;
        slot->block = NULL;
        slot++;
    }
    key = DAT_801dd1c0.keys;
    voice = DAT_801dd1c0.voices;
    i = 24;
    while (i--) {
        *key++ = 0;
        *voice++ = -1;
    }
    buf = &DAT_801dd1c0.bufs[0];
    for (i = 0; i < 2; i++) {
        buf->state = 0;
        buf->data = DAT_801cd1c0[i];
        buf++;
    }
    buf = &DAT_801dd1c0.bufs[2];
    for (i = 0; i < 4; i++) {
        buf->state = 0;
        buf->data = DAT_801d41c0[i];
        buf++;
    }
    buf = &DAT_801dd1c0.bufs[6];
    for (i = 0; i < 4; i++) {
        buf->state = 0;
        buf->data = DAT_801d01c0[i];
        buf++;
    }
    buf = &DAT_801dd1c0.bufs[10];
    for (i = 0; i < 4; i++) {
        buf->state = 0;
        buf->data = DAT_801d91c0[i];
        buf++;
    }
    DAT_801dd1c0.bufs[14].data = DAT_801cf1c0;
    DAT_801dd1c0.bufs[15].data = DAT_801d81c0;
}

/* Closes every open bank and stops the sound system. */
ADDRESS(0x80057f3c, 0x88)
void func_80057f3c(void)
{
    StVabSlot *slot;
    int n;

    SsSetMVol(0, 0);
    slot = DAT_801dd1c0.slots;
    n = 322;
    while (n--) {
        if (slot->vab != -1) {
            SsVabClose(slot->vab);
            slot->vab = -1;
        }
        slot++;
    }
    SsEnd();
    SsQuit();
}

/*
 * Plays sound `id` from `pos`: silent at `range` or beyond, louder towards
 * the listener over `fade`, panned by the direction from the listener's
 * yaw. Bit 15 of `id` keeps more of the volume on the far side.
 *
 * Not exact: the wide-volume clamp uses a1 for its temporary where retail
 * uses v1 (the three load/copy words around the clamp).
 */
ADDRESS(0x80057fc4, 0x25c)
s16 func_80057fc4(int id, VECTOR *pos, int volume, int range, int fade, int note)
{
    int dx = (pos->vx - DAT_801dd1c0.listener.vx) >> 3;
    int dy = (pos->vy - DAT_801dd1c0.listener.vy) >> 3;
    int dz = (pos->vz - DAT_801dd1c0.listener.vz) >> 3;
    int dist;
    int wide;
    int vol;
    int left;
    int right;
    int angle;

    wide = id & 0x8000;
    id &= 0xfff;
    dist = SquareRoot0(dx * dx + dy * dy + dz * dz) << 3;
    if (dist >= range)
        return -1;
    vol = (fade - dist) * volume / fade;
    if (vol <= 0)
        return -1;
    if (vol >= 128)
        vol = 127;
    if (wide == 0) {
        left = vol + (127 - vol) / 4;
    } else {
        int near = vol + 32;
        int capped;

        wide = near + (127 - vol) / 2;
        capped = 127;
        if (wide < 128)
            capped = wide;
        left = capped;
    }
    angle = func_8004a1b4(pos->vx - DAT_801dd1c0.listener.vx, pos->vz - DAT_801dd1c0.listener.vz)
            - DAT_801dd1c0.listener_rot.vy;
    if (angle < 0)
        angle += 4096;
    right = left;
    if (angle < 2048)
        right -= (right * rsin(angle) >> 12) * (127 - vol) / 128;
    else
        left = right + (right * rsin(angle) >> 12) * (127 - vol) / 128;
    return func_80058318(id, left, right, note);
}

ADDRESS(0x80058220, 0x34)
void func_80058220(int id, VECTOR *pos, short volume, int note)
{
    func_80057fc4(id, pos, volume, 20480, 18432, note);
}

ADDRESS(0x80058254, 0x34)
void func_80058254(int id, VECTOR *pos, short volume, int range, int fade, int note)
{
    func_80057fc4(id, pos, volume, range, fade, note);
}

/* Moves the listener; either argument may be NULL. */
ADDRESS(0x80058288, 0x68)
void func_80058288(VECTOR *a, SVECTOR *b)
{
    if (a != NULL)
        DAT_801dd1c0.listener = *a;
    if (b != NULL)
        DAT_801dd1c0.listener_rot = *b;
}

ADDRESS(0x800582f0, 0x28)
s16 func_800582f0(int sound, int volume)
{
    return func_80058318(sound, volume, volume, 0);
}

/*
 * Keys sound `id` on with the two volumes (scaled by the sound option) on a
 * free voice, else on one that is releasing; returns the voice or -1.
 *
 * Not exact: retail places the key-on block after the `note = 0` jump and
 * sends every early return to one shared `li v0,-1`; this build keeps the
 * block at the end and emits a return stub first (0x14 bytes longer).
 */
ADDRESS(0x80058318, 0x1f4)
s16 func_80058318(int id, int left, int right, int note)
{
    Data801cd120 *opt;
    StSound *snd;
    StVabSlot *slot;
    s16 voice;
    int i;

    if (id == 255)
        return -1;
    opt = &DAT_801cd120;
    if (opt->unk1 == 0)
        return -1;
    snd = &DAT_801dd1c0.sounds[id & 0x7ff];
    if (snd->slot == -1)
        return -1;
    slot = &DAT_801dd1c0.slots[snd->slot];
    if (slot->vab == -1)
        return -1;
    if (slot->vab == 254)
        return -1;
    if (slot->block[2] == 3)
        return -1;
    left = left * opt->unk1 >> 7;
    right = right * opt->unk1 >> 7;
    note += snd->note;
    if (note < 0)
        note = 0;
    else if (note >= 128)
        note = 127;
    SpuGetAllKeysStatus(DAT_801dd1c0.keys);
    i = 24;
    while (i--) {
        if (DAT_801dd1c0.keys[i] == 0)
            goto key_on;
    }
    i = 24;
    while (i--) {
        if (DAT_801dd1c0.keys[i] == 3) {
            SsUtKeyOffV(i);
            goto key_on;
        }
    }
    return -1;
key_on:
    voice = SsUtKeyOn(slot->vab, snd->prog, snd->tone, note, 0, left, right);
    DAT_801dd1c0.voices[voice] = id;
    return voice;
}
