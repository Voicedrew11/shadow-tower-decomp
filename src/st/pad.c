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
    pad_state.action_mask = pad_state.buttons[8] | pad_state.buttons[9] | pad_state.buttons[10] | pad_state.buttons[11];
}

ADDRESS(0x800573bc, 0x6c)
void pad_set_default_buttons(void)
{
    pad_state.buttons[0] = 0x10;
    pad_state.buttons[1] = 0x40;
    pad_state.buttons[2] = 0x80;
    pad_state.buttons[3] = 0x20;
    pad_state.buttons[4] = 0x400;
    pad_state.buttons[5] = 0x800;
    pad_state.buttons[6] = 0x100;
    pad_state.buttons[7] = 0x200;
    pad_state.buttons[8] = 0x1000;
    pad_state.buttons[9] = 0x8000;
    pad_state.buttons[10] = 0x4000;
    pad_state.buttons[11] = 0x2000;
}

/* Waits (up to 60 vsyncs) for the controller in port 1 to settle. */
ADDRESS(0x80057428, 0x98)
void pad_wait_stable(void)
{
    int start = VSync(-1);

    for (;;) {
        pad_state.port_state = PadGetState(0);
        if (pad_state.port_state == PadStateStable)
            break;
        if (pad_state.port_state == PadStateFindCTP1)
            break;
        if (VSync(-1) - start >= 60)
            break;
    }
}

ADDRESS(0x800574c0, 0x9c)
void pad_setup_actuators(void)
{
    pad_state.actuator_count = PadInfoAct(0, -1, 0);
    pad_wait_stable();
    if (pad_state.actuator_count) {
        pad_state.act_align[0] = 0;
        pad_state.act_align[1] = 0;
        if (pad_state.actuator_count == 2) {
            pad_state.act_align[2] = 1;
            pad_state.act_align[3] = 1;
        } else {
            pad_state.act_align[2] = 0xff;
            pad_state.act_align[3] = 0xff;
        }
        pad_state.act_align[4] = pad_state.act_align[5] = 0xff;
        while (PadSetActAlign(0, pad_state.act_align) == 0)
            ;
    }
}

ADDRESS(0x8005755c, 0x44)
void pad_setup_actuators_if_stable(void)
{
    if (pad_state.port_state == PadStateStable)
        pad_setup_actuators();
    else
        pad_state.actuator_count = 0;
}

