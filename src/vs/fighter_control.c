/*
 * A fighter's attacks and walking moves. One
 * translation unit: retail keeps fighter_update's switch table (0x800118a8)
 * eight-byte aligned after effect_spawn_attack's (0x80011588, 0x31c bytes), as
 * cc1 aligns each table within one object's read-only data, while the
 * next object's constants (0x800118c4) need no such alignment. -G8: the
 * walking moves' pad words are $gp-relative statics.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/effect.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/effect.h>
#include <vs/pad.h>
#include <vs/funcs.h>

/* The module's switch tables. */
RODATA(0x80011588, 0x33c)

/*
 * A fighter's attack: the effects each attack kind spawns from
 * the fighter's facing, or the direct hit. VS's form of
 * ST.EXE's effect_spawn_attack (src/st/creature_attack.c).
 */
void effect_random_offset(SVECTOR *offset, VECTOR *out, VECTOR *pos, short dy, short height);
void effect_jitter_offset(SVECTOR *off, VECTOR *out, VECTOR *base);
/* src/vs/fighter_damage.c's hit, called without a prototype (retail
 * passes the fighter number unconverted). */
void fighter_apply_attack_hit();

/* The attack's origin: its offset turned by the fighter's facing. */
#define ORIGIN(o)                                                              \
    s = rsin(f->facing - 0x800);                                             \
    c = rcos(f->facing - 0x800);                                             \
    (o).vx = f->position.vx + (a->offset_x * c + a->offset_z * s) / 4096;              \
    (o).vy = f->position.vy + a->offset_y;                                         \
    (o).vz = f->position.vz + (a->offset_z * c - a->offset_x * s) / 4096

/* The attack's index among the fighter's attacks. */
#define INDEX() ((u8)(a - data->attacks))

