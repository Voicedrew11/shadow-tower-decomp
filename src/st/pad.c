/*
 * ST.EXE: controller setup, state polling, the per-frame button words and
 * vibration (WIP module, 0x8005738c..; the vibration helpers after
 * pad_update_vibration open the next band and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBETC.H>
#include <LIBPAD.H>
#include <st/data.h>
#include <st/pad.h>
#include <st/funcs.h>
#include <st/player.h>

RODATA(0x80012854, 0x1c)

ADDRESS(0x8005738c, 0x30)
void pad_update_action_mask(void)
{
    pad_state.unk3A = pad_state.unk22[8] | pad_state.unk22[9] | pad_state.unk22[10] | pad_state.unk22[11];
}

ADDRESS(0x800573bc, 0x6c)
void pad_set_default_buttons(void)
{
    pad_state.unk22[0] = 0x10;
    pad_state.unk22[1] = 0x40;
    pad_state.unk22[2] = 0x80;
    pad_state.unk22[3] = 0x20;
    pad_state.unk22[4] = 0x400;
    pad_state.unk22[5] = 0x800;
    pad_state.unk22[6] = 0x100;
    pad_state.unk22[7] = 0x200;
    pad_state.unk22[8] = 0x1000;
    pad_state.unk22[9] = 0x8000;
    pad_state.unk22[10] = 0x4000;
    pad_state.unk22[11] = 0x2000;
}

/* Waits (up to 60 vsyncs) for the controller in port 1 to settle. */
ADDRESS(0x80057428, 0x98)
void pad_wait_stable(void)
{
    int start = VSync(-1);

    for (;;) {
        pad_state.unk15 = PadGetState(0);
        if (pad_state.unk15 == PadStateStable)
            break;
        if (pad_state.unk15 == PadStateFindCTP1)
            break;
        if (VSync(-1) - start >= 60)
            break;
    }
}

ADDRESS(0x800574c0, 0x9c)
void pad_setup_actuators(void)
{
    pad_state.unk18 = PadInfoAct(0, -1, 0);
    pad_wait_stable();
    if (pad_state.unk18) {
        pad_state.unk1A[0] = 0;
        pad_state.unk1A[1] = 0;
        if (pad_state.unk18 == 2) {
            pad_state.unk1A[2] = 1;
            pad_state.unk1A[3] = 1;
        } else {
            pad_state.unk1A[2] = 0xff;
            pad_state.unk1A[3] = 0xff;
        }
        pad_state.unk1A[4] = pad_state.unk1A[5] = 0xff;
        while (PadSetActAlign(0, pad_state.unk1A) == 0)
            ;
    }
}

ADDRESS(0x8005755c, 0x44)
void pad_setup_actuators_if_stable(void)
{
    if (pad_state.unk15 == PadStateStable)
        pad_setup_actuators();
    else
        pad_state.unk18 = 0;
}

ADDRESS(0x800575a0, 0xe4)
void pad_update_state(void)
{
    pad_state.unk15 = PadGetState(0);
    switch (pad_state.unk15) {
    case PadStateDiscon:
        pad_state.unk16 = 1;
        pad_state.unkE = 1;
        break;
    case PadStateFindPad:
        pad_state.unk17 = 1;
        break;
    case PadStateStable:
        if (pad_state.unk17 && !pad_state.unk16) {
            pad_state.unk17 = 0;
            pad_setup_actuators();
        }
        /* fall through */
    case PadStateFindCTP1:
        if (pad_state.unk16) {
            pad_state.unk16 = 0;
            pad_state.unkE = 0;
        }
        break;
    case PadStateFindCTP2:
    case PadStateReqInfo:
    case PadStateExecCmd:
        break;
    }
}

ADDRESS(0x80057684, 0xc0)
void pad_init(void)
{
    pad_buffers.unk44[0] = 0x40;
    pad_buffers.unk48[0] = 0x40;
    pad_buffers.unk44[1] = 0;
    pad_buffers.unk48[1] = 0;
    pad_buffers.unk44[2] = 0x40;
    pad_buffers.unk48[2] = 0x40;
    pad_buffers.unk44[3] = 0;
    pad_buffers.unk48[3] = 0;
    PadInitDirect((u8 *)&pad_buffers.unk0, (u8 *)pad_buffers.unk22);
    PadSetAct(0, (u8 *)pad_buffers.unk44, 4);
    PadStartCom();
    pad_wait_stable();
    pad_setup_actuators_if_stable();
    pad_state.unk10 = 1;
    pad_state.unkE = 1;
    pad_state.unk13 = 0;
    pad_state.unk11 = 0;
    pad_state.unk12 = 0;
    pad_state.unk14 = 0;
    pad_state.unk18 = 0;
    /* One word store clears each halfword pair. */
    *(u32 *)&pad_state.unk0 = 0;
    *(u32 *)&pad_state.unk4 = 0;
    *(u32 *)&pad_state.unk8 = 0;
    pad_set_default_buttons();
    pad_update_action_mask();
}

