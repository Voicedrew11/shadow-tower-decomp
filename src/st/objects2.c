/*
 * ST.EXE: the world object passes, continued (WIP module; split from
 * src/st/objects.c only because func_800246e4 there is not exact and the
 * jump table here addresses this code). Each pass walks the visible
 * entries of one object table, marks the model slots it uses in the
 * scratchpad flag table (which st.slots then loads or releases), places
 * ambient sounds, and draws each model with model_draw.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBSPU.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/scratch.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/mesh.h>
#include <st/objects.h>
#include <st/anim.h>
#include <st/funcs.h>

RODATA(0x80011478, 0x18)
SDATA(0x80087f0c, 0x8)

#define SLOT_C 588

ADDRESS(0x800251c8, 0x428)
void func_800251c8(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Vis48 *e;
    Vis44 *r;
    VECTOR unused;
    StAnimFile *model;
    int i;

    sp->unkF0 = sp->unkF8;
    sp->unkF4 = sp->unk244;
    fill_words((u32 *)sp->unkF8, 0, 32);
    i = vis_table_c.count;
    sp->clut_base = 0;
    while (--i != -1) {
        e = &vis_table_c.entries[vis_table_c.list[i]];
        r = &vis_table_c.unk0118[e->unk00];
        if (e->unk28 >= (s32)world_state.unk1EE00)
            e->unk24 |= 0x80;
        switch (e->unk22) {
        case 0:
        case 4:
            continue;
        case 1:
        case 2:
        case 3:
        case 5:
            break;
        }
        {
            SVECTOR scale = {4096, 4096, 4096};

            sp->unkF8[r->unk4] = 1;
            anim_get_file(r->unk4 + SLOT_C, &model);
            if (model == NULL)
                continue;
            sp->unkD8.vx = e->unk18;
            sp->unkD8.vy = e->unk1A;
            sp->unkD8.vz = e->unk1C;
            if (r->unk1E & 0x40) {
                MATRIX m;

                sp->ot = world_state.ot_cur + e->unk2D;
                sp->near_z = 4 - e->unk2D;
                sp->far_z = 4096 - e->unk2D;
                model_draw(0, r->unk4 + SLOT_C, &e->unk08, &sp->unkD8, &scale, e->unk04,
                              &world_state.unk1ECE8, 128, 0, 0, 4096, e->unk24, 0);
                sp->unkD8.vx = sp->unkD8.vy = sp->unkD8.vz = 0;
                sp->ot = world_state.ot_cur + e->unk2D;
                sp->near_z = 4 - e->unk2D;
                sp->far_z = 4096 - e->unk2D;
                m = world_state.unk1EDA8;
                if (e->unk22 != 2)
                    RotMatrixX(e->unk18, &m);
                model_draw(0, r->unk4 + SLOT_C, &e->unk08, &sp->unkD8, &scale, e->unk04, &m, 129,
                              0, 0, 4096, e->unk24, 0);
            } else {
                sp->ot = world_state.ot_cur + e->unk2D;
                sp->near_z = 4 - e->unk2D;
                sp->far_z = 4096 - e->unk2D;
                model_draw(0, r->unk4 + SLOT_C, &e->unk08, &sp->unkD8, &scale, e->unk04,
                              &world_state.unk1ECE8, 128, 0, 0, 4096, e->unk24, 0);
            }
        }
    }
    if (area_state.unk1F == 0)
        func_80023ed8(5, 0, SLOT_C, 128);
    sp->unkF0 = &sp->unkF8[128];
}

ADDRESS(0x800255f0, 0x634)
void func_800255f0(void)
{
    ScratchPad *sp = SCRATCHPAD;
    StObject88 *o;
    StObject24 *f;
    StAnimFile *model;
    VECTOR *at;
    int mode;
    int kind;
    int i;

    for (o = DAT_8018e318, i = 127; i != -1; o++, i--) {
        if (o->unk0 == 0xff)
            continue;
        mode = o->unk6 & 3;
        if (mode == 0)
            continue;
        *(VECTOR *)&sp->unkE0 = o->unk20;
        at = &o->unk20;
        if (mode != 2 && !func_80016554())
            continue;
        sp->clut_base = o->unkE;
        anim_get_file(o->unkA + 96, &model);
        if (model == NULL)
            continue;
        switch (o->unk6 & 0xc) {
        case 0:
            sp->unkD8.vx = o->unk30.vx;
            sp->unkD8.vy = o->unk30.vy;
            sp->unkD8.vz = o->unk30.vz;
            sp->ot = world_state.ot_cur - 7;
            sp->near_z = 11;
            sp->far_z = 4103;
            model_draw(0, o->unkA + 96, at, &sp->unkD8, &o->unk38, o->unk48,
                          &world_state.unk1ECE8, o->unk2, o->unk14, o->unk12, o->unk12, o->unk7, 0);
            break;
        case 4:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = world_state.ot_cur - 7;
            model_draw(0, o->unkA + 96, at, &o->unk30, &o->unk38, o->unk48,
                          &DAT_8008040c, o->unk2, o->unk14, o->unk12, o->unk12, o->unk7, 0);
            break;
        case 8:
            sp->near_z = 11;
            sp->far_z = 4103;
            sp->ot = world_state.ot_cur - 7;
            model_draw(0, o->unkA + 96, at, &o->unk30, &o->unk38, o->unk48,
                          &world_state.unk1EDA8, o->unk2, o->unk14, o->unk12, o->unk12, o->unk7, 0);
            break;
        case 12:
            sp->near_z = -16;
            sp->far_z = 4076;
            sp->ot = world_state.ot_cur + 20;
            model_draw(0, o->unkA + 96, at, &o->unk30, &o->unk38, o->unk48,
                          NULL, o->unk2, o->unk14, o->unk12, o->unk12, o->unk7, 0);
            break;
        }
    }
    sp->unkD8.vx = sp->unkD8.vy = sp->unkD8.vz = 0;
    sp->clut_base = 0;
    for (f = DAT_8015c488.objects, i = 127; i != -1; f++, i--) {
        if (f->unk0 == 0xffff)
            continue;
        if (f->unk0 & 0x8000) {
            sp->ot = world_state.ot_cur;
            sp->near_z = 4;
            sp->far_z = 4096;
        } else {
            *(VECTOR *)&sp->unkE0 = f->unk8;
            at = &f->unk8;
            if (func_80016554()) {
                kind = f->unk2 & 0x3f;
                if (kind == 1) {
                    sp->unkD8.vy = (f->unk2 & 0xc0) << 4;
                    sp->near_z = 24;
                    sp->far_z = 4116;
                    sp->ot = world_state.ot_cur - 20;
                    model_draw(3, f->unk0 + 40, at, &sp->unkD8, NULL, NULL,
                                  &world_state.unk1ECE8, f->unk5 + 128, 0, 0, 4096, 7, 0);
                    sp->unkD8.vy = 0;
                    model_draw(3, f->unk0 + 40, at, &sp->unkD8, NULL, NULL,
                                  &world_state.unk1EDA8, f->unk5 + 128, 0, 0, 4096, 7, 0);
                } else if (kind == 2) {
                    sp->near_z = 24;
                    sp->far_z = 4116;
                    sp->ot = world_state.ot_cur - 20;
                    model_draw(3, f->unk0 + 40, at, &sp->unkD8, NULL, NULL,
                                  &world_state.unk1EDA8, f->unk5 + 128, 0, 0, 4096, 7, 0);
                } else if (kind == 3) {
                    sp->near_z = 24;
                    sp->far_z = 4116;
                    sp->ot = world_state.ot_cur - 20;
                    model_draw(3, f->unk0 + 40, at, &sp->unkD8, NULL, NULL,
                                  &world_state.unk1ECE8, f->unk5 + 128, 0, 0, 4096, 7, 0);
                } else {
                    sp->near_z = 4;
                    sp->far_z = 4096;
                    sp->ot = world_state.ot_cur;
                    model_draw(3, f->unk0 + 40, at, &sp->unkD8, NULL, NULL,
                                  &world_state.unk1EDA8, f->unk5 + 128, 0, 0, 4096, 5, 0);
                }
            }
        }
        if (f->unk4 != 0 && DAT_8015c488.frame % f->unk4 == 0) {
            if (++f->unk5 >= f->unk3)
                f->unk5 = 0;
        }
    }
    DAT_8015c488.frame++;
}
