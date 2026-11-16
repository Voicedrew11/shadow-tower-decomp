/*
 * ST.EXE: sound key-off and the VAB bank loads that run through the CD
 * request ring (WIP module, 0x8005850c..0x80058c04). It continues
 * src/st/sound.c, which ends at the non-exact func_80058318 so that these
 * functions keep their retail offsets in their own unit.
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

RODATA(0x80012870, 0x60)

ADDRESS(0x8005850c, 0x28)
void func_8005850c(s16 voice, int sound)
{
    func_80058534(voice, sound, 0);
}

/* Keys sound `id` off on `voice`. */
ADDRESS(0x80058534, 0xec)
void func_80058534(short voice, int id, int note)
{
    StSound *snd;
    StVabSlot *slot;

    if (id == 255)
        return;
    if (DAT_801cd120.unk1 == 0)
        return;
    snd = &DAT_801dd1c0.sounds[id & 0x7ff];
    if (snd->slot == -1)
        return;
    slot = &DAT_801dd1c0.slots[snd->slot];
    if (slot->vab == -1)
        return;
    if (slot->vab == 254)
        return;
    note += snd->note;
    if (note < 0)
        note = 0;
    else if (note >= 128)
        note = 127;
    SsUtKeyOff(voice, slot->vab, snd->prog, snd->tone, note);
    DAT_801dd1c0.voices[voice] = -1;
}

/*
 * Transfers the body of the bank the current CD request (kind 4) reads: a
 * failed transfer reloads the bank, a partial one seeks on, the last one
 * marks the bank's buffer loaded.
 *
 * Not exact: retail loads the reload call's archive byte (volatile unk10)
 * before the index halfword; this build hoists the halfword load.
 */
ADDRESS(0x80058620, 0x150)
void func_80058620(void)
{
    StCdRequest *request = cd_state.unk9D8;
    StVabSlot *slot;
    int r;

    if (request->unk0 != 4)
        return;
    if (request->unk11 != 2)
        return;
    slot = &DAT_801dd1c0.slots[request->unk4.x];
    for (;;) {
        r = SsVabTransBodyPartly(request->unk20, 0xa000, slot->vab);
        if (r == -1) {
            SsVabClose(slot->vab);
            slot->vab = r;
            func_80015848(request);
            func_80058a0c(request->unk10, request->unk4.y, request->unk4.x);
        } else if (r == -2) {
            request->unk1 = 0;
            request->unk11 = 1;
            func_80015a94(&request->unk18, request->unk28, &request->unk18);
            SsVabTransCompleted(1);
            while (CdControl(CdlSeekL, (u_char *)&request->unk18, NULL) == 0)
                ;
            return;
        } else {
            if (r == slot->vab) {
                SsVabTransCompleted(1);
                func_80015848(request);
                /* A bank request keeps its buffer in unk4.w..unk4.h. */
                (*(StSoundBuf **)&request->unk4.w)->state = 1;
                request->unk24 = 0;
            }
            return;
        }
    }
}

/*
 * Finds a free buffer among `bufs` from `buf`, else frees a loaded one
 * (closing the bank of the `count` slots from `first` that uses it); stores
 * its index and returns it, or NULL.
 */
ADDRESS(0x80058770, 0x144)
StSoundBuf *func_80058770(int first, int count, int buf, int bufs, int *index)
{
    StSoundBuf *b;
    StVabSlot *slot;
    int i;
    int n;

    i = 0;
    b = &DAT_801dd1c0.bufs[buf];
    n = bufs;
    while (n--) {
        if (b->state == 0) {
            *index = i;
            return b;
        }
        i++;
        b++;
    }
    i = 0;
    b = &DAT_801dd1c0.bufs[buf];
    n = bufs;
    while (n--) {
        if (b->state == 2) {
            slot = &DAT_801dd1c0.slots[first];
            while (count--) {
                if (slot->vab != -1 && slot->block == (s16 *)b) {
                    SsVabClose(slot->vab);
                    slot->vab = -1;
                    slot->block = NULL;
                    break;
                }
                slot++;
            }
            b->state = 0;
            *index = i;
            return b;
        }
        i++;
        b++;
    }
    return NULL;
}

/*
 * CD completion callback of a bank request: opens the bank's header once
 * read, then seeks on for its body (func_80058620 transfers it).
 */
ADDRESS(0x800588b4, 0x158)
void func_800588b4(StCdRequest *request)
{
    StVabSlot *slot;

    switch (request->unk11) {
    case 0:
        slot = &DAT_801dd1c0.slots[request->unk4.x];
        if (func_80015b78((u32 *)(*(StSoundBuf **)&request->unk4.w)->data, request->unk28)) {
            request->unk1 = 0;
            while (CdControl(CdlSeekL, (u_char *)&request->unk18, NULL) == 0)
                ;
            return;
        }
        if (slot->vab != -1 && slot->vab != 254) {
            SsVabClose(slot->vab);
            slot->vab = -1;
        }
        if ((slot->vab = SsVabOpenHeadSticky((*(StSoundBuf **)&request->unk4.w)->data, -1,
                                             *(u32 *)&request->unkC)) == -1) {
            func_80015848(request);
            return;
        }
        func_80015a94(&request->unk18, request->unk28, &request->unk18);
        request->unk11 = 1;
        request->unk20 = (u8 *)DAT_80088c88;
        request->unk1 = 0;
        request->unk28 = 20;
        while (CdControl(CdlSeekL, (u_char *)&request->unk18, NULL) == 0)
            ;
        break;
    case 1:
        request->unk11 = 2;
        break;
    }
}

/*
 * Loads member `index` of `archive` as the VAB of bank slot `slot`: slots
 * 0..23 have fixed buffers and SPU addresses, slots 24..323 share four.
 */
ADDRESS(0x80058a0c, 0x1f8)
void func_80058a0c(int archive, int index, int slot)
{
    StVabSlot *s = &DAT_801dd1c0.slots[slot];
    StSoundBuf *buf;
    u32 spu;
    StCdRequest *request;
    int n;

    switch (slot) {
    case 2:
    case 3:
    case 4:
    case 5:
        buf = &DAT_801dd1c0.bufs[slot + 8];
        spu = (slot - 2) * 0x3000 + 0x19800;
        break;
    case 1:
        buf = &DAT_801dd1c0.bufs[14];
        spu = 0x65400;
        break;
    case 0:
        buf = &DAT_801dd1c0.bufs[15];
        spu = 0x1000;
        break;
    case 6:
    case 7:
        buf = &DAT_801dd1c0.bufs[slot - 6];
        spu = (slot - 6) * 0x3000 + 0x25800;
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        buf = &DAT_801dd1c0.bufs[(slot - 8) % 4 + 2];
        spu = (slot - 8) % 4 * 0x9c00 + 0x3e400;
        break;
    default:
        if (slot < 324) {
            buf = func_80058770(24, 300, 6, 4, &n);
            spu = n * 0x3c00 + 0x2b800;
        } else {
            buf = NULL;
            spu = 0;
        }
        if (buf == NULL)
            return;
        s->vab = 254;
        break;
    }
    s->block = (s16 *)buf;
    buf->state = 3;
    request = cd_state.unk9D4;
    func_80015b2c(request);
    request->unk11 = 0;
    *(StSoundBuf **)&request->unk4.w = buf;
    request->unk4.x = slot;
    *(u32 *)&request->unkC = spu;
    request->unk10 = archive;
    request->unk4.y = index;
    func_80015da8(archive, index, buf->data, func_800588b4);
}
