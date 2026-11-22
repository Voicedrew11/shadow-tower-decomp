/*
 * Controller buttons as ST.EXE and VS.EXE decode them (pad_state, st/data.h;
 * vs_pad_state, vs/pad.h): the receive buffer's button halfword inverted, one
 * bit per button in the controller's own order. These are not LIBETC.H's
 * PADLup.. bits, which use PadRead()'s layout.
 *
 * Evidence: pad_set_default_buttons (src/st/pad.c) assigns the twelve actions
 * these defaults in the order of the button configuration screen's labels
 * (STAT.T member 0 messages 209..220: forward, back, left turn, right turn,
 * move left, move right, look up, look down, right hand, left hand, event,
 * magic/menu); player_update opens the pause screen on START.
 */
#ifndef ST_PAD_BUTTONS_H
#define ST_PAD_BUTTONS_H

#define PAD_BUTTON_SELECT 0x1
#define PAD_BUTTON_START 0x8
#define PAD_BUTTON_UP 0x10
#define PAD_BUTTON_RIGHT 0x20
#define PAD_BUTTON_DOWN 0x40
#define PAD_BUTTON_LEFT 0x80
#define PAD_BUTTON_L2 0x100
#define PAD_BUTTON_R2 0x200
#define PAD_BUTTON_L1 0x400
#define PAD_BUTTON_R1 0x800
#define PAD_BUTTON_TRIANGLE 0x1000
#define PAD_BUTTON_CIRCLE 0x2000
#define PAD_BUTTON_CROSS 0x4000
#define PAD_BUTTON_SQUARE 0x8000

/* The configurable actions: indexes of pad_state.buttons. */
#define PAD_ACTION_FORWARD 0
#define PAD_ACTION_BACK 1
#define PAD_ACTION_TURN_LEFT 2
#define PAD_ACTION_TURN_RIGHT 3
#define PAD_ACTION_MOVE_LEFT 4
#define PAD_ACTION_MOVE_RIGHT 5
#define PAD_ACTION_LOOK_UP 6
#define PAD_ACTION_LOOK_DOWN 7
#define PAD_ACTION_RIGHT_HAND 8
#define PAD_ACTION_LEFT_HAND 9
#define PAD_ACTION_EVENT 10       /* examine, take, use */
#define PAD_ACTION_MAGIC_MENU 11  /* back; twice opens the pause menu */
#define PAD_ACTIONS 12

/* pad_state.combo: two actions pressed in a row (pad_read). */
#define PAD_COMBO_NONE 0
#define PAD_COMBO_MENU 1          /* magic/menu, magic/menu: the pause menu */
#define PAD_COMBO_SWAP_RIGHT 2    /* event, right hand: swap with the reserve */
#define PAD_COMBO_SWAP_LEFT 3     /* event, left hand */
#define PAD_COMBO_CAST_RIGHT 4    /* magic/menu, right hand: cast */
#define PAD_COMBO_CAST_LEFT 5     /* magic/menu, left hand */

#endif
