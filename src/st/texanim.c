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

JUMPTABLES(0x800113b0, 0xc4)

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

/* Not exact: the kind 6..9 dispatch branches toward the case rather than
 * retail's next-slot path (first divergence +0xd4). Traversal-count loads
 * also follow the temporary RECT extent stores instead of preceding them.
 * The function extent and frame agree with retail. */
ADDRESS(0x80016a14, 0x5a4)
void func_80016a14(void)
{
    StTexAnim *slot;
    RECT r;
    int i;
    int x, y, offset;
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
                y = slot->dst.y + slot->phase;
                r.h = slot->src.h - slot->phase;
                MoveImage(&r, x, y);
                if (slot->phase) {
                    y = slot->dst.y;
                    r.y = slot->src.y + r.h;
                    r.h = slot->phase;
                    MoveImage(&r, x, y);
                }
                break;
            case 6 ... 9: {
                short row_phase, row_count;

                /* Scroll each row horizontally, one more pixel per row. */
                slot->phase += slot->step;
                if (slot->phase >= slot->src.w)
                    slot->phase -= slot->src.w;
                r.h = 1;
                row_count = slot->src.h;
                row_phase = slot->phase;
                y = slot->dst.y;
                for (; row_count--;) {
                    if (row_phase >= slot->src.w)
                        row_phase -= slot->src.w;
                    x = slot->dst.x + row_phase;
                    r.x = slot->src.x;
                    r.w = slot->src.w - row_phase;
                    MoveImage(&r, x, y);
                    if (row_phase) {
                        x = slot->dst.x;
                        r.x = slot->src.x + r.w;
                        r.w = row_phase;
                        MoveImage(&r, x, y);
                    }
                    y++;
                    row_phase++;
                    r.y++;
                }
                break;
            }
            case 33 ... 37: {
                short column_phase, column_count;

                /* Wave each column vertically. */
                slot->phase += slot->step;
                if (slot->phase >= slot->src.h)
                    slot->phase -= slot->src.h;
                r.w = 1;
                column_count = slot->src.w;
                column_phase = slot->phase;
                x = slot->dst.x;
                for (; column_count--;) {
                    offset = slot->src.h / 2 +
                             slot->src.h / 4 * rsin(4096 / slot->src.w * column_phase) / 4096;
                    if (offset >= slot->src.h)
                        offset -= slot->src.h;
                    y = slot->dst.y + offset;
                    r.y = slot->src.y;
                    r.h = slot->src.h - offset;
                    MoveImage(&r, x, y);
                    if (offset) {
                        y = slot->dst.y;
                        r.y = slot->src.y + r.h;
                        r.h = offset;
                        MoveImage(&r, x, y);
                    }
                    x++;
                    column_phase++;
                    r.x++;
                }
                break;
            }
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
                /* Entry zero is fixed; rotate the fourteen following colours. */
                {
                    u16 *colors = &DAT_8009ac88[1];

                    first = colors[0];
                    colors[0] = colors[1];
                    colors[1] = colors[2];
                    colors[2] = colors[3];
                    colors[3] = colors[4];
                    colors[4] = colors[5];
                    colors[5] = colors[6];
                    colors[6] = colors[7];
                    colors[7] = colors[8];
                    colors[8] = colors[9];
                    colors[9] = colors[10];
                    colors[10] = colors[11];
                    colors[11] = colors[12];
                    colors[12] = colors[13];
                    colors[13] = first;
                    LoadImage(&r, (u_long *)(colors - 1));
                }
                break;
            case TEXANIM_FREE:
                break;
            }
        } else {
            slot->countdown--;
        }
    }
}
