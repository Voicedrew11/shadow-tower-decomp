/*
 * Number words for ui_draw_number and ui_draw_number_fixed (src/st/render.c,
 * VS.EXE's ports in src/vs/ui.c): the value in the low halfword (signed),
 * the digit count in bits 24-27 and, in bits 28-31, flags that ask for a
 * plus sign. Leading zeros are blank; a value too wide for the digit count
 * shows as nines.
 */
#ifndef ST_UI_NUMBER_H
#define ST_UI_NUMBER_H

#define DIGITS(n) ((n) << 24)
#define NUMBER_PLUS 0x10000000       /* the flag callers set */
#define NUMBER_DIGITS(num) (((num) >> 24) & 0xf)
#define NUMBER_SIGNED 0xf0000000     /* the flag bits ui_draw_number tests */

#endif
