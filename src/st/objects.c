/*
 * ST.EXE: the world object passes (WIP module). Each pass walks the visible
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
#include <st/bank.h>
#include <st/funcs.h>

SDATA(0x80087f04, 0x8)

/* Table A model slots start at 128 (table B: 256, table C: 588). */
#define SLOT_A 128

ADDRESS(0x800241d0, 0x514)
void func_800241d0(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Vis212 *e;
    Vis192 *t;
    VECTOR unused;
    StAnimFile *model;
    VECTOR *at;
    u8 flags;
    int depth;
    u32 bit;
    int i;

    sp->unkF0 = sp->unkF8;
    sp->unkF4 = sp->unk244;
    fill_words((u32 *)sp->unkF8, 0, 32);
    fill_words(sp->unk244, 0, 4);
    i = vis_table_a.count;
    sp->clut_base = 0;
    while (--i != -1) {
        e = &vis_table_a.entries[vis_table_a.list[i]];
        t = &vis_table_a.unk3500[e->unk02];
        anim_get_file(e->unk0B + SLOT_A, &model);
        e->unk04 = 0;
        flags = e->unk1C.bytes[2] & 0x3f;
        e->unk1C.bytes[2] = flags;
        if (e->unk3C <= world_state.unk1EE04 * world_state.unk1EE04)
            e->unk1C.bytes[2] = flags | 0x40;
        else if (e->unk3C >= world_state.unk1EE00 * world_state.unk1EE00)
            e->unk1C.bytes[2] = flags | 0x80;
        if (e->unk1C.word & 0x300000)
            depth = e->unk34;
        else
            depth = e->unk36;
        if (t->unk10 != 0xffff) {
            bit = t->unk10;
            DAT_801cba08.unk14A8[bit >> 5] |= 1 << (bit & 0x1f);
        }
        if (model != NULL) {
        at = &e->unk74;
        if (e->unk70 & 0x20) {
            sp->unkD8.vx = sp->unkD8.vy = sp->unkD8.vz = 0;
            sp->ot = world_state.ot_cur + e->unk21;
            sp->near_z = 4 - e->unk21;
            sp->far_z = 4096 - e->unk21;
            model_draw(0, e->unk0B + SLOT_A, at, &sp->unkD8, &e->unk94, &e->unkA4,
                          &DAT_8008040c, e->unk08, e->unk2A, 4096 - depth, e->unk24,
                          e->unk1C.bytes[2], 0);
        } else {
            VECTOR pos;
            SVECTOR scale = {4096, 4096, 4096, 0};

            e->unk04 = 1;
            sp->unkD8.vx = e->unk8C;
            sp->unkD8.vy = e->unk8E;
            sp->unkD8.vz = e->unk90;
            if (e->unk03 != 3) {
                sp->ot = world_state.ot_cur + e->unk21;
                sp->near_z = 4 - e->unk21;
                sp->far_z = 4096 - e->unk21;
                model_draw(0, e->unk0B + SLOT_A, at, &sp->unkD8, &e->unk94, &e->unkA4,
                              &world_state.unk1ECE8, e->unk08, e->unk2A, 4096 - depth, e->unk24,
                              e->unk1C.bytes[2], 0);
            }
            if (e->unk32 != 0) {
                pos = e->unk74;
                scale.vx = e->unk32 * 2 + 1024;
                scale.vz = e->unk32 * 2 + 1024;
                sp->ot = world_state.ot_cur + e->unk21 - 4;
                sp->near_z = 8 - e->unk21;
                sp->far_z = 4100 - e->unk21;
                model_draw(0, 44, &pos, &sp->unkD8, &scale, NULL, &world_state.unk1ECE8, 128, 0,
                              0, 0, 4, 0);
            }
        }
        } else {
            e->unk04 = 2;
        }
        if (e->unk22 != -1) {
            SpuGetAllKeysStatus(DAT_801dd1c0.keys);
            if (DAT_801dd1c0.keys[e->unk22] == 0)
                e->unk22 = -1;
        }
        sp->unkF8[e->unk0B] = 1;
        sp->unkC8[e->unk0B] = e->unk0C;
    }
    if (area_state.unk1F == 0)
        func_80023c6c(2, 0, 128, 16);
}

