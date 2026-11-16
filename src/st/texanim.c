/*
 * ST.EXE: animated VRAM textures (WIP module). Sixteen slots each scroll,
 * wave or flip a source rectangle into a destination area with MoveImage,
 * or cycle a CLUT, once their countdown expires.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/gfx.h>

#define TEXANIM_FREE 255
/* Slot area that is active everywhere. */
#define TEXANIM_ANY_AREA 255

RODATA(0x800113b0, 0xc4)

ADDRESS(0x80016878, 0x2c)
void func_80016878(void)
{
    StTexAnim *slot = world_state.texanim;
    int i = ST_TEXANIM_SLOTS;

    while (i--) {
        slot->type = TEXANIM_FREE;
        slot++;
    }
}

ADDRESS(0x800168a4, 0x3c)
StTexAnim *func_800168a4(void)
{
    StTexAnim *slot = world_state.texanim;
    int i;

    for (i = ST_TEXANIM_SLOTS; i != 0; i--, slot++) {
        if (slot->type == TEXANIM_FREE)
            goto found;
    }
    return 0;
found:
    return slot;
}

/*
 * Not exact: retail fills the jump-table branch's delay slot with the index
 * shift and forms the table address after it; this build puts the table's
 * lui in the delay slot.
 */
ADDRESS(0x800168e0, 0x134)
void func_800168e0(RECT *rect, int period, int step, int type, short area)
{
    StTexAnim *slot = func_800168a4();
    RECT unused;

    if (slot == 0)
        return;
    slot->type = type;
    slot->phase = 0;
    slot->area = area;
    slot->period = period;
    slot->step = step;
    slot->countdown = 0;
    slot->dst = slot->src = *rect;
    switch (type) {
    case 1:
    case 2:
    case 6:
    case 33:
    case 34:
        slot->src.x += slot->src.w >> 2;
        slot->dst.w = slot->dst.w >> 2;
        slot->src.w = slot->src.w >> 2;
        break;
    case 49:
        StoreImage(&slot->src, (u_long *)DAT_8009ac88);
        break;
    }
}

ADDRESS(0x80016a14, 0x5a4)
void func_80016a14(void)
{
    StTexAnim *slot;
    RECT r;
    int i;
    int x, y, offset;
    short pos, n;
    u16 first;

    for (slot = world_state.texanim, i = ST_TEXANIM_SLOTS; i--; slot++) {
        if (slot->area != area_state.area && slot->area != TEXANIM_ANY_AREA)
            continue;
        if (slot->countdown == 0) {
            r = slot->src;
            slot->countdown = slot->period;
            switch (slot->type) {
            case 1 ... 5:
                /* Scroll vertically. */
                slot->phase += slot->step;
                if (slot->phase >= slot->src.h)
                    slot->phase -= slot->src.h;
                x = slot->dst.x;
                r.h = slot->src.h - slot->phase;
                MoveImage(&r, x, slot->dst.y + slot->phase);
                if (slot->phase) {
                    y = slot->dst.y;
                    r.y = slot->src.y + r.h;
                    r.h = slot->phase;
                    MoveImage(&r, x, y);
                }
                break;
            case 6 ... 9:
                /* Scroll each row horizontally, one more pixel per row. */
                slot->phase += slot->step;
                if (slot->phase >= slot->src.w)
                    slot->phase -= slot->src.w;
                r.h = 1;
                pos = slot->phase;
                y = slot->dst.y;
                for (n = slot->src.h; n--;) {
                    if (pos >= slot->src.w)
                        pos -= slot->src.w;
                    r.x = slot->src.x;
                    r.w = slot->src.w - pos;
                    MoveImage(&r, slot->dst.x + pos, y);
                    if (pos) {
                        x = slot->dst.x;
                        r.x = slot->src.x + r.w;
                        r.w = pos;
                        MoveImage(&r, x, y);
                    }
                    y++;
                    pos++;
                    r.y++;
                }
                break;
            case 33 ... 37:
                /* Wave each column vertically. */
                slot->phase += slot->step;
                if (slot->phase >= slot->src.h)
                    slot->phase -= slot->src.h;
                r.w = 1;
                pos = slot->phase;
                x = slot->dst.x;
                for (n = slot->src.w; n--;) {
                    offset = slot->src.h / 2 +
                             slot->src.h / 4 * rsin(4096 / slot->src.w * pos) / 4096;
                    if (offset >= slot->src.h)
                        offset -= slot->src.h;
                    r.y = slot->src.y;
                    r.h = slot->src.h - offset;
                    MoveImage(&r, x, slot->dst.y + offset);
                    if (offset) {
                        y = slot->dst.y;
                        r.y = slot->src.y + r.h;
                        r.h = offset;
                        MoveImage(&r, x, y);
                    }
                    x++;
                    pos++;
                    r.x++;
                }
                break;
            case 17 ... 20:
                /* Step through frames stacked below the source. */
                if (++slot->phase >= slot->step) {
                    slot->phase = 0;
                    LoadImage(&r, (u_long *)DAT_8009ac88);
                } else {
                    r.y += slot->phase * slot->src.h;
                    MoveImage(&r, slot->dst.x, slot->dst.y);
                }
                break;
            case 49:
                /* Rotate CLUT entries 1-14. */
                first = DAT_8009ac88[1];
                DAT_8009ac88[1] = DAT_8009ac88[2];
                DAT_8009ac88[2] = DAT_8009ac88[3];
                DAT_8009ac88[3] = DAT_8009ac88[4];
                DAT_8009ac88[4] = DAT_8009ac88[5];
                DAT_8009ac88[5] = DAT_8009ac88[6];
                DAT_8009ac88[6] = DAT_8009ac88[7];
                DAT_8009ac88[7] = DAT_8009ac88[8];
                DAT_8009ac88[8] = DAT_8009ac88[9];
                DAT_8009ac88[9] = DAT_8009ac88[10];
                DAT_8009ac88[10] = DAT_8009ac88[11];
                DAT_8009ac88[11] = DAT_8009ac88[12];
                DAT_8009ac88[12] = DAT_8009ac88[13];
                DAT_8009ac88[13] = DAT_8009ac88[14];
                DAT_8009ac88[14] = first;
                LoadImage(&r, (u_long *)DAT_8009ac88);
                break;
            case TEXANIM_FREE:
                break;
            }
        } else {
            slot->countdown--;
        }
    }
}
