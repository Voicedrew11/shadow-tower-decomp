/*
 * VS.EXE: the versus start-up module (WIP): sound, CD and TIM helpers, the
 * fighters' load and animation sounds, the graphics start-up and round
 * camera, the round's frame and loop, the end-of-match screens and main.
 * One translation unit: retail lays out its constants (0x80011000, the
 * file names here and main's strings) before its switch tables
 * (0x800110a0, func_80013c5c's and then func_80015020's), the order cc1
 * writes one object's read-only data in. $gp-relative accesses to the
 * round state, so -G8.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>

/* The first side's picked fighter and rounds won: one-word tentative
 * definitions ahead of vs/data.h, whose shared declarations leave the
 * extent open (func_80021984 and func_80029f28 index both sides from
 * them), so that ASPSX addresses them $gp-relative. */
s32 DAT_80063330[1];
s32 DAT_80063370[1];

#include <psyq/libc.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBSPU.H>
#include <LIBSND.H>
#include <LIBSN.H>
#include <st/mesh.h>
#include <st/anim.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/pad.h>
#include <vs/scratch.h>
#include <vs/sound.h>
#include <vs/funcs.h>
#include <INLINE_C.H>
#include <vs/menu.h>
#include <vs/main.h>
#include <psyq/kernel.h>
#include <LIBPAD.H>
#include <LIBMCRD.H>
#include <st/data.h>

/* The module's constants (the archive and music file names, the frame
 * statistics' format strings and main's file name), then its switch tables
 * (func_80013c5c's and func_80015020's phase table). */
RODATA(0x80011000, 0xa0)
JUMPTABLES(0x800110a0, 0x70)

/* The model renderers' light scale (func_80025204, func_80028914 and
 * func_800291d4 scale their light matrix by it). */
DATA(0x8005b2a8, 0x10, ".data")
VECTOR DAT_8005b2a8 = {ONE, ONE, ONE};

/* The module's initialized small data. */
DATA(0x80063220, 0x4, ".sdata")
s32 DAT_80063220 = 0;
DATA(0x80063224, 0x4, ".sdata")
s32 DAT_80063224 = 0;
DATA(0x80063228, 0x4, ".sdata")
s32 DAT_80063228 = 1;

/* The module's RECT initializer. */
SDATA(0x8006322c, 0x8)

/* Tentative definitions: ASPSX needs the size to address these $gp-relative.
 * An earlier object allocates them (docs/executable-linking.md). */
VsFighter *DAT_80063300;
s32 DAT_80063310;
u32 DAT_80063314;
s32 DAT_8006331c;
s32 DAT_80063320;
s32 DAT_80063324;
s32 DAT_80063338;
s32 DAT_80063350;
s32 DAT_80063354;
s16 DAT_8006335a;
s32 DAT_80063360;
u32 DAT_80063364;
s32 DAT_8006336c;

/* Loads archive member `member` into `buf`, nonzero on success
 * (src/vs/cd_8002e5e4.c). Declared without a prototype: the callers pass
 * the numbers as unmasked ints. */
int func_8002ee90();

/* The packet buffers' size in words. */
#define PACKET_WORDS (0x20000 / 4)

/* The file's mesh model and its part headers. */
#define ANIM_MODEL(file) ((u8 *)(file) + (file)->model)
#define MODEL_PARTS(model) ((MeshHeader *)((model) + 12))

/* Sets the CD (serial A) mixing volume, both channels. */
ADDRESS(0x800129bc, 0x2c)
void func_800129bc(short volume)
{
    SsSetSerialVol(SS_SERIAL_A, volume, volume);
}

/* Selects XA channel `channel` of the music file and seeks to it, muted. */
ADDRESS(0x800129e8, 0x88)
void func_800129e8(int channel)
{
    CdlFILTER filter;
    CdlFILE file;

    while (!CdSearchFile(&file, "\\ST\\VS\\VS_BGM.XA;1"))
        ;
    filter.file = 1;
    filter.chan = channel;
    func_800129bc(0);
    while (!CdControl(CdlSetfilter, (u_char *)&filter, 0))
        ;
    while (!CdControl(CdlSetloc, (u_char *)&file.pos, 0))
        ;
}

/* Opens sound bank `bank`'s VAB header and transfers its body to SPU
 * memory at 0x3e400 + bank * 0xa000. */
ADDRESS(0x80012a70, 0xe0)
void func_80012a70(int bank)
{
    VsSoundBank *b = &DAT_801604b0[bank];

    b->vab = SsVabOpenHeadSticky(b->head, -1, bank * 0xa000 + 0x3e400);
    if (b->vab != -1) {
        b->unk1002 = SsVabTransBodyPartly(DAT_800633b0.bank[bank], 0x12c00, b->vab);
        if (b->unk1002 == b->vab)
            SsVabTransCompleted(SS_WAIT_COMPLETED);
    }
}

/* Uploads a run of TIM images, shifting each image `page` texture pages
 * right and each CLUT `clut_y` lines down. */
ADDRESS(0x80012b50, 0xdc)
void func_80012b50(u_long *data, int page, int clut_y)
{
    u_long *tim = data;
    u_long flags;
    u_long *p;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    for (;;) {
        p = tim;
        if (p[0] != 0x10)
            return;
        tim += 2;
        flags = p[1];
        if (flags & 8) {
            clut_data = p + 5;
            clut = tim;
            tim = (u_long *)((u_char *)tim + p[2]);
        }
        image = tim;
        image_data = image + 3;
        tim = (u_long *)((u_char *)tim + image[0]);
        if (flags & 8) {
            ((RECT *)(clut + 1))->y += clut_y;
            LoadImage((RECT *)(clut + 1), clut_data);
        }
        ((RECT *)(image + 1))->x += page * 64;
        LoadImage((RECT *)(image + 1), image_data);
    }
}

/* Uploads a run of TIM images to the given image and CLUT positions. */
ADDRESS(0x80012c2c, 0xe4)
void func_80012c2c(u_long *data, DVECTOR *pos, DVECTOR *clut_pos)
{
    u_long *tim = data;
    u_long flags;
    u_long *p;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    for (;;) {
        p = tim;
        if (p[0] != 0x10)
            return;
        tim += 2;
        flags = p[1];
        if (flags & 8) {
            clut_data = p + 5;
            clut = tim;
            tim = (u_long *)((u_char *)tim + p[2]);
        }
        image = tim;
        image_data = image + 3;
        tim = (u_long *)((u_char *)tim + image[0]);
        if (flags & 8) {
            ((RECT *)(clut + 1))->x = clut_pos->vx;
            ((RECT *)(clut + 1))->y = clut_pos->vy;
            LoadImage((RECT *)(clut + 1), clut_data);
        }
        ((RECT *)(image + 1))->x = pos->vx;
        ((RECT *)(image + 1))->y = pos->vy;
        LoadImage((RECT *)(image + 1), image_data);
    }
}

/* Walks the faces of the current part (sp->header, from sp->faces) of one
 * primitive kind, applying `stmt` to each; face_of(type) is the face at
 * the cursor, reread through the scratchpad at each use. */
#define VS_PART_FACES(count, type, stmt)                                     \
    for (k = sp->header->count; k-- != 0; sp->faces += sizeof(type)) {      \
        stmt;                                                               \
    }
#define face_of(type) ((type *)sp->faces)

/* Applies `op` to every face of the current part, kind by kind. */
#define VS_PART_ALL_FACES(op)                                                \
    VS_PART_FACES(n_ft3, MeshFaceFT3, op(MeshFaceFT3));                      \
    VS_PART_FACES(n_ft4, MeshFaceFT4, op(MeshFaceFT4));                      \
    VS_PART_FACES(n_gt3, MeshFaceGT3, op(MeshFaceGT3));                      \
    VS_PART_FACES(n_gt4, MeshFaceGT4, op(MeshFaceGT4))