/* Fighter n attacks with attack record a. */
ADDRESS(0x8002f334, 0x12ec)
void effect_spawn_attack(int n, VsAttack *a)
{
    VECTOR p;
    SVECTOR dir;
    VECTOR q;
    SVECTOR step;
    VsFighter *f;
    VsFighterData *data;
    int s;
    int c;
    int i;

    f = &shared_buffer.fighter[n];
    data = &f->data;
    switch ((s16)a->effect) {
    case 0x30:
    case 0x38:
    case 0x66:
    case 0x7d:
    case 0xa1:
        ORIGIN(p);
        step.vx = rsin(f->facing - 0x800) / 16;
        step.vz = rcos(f->facing - 0x800) / 16;
        for (i = 0; i < 3; i++) {
            dir.vx = step.vx * (256 + i * 64) / 256;
            dir.vy = 0;
            dir.vz = step.vz * (256 + i * 64) / 256;
            effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 256, (SVECTOR *)&f->rot_x, i == 2);
        }
        break;
    case 0x3a:
        ORIGIN(q);
        step.vx = rsin(f->facing - 0x800) / 16;
        step.vz = rcos(f->facing - 0x800) / 16;
        for (i = 0; i < 3; i++) {
            dir.vx = step.vx * (512 + i * 128) / 512;
            dir.vy = 0;
            dir.vz = step.vz * (512 + i * 128) / 512;
            effect_jitter_offset(&dir, &p, &q);
            effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x1000, (SVECTOR *)&f->rot_x);
        }
        break;
    case 0x3e:
    case 0x48:
    case 0x5e:
    case 0x6c:
    case 0x71:
    case 0x7a:
    case 0x89:
    case 0x93:
        ORIGIN(q);
        dir.vx = rsin(f->facing - 0x800) / 16;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) / 16;
        effect_jitter_offset(&dir, &p, &q);
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x200, (SVECTOR *)&f->rot_x);
        break;
    case 0x57:
    case 0x61:
    case 0x68:
    case 0x74:
        ORIGIN(p);
        dir.vx = rsin(f->facing - 0x800) * 0x180 / 4096;
        if ((s16)a->effect == 0x74)
            dir.vy = 200;
        else
            dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) * 0x180 / 4096;
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x400);
        break;
    case 0x40:
    case 0x5d:
    case 0x65:
    case 0x69:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x80:
    case 0x88:
    case 0x8b:
    case 0xa2:
    case 0xa4:
    case 0xa6:
    case 0xa8:
        ORIGIN(p);
        dir.vx = rsin(f->facing - 0x800) * 0x180 / 4096;
        if ((s16)a->effect == 0xa6)
            dir.vy = 200;
        else
            dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) * 0x180 / 4096;
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x1000, (SVECTOR *)&f->rot_x);
        break;
    case 0x7c:
    case 0x83:
    case 0x84:
    case 0x8e:
    case 0x91:
    case 0x96:
    case 0xa0:
    case 0xa7:
    case 0xa9:
        ORIGIN(p);
        dir.vx = rsin(f->facing - 0x800) * 0x180 / 4096;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) * 0x180 / 4096;
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x200, 1);
        break;
    case 0x58:
    case 0x5c:
    case 0x62:
    case 0x63:
    case 0x67:
    case 0x8c:
    case 0x95:
    case 0x9c:
    case 0x9e:
        ORIGIN(p);
        dir.vx = rsin(f->facing - 0x800) / 8;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) / 8;
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x200);
        break;
    case 0x56:
    case 0x5f:
    case 0x70:
        ORIGIN(q);
        dir.vx = rsin(f->facing - 0x800) / 8;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) / 8;
        for (i = 0; i < 4; i++) {
            effect_jitter_offset(&dir, &p, &q);
            effect_spawn(n, INDEX(), 0x81, a->effect, &q, &dir, 0x400, 1);
        }
        break;
    case 0x55:
    case 0x59:
    case 0x97:
        ORIGIN(p);
        dir.vx = rsin(f->facing - 0x800) / 16;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) / 16;
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x200);
        break;
    case 0x31:
    case 0x32:
    case 0x6a:
        ORIGIN(q);
        for (i = 0; i < 4; i++) {
            effect_random_offset(&dir, &p, &q, -data->height, -48);
            effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x600, i & 1);
        }
        break;
    case 0x47:
    case 0x6b:
    case 0x9b:
    case 0xa3:
        ORIGIN(q);
        for (i = 0; i < 4; i++) {
            effect_random_offset(&dir, &p, &q, 0, -48);
            effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x600, i & 1);
        }
        break;
    case 0x33:
    case 0x34:
    case 0x36:
    case 0x37:
    case 0x39:
    case 0x3c:
    case 0x43:
    case 0x45:
    case 0x4a:
    case 0x4b:
    case 0x4d:
    case 0x4f:
    case 0x51:
    case 0x60:
    case 0x6e:
    case 0x6f:
    case 0x73:
    case 0x7f:
    case 0x85:
    case 0x87:
    case 0x8f:
    case 0x90:
    case 0x92:
    case 0x98:
    case 0x99:
    case 0x9d:
    case 0x9f:
    case 0xa5:
        ORIGIN(q);
        dir.vx = rsin(f->facing - 0x800) / 64;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) / 64;
        for (i = 0; i < 4; i++) {
            effect_jitter_offset(&dir, &p, &q);
            effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x400, i & 1);
        }
        break;
    case 0xc8:
    case 0xca:
    case 0xcd:
    case 0xd0:
    case 0xd3:
    case 0xd8:
    case 0xdb:
    case 0xde:
    case 0xe1:
    case 0xe4:
    case 0xe6:
    case 0xeb:
    case 0xee:
        ORIGIN(p);
        dir.vx = rsin(f->facing - 0x800) * 0x300 / 4096;
        dir.vy = 0;
        dir.vz = rcos(f->facing - 0x800) * 0x300 / 4096;
        effect_spawn(n, INDEX(), 0x81, a->effect, &p, &dir, 0x800, (SVECTOR *)&f->rot_x);
        break;
    case 0x3b:
    case 0x44:
    case 0x4c:
    case 0x4e:
    case 0x50:
    case 0x54:
    case 0x5b:
    case 0x9a:
    case 0xf0:
    case 0xf1:
    case 0xf2:
    case 0xf3:
    case 0xf4:
    case 0xf5:
    case 0xf6:
        fighter_apply_attack_hit(n, INDEX());
        break;
    }
}

