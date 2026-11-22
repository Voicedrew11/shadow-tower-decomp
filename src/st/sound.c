/*
 * ST.EXE: the sound module (WIP, 0x80057b48..0x8005850c): the fixed sound
 * effects, the sound system's start and stop, the positional and plain
 * sound players over the sound state block (sound_state). Its key-off and
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

/* On a landing speed (`damage`; its only caller passes viewer.unk2D4) of
 * 256 or more, plays the hit sound and shakes the pad by it. */
ADDRESS(0x80057b48, 0x64)
void play_landing_sound(int damage)
{
    if (damage >= 256) {
        damage -= 256;
        if (damage > 512)
            damage = 512;
        sound_play(20, (damage >> 3) + 64);
        pad_vibrate(damage / 4);
    }
}

/* Plays the footstep sound of the floor the viewer stands on. */
ADDRESS(0x80057bac, 0x60)
void play_footstep_sound(int volume)
{
    int flags = vis_lists.floor_kinds;
    int id;

    if (flags & 0x34)
        id = 19;
    else if ((flags & 3) == 1)
        id = 17;
    else if ((flags & 3) == 2)
        id = 18;
    else
        id = 17;
    sound_play(id, (volume >> 2) + 64);
}

ADDRESS(0x80057c0c, 0x24)
void play_death_sound(void)
{
    sound_play(3, 128);
}

ADDRESS(0x80057c30, 0x24)
void func_80057c30(void)
{
    sound_play(2, 128);
}

ADDRESS(0x80057c54, 0x24)
void play_warp_sound(void)
{
    sound_play(10, 128);
}

ADDRESS(0x80057c78, 0x2c)
void func_80057c78(void)
{
    sound_key_on(10, 128, 128, 50);
}

ADDRESS(0x80057ca4, 0x24)
void func_80057ca4(void)
{
    sound_play(21, 128);
}

ADDRESS(0x80057cc8, 0x24)
void play_confirm_sound(void)
{
    sound_play(12, 128);
}

ADDRESS(0x80057cec, 0x24)
void play_cancel_sound(void)
{
    sound_play(13, 128);
}

ADDRESS(0x80057d10, 0x24)
void play_cursor_sound(void)
{
    sound_play(11, 128);
}

ADDRESS(0x80057d34, 0x24)
void play_refuse_sound(void)
{
    sound_play(14, 128);
}

ADDRESS(0x80057d58, 0x24)
void play_unlock_sound(void)
{
    sound_play(16, 128);
}

ADDRESS(0x80057d7c, 0x24)
void play_locked_sound(void)
{
    sound_play(15, 128);
}

/* Starts the sound system and resets the bank slots, voices and buffers. */
ADDRESS(0x80057da0, 0x19c)
void sound_init(void)
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
    sound_state.unk0 = 0;
    slot = sound_state.slots;
    i = 322;
    while (i--) {
        slot->vab = -1;
        slot->block = NULL;
        slot++;
    }
    key = sound_state.keys;
    voice = sound_state.voices;
    i = 24;
    while (i--) {
        *key++ = 0;
        *voice++ = -1;
    }
    buf = &sound_state.bufs[0];
    for (i = 0; i < 2; i++) {
        buf->state = 0;
        buf->data = sound_buffers_magic[i];
        buf++;
    }
    buf = &sound_state.bufs[2];
    for (i = 0; i < 4; i++) {
        buf->state = 0;
        buf->data = sound_buffers_creature[i];
        buf++;
    }
    buf = &sound_state.bufs[6];
    for (i = 0; i < 4; i++) {
        buf->state = 0;
        buf->data = sound_buffers_object[i];
        buf++;
    }
    buf = &sound_state.bufs[10];
    for (i = 0; i < 4; i++) {
        buf->state = 0;
        buf->data = sound_buffers_equipment[i];
        buf++;
    }
    sound_state.bufs[14].data = sound_buffer_area;
    sound_state.bufs[15].data = sound_buffer_common;
}

