/*
 * ST.EXE font glyphs, text and 2D menu primitives (src/st/ui.c, WIP).
 */
#ifndef ST_TEXT_H
#define ST_TEXT_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* .sbss words the glyph drawer reads with lui; their owner is unknown. */
extern u16 DAT_800881a0;
extern u16 DAT_800881a2;
extern u16 DAT_800881ac;
extern u16 DAT_800881ae; /* set by menu_arrange_icons_draw around the grid columns */

/* Read by the title-card overlay; owners unknown. */
extern u16 DAT_80088190;

void ui_draw_glyph(u16 c, u16 x, u16 y, CVECTOR *color);
void ui_draw_text(u16 *s, u16 x, u16 y, CVECTOR *color);
void ui_draw_number(u32 num, u16 x, u16 y, CVECTOR *color);
void ui_draw_number_fixed(u32 num, u16 x, u16 y, CVECTOR *color);
void ui_draw_small_arrow(DVECTOR *pos, CVECTOR *color, u16 dir);
void ui_draw_rect_outline(RECT *r, CVECTOR *color, u16 border, u16 semi);
void ui_fill_rect(RECT *r, CVECTOR *color, u16 semi);
void ui_draw_image(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode);
void ui_add_draw_mode(int abr, int x, int y);
void talk_draw_overlay(void);

void ui_draw_gouraud_quad(DVECTOR *xy, CVECTOR *rgb, int semi);
void ui_fill_rect_gradient(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi);
void ui_draw_arrow(RECT *r, CVECTOR *color, u16 dir);
void menu_draw_backdrop(void);
void ui_draw_panel(int x, int y, int w, int h);
void ui_draw_window(short x, short y, short w, short h);

#endif
