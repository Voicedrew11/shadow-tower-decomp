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
    DAT_801cd180.unk3A = DAT_801cd180.unk22[8] | DAT_801cd180.unk22[9] | DAT_801cd180.unk22[10] | DAT_801cd180.unk22[11];
}

ADDRESS(0x800573bc, 0x6c)
void pad_set_default_buttons(void)
{
    DAT_801cd180.unk22[0] = 0x10;
    DAT_801cd180.unk22[1] = 0x40;
    DAT_801cd180.unk22[2] = 0x80;
    DAT_801cd180.unk22[3] = 0x20;
    DAT_801cd180.unk22[4] = 0x400;
    DAT_801cd180.unk22[5] = 0x800;
    DAT_801cd180.unk22[6] = 0x100;
    DAT_801cd180.unk22[7] = 0x200;
    DAT_801cd180.unk22[8] = 0x1000;
    DAT_801cd180.unk22[9] = 0x8000;
    DAT_801cd180.unk22[10] = 0x4000;
    DAT_801cd180.unk22[11] = 0x2000;
}

/* Waits (up to 60 vsyncs) for the controller in port 1 to settle. */
ADDRESS(0x80057428, 0x98)
void pad_wait_stable(void)
{
    int start = VSync(-1);

    for (;;) {
        DAT_801cd180.unk15 = PadGetState(0);
        if (DAT_801cd180.unk15 == PadStateStable)
            break;
        if (DAT_801cd180.unk15 == PadStateFindCTP1)
            break;
        if (VSync(-1) - start >= 60)
            break;
    }
}

ADDRESS(0x800574c0, 0x9c)
void pad_setup_actuators(void)
{
    DAT_801cd180.unk18 = PadInfoAct(0, -1, 0);
    pad_wait_stable();
    if (DAT_801cd180.unk18) {
        DAT_801cd180.unk1A[0] = 0;
        DAT_801cd180.unk1A[1] = 0;
        if (DAT_801cd180.unk18 == 2) {
            DAT_801cd180.unk1A[2] = 1;
            DAT_801cd180.unk1A[3] = 1;
        } else {
            DAT_801cd180.unk1A[2] = 0xff;
            DAT_801cd180.unk1A[3] = 0xff;
        }
        DAT_801cd180.unk1A[4] = DAT_801cd180.unk1A[5] = 0xff;
        while (PadSetActAlign(0, DAT_801cd180.unk1A) == 0)
            ;
    }
}

ADDRESS(0x8005755c, 0x44)
void pad_setup_actuators_if_stable(void)
{
    if (DAT_801cd180.unk15 == PadStateStable)
        pad_setup_actuators();
    else
        DAT_801cd180.unk18 = 0;
}