/* Closes every open bank and stops the sound system. */
ADDRESS(0x80057f3c, 0x88)
void sound_shutdown(void)
{
    StVabSlot *slot;
    int n;

    SsSetMVol(0, 0);
    slot = sound_state.slots;
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
 */
ADDRESS(0x80057fc4, 0x25c)
s16 sound_play_positional(int id, VECTOR *pos, int volume, int range, int fade, int note)
{
    int dx = (pos->vx - sound_state.listener.vx) >> 3;
    int dy = (pos->vy - sound_state.listener.vy) >> 3;
    int dz = (pos->vz - sound_state.listener.vz) >> 3;
    int dist;
    int left; /* the wide flag (bit 15 of id) until the volume is known */
    int vol;
    int right;
    int angle;

    left = id & 0x8000;
    id &= 0xfff;
    dist = SquareRoot0(dx * dx + dy * dy + dz * dz) << 3;
    if (dist >= range)
        return -1;
    vol = (fade - dist) * volume / fade;
    if (vol <= 0)
        return -1;
    if (vol >= 128)
        vol = 127;
    if (left == 0) {
        left = vol + (127 - vol) / 4;
    } else {
        int near = vol + 32;
        int capped;

        left = near + (127 - vol) / 2;
        capped = 127;
        if (left < 128)
            capped = left;
        left = capped;
    }
    angle = angle_from_xz(pos->vx - sound_state.listener.vx, pos->vz - sound_state.listener.vz)
            - sound_state.listener_rot.vy;
    if (angle < 0)
        angle += 4096;
    right = left;
    if (angle < 2048)
        right -= (right * rsin(angle) >> 12) * (127 - vol) / 128;
    else
        left = right + (right * rsin(angle) >> 12) * (127 - vol) / 128;
    return sound_key_on(id, left, right, note);
}

ADDRESS(0x80058220, 0x34)
void sound_play_at(int id, VECTOR *pos, short volume, int note)
{
    sound_play_positional(id, pos, volume, 20480, 18432, note);
}

ADDRESS(0x80058254, 0x34)
void sound_play_at_range(int id, VECTOR *pos, short volume, int range, int fade, int note)
{
    sound_play_positional(id, pos, volume, range, fade, note);
}

/* Moves the listener; either argument may be NULL. */
ADDRESS(0x80058288, 0x68)
void sound_set_listener(VECTOR *a, SVECTOR *b)
{
    if (a != NULL)
        sound_state.listener = *a;
    if (b != NULL)
        sound_state.listener_rot = *b;
}

ADDRESS(0x800582f0, 0x28)
s16 sound_play(int sound, int volume)
{
    return sound_key_on(sound, volume, volume, 0);
}

/*
 * Keys sound `id` on with the two volumes (scaled by the sound option) on a
 * free voice, else on one that is releasing; returns the voice or -1.
 */
ADDRESS(0x80058318, 0x1f4)
s16 sound_key_on(int id, int left, int right, int note)
{
    Data801cd120 *opt;
    StSound *snd;
    StVabSlot *slot;
    s16 voice;
    int i;

    if (id == 255)
        return -1;
    opt = &game_settings;
    if (opt->unk1 == 0)
        return -1;
    snd = &sound_state.sounds[id & 0x7ff];
    if (snd->slot == -1)
        return -1;
    slot = &sound_state.slots[snd->slot];
    if (slot->vab == -1 || slot->vab == 254)
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
    SpuGetAllKeysStatus(sound_state.keys);
    i = 24;
    while (i--) {
        if (sound_state.keys[i] == 0) {
            voice = SsUtKeyOn(slot->vab, snd->prog, snd->tone, note, 0, left, right);
            sound_state.voices[voice] = id;
            return voice;
        }
    }
    i = 24;
    while (i--) {
        if (sound_state.keys[i] == 3) {
            SsUtKeyOffV(i);
            voice = SsUtKeyOn(slot->vab, snd->prog, snd->tone, note, 0, left, right);
            sound_state.voices[voice] = id;
            return voice;
        }
    }
    return -1;
}
