/*
 * Prototypes of ST.EXE game functions that no source reconstructs yet. Each
 * signature is the WIP one its callers' argument registers support; the
 * function moves to its module's header once that module exists.
 */
#ifndef ST_FUNCS_H
#define ST_FUNCS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

void func_800385b0(void); /* the effect script interpreter: runs effects.cur for one frame */
int func_8003beb4(void *request);
void func_8003c0e0(void);
void player_update_status(int apply);
void func_8003cedc(void);
void player_hit(u16 *damage, u16 scale, u16 effect, long *from);
void func_8003e734(VECTOR *a, SVECTOR *b);
void func_8003eaf8(void);
int func_8003eb34(VECTOR *pos, int yaw, int range, int arg3);
void func_8003ec5c(void *data, DVECTOR *a, DVECTOR *b, int arg3);
void func_8003ee38(u16 id, u8 slot);
void func_8003f12c(u16 id, u8 slot, u8 reload);
void func_80042c10(void); /* src/st/player.c */
void func_80042d74(void);
void func_80044674(void);
void func_80044ee8(void);
void func_800456f8(void);
void func_80045ffc(void);
int func_800496c8(int angle, int to, int step);
void func_800498cc(MATRIX *m, MATRIX *out);
void func_80049cd4(short angle, MATRIX *out);
void func_80049e30(SVECTOR *rot, MATRIX *out);
void func_80049f08(SVECTOR *rot, SVECTOR *out);
void func_8004a078(SVECTOR *rot, SVECTOR *v, VECTOR *out);
void func_8004a0f8(int scale, SVECTOR *v);
int func_8004a148(int angle, int heading, int range);
int func_8004a1b4(int dx, int dz);
int func_8004a340(int x, int y, int z);
void func_8004a390(int dx, int dy, int dz, SVECTOR *out);
int func_8004978c(int v, int to, int step);
int func_800497f4(int angle, int heading, int speed, int accel, int decay);
void func_80049d48(short angle, MATRIX *out);
void func_80049dbc(short angle, MATRIX *out);
void func_80049e9c(SVECTOR *rot, MATRIX *out);
void func_80049fe4(SVECTOR *rot, SVECTOR *out);
void func_8004a0c0(short scale, long *v);
int func_8004a184(int a, int b);
int func_8004a194(int a, int b);
int func_8004a304(int dx, int dz);
void func_8004a40c(int dx, int dy, int dz, SVECTOR *out);
int func_8004a484(VECTOR *p, int x, int y, int z, int range, int above, int below);
int func_8004a580(VECTOR *p, int x, int y, int z, int range, int above, int below);
int func_8004a67c(VECTOR *a, int ra, VECTOR *b, int rb, int height);
int func_8004a798(int n);
int func_8004a7e0(int n);
int func_8004a848(int from, short to, int t);
void func_8004a8e8(VECTOR *p, SVECTOR *v, int s, VECTOR *out);
int func_8004a940(SVECTOR *p, SVECTOR *a, SVECTOR *b);
int func_8004aabc(VECTOR *p, VECTOR *a, VECTOR *b);
void func_8004afdc(void);
void *func_8004b658(VECTOR *pos, short yaw, short pitch, int range, int arg4, int arg5, int *dist,
                   int arg7);
void func_8004e324(void);
void func_8004eda8(void);
int func_8004fc94(int x, int z, int w, int d, int arg4);
int func_8004fd28(int x, int z, int w, int d, int arg4);
void func_800500ac(int arg0, int arg1, int arg2, int arg3);
void func_80050768(void);
void func_80052ca4(void);
void func_80053aa4(void);
void func_800559bc(void);
void func_8005735c(void);
void func_80057684(void);
void func_80057744(void);
void func_80057a28(void);
void func_80057a3c(void);
void func_80057c54(void);
void func_80057b10(void);
void func_80057da0(void);
void func_80057f3c(void);
void func_80058254(int id, VECTOR *pos, short volume, int range, int fade, int note);
void func_80058220(int id, VECTOR *pos, short volume, int note);
int func_80052d40(int id);
void player_compute_derived_stats(int arg);
void func_80058288(VECTOR *a, SVECTOR *b);
s16 func_800582f0(int sound, int volume);
void func_8005850c(s16 voice, int sound);
void func_80058620(void);
void func_80058a0c(int archive, int index, int slot);
void card_screen_open(int arg0, int arg1);
void card_screen_close(void);
int card_screen_run(int arg0);
void menu_save_prompt_run(void);
void func_80042cfc(void);
void func_80057cc8(void);
void func_80057d58(void);
void func_80057d7c(void);

/* Called by the band Q screens (src/st/equip_menu.c, src/st/idgrid.c). */

/* 0x8008809c: the unit z vector func_80049fe4 turns (load data, owner unknown). */
extern SVECTOR DAT_8008809c;

#endif