/* func_80012d10's shift: texture page and CLUT line. */
#define VS_SHIFT_TEXTURE(type)                                               \
    MESH_WORD_HI(face_of(type)->uv1_tpage) += page;                          \
    MESH_WORD_HI(face_of(type)->uv0_clut) += clut_y << 6

/* func_8001336c's semi-transparency mode (tpage bits 5-6). */
#define VS_SET_BLEND(type)                                                   \
    MESH_WORD_HI(face_of(type)->uv1_tpage) =                                 \
        (MESH_WORD_HI(face_of(type)->uv1_tpage) & ~0x60) | mode

/* Moves every face of fighter `id`'s model `id` * 4 texture pages right
 * and its CLUTs `id` * 12 lines down, then the second model's faces when
 * one is loaded. Kind 1 models walk every part of the model the caller
 * put in sp->model, kind 2 only its first. */
ADDRESS(0x80012d10, 0x65c)
void func_80012d10(int id)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    int page = id * 4;
    int clut_y = id * 12;
    int n;
    int k;

    switch (DAT_800633b0.fighter[id].unk4DF10) {
    case 1:
        sp->header = MODEL_PARTS(sp->model);
        for (n = ((u_long *)sp->model)[2]; n-- != 0; sp->header++) {
            sp->faces = sp->model + (sp->header->faces + 12);
            VS_PART_ALL_FACES(VS_SHIFT_TEXTURE);
        }
        break;
    case 2:
        sp->header = MODEL_PARTS(sp->model);
        sp->faces = sp->model + (sp->header->faces + 12);
        VS_PART_ALL_FACES(VS_SHIFT_TEXTURE);
        break;
    }
    if (DAT_800633b0.fighter[id].unk4B000[0] != 0) {
        sp->model = (u8 *)DAT_800633b0.fighter[id].unk4B000;
        sp->header = MODEL_PARTS(sp->model);
        for (n = ((u_long *)sp->model)[2]; n-- != 0; sp->header++) {
            sp->faces = sp->model + (sp->header->faces + 12);
            VS_PART_ALL_FACES(VS_SHIFT_TEXTURE);
        }
    }
}

/* Sets the semi-transparency mode (tpage bits 5-6) of every face of
 * fighter `id`'s model to `mode`: every part for a kind 1 model, the
 * first for kind 2. */
ADDRESS(0x8001336c, 0x374)
void func_8001336c(int id, int mode)
{
    VsScratchPad *sp = VS_SCRATCHPAD;
    StAnimFile *file = (StAnimFile *)DAT_800633b0.fighter[id].file;
    int n;
    int k;

    mode <<= 5;
    switch (DAT_800633b0.fighter[id].unk4DF10) {
    case 1:
        sp->model = ANIM_MODEL(file);
        sp->header = MODEL_PARTS(sp->model);
        for (n = ((u_long *)sp->model)[2]; n-- != 0; sp->header++) {
            sp->faces = sp->model + (sp->header->faces + 12);
            VS_PART_ALL_FACES(VS_SET_BLEND);
        }
        break;
    case 2:
        sp->model = ANIM_MODEL(file);
        sp->header = MODEL_PARTS(sp->model);
        sp->faces = sp->model + (sp->header->faces + 12);
        VS_PART_ALL_FACES(VS_SET_BLEND);
        break;
    }
}

/* Decodes both pad buffers: held buttons, newly pressed ones, and a
 * repeat after 30 frames, then every 5. */
ADDRESS(0x800136e0, 0x13c)
void func_800136e0(void)
{
    if (DAT_801961e0[0].status == 0) {
        pad_state.pads[0].unk0 = ~DAT_801961e0[0].buttons;
        if (pad_state.pads[0].unk0 != pad_state.pads[0].unk4) {
            pad_state.pads[0].unk10 = 30;
            pad_state.pads[0].unk8 |= pad_state.pads[0].unk0 & ~pad_state.pads[0].unk4;
            pad_state.pads[0].unkC |= pad_state.pads[0].unk0 & ~pad_state.pads[0].unk4;
        } else if (pad_state.pads[0].unk10 != 0) {
            if (--pad_state.pads[0].unk10 == 0) {
                pad_state.pads[0].unk10 = 5;
                pad_state.pads[0].unkC |= pad_state.pads[0].unk0;
            }
        }
    }
    if (DAT_801961e0[1].status == 0) {
        pad_state.pads[1].unk0 = ~DAT_801961e0[1].buttons;
        if (pad_state.pads[1].unk0 != pad_state.pads[1].unk4) {
            pad_state.pads[1].unk10 = 30;
            pad_state.pads[1].unk8 |= pad_state.pads[1].unk0 & ~pad_state.pads[1].unk4;
            pad_state.pads[1].unkC |= pad_state.pads[1].unk0 & ~pad_state.pads[1].unk4;
        } else if (pad_state.pads[1].unk10 != 0) {
            if (--pad_state.pads[1].unk10 == 0) {
                pad_state.pads[1].unk10 = 5;
                pad_state.pads[1].unkC |= pad_state.pads[1].unk0;
            }
        }
    }
    pad_state.pads[0].unk4 = pad_state.pads[0].unk0;
    pad_state.pads[1].unk4 = pad_state.pads[1].unk0;
}

/* Clears both controller records. */
ADDRESS(0x8001381c, 0x2c)
void func_8001381c(void)
{
    pad_state.pads[1].unkC = 0;
    pad_state.pads[0].unkC = 0;
    pad_state.pads[1].unk8 = 0;
    pad_state.pads[0].unk8 = 0;
    pad_state.pads[1].unk4 = 0;
    pad_state.pads[0].unk4 = 0;
    pad_state.pads[1].unk0 = 0;
    pad_state.pads[0].unk0 = 0;
}

/* A frame callback: counts. */
ADDRESS(0x80013848, 0x20)
void func_80013848(void)
{
    pad_state.unk30++;
}

/* A frame callback: counts and decodes the controllers. */
ADDRESS(0x80013868, 0x38)
void func_80013868(void)
{
    pad_state.unk3C++;
    func_800136e0();
}

/* Controller 1's buttons as the debug adjust reads them: the buffer's
 * second halfword, a constant address (retail shares one %hi of
 * DAT_801961e0 + 2 between the reads; a member access forms the buffer
 * address first). */
#define VS_PAD0_BUTTONS (((volatile s16 *)DAT_801961e0)[1])

/*
 * A debug display adjust: flips to the other frame, then while START is
 * held lets controller 1's d-pad move the display area 4 pixels a frame
 * (polling the host debugger each frame); restores the area afterwards.
 */
ADDRESS(0x800138a0, 0x24c)
void func_800138a0(void)
{
    RECT saved;
    DRAWENV *draw;
    DISPENV *disp;
    int bits;

    vs_gfx_state.draw[0].isbg = 0;
    vs_gfx_state.draw[1].isbg = 0;
    vs_gfx_state.frame = vs_gfx_state.frame == 0;
    DrawSync(0);
    VSync(0);
    PutDispEnv(&vs_gfx_state.disp[vs_gfx_state.frame]);
    PutDrawEnv(&vs_gfx_state.draw[vs_gfx_state.frame]);
    disp = &vs_gfx_state.disp[vs_gfx_state.frame];
    draw = &vs_gfx_state.draw[vs_gfx_state.frame];
    saved = disp->disp;
    while (!(VS_PAD0_BUTTONS & 0x8))
        ;
    for (;;) {
        bits = ~VS_PAD0_BUTTONS;
        if (bits & 0x20)
            disp->disp.x += 4;
        if (bits & 0x80)
            disp->disp.x -= 4;
        if (bits & 0x10)
            disp->disp.y -= 4;
        if (bits & 0x40)
            disp->disp.y += 4;
        if (bits & 0x8)
            break;
        disp->disp.x &= 0x3ff;
        disp->disp.y &= 0x1ff;
        VSync(0);
        PutDispEnv(disp);
        PutDrawEnv(draw);
        pollhost();
    }
    while (!(VS_PAD0_BUTTONS & 0x8))
        ;
    disp->disp = saved;
    vs_gfx_state.draw[0].isbg = 1;
    vs_gfx_state.draw[1].isbg = 1;
}