/* Per-frame button words: held, newly pressed, auto-repeat and the two-button
 * combinations of unk36/unk38 with unk32/unk34 (unk14 = 1..5). */
ADDRESS(0x80057744, 0x2e4)
void pad_read(void)
{
    if (pad_state.unk10)
        return;
    pad_update_state();
    if (pad_state.unkE)
        return;
    if (pad_buffers.unk0)
        return;

    pad_state.unk0 = pad_state.unk2;
    pad_state.unk2 = ~pad_buffers.unk2;
    if (pad_state.unk2 != pad_state.unk0) {
        pad_state.unk13 = 1;
        pad_state.unk11 = 0;
        pad_state.unkC = 0;
        if ((pad_state.unk2 | pad_state.unk0) != pad_state.unk0) {
            pad_state.unkC = pad_state.unk2 & ~pad_state.unk0;
            pad_state.unk4 |= pad_state.unkC;
        }
    } else {
        if (pad_state.unk11 && --pad_state.unk11 == 0)
            pad_state.unkC = pad_state.unk6;
        if (pad_state.unk2 != pad_state.unk6) {
            pad_state.unk6 = pad_state.unk2;
            if (pad_state.unk11 == 0)
                pad_state.unk11 = game_settings.unk7;
        } else if (pad_state.unk11 == 0) {
            pad_state.unk11 = game_settings.unk6;
        }
    }

    if ((pad_state.unk3A & (pad_state.unk2 | pad_state.unk0)) != (pad_state.unk0 & pad_state.unk3A)) {
        if (pad_state.unk12 == 0) {
            pad_state.unk8 = pad_state.unk2 & ~pad_state.unk0;
            pad_state.unk12 = game_settings.unk8;
        } else if (pad_state.unkA == 0) {
            pad_state.unkA = pad_state.unk2 & ~pad_state.unk0;
            if ((pad_state.unk8 & pad_state.unk22[11]) && (pad_state.unk22[11] & pad_state.unkA)) {
                pad_state.unk14 = 1;
                pad_state.unk4 &= ~pad_state.unk22[11];
            } else if ((pad_state.unk8 & pad_state.unk22[10]) &&
                       (pad_state.unkA & pad_state.unk22[8])) {
                pad_state.unk14 = 2;
                pad_state.unk4 = pad_state.unk4 & ~pad_state.unk22[8] & ~pad_state.unk22[10];
            } else if ((pad_state.unk8 & pad_state.unk22[10]) &&
                       (pad_state.unkA & pad_state.unk22[9])) {
                pad_state.unk14 = 3;
                pad_state.unk4 = pad_state.unk4 & ~pad_state.unk22[9] & ~pad_state.unk22[10];
            } else if ((pad_state.unk8 & pad_state.unk22[11]) &&
                       (pad_state.unkA & pad_state.unk22[8])) {
                pad_state.unk14 = 4;
                pad_state.unk4 = pad_state.unk4 & ~pad_state.unk22[8] & ~pad_state.unk22[11];
            } else if ((pad_state.unk8 & pad_state.unk22[11]) &&
                       (pad_state.unkA & pad_state.unk22[9])) {
                pad_state.unk14 = 5;
                pad_state.unk4 = pad_state.unk4 & ~pad_state.unk22[9] & ~pad_state.unk22[11];
            }
            pad_state.unkA = 0;
            pad_state.unk8 = 0;
            pad_state.unk12 = 0;
        }
    } else if (pad_state.unk12 && pad_state.unkA == 0 && --pad_state.unk12 == 0) {
        pad_state.unk8 = 0;
    }
}

ADDRESS(0x80057a28, 0x14)
void pad_enable(void)
{
    pad_state.unk10 = 0;
    pad_state.unkE = 0;
}

ADDRESS(0x80057a3c, 0x2c)
void pad_clear_input(void)
{
    pad_state.unk10 = 1;
    pad_state.unk13 = 0;
    pad_state.unk12 = 0;
    pad_state.unk14 = 0;
    *(u32 *)&pad_state.unk4 = 0;
    pad_state.unkC = 0;
    pad_state.unk10 = 0;
}

/* Starts the large motor at `strength` + 64 (and the small one on a two-motor
 * controller) for (strength + 64) / 16 + 5 frames; 0 or less stops both. */
ADDRESS(0x80057a68, 0xa8)
void pad_vibrate(int strength)
{
    if (game_settings.unk5) {
        if (strength > 0) {
            strength += 64;
            if (strength > 255)
                strength = 255;
            pad_state.unk19 = strength / 16 + 5;
            if (pad_state.unk18 == 1)
                pad_buffers.unk44[1] = 1;
            pad_buffers.unk44[3] = strength;
        } else {
            if (pad_state.unk18 == 1)
                pad_buffers.unk44[1] = 0;
            pad_buffers.unk44[3] = 0;
        }
    }
}

ADDRESS(0x80057b10, 0x38)
void pad_update_vibration(void)
{
    if (pad_state.unk19)
        pad_state.unk19--;
    else {
        pad_buffers.unk44[1] = 0;
        pad_buffers.unk44[3] = 0;
    }
}