ADDRESS(0x800575a0, 0xe4)
void pad_update_state(void)
{
    pad_state.port_state = PadGetState(0);
    switch (pad_state.port_state) {
    case PadStateDiscon:
        pad_state.disconnected = 1;
        pad_state.no_input = 1;
        break;
    case PadStateFindPad:
        pad_state.align_pending = 1;
        break;
    case PadStateStable:
        if (pad_state.align_pending && !pad_state.disconnected) {
            pad_state.align_pending = 0;
            pad_setup_actuators();
        }
        /* fall through */
    case PadStateFindCTP1:
        if (pad_state.disconnected) {
            pad_state.disconnected = 0;
            pad_state.no_input = 0;
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
    pad_state.disabled = 1;
    pad_state.no_input = 1;
    pad_state.held_changed = 0;
    pad_state.repeat_timer = 0;
    pad_state.combo_timer = 0;
    pad_state.combo = 0;
    pad_state.actuator_count = 0;
    /* One word store clears each halfword pair. */
    *(u32 *)&pad_state.prev_held = 0;
    *(u32 *)&pad_state.pressed = 0;
    *(u32 *)&pad_state.combo_first = 0;
    pad_set_default_buttons();
    pad_update_action_mask();
}

/* Per-frame button words: held, newly pressed, auto-repeat and the two-button
 * combinations of unk36/unk38 with unk32/unk34 (unk14 = 1..5). */
ADDRESS(0x80057744, 0x2e4)
void pad_read(void)
{
    if (pad_state.disabled)
        return;
    pad_update_state();
    if (pad_state.no_input)
        return;
    if (pad_buffers.unk0)
        return;

    pad_state.prev_held = pad_state.held;
    pad_state.held = ~pad_buffers.unk2;
    if (pad_state.held != pad_state.prev_held) {
        pad_state.held_changed = 1;
        pad_state.repeat_timer = 0;
        pad_state.pressed_repeat = 0;
        if ((pad_state.held | pad_state.prev_held) != pad_state.prev_held) {
            pad_state.pressed_repeat = pad_state.held & ~pad_state.prev_held;
            pad_state.pressed |= pad_state.pressed_repeat;
        }
    } else {
        if (pad_state.repeat_timer && --pad_state.repeat_timer == 0)
            pad_state.pressed_repeat = pad_state.repeat_held;
        if (pad_state.held != pad_state.repeat_held) {
            pad_state.repeat_held = pad_state.held;
            if (pad_state.repeat_timer == 0)
                pad_state.repeat_timer = game_settings.repeat_delay;
        } else if (pad_state.repeat_timer == 0) {
            pad_state.repeat_timer = game_settings.repeat_interval;
        }
    }

    if ((pad_state.action_mask & (pad_state.held | pad_state.prev_held)) != (pad_state.prev_held & pad_state.action_mask)) {
        if (pad_state.combo_timer == 0) {
            pad_state.combo_first = pad_state.held & ~pad_state.prev_held;
            pad_state.combo_timer = game_settings.combo_window;
        } else if (pad_state.combo_second == 0) {
            pad_state.combo_second = pad_state.held & ~pad_state.prev_held;
            if ((pad_state.combo_first & pad_state.buttons[11]) && (pad_state.buttons[11] & pad_state.combo_second)) {
                pad_state.combo = 1;
                pad_state.pressed &= ~pad_state.buttons[11];
            } else if ((pad_state.combo_first & pad_state.buttons[10]) &&
                       (pad_state.combo_second & pad_state.buttons[8])) {
                pad_state.combo = 2;
                pad_state.pressed = pad_state.pressed & ~pad_state.buttons[8] & ~pad_state.buttons[10];
            } else if ((pad_state.combo_first & pad_state.buttons[10]) &&
                       (pad_state.combo_second & pad_state.buttons[9])) {
                pad_state.combo = 3;
                pad_state.pressed = pad_state.pressed & ~pad_state.buttons[9] & ~pad_state.buttons[10];
            } else if ((pad_state.combo_first & pad_state.buttons[11]) &&
                       (pad_state.combo_second & pad_state.buttons[8])) {
                pad_state.combo = 4;
                pad_state.pressed = pad_state.pressed & ~pad_state.buttons[8] & ~pad_state.buttons[11];
            } else if ((pad_state.combo_first & pad_state.buttons[11]) &&
                       (pad_state.combo_second & pad_state.buttons[9])) {
                pad_state.combo = 5;
                pad_state.pressed = pad_state.pressed & ~pad_state.buttons[9] & ~pad_state.buttons[11];
            }
            pad_state.combo_second = 0;
            pad_state.combo_first = 0;
            pad_state.combo_timer = 0;
        }
    } else if (pad_state.combo_timer && pad_state.combo_second == 0 && --pad_state.combo_timer == 0) {
        pad_state.combo_first = 0;
    }
}

ADDRESS(0x80057a28, 0x14)
void pad_enable(void)
{
    pad_state.disabled = 0;
    pad_state.no_input = 0;
}

ADDRESS(0x80057a3c, 0x2c)
void pad_clear_input(void)
{
    pad_state.disabled = 1;
    pad_state.held_changed = 0;
    pad_state.combo_timer = 0;
    pad_state.combo = 0;
    *(u32 *)&pad_state.pressed = 0;
    pad_state.pressed_repeat = 0;
    pad_state.disabled = 0;
}

/* Starts the large motor at `strength` + 64 (and the small one on a two-motor
 * controller) for (strength + 64) / 16 + 5 frames; 0 or less stops both. */
ADDRESS(0x80057a68, 0xa8)
void pad_vibrate(int strength)
{
    if (game_settings.vibration) {
        if (strength > 0) {
            strength += 64;
            if (strength > 255)
                strength = 255;
            pad_state.vibration_frames = strength / 16 + 5;
            if (pad_state.actuator_count == 1)
                pad_buffers.unk44[1] = 1;
            pad_buffers.unk44[3] = strength;
        } else {
            if (pad_state.actuator_count == 1)
                pad_buffers.unk44[1] = 0;
            pad_buffers.unk44[3] = 0;
        }
    }
}

ADDRESS(0x80057b10, 0x38)
void pad_update_vibration(void)
{
    if (pad_state.vibration_frames)
        pad_state.vibration_frames--;
    else {
        pad_buffers.unk44[1] = 0;
        pad_buffers.unk44[3] = 0;
    }
}