/* Starts libsnd: the SEQ attribute table, 60 Hz ticks, studio reverb at
 * depth 40, CD audio mixed in, full master volume. */
ADDRESS(0x80013aec, 0x88)
void func_80013aec(void)
{
    SsInit();
    SsSetMVol(0, 0);
    SsSetTableSize(DAT_801175a8, 2, 1);
    SsSetTickMode(SS_TICK60);
    SsStart2();
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(40, 40);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetMVol(127, 127);
}

/* Keys on `tone` of sound bank `bank` at `note` (clamped to 0-127) when a
 * voice is free; returns the voice SsUtKeyOn picked, or -1. The scan reads
 * the states one entry low (status[22] down to status[-1]), as retail does. */
ADDRESS(0x80013b74, 0xe8)
short func_80013b74(u_char tone, int note, int bank)
{
    int i;

    if (note < 0)
        note = 0;
    else if (note >= 128)
        note = 127;
    SpuGetAllKeysStatus(DAT_80117580);
    for (i = 23; i != -1; i--) {
        if (DAT_80117580[i - 1] == SPU_OFF)
            return SsUtKeyOn(DAT_801604b0[bank].vab, 0, tone, note, 0, 32, 32);
    }
    return -1;
}

/* Keys the entry's tone on for fighter i, at a random note from 58 to 62.
 * Every trigger test ends in its own copy of this call. */
#define KEY_ON_TONE() func_80013b74(tone, (rand() * 5 >> 15) + 58, i)

/*
 * Plays both fighters' animation sound effects. The fighter's state (and
 * in states 2 and 6 its key frame) picks a sound entry, which keeps the
 * last pick for other states; its trigger fires when the animation frame
 * passes the entry's frame (kind 1, or kind 2 on 11 frame bits; kind 8
 * then at random), or every `period` / 20 frames (kind 4). The tone is
 * keyed on at a random note from 58 to 62.
 */
ADDRESS(0x80013c5c, 0x414)
void func_80013c5c(void)
{
    int i;
    int entry;
    VsFighter *f;
    VsFighterSfx *sfx;
    u16 trigger;
    u16 tone;
    int operand;
    int kind;

    for (i = 0; i < 2; i++) {
        f = &DAT_800633b0.fighter[i];
        sfx = (VsFighterSfx *)&f->unk4DF18;
        switch (f->unk4DF0E) {
        case 0:
        case 5:
            entry = 0;
            break;
        case 1:
            entry = 1;
            break;
        case 2:
            if (f->unk4DEE8 == sfx->keys[0].frame)
                entry = 2;
            else if (f->unk4DEE8 == sfx->keys[1].frame)
                entry = 3;
            else if (f->unk4DEE8 == sfx->keys[2].frame)
                entry = 4;
            break;
        case 6:
            if (f->unk4DEE8 == sfx->keys[3].frame)
                entry = 5;
            else if (f->unk4DEE8 == sfx->keys[4].frame)
                entry = 6;
            else if (f->unk4DEE8 == sfx->keys[5].frame)
                entry = 7;
            break;
        case 3:
            entry = 8;
            break;
        case 4:
            entry = 9;
            break;
        }
        trigger = sfx->sounds[entry].trigger;
        tone = sfx->sounds[entry].tone;
        kind = trigger & 0xf000;
        operand = trigger & 0xfff;
        switch (kind) {
        case 0x1000:
            if (f->unk4DEEC > 0) {
                int frame = f->unk4DEEA;
                u16 at = operand;

                if (at >= frame - f->unk4DEEC * DAT_80063220 && at < frame)
                    KEY_ON_TONE();
            } else {
                int frame = f->unk4DEEA;
                u16 at = operand;

                if (frame - f->unk4DEEC * DAT_80063220 >= at && frame < at)
                    KEY_ON_TONE();
            }
            break;
        case 0x2000:
            if (f->unk4DEEC > 0) {
                int at = trigger & 0x7ff;
                int frame = f->unk4DEEA & 0x7ff;

                if (at >= frame - f->unk4DEEC * DAT_80063220 && at < frame)
                    KEY_ON_TONE();
            } else {
                int at = trigger & 0x7ff;
                int frame = f->unk4DEEA & 0x7ff;

                if (frame - f->unk4DEEC * DAT_80063220 >= at && frame < at)
                    KEY_ON_TONE();
            }
            break;
        case 0x8000:
            if (f->unk4DEEC > 0) {
                int frame = f->unk4DEEA;
                u16 at = operand;

                if (at >= frame - f->unk4DEEC * DAT_80063220 && at < frame && (rand() >> 3) < at)
                    KEY_ON_TONE();
            } else {
                int frame = f->unk4DEEA;
                u16 at = operand;

                if (frame - f->unk4DEEC * DAT_80063220 >= at && frame < at && (rand() >> 3) < at)
                    KEY_ON_TONE();
            }
            break;
        case 0x4000: {
            u16 at = operand;

            if (at != 0 && pad_state.unk40 / 20 % at == 0)
                KEY_ON_TONE();
            break;
        }
        }
    }
}

/* Opens the five archives versus mode reads. */
ADDRESS(0x80014070, 0x68)
void func_80014070(void)
{
    func_8002f014(0, "VS\\VS_CRT.T");
    func_8002f014(5, "VS\\VS_ETC.T");
    func_8002f014(8, "COM\\STAT.T");
    func_8002f014(10, "COM\\CRT.T");
    func_8002f014(11, "COM\\ICON.T");
}

/* The fighter being loaded. */
#define FIGHTER (DAT_800633b0.fighter[i])

/* Loads both fighters of the current round: each one's model, sound bank,
 * animations and second model from archive 0 (five members per
 * character), registers its animations, and resets its state from its
 * character's data block.
 *
 * Not exact: the frame only. Retail's is 80 bytes (ours 64): sp+16..31 is
 * never referenced and the loop's spilled offset sits at sp+32. Sixteen
 * bytes ahead of the spill slot are either a 32-byte outgoing argument
 * area (a call with seven or eight arguments that left no code) or an
 * unused 16-byte local; nothing in the image names either, so none is
 * invented. The code itself is retail's once the fighter is indexed as
 * DAT_800633b0.fighter[i] at every use and the page and palette row are
 * i * 4 and i * 12 at the call.
 */
