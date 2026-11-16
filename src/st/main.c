/*
 * ST.EXE: main sets up the heap above the overlay area, the CD and the
 * memory card, then runs the game; the shutdown and the FN. start-up table
 * reader follow (WIP module). The unit is -G8: the "FN." file name is an
 * .sdata literal, and the heap words, defined elsewhere, get the absolute
 * lui pairs ASPSX gives small data it does not own. It ends at
 * func_80014544, which is not exact, so that the game loop after it keeps
 * its retail offsets in its own unit (src/st/loop.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBPAD.H>
#include <st/memory.h>
#include <st/data.h>
#include <st/main.h>
#include <st/funcs.h>

/* Top of the heap: main RAM below the 8 KB stack. */
#define HEAP_TOP ((u8 *)0x801fdff8)

/* The heap starts at the first 2 KB boundary past the overlay area. */
#define HEAP_ALIGN 2048

/* FN. table terminator and markers (the second number on a marker row must
 * not be 999). */
#define FN_END 999
#define FN_SKIP 998
#define FN_ALL_ITEMS 997
#define FN_CLEAR_FLAGS 996
#define FN_SLOT_FIRST 970
#define FN_SLOTS 21

SDATA(0x80087ec0, 0x4)

ADDRESS(0x8001435c, 0xb4)
int main(void)
{
    u8 *heap;
    u_char result[8];

    heap = DAT_80013980 + ((DAT_80013988 - DAT_80013980 + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));
    DAT_8001398c = heap;
    InitHeap3((unsigned long *)heap, HEAP_TOP - heap);
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    InitCARD(0);
    StartCARD();
    _bu_init();
    func_80057684();
    ExitCriticalSection();
    func_80014a30();
}

ADDRESS(0x80014410, 0x40)
void func_80014410(void)
{
    func_80057f3c();
    func_80016454();
    PadStopCom();
    ResetGraph(3);
    StopCallback();
}

/*
 * Reads a `digits`-wide decimal field (an optional '-' among the digits)
 * at *cursor, then moves *cursor past the end of the line.
 */
ADDRESS(0x80014450, 0xf4)
int func_80014450(u8 **cursor, int digits)
{
    int value = 0;
    int scale = 1;
    int sign = 1;
    int i = digits;

    *cursor += digits - 1;
    while (i--) {
        if (**cursor == '-')
            sign = -1;
        else if (**cursor >= '0' && **cursor <= '9') {
            value += (**cursor - '0') * scale;
            scale *= 10;
        }
        (*cursor)--;
    }
    *cursor += digits + 2;
    while (**cursor != '\n')
        (*cursor)++;
    (*cursor)++;
    return value * sign;
}

/*
 * Loads the start-up table file FN.: six bytes of DAT_801b5920, three
 * DAT_80198f28 positions and two optional values, then id/value rows until
 * 999. Rows 970..990 fill DAT_80198f28.unk38; other ids set an id's count.
 * Not exact: GCC keeps &DAT_80198f28 in $s2 across the row loop where retail
 * keeps the constant 1 of the bit masks there and rematerializes the address
 * (first divergence at +0x130, `li s2,1`); the clear_flags block also
 * addresses DAT_801cceb0[0] from a new base. Retail is 0x4ec bytes.
 */
ADDRESS(0x80014544, 0x4ec)
void func_80014544(void)
{
    u8 *buf;
    u8 *cursor;
    u8 *second;
    int all_items = 0;
    int clear_flags = 0;
    int id;
    int i;
    u32 slot;
    int value;
    int x;
    int y;

    buf = func_800152d0(0x4000);
    func_8001618c(buf, "FN.", 0x4000);
    cursor = buf;
    DAT_801b5920.unk4 = DAT_801b5920.unk5 = DAT_801b5920.unk6 = DAT_801b5920.unk7 =
        DAT_801b5920.unk8 = DAT_801b5920.unk9 = func_80014450(&cursor, 3);
    DAT_80198f28.unk248 = (func_80014450(&cursor, 7) + 64) << 12;
    DAT_80198f28.unk24C = ((func_80014450(&cursor, 7) + 64) << 12) - 1600;
    DAT_80198f28.unk250 = (func_80014450(&cursor, 7) + 64) << 12;
    x = func_80014450(&cursor, 5);
    y = func_80014450(&cursor, 5);
    for (;;) {
        second = cursor + 4;
        id = func_80014450(&cursor, 3);
        value = func_80014450(&second, 3);
        if (id == FN_END)
            break;
        if (id == FN_SKIP && value != FN_END)
            continue;
        if (id == FN_ALL_ITEMS && value != FN_END) {
            all_items = 1;
            continue;
        }
        if (id == FN_CLEAR_FLAGS && value != FN_END) {
            clear_flags = 1;
            continue;
        }
        slot = id - FN_SLOT_FIRST;
        if (slot < FN_SLOTS) {
            if (value == FN_END)
                continue;
            DAT_80198f28.unk38[slot] = value;
            DAT_801c7590[value].count++;
            if (value < 266 && !(DAT_801c76f8.unk70[value / 32] & (1 << (value % 32))))
                DAT_80198f28.unk28++;
            DAT_801c76f8.unk70[value / 32] |= 1 << (value % 32);
        } else {
            if (value == FN_END)
                continue;
            if (value <= 0)
                continue;
            DAT_801c7590[id].count = value;
            if (id < 266 && !(DAT_801c76f8.unk70[id / 32] & (1 << (id % 32))))
                DAT_80198f28.unk28++;
            DAT_801c76f8.unk70[id / 32] |= 1 << (id % 32);
        }
    }
    func_8003cedc();
    if (x != 0)
        DAT_80198f28.unk0 = DAT_80198f28.unk2 = x;
    if (y != 0)
        DAT_80198f28.unk4 = DAT_80198f28.unk6 = y;
    func_800152f8(buf);
    if (all_items) {
        for (i = 0; i < 314; i++) {
            if (DAT_801c76f8.unk11C[i].unk0 != 0xff)
                DAT_801c7590[i].count++;
            if (i < 266 && !(DAT_801c76f8.unk70[i / 32] & (1 << (i % 32))))
                DAT_80198f28.unk28++;
            DAT_801c76f8.unk70[i / 32] |= 1 << (i % 32);
        }
    }
    if (clear_flags) {
        func_80014d5c(DAT_801cceb0[1], -1, 5);
        func_80014d5c(DAT_801cceb0[0], -1, 5);
        DAT_801cceb0[1][4] &= ~0x4000;
        DAT_801cceb0[0][4] &= ~0x4000;
    }
    func_8003f12c(DAT_80198f28.unk38[3], 0, 0);
    func_8003f12c(DAT_80198f28.unk38[16], 1, 0);
    func_8003f12c(DAT_80198f28.unk38[4], 2, 0);
    func_8003f12c(DAT_80198f28.unk38[17], 3, 0);
    func_8003ee38(DAT_80198f28.unk38[6], 0);
    func_8003ee38(DAT_80198f28.unk38[12], 1);
}
