#ifndef ST_FDAT_H
#define ST_FDAT_H

/*
 * The FDAT.T map code overlays: ST.EXE loads one entry at 0x801eb560 (the
 * end of its bss) and calls through the handler table at its first byte.
 * Every overlay starts with the same 21 functions (src/lib/fdat_common.inc),
 * spelled with their FDAT_002.BIN addresses; its own handlers follow.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>

/* The table at the overlay's first byte (WIP layout). */
typedef struct {
    s32 id;              /* 0x00: the overlay's id (0x37 in FDAT_002.BIN) */
    void (*unk04[32])(); /* 0x04: handlers */
    s32 unk84;           /* 0x84: id + 0x33 in every overlay */
} FdatHeader;

/* ST.EXE, no source yet. */
void func_8004cee8(Vis212 *e, int arg, VECTOR *pos);
void func_8004ed54(Vis88 *e, int sound);

/* ST.EXE functions declared in st/funcs.h, which the overlay units do not
 * include (see below). */
void func_80053aa4(void);
void func_800402ac(u8 hand);
void func_8003f12c(u16 id, u8 slot, u8 reload);
void func_8005a3a8(int arg);
void func_8003d030(void);

/*
 * ST.EXE's positional sound starters and player_hit (src/st/sound.c,
 * src/st/player_damage.c). The overlays keep the voice the starters return
 * (Vis212.unk22, stored from $v0 after the call), which ST.EXE's own
 * st/funcs.h declares void; the overlay units therefore declare these here
 * and do not include st/funcs.h.
 */
int func_80058254(int id, VECTOR *pos, short volume, int range, int fade, int note);
int func_80058220(int id, VECTOR *pos, short volume, int note);
void player_hit(u16 *damage, u16 scale, u16 effect, long *from);

/* src/lib/fdat_common.inc */
void fdat_801eb7ac(Vis212 *e);
void fdat_801eb7b4(Vis212 *e, int id);
void fdat_801eb814(short angle, int step);
void fdat_801eb880(short angle, int speed);
void fdat_801eb8f4(int y, int step);
int fdat_801eb940(Vis212 *e);
void fdat_801eb96c(Vis212 *e);
void fdat_801eba4c(Vis212 *e);
void fdat_801ebbc8(Vis212 *e, VisScript *s);
void fdat_801ebc88(Vis212 *e, VisScript *s);
void fdat_801ebec4(Vis212 *e);
void fdat_801ec6a4(void);
int fdat_801ecbcc(void);
int fdat_801ece7c(Vis212 *e);
void fdat_801ecf44(Vis212 *e);
void fdat_801ecfc4(VECTOR *pos, SVECTOR *rot);
void fdat_801ed1e4(Vis212 *e);
void fdat_801ed2f0(Vis212 *e);
void fdat_801ed388(Vis212 *e, int fixed, int arg2);
void fdat_801ed410(Vis212 *e);
void fdat_801ed498(void);

/* The map handlers several overlays share (src/lib/fdat_801ed4a0.inc,
 * src/lib/fdat_801ed6e4.inc), spelled with their FDAT_002.BIN addresses. */
void fdat_801ed4a0(void);
void fdat_801ed6e4(void);

/*
 * Overlay words one map unit defines and a later unit of the same overlay
 * reads (each name exists in its own overlay only).
 */
extern int fdat_801edfac; /* FDAT_012.BIN: src/fdat_012/map_801ed4a0.c */
extern int fdat_801edb3c; /* FDAT_332.BIN: src/fdat_332/map_801ed614.c */
extern int fdat_801ed96c; /* FDAT_042.BIN: src/fdat_042/map_801ed4a0.c */
extern int fdat_801ed970; /* FDAT_042.BIN: src/fdat_042/map_801ed7b4.c */
extern int fdat_801ed938; /* FDAT_122.BIN: src/fdat_122/map_801ed4a0.c */
extern int fdat_801eda7c; /* FDAT_102.BIN: src/fdat_102/map_801ed6b8.c */
/* FDAT_272.BIN: each word is defined by one of its map units (one COMMON
 * word per unit; see the DATA() claims in src/fdat_272/). */
extern int fdat_801ef93c;
extern int fdat_801ef504[6];
extern int fdat_801ef920;
extern int fdat_801ef924;
extern int fdat_801ef928;
extern int fdat_801ef92c;
extern int fdat_801ef930;
extern int fdat_801ef934;
extern int fdat_801ef938;

#endif