ADDRESS(0x800140d8, 0x3a8)
void func_800140d8(void)
{
    int member;
    int i;

    for (i = 0; i < 2; i++) {
        member = DAT_801624b8[i].slot[DAT_80063338] * 5;
        func_8002ee90(0, member + 1, (u_long *)&FIGHTER);
        func_80012b50((u_long *)&FIGHTER, i * 4, i * 12);
        DrawSync(0);
        func_8002ee90(0, member + 2, (u_long *)DAT_801604b0[i].head);
        func_8002ee90(0, member + 3, (u_long *)&FIGHTER);
        func_80012a70(i);
        if (func_8002ee90(0, member + 4, FIGHTER.unk4B000))
            func_8002345c((ModelFile *)FIGHTER.unk4B000);
        else
            FIGHTER.unk4B000[0] = 0;
        func_8002ee90(0, member, (u_long *)&FIGHTER);
        if (((StAnimFile *)FIGHTER.file)->unk6) {
            FIGHTER.unk4DF10 = 1;
            func_80023764(i, (StAnimFile *)FIGHTER.file);
        } else {
            FIGHTER.unk4DF10 = 2;
            func_80027fec(i, (StAnimFile *)FIGHTER.file);
        }
        func_80012d10(i);
        FIGHTER.unk4DF18 = DAT_8004a224[DAT_801624b8[i].slot[DAT_80063338]];
        FIGHTER.unk4DF0E = 0;
        FIGHTER.unk4DEEA = 0;
        FIGHTER.unk4DEF0.vz = 0;
        FIGHTER.unk4DF00 = 0;
        FIGHTER.unk4DEE8 = FIGHTER.unk4DF18.unk0[0].unk0;
        FIGHTER.unk4DEEC = FIGHTER.unk4DF18.unk0[0].unk2;
        FIGHTER.unk4DF08 = FIGHTER.unk4DF18.unk1B8;
        FIGHTER.unk4DF0A = FIGHTER.unk4DF18.unk1BA;
        FIGHTER.unk4DEF0.vy = -FIGHTER.unk4DF18.unk1B2;
        FIGHTER.unk4DF02 = i == 0 ? 3072 : 1024;
        FIGHTER.unk4DF12 = 128;
        FIGHTER.unk4DF04 = 0;
        FIGHTER.unk4DEE0 = 0;
        FIGHTER.unk4DEE4 = 0;
        FIGHTER.unk4DF14 = 0;
        FIGHTER.unk4DF0C = 0;
    }
    if (DAT_800633b0.fighter[0].unk4DF18.unk1B4 >= 1400
        || DAT_800633b0.fighter[1].unk4DF18.unk1B4 >= 1400) {
        DAT_800633b0.fighter[0].unk4DEF0.vx = -3072;
        DAT_800633b0.fighter[1].unk4DEF0.vx = 3072;
    } else {
        DAT_800633b0.fighter[0].unk4DEF0.vx = -2048;
        DAT_800633b0.fighter[1].unk4DEF0.vx = 2048;
    }
}

/* The camera's height over the fighters' midpoint and its distance from
 * them, by the fighters' size. */
#define CAMERA_NEAR 1400
#define CAMERA_NEAR_TALL 1700
#define CAMERA_FAR 8000

/* The graphics start-up, VS's version of ST.EXE's func_80016fb8
 * (src/st/gfx.c): the 320x240 double buffer, the GTE offset, screen, back
 * and far colours, and the two packet buffers. */
ADDRESS(0x80014480, 0x1b4)
void func_80014480(void)
{
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&vs_gfx_state.draw[0], 0, 0, 320, 240);
    SetDefDispEnv(&vs_gfx_state.disp[0], 0, 240, 320, 240);
    SetDefDrawEnv(&vs_gfx_state.draw[1], 0, 240, 320, 240);
    SetDefDispEnv(&vs_gfx_state.disp[1], 0, 0, 320, 240);
    vs_gfx_state.draw[0].dtd = 1;
    vs_gfx_state.draw[1].dtd = 1;
    vs_gfx_state.draw[0].isbg = 1;
    vs_gfx_state.draw[1].isbg = 1;
    setRGB0(&vs_gfx_state.draw[0], 0, 0, 0);
    setRGB0(&vs_gfx_state.draw[1], 0, 0, 0);
    PutDispEnv(&vs_gfx_state.disp[0]);
    PutDrawEnv(&vs_gfx_state.draw[0]);
    gte_SetBackColor(80, 80, 80);
    gte_SetFarColor(5, 5, 5);
    vs_gfx_state.frame = 255;
    vs_gfx_state.unkE0 = DAT_8011c528;
    vs_gfx_state.packets[0].start = vs_gfx_state.unkE0;
    vs_gfx_state.packets[0].end = vs_gfx_state.packets[0].start + PACKET_WORDS;
    vs_gfx_state.packets[1].start = vs_gfx_state.packets[0].end;
    vs_gfx_state.packets[1].end = vs_gfx_state.packets[1].start + PACKET_WORDS;
    SetDispMask(1);
}

/*
 * Moves the camera: after a round it circles the winner (fighter
 * DAT_80063324 & 1), framed by its height; otherwise it looks at the
 * fighters' midpoint from the side, from a distance that follows their
 * separation, sweeping in during the round start. The camera eases toward
 * its target at DAT_80063354 units per frame, keeps within +-32767 on x
 * and z, and faces the target.
 */
ADDRESS(0x80014634, 0x9ec)
void func_80014634(void)
{
    SVECTOR look;
    SVECTOR eye;
    VsFighter *f;
    int height;
    int dist;
    int near;
    int far;
    int x;
    int y;
    int z;

    if (DAT_80063324 & 8) {
        f = &DAT_800633b0.fighter[DAT_80063324 & 1];
        height = f->unk4DF18.unk1B6 + f->unk4DF18.unk1B2;
        if (height > 3000) {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1800;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536) * 3 / 2;
            eye.vy = -800;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536) * 3 / 2;
        } else if (height > 2000) {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1500;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536);
            eye.vy = -500;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536);
        } else if (height < 1000) {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1000;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536);
            eye.vy = -1000;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536);
        } else {
            look.vx = f->unk4DEF0.vx;
            look.vz = f->unk4DEF0.vz;
            look.vy = -1200;
            eye.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 1536) * 3 / 4;
            eye.vy = -500;
            eye.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 1536) * 3 / 4;
        }
        if (pad_state.unk3C < DAT_80063314 + 5) {
            vs_gfx_state.unk2CA78.vx = f->unk4DEF0.vx + rsin(f->unk4DF02 + 3072) * 2;
            vs_gfx_state.unk2CA78.vy = -7000;
            vs_gfx_state.unk2CA78.vz = f->unk4DEF0.vz + rcos(f->unk4DF02 + 3072) * 2;
        }
    } else {
        int delta_x;
        int delta_z;

        look.vx = (DAT_800633b0.fighter[0].unk4DEF0.vx + DAT_800633b0.fighter[1].unk4DEF0.vx) / 2;
        delta_x = DAT_800633b0.fighter[1].unk4DEF0.vx - look.vx;
        look.vz = (DAT_800633b0.fighter[0].unk4DEF0.vz + DAT_800633b0.fighter[1].unk4DEF0.vz) / 2;
        delta_z = DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz;
        look.vy = (DAT_800633b0.fighter[0].unk4DEF0.vy + DAT_800633b0.fighter[1].unk4DEF0.vy) / 2;
        dist = SquareRoot0(delta_z * delta_z + delta_x * delta_x) + 1;
        near = CAMERA_NEAR;
        far = CAMERA_FAR;
        if (DAT_800633b0.fighter[0].unk4DF18.unk1B6 > 2500
            || DAT_800633b0.fighter[1].unk4DF18.unk1B6 > 2500)
            near = CAMERA_NEAR_TALL;
        if (dist < near) {
            eye.vy = -near * 2;
            eye.vx = look.vx + (DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz) * near * 3 / dist;
            eye.vz = look.vz + (look.vx - DAT_800633b0.fighter[1].unk4DEF0.vx) * near * 3 / dist;
        } else if (dist > far) {
            eye.vy = -far * 2;
            eye.vx = look.vx + (DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz) * far * 3 / dist;
            eye.vz = look.vz + (look.vx - DAT_800633b0.fighter[1].unk4DEF0.vx) * far * 3 / dist;
        } else {
            eye.vx = look.vx + (DAT_800633b0.fighter[1].unk4DEF0.vz - look.vz) * 3;
            eye.vy = -dist * 2;
            eye.vz = look.vz + (look.vx - DAT_800633b0.fighter[1].unk4DEF0.vx) * 3;
        }
        if (DAT_80063324 == 0) {
            if ((s32)DAT_80063364 < 34) {
                look.vx = 0;
                look.vz = DAT_80063364 * 300 - 10000;
            } else {
                look.vx = 0;
                look.vz = 0;
            }
        }
    }
    eye.vx -= vs_gfx_state.unk2CA78.vx;
    eye.vy -= vs_gfx_state.unk2CA78.vy;
    eye.vz -= vs_gfx_state.unk2CA78.vz;
    dist = SquareRoot0(eye.vx * eye.vx + eye.vy * eye.vy + eye.vz * eye.vz) + 1;
    if (dist < 200) {
        DAT_80063354 = 0;
    } else if (dist < 1000) {
        if (DAT_80063354 > 100)
            DAT_80063354 -= 20;
        else if (DAT_80063354 < 100)
            DAT_80063354 += 20;
    } else if (dist < 3000) {
        if (DAT_80063354 > 200)
            DAT_80063354 -= 20;
        else if (DAT_80063354 < 200)
            DAT_80063354 += 20;
    } else {
        if (DAT_80063354 > 500)
            DAT_80063354 -= 20;
        else if (DAT_80063354 < 500)
            DAT_80063354 += 20;
    }
    x = eye.vx * DAT_80063354 / dist;
    y = eye.vy * DAT_80063354 / dist;
    z = eye.vz * DAT_80063354 / dist;
    vs_gfx_state.unk2CA78.vx += x;
    vs_gfx_state.unk2CA78.vy += y;
    vs_gfx_state.unk2CA78.vz += z;
    if (vs_gfx_state.unk2CA78.vx > 32767) {
        vs_gfx_state.unk2CA78.vy += 32767 - vs_gfx_state.unk2CA78.vx;
        vs_gfx_state.unk2CA78.vx = 32767;
    }
    if (vs_gfx_state.unk2CA78.vx < -32767) {
        vs_gfx_state.unk2CA78.vy += 32767 + vs_gfx_state.unk2CA78.vx;
        vs_gfx_state.unk2CA78.vx = -32767;
    }
    if (vs_gfx_state.unk2CA78.vz > 32767) {
        vs_gfx_state.unk2CA78.vy += 32767 - vs_gfx_state.unk2CA78.vz;
        vs_gfx_state.unk2CA78.vz = 32767;
    }
    if (vs_gfx_state.unk2CA78.vz < -32767) {
        vs_gfx_state.unk2CA78.vy += 32767 + vs_gfx_state.unk2CA78.vz;
        vs_gfx_state.unk2CA78.vz = -32767;
    }
    if (look.vz == vs_gfx_state.unk2CA78.vz)
        vs_gfx_state.unk2CA88.vy = 2048;
    else
        vs_gfx_state.unk2CA88.vy = ratan2(look.vx - vs_gfx_state.unk2CA78.vx,
                                          look.vz - vs_gfx_state.unk2CA78.vz);
    look.vx -= vs_gfx_state.unk2CA78.vx;
    look.vz -= vs_gfx_state.unk2CA78.vz;
    if (DAT_80063324 & 8)
        vs_gfx_state.unk2CA88.vx = ratan2(vs_gfx_state.unk2CA78.vy - look.vy,
                                          SquareRoot0(look.vx * look.vx + look.vz * look.vz));
    else
        vs_gfx_state.unk2CA88.vx = ratan2((vs_gfx_state.unk2CA78.vy - look.vy) * 2 / 3,
                                          SquareRoot0(look.vx * look.vx + look.vz * look.vz));
    RotMatrixZYX(&vs_gfx_state.unk2CA88, &vs_gfx_state.unk2C998);
    TransposeMatrix(&vs_gfx_state.unk2C998, &vs_gfx_state.unk2C998);
    vs_gfx_state.unk2C998.t[0] = 0;
    vs_gfx_state.unk2C998.t[1] = 0;
    vs_gfx_state.unk2C998.t[2] = 0;
}

