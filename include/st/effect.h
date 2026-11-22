/*
 * ST.EXE effects (src/st/effect.c, WIP): a pool of 128 timed objects at
 * effects.slots, each spawned with a kind, a definition index, a position
 * and a velocity, and run once per frame by the per-kind effect update
 * (0x800385b0) through effects.cur and effects.def. Address-derived names
 * stay until behaviour is proven.
 */
#ifndef ST_EFFECT_H
#define ST_EFFECT_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/gfx.h>
#include <st/effect_kinds.h>

struct Creature; /* st/vis.h */

/* One 24-byte effect definition (viewer.magic_defs points at the ones the
 * hands cast). */
typedef struct StEffectDef {
    u8 unk0;
    u8 unk1;
    u16 mp_cost;         /* 0x02: shown by the equipment screen (src/st/menu_equip_arrange.c) */
    u16 physical_damage[3]; /* 0x04 */
    u16 element_damage[6]; /* 0x0a */
    u16 hit_effect;      /* 0x16 */
} StEffectDef;

/* One 88-byte effect. */
typedef struct StEffect {
    u8 flags;            /* 0x00: kind (low 3 bits) and flags; 255 when free */
    u8 kind;             /* 0x01: definition index */
    u8 part;
    u8 unk3;
    u8 spawn_type;       /* 0x04: ST.EXE: the spawner's first argument */
    u8 step;             /* 0x05: per-kind step */
    u8 draw_mode;        /* 0x06: bits 0-1 cull mode, bits 2-3 kind (world_draw_effects_placements) */
    u8 draw_flags;
    u8 attack_delay;     /* 0x08: frames left before the next attack */
    u8 unk9;
    s16 model;           /* 0x0a: model id - 96 (world_draw_effects_placements) */
    u16 reach;           /* 0x0c: attack reach */
    u16 clut_base;       /* 0x0e: CLUT base (world_draw_effects_placements) */
    s16 life;            /* 0x10: frames left to live, -1 frees the slot */
    u16 light;           /* 0x12 */
    u16 anim_frame;      /* 0x14 */
    u8 unk16[2];
    u_long attack_height; /* 0x18: attack height, with 0x80000000 */
    struct StEffect *parent; /* 0x1c: the effect this one follows */
    VECTOR position;     /* 0x20: position */
    SVECTOR rotation;    /* 0x30: rotation */
    SVECTOR scale;       /* 0x38: scale */
    SVECTOR velocity;    /* 0x40: velocity */
    StAnimState anim_slot; /* 0x48: its animation state (model_draw) */
    union {
        struct {
            u8 fighter;  /* 0x4c: VS.EXE: the owner fighter's number */
            u8 attack_index; /* 0x4d: VS.EXE: the attack's index among the fighter's */
            u8 unk4E[2];
        } vs;
        struct Creature *entry; /* 0x4c: ST.EXE: the table A entry the effect belongs to */
    } owner;
    union {
        s16 h;
        s8 b[2];
        u8 ub[2];
    } state;             /* 0x50: state, by kind */
    union {
        s16 h;
        s8 b[2];
        u8 ub[2];
    } state2;            /* 0x52: more of it */
    struct Creature *hit_target; /* 0x54: ST.EXE: the table A entry a bolt hit */
} StEffect;

typedef char StEffectAnimSlotAt0x48[offsetof(StEffect, anim_slot) == 0x48 ? 1 : -1];
typedef char StEffectOwnerAt0x4C[offsetof(StEffect, owner) == 0x4c ? 1 : -1];
typedef char StEffectAttackIndexAt0x4D[offsetof(StEffect, owner.vs.attack_index) == 0x4d ? 1 : -1];
typedef char StEffectStateAt0x50[offsetof(StEffect, state) == 0x50 ? 1 : -1];
typedef char StEffectHitTargetAt0x54[offsetof(StEffect, hit_target) == 0x54 ? 1 : -1];
typedef char StEffectSize[sizeof(StEffect) == 88 ? 1 : -1];

#define ST_EFFECT_FREE 255
#define ST_EFFECT_SLOTS 128

/* The definitions, the effects and the current ones (WIP
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
/* VS.EXE has its own port under this name, with another signature; VS units define VS_EXE. */
#ifndef VS_EXE
void effect_preset_1();
void effect_preset_5();
void effect_preset_14();
#endif

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
