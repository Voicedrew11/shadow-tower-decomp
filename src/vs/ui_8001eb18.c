/*
 * VS.EXE: the menu screens' gradient fill and pointer triangle (WIP
 * module). VS's copies of ST.EXE's func_80028db8 and func_8002916c
 * (src/st/ui.c); VS advances the packet cursor before its bounds check.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>

/*
 * Takes the next primitive from the current packet buffer, or returns once
 * the buffer has overflowed (the cursor is advanced first).
 */
#define NEW_PRIM(buf, p, type)                                       \
    (buf) = DAT_801624d0.packet;                                     \
    (p) = (type *)(buf)->cur;                                        \
    (buf)->cur = (u_long *)((p) + 1);                                \
    if (DAT_801624d0.packet->cur > DAT_801624d0.packet->end)         \
        return;                                                      \
    DAT_801624d0.unk8328++

/* Fills `r` with a vertical gradient from `edge` at the top and bottom to
 * `mid` across the middle. */
/* Not exact: as ST.EXE's copy, retail schedules the halved height around
 * the packet-buffer load with other temporaries. */
ADDRESS(0x8001eb18, 0x3dc)
void func_8001eb18(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi)
{
    StPacketBuffer *buf;
    POLY_G4 *p;
    RECT b;

    b = *r;
    b.h = b.h >> 1;
    NEW_PRIM(buf, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, edge->r, edge->g, edge->b);
    setRGB1(p, edge->r, edge->g, edge->b);
    setRGB2(p, mid->r, mid->g, mid->b);
    setRGB3(p, mid->r, mid->g, mid->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *DAT_801624d0.ot_cur = (u_long)p & 0xffffff;
    b = *r;
    b.h = b.h >> 1;
    b.y += b.h;
    NEW_PRIM(buf, p, POLY_G4);
    p->code = semi == 0 ? 0x38 : 0x3a;
    p->tag = (*DAT_801624d0.ot_cur & 0xffffff) | 0x08000000;
    setRGB0(p, mid->r, mid->g, mid->b);
    setRGB1(p, mid->r, mid->g, mid->b);
    setRGB2(p, edge->r, edge->g, edge->b);
    setRGB3(p, edge->r, edge->g, edge->b);
    setXY4(p, b.x, b.y, b.x + b.w, b.y, b.x, b.y + b.h, b.x + b.w, b.y + b.h);
    *DAT_801624d0.ot_cur = (u_long)p & 0xffffff;
}

/* Draws a triangle filling `r` that points along `dir` (0, 1024, 2048 or
 * 3072), over a copy offset by one pixel. */
ADDRESS(0x8001eef4, 0x348)
void func_8001eef4(RECT *r, CVECTOR *color, u16 dir)
{
    short x0 = 0, x1 = 0, x2 = 0, y0 = 0, y1 = 0, y2 = 0;
    StPacketBuffer *buf;
    POLY_F3 *p;

    switch (dir) {
    case 0:
        x0 = r->x - r->h / 2;
        x1 = r->x + r->h / 2;
        x2 = x0;
        y0 = r->y - r->w / 2;
        y1 = r->y;
        y2 = y1 + r->w / 2;
        break;
    case 1024:
        x0 = r->x - r->w / 2;
        x1 = r->x;
        x2 = x1 + r->w / 2;
        y0 = r->y - r->h / 2;
        y1 = r->y + r->h / 2;
        y2 = y0;
        break;
    case 2048:
        x0 = r->x + r->h / 2;
        x1 = r->x - r->h / 2;
        x2 = x0;
        y0 = r->y - r->w / 2;
        y1 = r->y;
        y2 = y1 + r->w / 2;
        break;
    case 3072:
        x0 = r->x - r->w / 2;
        x1 = r->x;
        x2 = x1 + r->w / 2;
        y0 = r->y + r->h / 2;
        y1 = r->y - r->h / 2;
        y2 = y0;
        break;
    }
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0, y0, x1, y1, x2, y2);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(DAT_801624d0.ot_cur, p);
    NEW_PRIM(buf, p, POLY_F3);
    setlen(p, 4);
    setcode(p, 0x22);
    setXY3(p, x0 + 1, y0 + 1, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
    setRGB0(p, color->r, color->g, color->b);
    addPrim(DAT_801624d0.ot_cur, p);
}