/*
 * A fighter's walking moves. -G8: the pad word the
 * moves test is a $gp-relative .sbss static.
 */
/* The buttons the moves test (0x10 forward, 0x40 back). */
DATA(0x800633a8, 0x4, ".sbss")
static s32 fighter_pad_held;

/* The buttons newly pressed this frame (fighter_update: 0x1000 and 0x8000
 * start attacks, 0x2000 and 0x4000 chain them). */
DATA(0x800633ac, 0x4, ".sbss")
static s32 fighter_pad_pressed;

/* Called without prototypes (retail passes int arguments unconverted):
 * src/vs/fighter_hit_test.c's reach test, src/vs/fighter_damage.c's hit
 * and the attack effect effect_spawn_attack (not reconstructed). */
int fighter_hit_test();
void fighter_apply_hit();
void effect_spawn_attack();

/* Moves the fighter a step along its facing, forward or back. */
#define VS_WALK_ANGLE(f, d, op, angle)                                         \
    do {                                                                        \
        (f)->position.vx op rsin((angle) + 2048) * (d)->walk_speed * frames_per_draw / 4096; \
        (f)->position.vz op rcos((f)->facing + 2048) * (d)->walk_speed * frames_per_draw / 4096; \
    } while (0)
#define VS_WALK(f, d, op) VS_WALK_ANGLE(f, d, op, (f)->facing)

/* Nonzero when from is 0 or the animation frame lies in [from, to]. */
#define VS_IN_FRAMES(f, from, to)     ((from) == 0 || ((f)->anim_frame >= (from) && (to) >= (f)->anim_frame))

/* Walks with the start/loop animation pair 0/1; the step only moves the
 * fighter inside frames [from, to] when from is nonzero. */
ADDRESS(0x80030620, 0x29c)
void fighter_walk_reversible(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->data;

    f->anim_frame &= 0xfff;
    if (fighter_pad_held & PAD_BUTTON_UP) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim == d->anims[FIGHTER_ANIM_STAND].anim) {
            f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
            f->anim_frame = 0;
        }
        f->anim_step = d->anims[FIGHTER_ANIM_WALK].anim_step;
        if (from != 0 && (f->anim_frame < from || to < f->anim_frame))
            return;
        VS_WALK(f, d, +=);
    } else if (fighter_pad_held & PAD_BUTTON_DOWN) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim == d->anims[FIGHTER_ANIM_STAND].anim) {
            f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
            f->anim_frame = 4095;
        }
        f->anim_step = -d->anims[FIGHTER_ANIM_WALK].anim_step;
        if (from != 0 && (f->anim_frame < from || to < f->anim_frame))
            return;
        VS_WALK(f, d, -=);
    } else {
        f->state = FIGHTER_STATE_STAND;
        if (f->anim == d->anims[FIGHTER_ANIM_WALK].anim) {
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
    }
}

/* Walks forward with animation 1 and back with animation 12. */
ADDRESS(0x800308bc, 0x280)
void fighter_walk_back_anim(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->data;

    f->anim_frame &= 0xfff;
    if (fighter_pad_held & PAD_BUTTON_UP) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim != d->anims[FIGHTER_ANIM_WALK].anim) {
            f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_WALK].anim_step;
            f->anim_frame = 0;
        }
        if (from != 0 && (f->anim_frame < from || to < f->anim_frame))
            return;
        VS_WALK(f, d, +=);
    } else if (fighter_pad_held & PAD_BUTTON_DOWN) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim != d->anims[FIGHTER_ANIM_WALK_BACK].anim) {
            f->anim = d->anims[FIGHTER_ANIM_WALK_BACK].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_WALK_BACK].anim_step;
            f->anim_frame = 0;
        }
        if (from != 0 && (f->anim_frame < from || to < f->anim_frame))
            return;
        VS_WALK(f, d, -=);
    } else {
        f->state = FIGHTER_STATE_STAND;
        if (f->anim != d->anims[FIGHTER_ANIM_STAND].anim) {
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
    }
}

