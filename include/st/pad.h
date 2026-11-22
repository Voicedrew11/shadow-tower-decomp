/*
 * ST.EXE controller (LIBPAD direct connection) state and the functions that
 * read it (src/st/pad.c). Field names stay address-derived until their
 * behaviour is proven; the state, the vibration bytes and the button masks
 * live in pad_state (st/data.h).
 */
#ifndef ST_PAD_H
#define ST_PAD_H

#include <kf/lib/types.h>
#include <sys/types.h>

/*
 * The LIBPAD direct-connection receive buffers and actuator data at
 * 0x801cd130. LIBPAD's interrupt handler fills the buffers, so the object
 * is volatile.
 */
typedef struct {
    u8 unk0;             /* 0x00: receive status, 0 when the data is valid */
    u8 unk1;
    u16 unk2;            /* 0x02: button bits, active low */
    u8 unk4[0x22 - 0x4];
    u8 unk22[0x22];      /* 0x22: port 2 receive buffer */
    u8 unk44[4];         /* 0x44: actuator data PadSetAct reads (+1 small motor, +3 large) */
    u8 unk48[4];
} StPadData;

extern volatile StPadData pad_buffers;

void pad_update_action_mask(void);
void pad_set_default_buttons(void);
void pad_wait_stable(void);
void pad_setup_actuators(void);
void pad_setup_actuators_if_stable(void);
void pad_update_state(void);
void pad_init(void);
void pad_read(void);
void pad_enable(void);
void pad_clear_input(void);
void pad_vibrate(int arg0);
void pad_update_vibration(void);

#endif