/*
 * Not exact: in the type-236 sound, retail computes e->unk48 << 7 into $v0
 * and copies it to $a1 for `outer` (loads of +0x48 and +0x4c in that
 * order); this build shifts straight into $a1 after loading +0x4c first.
 */
ADDRESS(0x800246e4, 0xae4)
void func_800246e4(void)
{
    ScratchPad *sp = SCRATCHPAD;
    Vis88 *e;
    Vis24 *r;
    VECTOR unused;
    StAnimFile *model;
    SVECTOR *size;
    u32 now;
    u8 flags;
    u16 id;
    int volume, range, d, dz;
    int outer, inner;
    int i;

    sp->unkF0 = sp->unkF8;
    sp->unkF4 = sp->unk244;
    fill_words((u32 *)sp->unkF8, 0, 83);
    fill_words(sp->unk244, 0, 75);
    sp->clut_base = 0;
    i = vis_table_b.count;
    now = cd_state.unk4;
    while (--i != -1) {
        e = &vis_table_b.entries[vis_table_b.list[i]];
        e->unk07 = 0;
        if (e->unk00 == 2)
            continue;
        flags = e->unk03 & 0x3f;
        e->unk03 = flags;
        if (e->unk18 <= world_state.unk1EE04 * world_state.unk1EE04)
            e->unk03 = flags | 0x40;
        else if (e->unk18 >= world_state.unk1EE00 * world_state.unk1EE00)
            e->unk03 = flags | 0x80;
        switch (e->unk05) {
        case 37:
        case 226:
        case 229:
        case 233:
            continue;
        case 31:
            /* A sound over a rectangle of the floor. */
            if (func_8004fc94(e->unk24.vx >> 12, e->unk24.vz >> 12, e->unk48, e->unk49, 0x8000)) {
                if ((s32)(e->unk50 - now) >= 0)
                    continue;
                if (e->unk4A.snd.unk4D & 2)
                    e->unk50 = now + (e->unk4A.snd.unk4E * 6 + (rand() >> 8));
                if (e->unk4A.snd.unk4D & 4)
                    e->unk50 = now + (e->unk4A.snd.unk4E * 6 + (rand() >> 6));
                else
                    e->unk50 = now + e->unk4A.snd.unk4E * 6;
                range = e->unk48 << 11;
                d = viewer.position.vx - (e->unk24.vx + range);
                if (d < 0)
                    d = -d;
                d = range - d;
                range = e->unk49 << 11;
                dz = viewer.position.vz - (e->unk24.vz + range);
                if (dz < 0)
                    dz = -dz;
                dz = range - dz;
                if (dz < d)
                    d = dz;
                volume = e->unk4A.snd.unk4C << 12;
                if (d >= volume) {
                    volume = e->unk4A.snd.unk4B;
                } else {
                    if (volume == 0)
                        continue;
                    volume = e->unk4A.snd.unk4B * d / volume;
                }
                if (e->unk4A.snd.unk4D & 1) {
                    int dy = viewer.position.vy - e->unk24.vy;

                    if (dy < 0)
                        dy = -dy;
                    dy = e->unk4A.snd.unk4B * dy >> 13;
                    volume -= dy;
                }
                if (volume < 20)
                    continue;
                if (e->unk54 != -1) {
                    func_8005850c(e->unk54, e->unk4A.snd.unk4A + 208);
                    e->unk54 = -1;
                }
                e->unk54 = func_800582f0(e->unk4A.snd.unk4A + 208, volume);
            } else {
                e->unk50 = now + e->unk4A.snd.unk4E * 6;
                if (e->unk54 == -1)
                    continue;
                func_8005850c(e->unk54, e->unk4A.snd.unk4A + 208);
                e->unk54 = -1;
            }
            break;
        case 236:
            /* A sound around a point. */
            if (func_8004fd28(e->unk24.vx, e->unk24.vz, e->unk48 << 8, e->unk49 << 8, 0x8000)) {
                if ((s32)(e->unk50 - now) >= 0)
                    continue;
                if (e->unk4A.snd.unk4D & 2)
                    e->unk50 = now + (e->unk4A.snd.unk4E * 60 + (rand() >> 11));
                if (e->unk4A.snd.unk4D & 4)
                    e->unk50 = now + (e->unk4A.snd.unk4E * 60 + (rand() >> 9));
                else
                    e->unk50 = now + e->unk4A.snd.unk4E * 60;
                inner = (outer = e->unk48 << 7) - (e->unk4A.snd.unk4C << 7);
                if (e->unk1C >= inner && outer != inner)
                    volume = e->unk4A.snd.unk4B * (outer - e->unk1C) / (outer - inner);
                else
                    volume = e->unk4A.snd.unk4B;
                if (e->unk4A.snd.unk4D & 1) {
                    int dy = viewer.position.vy - e->unk24.vy;

                    if (dy < 0)
                        dy = -dy;
                    dy = e->unk4A.snd.unk4B * dy >> 13;
                    volume -= dy;
                }
                if (volume < 20)
                    continue;
                if (e->unk54 != -1) {
                    func_8005850c(e->unk54, e->unk4A.snd.unk4A + 1108);
                    e->unk54 = -1;
                }
                e->unk54 = func_800582f0(e->unk4A.snd.unk4A + 1108, volume);
            } else {
                e->unk50 = now + e->unk4A.snd.unk4E * 60;
                if (e->unk54 == -1)
                    continue;
                func_8005850c(e->unk54, e->unk4A.snd.unk4A + 1108);
                e->unk54 = -1;
            }
            break;
        case 38:
            sp->unkF8[e->unk0a] = 1;
            size = &e->unk3C;
            id = func_80023208(e->unk0a);
            anim_get_file(id, &model);
            if (model == NULL)
                continue;
            sp->unkD8.vx = e->unk34;
            sp->unkD8.vy = e->unk36;
            sp->unkD8.vz = e->unk38;
            sp->ot = world_state.ot_cur + e->unk14;
            sp->near_z = 4 - e->unk14;
            sp->far_z = 4096 - e->unk14;
            model_draw(0, id, &e->unk24, &sp->unkD8, size, e->unk44,
                          &world_state.unk1ECE8, e->unk02, e->unk10, 0, e->unk16, e->unk03, 0);
            break;
        default:
            r = &vis_table_b.unk0000[e->unk0a];
            sp->unkF8[e->unk0a] = 1;
            id = func_80023208(e->unk0a);
            anim_get_file(id, &model);
            size = &e->unk3C;
            if (model == NULL)
                continue;
            sp->unkD8.vx = e->unk34;
            sp->unkD8.vy = e->unk36;
            sp->unkD8.vz = e->unk38;
            if (e->unk04 & 0x40) {
                sp->unkD8.vx = e->unk34;
                sp->unkD8.vy = 0;
                sp->unkD8.vz = e->unk38;
                sp->ot = world_state.ot_cur + e->unk14;
                sp->near_z = 4 - e->unk14;
                sp->far_z = 4096 - e->unk14;
                model_draw(0, id, &e->unk24, &sp->unkD8, size, e->unk44,
                              &world_state.unk1EDA8, e->unk02, e->unk10, e->unk16, e->unk16,
                              e->unk03, 0);
                break;
            }
            {
                VECTOR pos;
                SVECTOR scale = {4096, 4096, 4096, 0};

                e->unk07 = 1;
                sp->ot = world_state.ot_cur + e->unk14;
                sp->near_z = 4 - e->unk14;
                sp->far_z = 4096 - e->unk14;
                model_draw(0, id, &e->unk24, &sp->unkD8, size, e->unk44,
                              &world_state.unk1ECE8, e->unk02, e->unk10, e->unk16, e->unk16, e->unk03, 0);
                if (e->unk04 & 1)
                    continue;
                pos = e->unk24;
                if (e->unk04 & 4) {
                    scale.vx = r->unk0C * 2 + (r->unk0C >> 1);
                    scale.vz = r->unk0E * 2 + (r->unk0E >> 1);
                } else {
                    scale.vx = r->unk04 * 2 + (r->unk04 >> 1);
                    scale.vz = r->unk04 * 2 + (r->unk04 >> 1);
                }
                sp->ot = world_state.ot_cur + e->unk14 - 5;
                sp->near_z = 9 - e->unk14;
                sp->far_z = 4101 - e->unk14;
                model_draw(0, (r->unk03 >> 2 & 1) | 44, &pos, &sp->unkD8, &scale, e->unk44,
                              &world_state.unk1ECE8, 128, 0, e->unk16, 4096, 4, 0);
            }
            break;
        }
    }
    if (area_state.unk1F == 0)
        func_80024004(0, 0, 256, 249);
    sp->unkF0 = &sp->unkF8[300];
}