/* Not reconstructed callees (src/vs/hud_80029f28.c and others). */
void func_8002a360(int kind, int level);
void func_8002ab58(int level, int wide);
void func_800297fc(int time);
void func_80029b10(int level);
void func_80029f28(int phase, int scale);
void func_80028fcc(int hold);
void func_8002bfe8(int n, int scale);
void func_8002d998(void);
void func_800237e4(void);
void func_800238c0(void);
void func_80028074(void);
void func_800280e0(void);
void func_800129bc(short volume);
int func_8002b078(int frame);
void func_8002ad40(int level);
void func_80025204(void);
void func_80028914(void);
void func_8002c2c4(int n);
void func_80035b68(void);
void func_800291d4(u_long *model, int flags, SVECTOR *a, SVECTOR *b, SVECTOR *c);

/* The callback's frame clock and the current screen's starting frame. */
#define FRAME_CLOCK() (pad_state.unk3C)
#define SCREEN_START() (DAT_80063314)
#define ELAPSED() (FRAME_CLOCK() - SCREEN_START())

/* Arena pieces beyond this distance from the centre are hidden. */
#define ARENA_EDGE 12000

/*
 * Draws one frame of the round: swaps the buffers, runs the round start
 * and the screen of the current phase (DAT_80063324: the captions and
 * menus func_8002a360 fades by level), the clock, both fighters, their
 * gauges and the arena (the pieces past the camera hidden), then shows the
 * frame and measures its length in fields (DAT_80063220, at most 5).
 */