/* Slides along the facing without changing the animation. */
ADDRESS(0x80030b3c, 0x1ac)
void fighter_slide(VsFighter *f)
{
    VsFighterData *d = &f->data;

    f->anim_frame &= 0xfff;
    if (fighter_pad_held & PAD_BUTTON_UP) {
        int angle = f->facing;

        f->state = FIGHTER_STATE_WALK;
        VS_WALK_ANGLE(f, d, +=, angle);
    } else if (fighter_pad_held & PAD_BUTTON_DOWN) {
        int angle = f->facing;

        f->state = FIGHTER_STATE_WALK;
        VS_WALK_ANGLE(f, d, -=, angle);
    } else {
        f->state = FIGHTER_STATE_STAND;
    }
}

/* Walks with animation 1 entered through the start animation 10 and
 * backs off with animations 11/12 by movement kind. */
ADDRESS(0x80030ce8, 0x490)
void fighter_walk_with_start(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->data;
    s16 kind;

    if (fighter_pad_held & PAD_BUTTON_UP) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim == d->anims[FIGHTER_ANIM_WALK].anim) {
            f->anim_step = d->anims[FIGHTER_ANIM_WALK].anim_step;
            if (VS_IN_FRAMES(f, from, to))
                VS_WALK(f, d, +=);
        } else if (f->anim == d->anims[FIGHTER_ANIM_WALK_START].anim) {
            f->anim_step = d->anims[FIGHTER_ANIM_WALK_START].anim_step;
            if (f->anim_frame >= 4096) {
                f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
                f->anim_step = d->anims[FIGHTER_ANIM_WALK].anim_step;
                f->anim_frame = 0;
            }
            if (d->move_kind == 5 || d->move_kind == 6 || d->move_kind == 13)
                VS_WALK(f, d, +=);
        } else {
            f->anim = d->anims[FIGHTER_ANIM_WALK_START].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_WALK_START].anim_step;
            f->anim_frame = 0;
        }
    } else if (fighter_pad_held & PAD_BUTTON_DOWN) {
        f->state = FIGHTER_STATE_WALK;
        kind = d->move_kind;
        if (kind == 3 || kind == 5 || kind == 13) {
            if (f->anim == d->anims[FIGHTER_ANIM_WALK].anim) {
                f->anim_step = -d->anims[FIGHTER_ANIM_WALK].anim_step;
                if (VS_IN_FRAMES(f, from, to))
                    VS_WALK(f, d, -=);
            } else if (f->anim == d->anims[FIGHTER_ANIM_WALK_START_BACK].anim) {
                f->anim_step = -d->anims[FIGHTER_ANIM_WALK_START_BACK].anim_step;
                if (f->anim_frame <= 0) {
                    f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
                    f->anim_step = -d->anims[FIGHTER_ANIM_WALK].anim_step;
                    f->anim_frame = 4095;
                }
                if (d->move_kind == 5 || d->move_kind == 13)
                    VS_WALK(f, d, -=);
            } else {
                f->anim = d->anims[FIGHTER_ANIM_WALK_START_BACK].anim;
                f->anim_step = -d->anims[FIGHTER_ANIM_WALK_START_BACK].anim_step;
                f->anim_frame = 4095;
            }
        } else {
            if (f->anim == d->anims[FIGHTER_ANIM_WALK].anim) {
                f->anim_step = -d->anims[FIGHTER_ANIM_WALK].anim_step;
                if (VS_IN_FRAMES(f, from, to))
                    VS_WALK(f, d, -=);
            } else if (f->anim == d->anims[FIGHTER_ANIM_WALK_START].anim) {
                f->anim_step = d->anims[FIGHTER_ANIM_WALK_START].anim_step;
                if (f->anim_frame >= 4096) {
                    f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
                    f->anim_step = -d->anims[FIGHTER_ANIM_WALK].anim_step;
                    f->anim_frame = 4095;
                }
                if (d->move_kind == 6)
                    VS_WALK(f, d, -=);
            } else {
                f->anim = d->anims[FIGHTER_ANIM_WALK_START].anim;
                f->anim_step = d->anims[FIGHTER_ANIM_WALK_START].anim_step;
                f->anim_frame = 0;
            }
        }
    } else {
        f->state = FIGHTER_STATE_STAND;
        if (f->anim != d->anims[FIGHTER_ANIM_STAND].anim) {
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
    }
    f->anim_frame &= 0xfff;
}

