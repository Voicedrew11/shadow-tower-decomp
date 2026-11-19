/*
 * ST.EXE effects (src/st/effect.c, WIP): a pool of 128 timed objects at
 * effects.slots, each spawned with a kind, a definition index, a position
 * and a velocity, and run once per frame by the effect script interpreter
 * (0x800385b0) through effects.cur and effects.def. Address-derived names
 * stay until behaviour is proven.
 */
#ifndef ST_EFFECT_H
#define ST_EFFECT_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* One 24-byte effect definition. */
typedef struct {
    u8 unk0;
    u8 unk1;
    u16 unk2;            /* 0x02: shown by the equipment screen (src/st/equip_menu.c) */
    u16 unk4[3];         /* 0x04 */
    u16 unkA[6];         /* 0x0a */
    u16 unk16;           /* 0x16 */
} StEffectDef;

/* One 88-byte effect. */
typedef struct StEffect {
    u8 unk0;             /* 0x00: kind (low 3 bits) and flags; 255 when free */
    u8 unk1;             /* 0x01: definition index */
    u8 unk2;
    u8 unk3;
    u8 unk4;             /* 0x04: ST.EXE: the spawner's first argument */
    u8 unk5;             /* 0x05: script step */
    u8 unk6;             /* 0x06: bits 0-1 cull mode, bits 2-3 kind (func_800255f0) */
    u8 unk7;
    u8 unk8;             /* 0x08: frames left before the next attack */
    u8 unk9;
    s16 unkA;            /* 0x0a: model id - 96 (func_800255f0) */
    u16 unkC;            /* 0x0c: attack reach */
    u16 unkE;            /* 0x0e: CLUT base (func_800255f0) */
    s16 unk10;           /* 0x10: frames left to live, -1 frees the slot */
    u16 unk12;           /* 0x12 */
    u16 unk14;           /* 0x14 */
    u8 unk16[2];
    u_long unk18;        /* 0x18: attack height, with 0x80000000 */
    struct StEffect *unk1C; /* 0x1c: the effect this one follows */
    VECTOR unk20;        /* 0x20: position */
    SVECTOR unk30;       /* 0x30: rotation */
    SVECTOR unk38;       /* 0x38: scale */
    SVECTOR unk40;       /* 0x40: velocity */
    u8 unk48[4];
    u8 unk4C;            /* 0x4c: VS.EXE: the owner fighter's number; ST.EXE: a word, ST_EFFECT_ENTRY */
    u8 unk4D;            /* 0x4d: VS.EXE: the attack's index among the fighter's */
    u8 unk4E[2];
    union {
        s16 h;
        s8 b[2];
        u8 ub[2];
    } unk50;             /* 0x50: script state, by kind */
    union {
        s16 h;
        s8 b[2];
        u8 ub[2];
    } unk52;             /* 0x52: more of it */
    void *unk54;         /* 0x54: ST.EXE: the table A entry (Vis212) a bolt hit */
} StEffect;

/* ST.EXE: the table A entry (Vis212, st/vis.h) the effect belongs to. */
#define ST_EFFECT_ENTRY(e) (*(Vis212 **)&(e)->unk4C)

#define ST_EFFECT_FREE 255
#define ST_EFFECT_SLOTS 128

/* The definitions, the effects and the interpreter's current ones (WIP
 * extent; 0x8003e... reads a word past it). */
typedef struct {
    StEffectDef defs[176];
    StEffect slots[ST_EFFECT_SLOTS]; /* 0x1080 */
    StEffectDef *def;    /* 0x3c80: the current effect's definition */
    StEffect *cur;       /* 0x3c84: the effect being run */
    int index;           /* 0x3c88: its slot index */
} StEffects;

extern StEffects effects;

/* src/st/effect.c */
void effect_play_sound(StEffect *e, int id);
int effect_attack(int arg0, u_long height, int drop);
void effect_emit(u_long flags, int kind, StEffectDef *def, int arg, VECTOR *pos);
void effect_emit_current(u_long flags, int arg, VECTOR *pos);
void effect_emit_behind(u_long flags);
void effect_vertex(StEffect *e, int index, VECTOR *out, SVECTOR *scale);
void effect_vertex_world(StEffect *e, int index, VECTOR *out, SVECTOR *scale);
StEffect *effect_alloc(void);
/* The presets are old-style definitions: the spawner (src/st/effect_spawn.c)
 * passes their arguments unconverted. */
void effect_preset_1();
void effect_preset_5();
void effect_preset_14();

/* src/st/effect_move.c */
int effect_accelerate(int scale, int limit, int attack, int height, SVECTOR *accel);
int effect_home(int limit, int scale, int turn, int attack, int height, int near,
                int scale_near, int range);
int effect_fire(int arg0, int height, int spin);
int effect_attack_move(int arg0, u_long height, int spin);
void effect_step_back(void);
void effect_step_back_half(void);

/* src/st/effect_run.c */
void effects_clear(void);
void effects_run(void);

#endif