ADDRESS(0x80015020, 0xe30)
void func_80015020(void)
{
    SVECTOR rot;
    SVECTOR pos;
    SVECTOR scale;
    StPacketBuffer *packet;
    int i;
    u8 mask;
    s16 a;
    s16 b;

    vs_gfx_state.frame = vs_gfx_state.frame == 0;
    packet = &vs_gfx_state.packets[vs_gfx_state.frame];
    vs_gfx_state.packet = packet;
    packet->cur = packet->start;
    vs_gfx_state.ot_cur = vs_gfx_state.ot[vs_gfx_state.frame];
    ClearOTagR(vs_gfx_state.ot_cur, VS_OT_LENGTH);
    vs_gfx_state.unk8328 = 0;
    vs_gfx_state.unk8324 = 0;
    vs_gfx_state.unk831C[1] = 0;
    vs_gfx_state.unk832C = 0;
    vs_gfx_state.unk8330 = 0;
    vs_gfx_state.unk8338[0] = 0;
    vs_gfx_state.unk8338[1] = 0;
    vs_gfx_state.unk8338[2] = 0;
    vs_gfx_state.unk8338[3] = 0;
    vs_gfx_state.unk8338[4] = 0;
    func_800237e4();
    func_800280e0();
    if (DAT_80063324 == 0 && func_8002b078(DAT_80063364++)) {
        DAT_8006331c = 3;
        DAT_80063324 = 1;
        DAT_80063364 = pad_state.unk3C + 1800;
        DAT_80063314 = pad_state.unk3C;
        while (!CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSF))
            ;
        func_800129bc(64);
    }
    if (DAT_80063324 != 0)
        func_8002ad40(32);
    switch (DAT_80063324) {
    case 1: {
        u32 end;

        if (ELAPSED() < 16) {
            u32 fade = SCREEN_START();

            fade += 16;
            func_8002a360(DAT_8006331c, fade - FRAME_CLOCK());
            if (DAT_8006331c == 12) {
                u32 left;
                u32 rest = 16 - ELAPSED();

                func_8002a360(17, rest - DAT_8006336c + 2);
                func_8002a360(18, DAT_8006336c - ELAPSED() + 6);
                left = SCREEN_START();
                left += 16;
                func_8002ab58(left - FRAME_CLOCK(), 0);
            }
        }
        end = DAT_80063364;
        if (FRAME_CLOCK() < end)
            func_800297fc(end - FRAME_CLOCK());
        else
            func_800297fc(0);
        break;
    }
    case 2:
        if (ELAPSED() < 16) {
            func_8002a360(12, ELAPSED());
            func_8002a360(17, ELAPSED());
            func_8002a360(18, ELAPSED() / 3);
            func_8002ab58(ELAPSED(), 0);
        } else {
            func_8002a360(12, 16);
            func_8002a360(17, 16 - DAT_8006336c);
            func_8002a360(18, DAT_8006336c + 6);
            func_8002ab58(16, 0);
        }
        func_800297fc(DAT_80063364);
        break;
    case 4:
    draw:
        if (ELAPSED() <= 200)
            func_8002a360(0, ELAPSED());
        if (ELAPSED() > 200) {
            if (DAT_80063338 == DAT_80063228 - 1)
                func_8002a360(0, 216 - ELAPSED());
            else
                func_8002a360(0, 16);
        }
        func_800297fc(DAT_80063364);
        break;
    case 5:
        if (ELAPSED() < 30) {
            func_8002a360(5, ELAPSED());
            func_800297fc(0);
            break;
        }
        if (ELAPSED() < 46) {
            u32 left = SCREEN_START();

            left += 46;
            func_8002a360(5, left - FRAME_CLOCK());
            func_800297fc(0);
            break;
        }
        a = (DAT_800633b0.fighter[0].unk4DF08 << 12) / DAT_800633b0.fighter[0].unk4DF18.unk1B8;
        b = (DAT_800633b0.fighter[1].unk4DF08 << 12) / DAT_800633b0.fighter[1].unk4DF18.unk1B8;
        DAT_80063364 = 0;
        DAT_80063314 = pad_state.unk3C;
        if (a == b) {
            DAT_80063324 = 4;
            goto draw;
        }
        if (b < a) {
            DAT_80063324 = 8;
            func_8001336c(1, 1);
        } else {
            DAT_80063324 = 9;
            func_8001336c(0, 1);
        }
        goto result;
    case 6:
    case 7:
        if (ELAPSED() < 30) {
            func_8002a360(1, ELAPSED());
            func_8002a360(4, ELAPSED());
        } else if (ELAPSED() < 46) {
            u32 left = SCREEN_START() + 46;
            u32 left2;

            func_8002a360(1, left - FRAME_CLOCK());
            left2 = SCREEN_START() + 46;
            func_8002a360(4, left2 - FRAME_CLOCK());
        } else {
            DAT_80063314 = pad_state.unk3C;
            DAT_80063324 += 2;
            func_8001336c(9 - DAT_80063324, 1);
            goto result;
        }
        func_800297fc(DAT_80063364);
        break;
    case 8:
    case 9:
    result:
        if (ELAPSED() > 30 && ELAPSED() < 159)
            DAT_800633b0.fighter[~DAT_80063324 & 1].unk4DF12 =
                DAT_80063314 - (pad_state.unk3C - 158);
        if (ELAPSED() > 100 && ELAPSED() <= 200)
            func_8002a360(2, ELAPSED() - 100);
        if (ELAPSED() > 200) {
            if (DAT_80063338 == DAT_80063228 - 1)
                func_8002a360(2, 216 - ELAPSED());
            else
                func_8002a360(2, 16);
        }
        func_800297fc(DAT_80063364);
        break;
    case 16:
        func_8002a360(9, ELAPSED());
        func_8002a360(19, ELAPSED());
        func_8002ab58(ELAPSED(), 1);
        func_800297fc(DAT_80063364);
        break;
    case 17:
        func_8002a360(19, 16 - ELAPSED());
        func_8002a360(9, 16 - ELAPSED());
        func_8002a360(15, ELAPSED());
        func_8002a360(14, ELAPSED());
        func_8002a360(11, ELAPSED());
        func_8002ab58(11, 1);
        func_800297fc(DAT_80063364);
        break;
    case 18:
        if (ELAPSED() >= 20)
            goto menu;
        func_8002a360(19, 16 - ELAPSED());
        func_8002a360(9, 16 - ELAPSED());
        func_8002a360(10, ELAPSED() - 4);
        func_8002a360(16, ELAPSED() - 4);
        func_8002a360(13, ELAPSED() - 4);
        func_8002a360(17, ELAPSED() - 4);
        func_8002a360(18, ELAPSED() / 3);
        func_8002ab58(16, 1);
        func_800297fc(DAT_80063364);
        break;
    case 19:
        func_8002a360(10, 16 - ELAPSED());
        func_8002a360(16, 16 - ELAPSED());
        func_8002a360(13, 16 - ELAPSED());
        {
            u32 rest = 16 - ELAPSED();

            func_8002a360(17, rest - DAT_8006336c + 2);
        }
        func_8002a360(18, DAT_8006336c - ELAPSED() + 6);
        func_8002a360(19, ELAPSED() - 4);
        func_8002a360(9, ELAPSED() - 4);
        func_8002ab58(16, 1);
        func_800297fc(DAT_80063364);
        break;
    case 20:
        if (ELAPSED() < 20) {
            func_8002a360(11, 16 - ELAPSED());
            func_8002a360(15, 16 - ELAPSED());
            func_8002a360(14, 16 - ELAPSED());
            func_8002a360(10, ELAPSED() - 4);
            func_8002a360(16, ELAPSED() - 4);
            func_8002a360(13, ELAPSED() - 4);
            func_8002a360(17, ELAPSED() - 4);
            func_8002a360(18, ELAPSED() / 3);
        } else {
        menu:
            func_8002a360(10, 16);
            func_8002a360(16, 16);
            func_8002a360(13, 16);
            func_8002a360(17, 16 - DAT_8006336c);
            func_8002a360(18, DAT_8006336c + 6);
        }
        func_8002ab58(16, 1);
        func_800297fc(DAT_80063364);
        break;
    }
    if (DAT_80063324 != 0) {
        for (i = 0; i < 2; i++) {
            DAT_80063300 = &DAT_800633b0.fighter[i];
            switch (DAT_80063300->unk4DF10) {
            case 1:
                func_80025204();
                break;
            case 2:
                DAT_80063360 = 0;
                func_80028914();
                break;
            }
            if (DAT_80063300->unk4DF14) {
                func_8002c2c4(i);
                DAT_80063300->unk4DF14--;
            }
            func_8002bfe8(i, 4096);
        }
        func_80029b10(32);
        func_80029f28((u32)pad_state.unk3C % 96, 32);
    }
    func_80028fcc(DAT_80063324 == 2 || DAT_80063324 >= 16);
    rot.vz = 0;
    rot.vy = 0;
    rot.vx = 0;
    pos.vz = 0;
    pos.vy = 0;
    pos.vx = 0;
    scale.vz = 4;
    scale.vy = 4;
    scale.vx = 4;
    func_80035b68();
    func_8002d998();
    DAT_80063224 = 2;
    func_800291d4(DAT_80117708, 0, &rot, &rot, &scale);
    DAT_80063224 = 4;
    func_800291d4((u_long *)DAT_8015c528, 0, &rot, &rot, &scale);
    DAT_80063224 = 3;
    mask = 0;
    if (vs_gfx_state.unk2CA78.vz > ARENA_EDGE)
        mask = 0x83;
    if (vs_gfx_state.unk2CA78.vx > ARENA_EDGE)
        mask |= 0x0e;
    if (vs_gfx_state.unk2CA78.vz < -ARENA_EDGE)
        mask |= 0x38;
    if (vs_gfx_state.unk2CA78.vx < -ARENA_EDGE)
        mask |= 0xe0;
    for (i = 0; i < 8; i++) {
        DAT_80063350 = mask & (1 << i);
        func_800291d4((u_long *)DAT_8015d8b0, 0, &rot, &pos, &scale);
        pos.vy += 512;
    }
    DAT_80063224 = 0;
    DrawSync(0);
    DAT_80063220 = VSync(0) / 256 + 1;
    if (DAT_80063220 >= 6)
        DAT_80063220 = 5;
    FntFlush(-1);
    PutDispEnv(&vs_gfx_state.disp[vs_gfx_state.frame]);
    PutDrawEnv(&vs_gfx_state.draw[vs_gfx_state.frame]);
    DrawOTag(vs_gfx_state.ot_cur + VS_OT_LENGTH - 1);
    func_800238c0();
    func_80028074();
    vs_gfx_state.unk2C970++;
}

/* Top of the heap: main RAM below the 8 KB stack. */
#define HEAP_TOP ((u8 *)0x801fe000)

