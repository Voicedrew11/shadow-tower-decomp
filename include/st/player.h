/*
 * ST.EXE player state functions (src/st/player*.c, WIP modules): the
 * viewer block's hit points, status timers, equipment bonuses and
 * durability, damage and its screen shake, the equipment model and item
 * slots, and the equipment effects. The viewer block itself is st/data.h's
 * Viewer. Address-derived names stay until behaviour is proven; the band's
 * functions that other modules already call keep their st/funcs.h
 * prototypes (func_8003beb4, player_update_status, player_hit,
 * func_8003eb34, func_8003ee38, func_8003f12c),
 * as do st.math's (st/funcs.h), st.light's (st/light.h) and st.anim's
 * (st/anim.h).
 */
#ifndef ST_PLAYER_H
#define ST_PLAYER_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* Clamps the hit points to their maximum. */
#define PLAYER_CLAMP_HP()                                                      \
    if (viewer.unk0 < viewer.unk2) {                                           \
        viewer.unk2 = viewer.unk0;                                             \
    }

/* What func_8003beb4 measures the viewer against: a position and a reach. */
typedef struct {
    s32 unk0;
    VECTOR pos;          /* 0x04 */
    SVECTOR unk14;
    u16 unk1C;           /* 0x1c: added to the reach */
    u16 unk1E;           /* 0x1e: height below pos.vy */
    s16 unk20;           /* 0x20: heading */
    s16 unk22;           /* 0x22: reach along the heading */
} PlayerTarget;

/* One TIM block header (the bytes after the flags word). */
typedef struct {
    u_long len;
    RECT rect;
} TimBlock;

/* Not reconstructed yet. */
void func_80057c0c(void);
void func_8005a3a8(int arg0);
void func_80057a68(int arg0);
void func_80055990(void);
void func_800595d4(void);
void func_80042c44(SVECTOR *a, SVECTOR *b, int c, int d);
void func_80036484(int type, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...);
void func_80045044(u8 r, u8 g, u8 b, u8 arg3, u8 arg4, int arg5, int arg6, int arg7,
                   int arg8, int arg9, int arg10);

/* src/st/player.c .. player_model.c */
void player_kill(int unused);
void func_8003bdb0(void);
void func_8003be10(void);
int func_8003c000(int x, int y, int z, int range, int kind);
void func_8003c054(void);
short func_8003c17c(short *level, short *peak, int ramp, int max, int apply);
void func_8003c95c(int r, int g, int b, int arg3, int arg4);
void func_8003cae4(void);
void player_equipment_bonuses(void);
void player_add_hp(int delta);
void player_add_mp(int delta);
void func_8003cfdc(int id);
void func_8003d030(void);
void player_wear_item(int amount, u16 *slot);
void player_wear_armour(int damage, u16 flags);
void player_update_shake(void);
void player_take_damage(int damage, int flags, long *from);
void func_8003e5e8(u16 *damage, u16 effect, long *pos, int min, int range, int kind);
void func_8003e794(void);
void func_8003e840(void);
void func_8003e874(void);
void func_8003ea20(void);
void func_8003ead0(void);
int func_8003ebfc(int x, int y, int z, int range, int kind);
void func_8003ec4c(volatile u8 *p);
void func_8003f4ec(u8 value, u8 slot);
void func_8003f670(u8 unused, u8 slot);
int func_8003f704(int length, VECTOR *pos, SVECTOR *dir, SVECTOR *rot, int slot);

/* Not reconstructed exactly yet (0x8003f910); called with two arguments. */
void func_8003f910(int kind, int slot, ...);

#endif