/* Walks forward through the start animation 10 (moving only for movement
 * kinds 5 and 6 while it plays) and back with animation 12. */
ADDRESS(0x80031178, 0x300)
void fighter_walk_start_back_anim(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->data;

    if (fighter_pad_held & PAD_BUTTON_UP) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim == d->anims[FIGHTER_ANIM_WALK].anim) {
            if (VS_IN_FRAMES(f, from, to))
                VS_WALK(f, d, +=);
        } else if (f->anim == d->anims[FIGHTER_ANIM_WALK_START].anim) {
            if (f->anim_frame >= 4096) {
                f->anim = d->anims[FIGHTER_ANIM_WALK].anim;
                f->anim_step = d->anims[FIGHTER_ANIM_WALK].anim_step;
                f->anim_frame = 0;
            }
            if (d->move_kind == 5 || d->move_kind == 6)
                VS_WALK(f, d, +=);
        } else {
            f->anim = d->anims[FIGHTER_ANIM_WALK_START].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_WALK_START].anim_step;
            f->anim_frame = 0;
        }
    } else if (fighter_pad_held & PAD_BUTTON_DOWN) {
        f->state = FIGHTER_STATE_WALK;
        if (f->anim == d->anims[FIGHTER_ANIM_WALK_BACK].anim) {
            if (VS_IN_FRAMES(f, from, to))
                VS_WALK(f, d, -=);
        } else {
            f->anim = d->anims[FIGHTER_ANIM_WALK_BACK].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_WALK_BACK].anim_step;
            f->anim_frame = 0;
        }
    } else {
        f->state = FIGHTER_STATE_STAND;
        if (f->anim != d->anims[FIGHTER_ANIM_STAND].anim) {
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
    }
    f->anim_frame &= 0xfff;
}

/*
 * Runs fighter n's controls for one frame: reads its pad, steps its
 * animation, refills its second gauge, turns it toward the opponent while
 * button 0x800 is held, then by state: 0, 1 and 5 walk (by movement kind),
 * turn and start attacks; 2 and 6 run an attack (combo follow-ups, hit
 * frames and lunges); 3 and 4 drain hit points while hit or down.
 */