/* The heap starts at the first 2 KB boundary past the load image. */
#define HEAP_ALIGN 2048

/* Frames each screen fades over. */
#define FADE_FRAMES 20

/* DAT_80063324: the round loop's screens (func_80015020 draws them). */
#define SCREEN_FADE_IN 16
#define SCREEN_SAVE_ASK 17
#define SCREEN_RETRY_ASK 18
#define SCREEN_FADE_OUT 19
#define SCREEN_SAVE 20

/* DAT_80063324: the round's phase. */
#define PHASE_START 0
#define PHASE_FIGHT 1
#define PHASE_PAUSE 2
#define PHASE_TIME_UP 5
#define PHASE_DRAW 4
#define PHASE_KO_SECOND 6
#define PHASE_KO_FIRST 7
#define PHASE_FIRST_WINS 8
#define PHASE_SECOND_WINS 9

/* Frames a round's result shows. */
#define RESULT_FRAMES 240

/*
 * Plays one round: loads the fighters, places the camera, starts the
 * round's music, then runs the frame loop: the pause menu (resume or
 * quit), the round clock, knock-outs and the result, until the result has
 * shown for RESULT_FRAMES frames. Button 1 with 8 held toggles the frame
 * statistics, button 1 held for a second calls func_800138a0.
 */
ADDRESS(0x80015e50, 0x638)
void func_80015e50(void)
{
    int choice;
    int ko;
    int side;

    DAT_80063354 = 100;
    DAT_80063324 = PHASE_START;
    DAT_80063364 = 0;
    if (DAT_80063330[0] != -1 && DAT_80063334 != -1)
        DAT_80063320 = 1;
    else
        DAT_80063320 = 0;
    DAT_80063310 = 0;
    func_800140d8();
    func_800352cc();
    func_8002d710();
    vs_gfx_state.unk2CA78.vy = -14000;
    vs_gfx_state.unk2CA78.vx = 0;
    vs_gfx_state.unk2CA78.vz = -15000;
    vs_gfx_state.unk2CA88.vx = 0;
    vs_gfx_state.unk2CA88.vy = 0;
    vs_gfx_state.unk2CA88.vz = 0;
    if (DAT_80063338 == 0) {
        func_80016d94();
        DrawSync(0);
    }
    func_800129e8(DAT_80063338);
    for (;;) {
        func_80014634();
        if (DAT_80063324 == PHASE_PAUSE) {
            choice = 0;
            DAT_8006336c = 0;
            for (;;) {
                if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x10)
                    choice = 0;
                if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x40)
                    choice = 1;
                if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 9) {
                    DAT_80063364 += pad_state.unk3C;
                    DAT_80063324 = PHASE_FIGHT;
                    DAT_80063314 = pad_state.unk3C;
                    while (!CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSF))
                        ;
                    break;
                }
                if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x4000) {
                    if (choice == 0) {
                        DAT_80063364 += pad_state.unk3C;
                        DAT_80063324 = PHASE_FIGHT;
                        DAT_80063314 = pad_state.unk3C;
                        while (!CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSF))
                            ;
                        break;
                    }
                    DAT_80063338 = DAT_80063228;
                    DAT_80063370[0] = 0;
                    DAT_80063374 = 0;
                    return;
                }
                if (choice) {
                    if (++DAT_8006336c > 10)
                        DAT_8006336c = 10;
                } else {
                    if (--DAT_8006336c < 0)
                        DAT_8006336c = 0;
                }
                pad_state.pads[0].unk8 = 0;
                pad_state.pads[1].unk8 = 0;
                func_80015020();
            }
        } else if (DAT_80063324 == PHASE_FIGHT
                   && ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 9)) {
            while (!CdControl(CdlPause, 0, 0))
                ;
            DAT_8006331c = 12;
            DAT_80063324 = PHASE_PAUSE;
            DAT_80063364 -= pad_state.unk3C;
            DAT_80063314 = pad_state.unk3C;
        }
        if ((pad_state.pads[0].unk8 & 1) && (pad_state.pads[0].unk0 & 8))
            pad_state.unk28 = 1 - pad_state.unk28;
        if (pad_state.pads[0].unk0 & 1) {
            if (++pad_state.unk2C >= 60)
                func_800138a0();
        } else {
            pad_state.unk2C = 0;
        }
        if (DAT_80063324 == PHASE_FIGHT) {
            if (DAT_80063364 <= pad_state.unk3C) {
                DAT_80063324 = PHASE_TIME_UP;
                DAT_80063314 = pad_state.unk3C;
            } else {
                if (DAT_800633b0.fighter[0].unk4DF08 == 0) {
                    if (DAT_800633b0.fighter[1].unk4DF08 == 0) {
                        DAT_80063324 = PHASE_DRAW;
                        DAT_80063364 -= pad_state.unk3C;
                        DAT_80063314 = pad_state.unk3C;
                    } else {
                        ko = 0;
                        side = 1;
                        DAT_80063324 = PHASE_SECOND_WINS;
                        DAT_80063364 -= pad_state.unk3C;
                        DAT_80063314 = pad_state.unk3C;
                        func_8001336c(ko, side);
                    }
                } else if (DAT_800633b0.fighter[1].unk4DF08 == 0) {
                    ko = 1;
                    side = 1;
                    DAT_80063324 = PHASE_FIRST_WINS;
                    DAT_80063364 -= pad_state.unk3C;
                    DAT_80063314 = pad_state.unk3C;
                    func_8001336c(ko, side);
                }
                if (func_8002e510(0)) {
                    DAT_80063324 = PHASE_KO_FIRST;
                    DAT_80063364 -= pad_state.unk3C;
                    DAT_80063314 = pad_state.unk3C;
                }
                if (func_8002e510(1)) {
                    DAT_80063324 = PHASE_KO_SECOND;
                    DAT_80063364 -= pad_state.unk3C;
                    DAT_80063314 = pad_state.unk3C;
                }
            }
        } else if (DAT_80063324 == PHASE_DRAW || DAT_80063324 == PHASE_FIRST_WINS
                   || DAT_80063324 == PHASE_SECOND_WINS) {
            func_8001381c();
            if (pad_state.unk3C - DAT_80063314 > RESULT_FRAMES) {
                if (DAT_80063324 == PHASE_FIRST_WINS)
                    DAT_80063370[0]++;
                else if (DAT_80063324 == PHASE_SECOND_WINS)
                    DAT_80063374++;
                DAT_80063338++;
                return;
            }
        }
        func_800352fc();
        if (DAT_80063324 != PHASE_START && DAT_80063324 != PHASE_PAUSE) {
            func_80031478(DAT_80063364 & 1);
            func_80031478(1 - (DAT_80063364 & 1));
            func_8002dcd8();
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        func_80015020();
        func_80013c5c();
        vs_gfx_state.unk831C[1] = 0;
        vs_gfx_state.unk8324 = 0;
        if (pad_state.unk40 + 59 < pad_state.unk3C) {
            vs_gfx_state.unk2C971 = vs_gfx_state.unk2C970 * 60
                                    / (pad_state.unk3C - pad_state.unk40);
            vs_gfx_state.unk2C970 = 0;
            pad_state.unk40 = pad_state.unk3C;
        }
        if (pad_state.unk28 == 1) {
            FntPrint("frm      =%d\n", vs_gfx_state.unk2C971);
            FntPrint("frm/draw =%d\n", DAT_80063220);
            FntPrint("calc_poly=%d\n", vs_gfx_state.unk831C[1]);
            FntPrint("draw_poly=%d\n", vs_gfx_state.unk8324);
        }
    }
}

/* Loads the menu screens' resources: the item names (archive 8 member 2)
 * with the picture behind them, the font pages (archive 5 member 10) and
 * the four files func_8002345c sets up, then clears the round state. */
