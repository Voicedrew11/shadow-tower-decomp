/*
 * VS.EXE: the first run of its first translation unit (WIP module): the
 * CD mixing volume and XA music channel, the sound bank load and TIM
 * uploads, the fighters' face texture and semi-transparency fix-ups, the
 * controller decode, a debug display adjust, the sound start-up and
 * key-on, the fighters' animation sounds and the archive opens. The
 * unit's constant pool runs on into func_800140d8's module.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
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

/* This module's slice of the constant pool (file names) and the switch
 * table of func_80013c5c. */
RODATA(0x80011000, 0x50)
JUMPTABLES(0x800110a0, 0x1c)

/* A tentative definition: ASPSX needs the size to address this .sdata
 * word $gp-relative (its initialized definition is not reconstructed). */
s32 DAT_80063220;

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
