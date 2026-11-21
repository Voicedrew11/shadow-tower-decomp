/*
 * ST.EXE area loading (src/st/area_*.c): the area request in area_state
 * (st/data.h), its load state machine and the CD callbacks it queues.
 * Address-derived names stay until behaviour is proven.
 */
#ifndef ST_AREA_H
#define ST_AREA_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/cd.h>

/* 0x800881dc: set to the overlay area start next to DAT_800881d8; the area
 * module addresses it as an extern with lui, not $gp, and as an aggregate:
 * retail keeps its load in front of a store through a pointer, which the
 * scheduler only does for an in-struct read. */
extern u8 *DAT_800881dc[1];
/* Tables func_80044674 fills from its pack; WIP extents. */
extern u8 DAT_8009b088[];

/* src/st/area_a.c */
extern u8 *DAT_800881d8;
void area_hook_none(void);
void func_800443f4(StCdRequest *request);
void func_80044404(void);
void func_80044674(void);
void func_8003ea20(void);
void func_80052d10(void);

/* 0x800f2c88 (after the block heap) and 0x800fc488: the buffers
 * func_80044ee8 puts in world_state.unk8308[1] and [0]; WIP extents. */
extern u_long DAT_800f2c88[];
extern u_long DAT_800fc488[];

/* src/st/area_wait.c */
void func_80044ee8(void);
void func_80045010(int dx, int dy, int dz);

/* src/st/area_cb.c */
void func_80045568(void);
void func_8004557c(void);
void func_80045594(void);
void func_800455ac(void);
void func_800455c4(void);
void func_800455dc(void);
void func_800455f4(StCdRequest *request);
void func_80045648(StCdRequest *request);
void func_80045664(void);
void func_8004567c(void);

/* src/st/area_cd.c */
void func_80045fb8(StCdRequest *request);
void func_80045ffc(void);

/* 0x8008038c: the handlers an area uses until its own are loaded (load
 * data, WIP owner; src/st/area_load.c). */
extern AreaHooks area_hooks_default;

/* src/st/area_request.c (also declared in st/player.h) */
#ifndef ST_FUNC_80045044_OLD_STYLE /* one caller has no prototype in scope */
KF_VALUELESS_S32 func_80045044(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5);
#endif

/* WIP prototypes of the rest of the area code */
void func_800456f8(void);
void func_80045f90(int archive, int member, void (*done)());

#endif
