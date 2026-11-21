/*
 * ST.EXE player state functions (src/st/player.c, WIP module): the
 * viewer block's hit points, status timers, equipment bonuses and
 * durability, damage and its screen shake, the equipment model and item
 * slots, and the equipment effects. The viewer block itself is st/data.h's
 * Viewer. Address-derived names stay until behaviour is proven; the band's
 * functions that other modules already call keep their st/funcs.h
 * prototypes (func_8003beb4, player_update_status, player_hit,
 * func_8003eb34, func_8003ee38, func_8003f12c),
 * as do st.math's (st/funcs.h), st.light's (st/light.h) and st.anim's
 * (st/anim.h). The held items and magic follow in the same unit.
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
void func_80057a68(int arg0);
void func_80055990(void);
void func_800595d4(void);
struct StEffect *func_80036484(u8 type, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...); /* src/st/effect_spawn.c: the effect spawned (st/effect.h) or NULL */
#ifndef ST_FUNC_80045044_OLD_STYLE /* one caller has no prototype in scope */
KF_VALUELESS_S32 func_80045044(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5);           /* src/st/area_request.c */
#endif

/* src/st/player.c: state, stats, wear, damage, models */
void player_kill(int unused);
void func_8003bdb0(void);
void func_8003be10(void);
int func_8003c000(int x, int y, int z, int range, int kind);
void func_8003c054(void);
short func_8003c17c(short *level, short *peak, int ramp, int max, int apply);
void func_8003c95c(int r, int g, int b, int arg3, int arg4, int arg5);
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

/* A view of one 24-byte effects.defs record (StEffectDef, st/effect.h) as
 * a magic record: viewer.unk118 holds the one each hand casts. */
typedef struct {
    u8 unk0[2];
    u16 unk2;            /* 0x02: cost, taken from viewer.unk6 */
    u8 unk4[0x14];
} StMagic;

/* src/st/player.c: held items, magic, view, hand swaps */
void func_80040024(u8 hand);
void func_80040f04(u8 magic, u8 hand);
void func_80042620(u8 a, u8 b);
void func_80042c44(SVECTOR *a, SVECTOR *b, s16 c, int d);

/* Not reconstructed yet. */
void func_80055374(long *pos, SVECTOR *rot);

#endif
