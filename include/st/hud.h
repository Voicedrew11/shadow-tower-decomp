/*
 * ST.EXE HUD sprite records and the per-frame world update (src/st/hud.c),
 * and the world object passes the frame runs.
 */
#ifndef ST_HUD_H
#define ST_HUD_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/objects.h>

/*
 * One 20-byte HUD sprite record: flags (bit 0 textured, bit 1 flat, bits 2
 * and 3 tint the colour; 0xff ends a run), the semi-transparency rate
 * (0xff opaque), the texture rectangle, the screen rectangle (width 0xffff
 * draws at the texture size), CLUT, texture page and OT depth.
 */
typedef struct {
    u8 flags;
    u8 abr;
    u8 u;
    u8 v;
    u8 uw;
    u8 vh;
    RECT r;
    u16 clut;
    u16 tpage;
    s16 otz;
} StHudSprite;

extern StHudSprite hud_sprites[];

/* Nonzero while the HUD is shown. */

/* src/st/hud.c */
void hud_draw_sprite();
void hud_draw_sprite_scaled();
void hud_draw_rect();
void hud_set_number(int value, int first, u8 width);
void hud_update(void);
void hud_draw_records(u8 first);
void hud_queue_damage_number(short value);
void world_render_frame(VECTOR *pos, SVECTOR *rot);
/* Returns int as far as its callers can tell: FDAT_272's map_turn_view_during_script only
 * matches when this call has a value (reload then keeps v0 free). Nothing
 * uses the result. */
int world_run_frame(VECTOR *pos, SVECTOR *rot, int arg);
void gfx_store_screen(void);
void gfx_restore_buffers(void);

#endif