ADDRESS(0x80016488, 0x16c)
void func_80016488(void)
{
    RECT pos;
    RECT clut;
    RECT unused = {0, 0, 1023, 511};
    u_long *p;

    pos.x = 640;
    pos.y = 0;
    clut.x = 640;
    clut.y = 480;
    p = DAT_80117708;
    func_8002ee90(8, 2, p);
    func_8002e630((u32 *)DAT_80196230[1].offsets, (u32 *)(p + 1), 0x1500);
    p = (u_long *)((u8 *)p + (p[0] + 4));
    func_80012c2c(p + 1, (DVECTOR *)&pos, (DVECTOR *)&clut);
    DrawSync(0);
    func_8002ee90(5, 10, DAT_80117708);
    func_80012b50(DAT_80117708, 0, 0);
    DrawSync(0);
    func_8002ee90(5, 0, DAT_80117708);
    func_8002345c((ModelFile *)DAT_80117708);
    func_8002ee90(5, 1, (u_long *)DAT_8015c528);
    func_8002345c((ModelFile *)DAT_8015c528);
    func_8002ee90(5, 2, (u_long *)DAT_8015d8b0);
    func_8002345c((ModelFile *)DAT_8015d8b0);
    func_8002ee90(5, 3, (u_long *)DAT_8015ec38);
    func_8002345c((ModelFile *)DAT_8015ec38);
    DAT_80063370[0] = 0;
    DAT_80063374 = 0;
    DAT_80063338 = 0;
}

/*
 * The end of a match: fades in, then asks whether to save (when a card is
 * ready; the winner side passed) or to retry, and repeats until the
 * player picks retry or the save is done. Left/right (0x10/0x40) move the
 * choice, 0x4000 confirms; the choice also slides DAT_8006336c (0..10).
 */
ADDRESS(0x800165f4, 0x248)
void func_800165f4(void)
{
    int asked = 0;
    int choice;
    int bits;
    int i;

    StartCARD();
    _bu_init();
    func_8001381c();
    DAT_80063324 = SCREEN_FADE_IN;
    DAT_80063338--;
    DAT_80063314 = pad_state.unk3C;
    for (i = FADE_FRAMES - 1; i != -1; i--) {
        func_80015020();
        DrawSync(0);
    }
    for (;;) {
        if (func_8001c530() == 0) {
            DAT_80063324 = SCREEN_SAVE_ASK;
        } else {
            choice = 0;
            DAT_80063324 = SCREEN_RETRY_ASK;
            DAT_8006336c = 0;
        }
        DAT_80063314 = pad_state.unk3C;
        for (i = FADE_FRAMES - 1; i != -1; i--) {
            func_80015020();
            DrawSync(0);
        }
        if (DAT_80063324 == SCREEN_SAVE_ASK) {
            if (func_8001c5e0(DAT_80063370[0] < DAT_80063374, asked) == 0) {
                DrawSync(0);
                return;
            }
            asked = 1;
            choice = 0;
            DAT_80063324 = SCREEN_SAVE;
            DAT_8006336c = 0;
            DAT_80063314 = pad_state.unk3C;
            for (i = FADE_FRAMES - 1; i != -1; i--) {
                func_80015020();
                DrawSync(0);
            }
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        for (;;) {
            bits = pad_state.pads[0].unk8 | pad_state.pads[1].unk8;
            if (bits & 0x10)
                choice = 0;
            if (bits & 0x40)
                choice = 1;
            if (bits & 0x4000) {
                if (choice) {
                    DrawSync(0);
                    return;
                }
                break;
            }
            if (choice) {
                if (++DAT_8006336c > 10)
                    DAT_8006336c = 10;
            } else {
                if (--DAT_8006336c < 0)
                    DAT_8006336c = 0;
            }
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[1].unk8 = 0;
            func_80015020();
            DrawSync(0);
        }
        DAT_80063324 = SCREEN_FADE_OUT;
        DAT_80063314 = pad_state.unk3C;
        for (i = FADE_FRAMES - 1; i != -1; i--) {
            func_80015020();
            DrawSync(0);
        }
    }
}

/*
 * VS.EXE's main: the heap, controllers, CD and memory card, the two root
 * counter events, the graphics, the loading picture and the menu sound
 * bank; then the character select / match loop until func_80016f10
 * declines, and the shutdown back to TITLE.EXE (next program 1).
 */
ADDRESS(0x8001683c, 0x40c)
int main(void)
{
    u8 *heap;
    u_char result[8];
    DVECTOR pos;
    DVECTOR clut;
    DVECTOR pos2;
    DVECTOR clut2;
    u_long *picture;

    ResetCallback();
    heap = DAT_80011f40 + ((DAT_80011f48 - DAT_80011f40 + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));
    DAT_80011f4c = heap;
    InitHeap((unsigned long *)heap, HEAP_TOP - heap);
    InitPAD((char *)&DAT_801961e0[0], 16, (char *)&DAT_801961e0[1], 16);
    StartPAD();
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    InitCARD(1);
    ExitCriticalSection();
    pad_state.unk44 = OpenEvent(RCntCNT3, EvSpINT, EvMdINTR, func_80013868);
    EnableEvent(pad_state.unk44);
    SetRCnt(RCntCNT3, 60000, RCntMdINTR);
    StartRCnt(RCntCNT3);
    pad_state.unk38 = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, func_80013848);
    EnableEvent(pad_state.unk38);
    SetRCnt(RCntCNT2, 60000, RCntMdINTR);
    StartRCnt(RCntCNT2);
    pad_state.unk28 = 0;
    func_8002f0f4();
    func_80014480();
    {
        u_long *loading_picture = (u_long *)&DAT_800633b0;

        pos.vx = 0;
        pos.vy = 300;
        memset(&clut, 0, sizeof(clut));
        func_8002ef3c(loading_picture, "\\LOAD02.TIM;1", 0);
        func_80012c2c(loading_picture, &pos, &clut);
    }
    DrawSync(0);
    func_80013aec();
    func_80014070();
    func_8002ee90(5, 4, DAT_80116580);
    func_8002ee90(5, 5, DAT_800ff580);
    func_8001c9b8();
    func_8002ee90(5, 20, (u_long *)DAT_80191d00.unk0118);
    DAT_801624b8[0].slot[0] = -1;
    DAT_801624b8[0].slot[1] = -1;
    DAT_801624b8[0].slot[2] = -1;
    DAT_801624b8[1].slot[0] = -1;
    DAT_801624b8[1].slot[1] = -1;
    DAT_801624b8[1].slot[2] = -1;
    DAT_80063330[0] = -1;
    DAT_80063334 = -1;
    DAT_80063380 = 0;
    DAT_80063381 = 0;
    while (func_80016f10()) {
        func_80016488();
        do {
            func_80015e50();
            DrawSync(0);
            SsVabClose(DAT_801604b0[0].vab);
            SsVabClose(DAT_801604b0[1].vab);
        } while (DAT_80063338 < DAT_80063228 && DAT_80063370[0] < 2 && DAT_80063374 < 2);
        while (CdControl(CdlStop, 0, 0) == 0)
            ;
        if (DAT_80063320 != 0 && DAT_80063370[0] != DAT_80063374) {
            func_800165f4();
            DAT_80063330[0] = -1;
            DAT_80063334 = -1;
        }
    }
    DrawSync(0);
    picture = (u_long *)&DAT_800633b0;
    pos2.vx = 0;
    pos2.vy = 60;
    memset(&clut2, 0, sizeof(clut2));
    func_80016d94();
    func_8002ef3c(picture, "\\LOAD02.TIM;1", 0);
    func_80012c2c(picture, &pos2, &clut2);
    DrawSync(0);
    *NEXT_PROGRAM_BYTE = 1;
    SsSetMVol(0, 0);
    SsVabClose(DAT_8006335a);
    SsEnd();
    *MAILBOX_BYTE_2 = 0;
    PadStop();
    ResetGraph(3);
    EnterCriticalSection();
    CloseEvent(pad_state.unk44);
    ExitCriticalSection();
    StopCallback();
}