ADDRESS(0x800575a0, 0xe4)
void pad_update_state(void)
{
    DAT_801cd180.unk15 = PadGetState(0);
    switch (DAT_801cd180.unk15) {
    case PadStateDiscon:
        DAT_801cd180.unk16 = 1;
        DAT_801cd180.unkE = 1;
        break;
    case PadStateFindPad:
        DAT_801cd180.unk17 = 1;
        break;
    case PadStateStable:
        if (DAT_801cd180.unk17 && !DAT_801cd180.unk16) {
            DAT_801cd180.unk17 = 0;
            pad_setup_actuators();
        }
        /* fall through */
    case PadStateFindCTP1:
        if (DAT_801cd180.unk16) {
            DAT_801cd180.unk16 = 0;
            DAT_801cd180.unkE = 0;
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
    DAT_801cd130.unk44[0] = 0x40;
    DAT_801cd130.unk48[0] = 0x40;
    DAT_801cd130.unk44[1] = 0;
    DAT_801cd130.unk48[1] = 0;
    DAT_801cd130.unk44[2] = 0x40;
    DAT_801cd130.unk48[2] = 0x40;
    DAT_801cd130.unk44[3] = 0;
    DAT_801cd130.unk48[3] = 0;
    PadInitDirect((u8 *)&DAT_801cd130.unk0, (u8 *)DAT_801cd130.unk22);
    PadSetAct(0, (u8 *)DAT_801cd130.unk44, 4);
    PadStartCom();
    pad_wait_stable();
    pad_setup_actuators_if_stable();
    DAT_801cd180.unk10 = 1;
    DAT_801cd180.unkE = 1;
    DAT_801cd180.unk13 = 0;
    DAT_801cd180.unk11 = 0;
    DAT_801cd180.unk12 = 0;
    DAT_801cd180.unk14 = 0;
    DAT_801cd180.unk18 = 0;
    /* One word store clears each halfword pair. */
    *(u32 *)&DAT_801cd180.unk0 = 0;
    *(u32 *)&DAT_801cd180.unk4 = 0;
    *(u32 *)&DAT_801cd180.unk8 = 0;
    pad_set_default_buttons();
    pad_update_action_mask();
}

/* Per-frame button words: held, newly pressed, auto-repeat and the two-button
 * combinations of unk36/unk38 with unk32/unk34 (unk14 = 1..5). */
ADDRESS(0x80057744, 0x2e4)
void pad_read(void)
{
    if (DAT_801cd180.unk10)
        return;
    pad_update_state();
    if (DAT_801cd180.unkE)
        return;
    if (DAT_801cd130.unk0)
        return;

    DAT_801cd180.unk0 = DAT_801cd180.unk2;
    DAT_801cd180.unk2 = ~DAT_801cd130.unk2;
    if (DAT_801cd180.unk2 != DAT_801cd180.unk0) {
        DAT_801cd180.unk13 = 1;
        DAT_801cd180.unk11 = 0;
        DAT_801cd180.unkC = 0;
        if ((DAT_801cd180.unk2 | DAT_801cd180.unk0) != DAT_801cd180.unk0) {
            DAT_801cd180.unkC = DAT_801cd180.unk2 & ~DAT_801cd180.unk0;
            DAT_801cd180.unk4 |= DAT_801cd180.unkC;
        }
    } else {
        if (DAT_801cd180.unk11 && --DAT_801cd180.unk11 == 0)
            DAT_801cd180.unkC = DAT_801cd180.unk6;
        if (DAT_801cd180.unk2 != DAT_801cd180.unk6) {
            DAT_801cd180.unk6 = DAT_801cd180.unk2;
            if (DAT_801cd180.unk11 == 0)
                DAT_801cd180.unk11 = DAT_801cd120.unk7;
        } else if (DAT_801cd180.unk11 == 0) {
            DAT_801cd180.unk11 = DAT_801cd120.unk6;
        }
    }

    if ((DAT_801cd180.unk3A & (DAT_801cd180.unk2 | DAT_801cd180.unk0)) != (DAT_801cd180.unk0 & DAT_801cd180.unk3A)) {
        if (DAT_801cd180.unk12 == 0) {
            DAT_801cd180.unk8 = DAT_801cd180.unk2 & ~DAT_801cd180.unk0;
            DAT_801cd180.unk12 = DAT_801cd120.unk8;
        } else if (DAT_801cd180.unkA == 0) {
            DAT_801cd180.unkA = DAT_801cd180.unk2 & ~DAT_801cd180.unk0;
            if ((DAT_801cd180.unk8 & DAT_801cd180.unk22[11]) && (DAT_801cd180.unk22[11] & DAT_801cd180.unkA)) {
                DAT_801cd180.unk14 = 1;
                DAT_801cd180.unk4 &= ~DAT_801cd180.unk22[11];
            } else if ((DAT_801cd180.unk8 & DAT_801cd180.unk22[10]) &&
                       (DAT_801cd180.unkA & DAT_801cd180.unk22[8])) {
                DAT_801cd180.unk14 = 2;
                DAT_801cd180.unk4 = DAT_801cd180.unk4 & ~DAT_801cd180.unk22[8] & ~DAT_801cd180.unk22[10];
            } else if ((DAT_801cd180.unk8 & DAT_801cd180.unk22[10]) &&
                       (DAT_801cd180.unkA & DAT_801cd180.unk22[9])) {
                DAT_801cd180.unk14 = 3;
                DAT_801cd180.unk4 = DAT_801cd180.unk4 & ~DAT_801cd180.unk22[9] & ~DAT_801cd180.unk22[10];
            } else if ((DAT_801cd180.unk8 & DAT_801cd180.unk22[11]) &&
                       (DAT_801cd180.unkA & DAT_801cd180.unk22[8])) {
                DAT_801cd180.unk14 = 4;
                DAT_801cd180.unk4 = DAT_801cd180.unk4 & ~DAT_801cd180.unk22[8] & ~DAT_801cd180.unk22[11];
            } else if ((DAT_801cd180.unk8 & DAT_801cd180.unk22[11]) &&
                       (DAT_801cd180.unkA & DAT_801cd180.unk22[9])) {
                DAT_801cd180.unk14 = 5;
                DAT_801cd180.unk4 = DAT_801cd180.unk4 & ~DAT_801cd180.unk22[9] & ~DAT_801cd180.unk22[11];
            }
            DAT_801cd180.unkA = 0;
            DAT_801cd180.unk8 = 0;
            DAT_801cd180.unk12 = 0;
        }
    } else if (DAT_801cd180.unk12 && DAT_801cd180.unkA == 0 && --DAT_801cd180.unk12 == 0) {
        DAT_801cd180.unk8 = 0;
    }
}

ADDRESS(0x80057a28, 0x14)
void pad_enable(void)
{
    DAT_801cd180.unk10 = 0;
    DAT_801cd180.unkE = 0;
}

ADDRESS(0x80057a3c, 0x2c)
void pad_clear_input(void)
{
    DAT_801cd180.unk10 = 1;
    DAT_801cd180.unk13 = 0;
    DAT_801cd180.unk12 = 0;
    DAT_801cd180.unk14 = 0;
    *(u32 *)&DAT_801cd180.unk4 = 0;
    DAT_801cd180.unkC = 0;
    DAT_801cd180.unk10 = 0;
}

/* Starts the large motor at `strength` + 64 (and the small one on a two-motor
 * controller) for (strength + 64) / 16 + 5 frames; 0 or less stops both. */
ADDRESS(0x80057a68, 0xa8)
void pad_vibrate(int strength)
{
    if (DAT_801cd120.unk5) {
        if (strength > 0) {
            strength += 64;
            if (strength > 255)
                strength = 255;
            DAT_801cd180.unk19 = strength / 16 + 5;
            if (DAT_801cd180.unk18 == 1)
                DAT_801cd130.unk44[1] = 1;
            DAT_801cd130.unk44[3] = strength;
        } else {
            if (DAT_801cd180.unk18 == 1)
                DAT_801cd130.unk44[1] = 0;
            DAT_801cd130.unk44[3] = 0;
        }
    }
}

ADDRESS(0x80057b10, 0x38)
void pad_update_vibration(void)
{
    if (DAT_801cd180.unk19)
        DAT_801cd180.unk19--;
    else {
        DAT_801cd130.unk44[1] = 0;
        DAT_801cd130.unk44[3] = 0;
    }
}