ADDRESS(0x80031478, 0xc90)
void fighter_update(int n)
{
    VsFighter *f = &shared_buffer.fighter[n];
    VsFighterData *d = &f->data;
    VsHitRecord *h;
    VsAttack *a;
    int i;
    int step;

    if (n == 0) {
        fighter_pad_held = vs_pad_state.pads[0].held;
        fighter_pad_pressed = vs_pad_state.pads[0].pressed;
    } else {
        fighter_pad_held = vs_pad_state.pads[1].held;
        fighter_pad_pressed = vs_pad_state.pads[1].pressed;
    }
    f->anim_frame += f->anim_step * frames_per_draw;
    if (f->gauge < d->max_gauge) {
        f->gauge += frames_per_draw;
        if (d->max_gauge < f->gauge)
            f->gauge = d->max_gauge;
    }
    if (f->state != FIGHTER_STATE_GUARD && (fighter_pad_held & PAD_BUTTON_R1)) {
        VsFighter *base = shared_buffer.fighter;
        VsFighter *o = base + (1 - n);
        s16 target;

        target = ratan2(f->position.vx - o->position.vx, f->position.vz - o->position.vz) & 0xfff;
        if (target < f->facing) {
            if (f->facing - target < 2048) {
                f->facing -= d->turn_speed * frames_per_draw * 2;
                if (f->facing < target)
                    f->facing = target;
            } else {
                f->facing = (f->facing + d->turn_speed * frames_per_draw * 2) & 0xfff;
            }
        } else {
            if (target - f->facing < 2048) {
                f->facing += d->turn_speed * frames_per_draw * 2;
                if (target < f->facing)
                    f->facing = target;
            } else {
                f->facing = (f->facing - d->turn_speed * frames_per_draw * 2) & 0xfff;
            }
        }
        f->facing &= 0xfff;
    }
    switch (f->state) {
    case FIGHTER_STATE_COMBO:
        if (f->anim_frame < 4096) {
        if (f->anim == d->anims[FIGHTER_ANIM_COMBO].anim) {
            h = &d->combo_hits[0];
            if (f->anim_frame / f->anim_step < 20) {
                if (d->anims[FIGHTER_ANIM_COMBO_CIRCLE].anim != 255
                    && (fighter_pad_pressed & PAD_BUTTON_CIRCLE)) {
                    f->anim = d->anims[FIGHTER_ANIM_COMBO_CIRCLE].anim;
                    f->anim_step = d->anims[FIGHTER_ANIM_COMBO_CIRCLE].anim_step;
                    f->anim_frame = 0;
                    f->hits_left = 1;
                    break;
                }
                if (d->anims[FIGHTER_ANIM_COMBO_CROSS].anim != 255
                    && (fighter_pad_pressed & PAD_BUTTON_CROSS)) {
                    f->anim = d->anims[FIGHTER_ANIM_COMBO_CROSS].anim;
                    f->anim_step = d->anims[FIGHTER_ANIM_COMBO_CROSS].anim_step;
                    f->anim_frame = 0;
                    f->hits_left = 1;
                    break;
                }
            }
        } else if (f->anim == d->anims[FIGHTER_ANIM_COMBO_CIRCLE].anim) {
            h = &d->combo_hits[1];
        } else if (f->anim == d->anims[FIGHTER_ANIM_COMBO_CROSS].anim) {
            h = &d->combo_hits[2];
        }
        i = h->hit_count;
        if (i == 3 && h->hits[0].frame == 0 && h->hits[1].frame == 0 && h->hits[2].frame == 0) {
            if (f->hits_left != 0 && fighter_hit_test(n, d->radius, 2048)) {
                fighter_apply_hit(n, &h->hits[0].hit);
                f->hits_left--;
            }
        } else {
            for (i--; i != -1; i--) {
                if (f->anim_frame - f->anim_step * frames_per_draw < h->hits[i].frame &&
                    f->anim_frame >= h->hits[i].frame &&
                    fighter_hit_test(n, h->hits[i].range + d->radius, h->hits[i].arc))
                    fighter_apply_hit(n, &h->hits[i].hit);
            }
        }
        i = h->move_count;
        while (i--) {
            if (f->anim_frame > h->moves[i].from && f->anim_frame < h->moves[i].to) {
                step = h->moves[i].speed * f->anim_step * frames_per_draw /
                       (h->moves[i].to - h->moves[i].from);
                f->position.vx += rsin(f->facing + 2048) * step / 4096;
                f->position.vz += rcos(f->facing + 2048) * step / 4096;
            }
        }
        } else {
            f->state = FIGHTER_STATE_STAND;
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
        break;
    case FIGHTER_STATE_GAUGE_ATTACK:
        if (f->anim_frame < 4096) {
        if (f->anim == d->anims[FIGHTER_ANIM_GAUGE_ATTACK].anim) {
            a = &d->attacks[0];
            if (f->anim_frame / f->anim_step < 20) {
                if (d->anims[FIGHTER_ANIM_GAUGE_CIRCLE].anim != 255
                    && (fighter_pad_pressed & PAD_BUTTON_CIRCLE)) {
                    f->anim = d->anims[FIGHTER_ANIM_GAUGE_CIRCLE].anim;
                    f->anim_step = d->anims[FIGHTER_ANIM_GAUGE_CIRCLE].anim_step;
                    f->anim_frame = 0;
                    break;
                }
                if (d->anims[FIGHTER_ANIM_GAUGE_CROSS].anim != 255
                    && (fighter_pad_pressed & PAD_BUTTON_CROSS)) {
                    f->anim = d->anims[FIGHTER_ANIM_GAUGE_CROSS].anim;
                    f->anim_step = d->anims[FIGHTER_ANIM_GAUGE_CROSS].anim_step;
                    f->anim_frame = 0;
                    break;
                }
            }
        } else if (f->anim == d->anims[FIGHTER_ANIM_GAUGE_CIRCLE].anim) {
            a = &d->attacks[1];
        } else if (f->anim == d->anims[FIGHTER_ANIM_GAUGE_CROSS].anim) {
            a = &d->attacks[2];
        }
        if (f->anim_frame - f->anim_step * frames_per_draw < a->strike_frame && f->anim_frame >= a->strike_frame) {
            effect_spawn_attack(n, a);
            f->gauge = d->max_gauge - a->gauge_cost;
        }
        } else {
            f->state = FIGHTER_STATE_STAND;
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
        break;
    case FIGHTER_STATE_HURT:
        if (f->anim_frame < 4096) {
            f->anim = d->anims[FIGHTER_ANIM_HURT].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_HURT].anim_step;
            f->hp -= d->max_hp >> 4;
            if (f->hp < f->hp_target)
                f->hp = f->hp_target;
        } else {
            f->hp = f->hp_target;
            f->state = FIGHTER_STATE_STAND;
            f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim_step;
            f->anim_frame = 0;
        }
        break;
    case FIGHTER_STATE_DOWN:
        if (f->anim_frame < 4096) {
            f->anim = d->anims[FIGHTER_ANIM_DOWN].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_DOWN].anim_step;
            f->hp -= d->max_hp >> 4;
            if (f->hp < 0)
                f->hp = 0;
        } else {
            f->hp = 0;
            f->anim_frame = 4095;
        }
        break;
    case FIGHTER_STATE_STAND:
    case FIGHTER_STATE_WALK:
    case FIGHTER_STATE_GUARD:
        if (fighter_pad_held & PAD_BUTTON_CIRCLE) {
            f->state = FIGHTER_STATE_GUARD;
            f->anim_frame &= 0xfff;
            if (f->anim != d->anims[FIGHTER_ANIM_STAND].anim) {
                f->anim = d->anims[FIGHTER_ANIM_STAND].anim;
                /* retail sets the step from the animation id */
                f->anim_step = d->anims[FIGHTER_ANIM_STAND].anim;
                f->anim_frame = 0;
            }
            break;
        }
        switch (d->move_kind) {
        case 1:
            if (f->model_kind == 1)
                fighter_walk_back_anim(f, 0, 0);
            else if (f->model_kind == 2)
                fighter_walk_reversible(f, 0, 0);
            break;
        case 2:
            fighter_slide(f);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            if (f->model_kind == 1)
                fighter_walk_start_back_anim(f, 0, 0);
            else if (f->model_kind == 2)
                fighter_walk_with_start(f, 0, 0);
            break;
        case 10:
            fighter_walk_reversible(f, 1400, 3469);
            break;
        case 11:
            fighter_walk_reversible(f, 1536, 2816);
            break;
        case 12:
            fighter_walk_back_anim(f, 2048, 3072);
            break;
        case 13:
            fighter_walk_with_start(f, 2048, 4096);
            break;
        case 99:
            f->anim_frame &= 0xfff;
            break;
        }
        if (!(fighter_pad_held & PAD_BUTTON_R1)) {
            if (fighter_pad_held & PAD_BUTTON_LEFT)
                f->facing = (f->facing - d->turn_speed * frames_per_draw) & 0xfff;
            if (fighter_pad_held & PAD_BUTTON_RIGHT)
                f->facing = (f->facing + d->turn_speed * frames_per_draw) & 0xfff;
        }
        if ((fighter_pad_pressed & PAD_BUTTON_TRIANGLE) && d->anims[FIGHTER_ANIM_COMBO].anim != 255) {
            f->state = FIGHTER_STATE_COMBO;
            f->anim = d->anims[FIGHTER_ANIM_COMBO].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_COMBO].anim_step;
            f->anim_frame = 0;
            f->hits_left = 1;
        }
        if ((fighter_pad_pressed & PAD_BUTTON_SQUARE) && d->anims[FIGHTER_ANIM_GAUGE_ATTACK].anim != 255
            && f->gauge == d->max_gauge) {
            f->state = FIGHTER_STATE_GAUGE_ATTACK;
            f->anim = d->anims[FIGHTER_ANIM_GAUGE_ATTACK].anim;
            f->anim_step = d->anims[FIGHTER_ANIM_GAUGE_ATTACK].anim_step;
            f->anim_frame = 0;
        }
        break;
    }
}
