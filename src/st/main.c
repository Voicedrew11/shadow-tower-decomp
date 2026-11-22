/*
 * ST.EXE: main sets up the heap above the overlay area, the CD and the
 * memory card, then runs the game; the shutdown and the FN. start-up table
 * reader follow (WIP module). The unit is -G8: the "FN." file name is an
 * .sdata literal, and the heap words, defined elsewhere, get the absolute
 * lui pairs ASPSX gives small data it does not own. It ends at
 * game_load_start_file; the game loop follows in its own unit (src/st/loop.c).
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
#include <st/menu.h>
#include <st/main.h>
#include <st/funcs.h>
#include <st/cd.h>
#include <st/vis.h>
#include <st/hud.h>
#include <st/light.h>

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

    heap = overlay_area_start + ((overlay_area_end - overlay_area_start + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));
    heap_base = heap;
    InitHeap3((unsigned long *)heap, HEAP_TOP - heap);
    while (CdInit() == 0)
        CdSync(0, result);
    CdSetDebug(0);
    InitCARD(0);
    StartCARD();
    _bu_init();
    pad_init();
    ExitCriticalSection();
    game_loop();
}

ADDRESS(0x80014410, 0x40)
void game_shutdown(void)
{
    sound_shutdown();
    cd_shutdown();
    PadStopCom();
    ResetGraph(3);
    StopCallback();
}

/*
 * Reads a `digits`-wide decimal field (an optional '-' among the digits)
 * at *cursor, then moves *cursor past the end of the line.
 */
ADDRESS(0x80014450, 0xf4)
int text_read_number(u8 **cursor, int digits)
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
 * Loads the start-up table file FN.: six bytes of area_state, three
 * viewer positions and two optional values, then id/value rows until
 * 999. Rows 970..990 fill viewer.equipment; other ids set an id's count.
 */
ADDRESS(0x80014544, 0x4ec)
void game_load_start_file(void)
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

    buf = mem_alloc(0x4000);
    load_st_file(buf, "FN.", 0x4000);
    cursor = buf;
    area_state.loaded_area = area_state.loaded_world = area_state.loaded_images = area_state.loaded_sound_bank =
        area_state.unk8 = area_state.area = text_read_number(&cursor, 3);
    viewer.position.vx = (text_read_number(&cursor, 7) + 64) << 12;
    viewer.position.vy = ((text_read_number(&cursor, 7) + 64) << 12) - 1600;
    viewer.position.vz = (text_read_number(&cursor, 7) + 64) << 12;
    x = text_read_number(&cursor, 5);
    y = text_read_number(&cursor, 5);
    for (;;) {
        second = cursor + 4;
        id = text_read_number(&cursor, 3);
        value = text_read_number(&second, 3);
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
            u16 *equipment = viewer.equipment;

            if (value == FN_END)
                continue;
            equipment[slot] = value;
            inventory.counts[value].count++;
            if (value < 266 && !(vis_table_c.collected_bits[value / 32] & (1 << (value % 32))))
                viewer.items_found++;
            vis_table_c.collected_bits[value / 32] |= 1 << (value % 32);
        } else {
            if (value == FN_END)
                continue;
            if (value <= 0)
                continue;
            inventory.counts[id].count = value;
            if (id < 266 && !(vis_table_c.collected_bits[id / 32] & (1 << (id % 32))))
                viewer.items_found++;
            vis_table_c.collected_bits[id / 32] |= 1 << (id % 32);
        }
    }
    player_recalc_stats();
    if (x != 0)
        viewer.max_hp = viewer.hp = x;
    if (y != 0)
        viewer.max_mp = viewer.mp = y;
    mem_free(buf);
    if (all_items) {
        for (i = 0; i < 314; i++) {
            if (vis_table_c.types[i].model != 0xff)
                inventory.counts[i].count++;
            if (i < 266 && !(vis_table_c.collected_bits[i / 32] & (1 << (i % 32))))
                viewer.items_found++;
            vis_table_c.collected_bits[i / 32] |= 1 << (i % 32);
        }
    }
    if (clear_flags) {
        u32 (*row)[5] = &DAT_801cceb0[1];

        fill_words(row[0], -1, 5);
        fill_words(row[-1], -1, 5);
        row[0][4] &= ~0x4000;
        row[-1][4] &= ~0x4000;
    }
    player_load_equipment_model(viewer.equipment[3], 0, 0);
    player_load_equipment_model(viewer.equipment[16], 1, 0);
    player_load_equipment_model(viewer.equipment[4], 2, 0);
    player_load_equipment_model(viewer.equipment[17], 3, 0);
    player_load_magic(viewer.equipment[6], 0);
    player_load_magic(viewer.equipment[12], 1);
}

/*
 * The HUD sprite records, the first game .data (WIP owner, decision D1):
 * 46 records in runs that end with flags 0xff (src/st/hud.c draws them).
 */

/* Unreferenced: sixteen zero bytes between the end of .text and the records. */
DATA(0x8007f3e4, 0x10, ".data")
static u32 DAT_8007f3e4[4] = {0, 0, 0, 0};

DATA(0x8007f3f4, 0x398, ".data")
StHudSprite hud_sprites[46] = {
    {0x01, 0xff, 104, 162, 7, 10, {13, 32, 112, 5}, 0x7e62, 0x0019, 0},
    {0x01, 0xff, 104, 162, 7, 10, {195, 32, 112, 5}, 0x7e62, 0x0019, 0},
    {0x02, 0x03, 104, 162, 7, 10, {13, 32, 112, 5}, 0x7e62, 0x0019, 0},
    {0x02, 0x03, 104, 162, 7, 10, {195, 32, 112, 5}, 0x7e62, 0x0019, 0},
    {0x01, 0x01, 0, 240, 16, 16, {291, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x00, 0x01, 0, 240, 16, 16, {275, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x00, 0x01, 0, 240, 16, 16, {259, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 16, 240, 16, 16, {291, 160, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x00, 0x01, 16, 240, 16, 16, {275, 160, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 32, 240, 16, 16, {291, 176, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x00, 0x01, 32, 240, 16, 16, {275, 176, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 48, 240, 16, 16, {291, 192, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 64, 240, 16, 16, {291, 208, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 112, 160, 32, 16, {13, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {61, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {77, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {93, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {109, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 144, 160, 32, 16, {195, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {243, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {259, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {275, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 240, 16, 16, {291, 16, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 104, 162, 7, 10, {13, 32, 112, 10}, 0x7e62, 0x0019, 0},
    {0x01, 0x01, 104, 162, 7, 10, {195, 32, 112, 10}, 0x7e62, 0x0019, 0},
    {0xff, 0x00, 0, 0, 0, 0, {0, 0, 0, 0}, 0x0000, 0x0000, 0},
    {0x01, 0x01, 0, 0, 24, 24, {27, 192, 24, 24}, 0x7fe3, 0x011e, 0},
    {0x01, 0x01, 0, 24, 24, 24, {85, 192, 24, 24}, 0x7fe3, 0x011e, 0},
    {0x01, 0x01, 0, 48, 24, 24, {143, 192, 24, 24}, 0x7fe3, 0x011e, 0},
    {0x01, 0x01, 0, 72, 24, 24, {201, 192, 24, 24}, 0x7fe3, 0x011e, 0},
    {0x01, 0x01, 0, 96, 24, 24, {259, 192, 24, 24}, 0x7fe3, 0x011e, 0},
    {0xff, 0x00, 0, 0, 0, 0, {0, 0, 0, 0}, 0x0000, 0x0000, 0},
    {0x01, 0x01, 96, 176, 8, 16, {10, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 176, 8, 16, {18, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 176, 8, 16, {26, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0x01, 0x01, 96, 176, 8, 16, {34, 144, 255, 255}, 0x7fe2, 0x0019, 0},
    {0xff, 0x00, 0, 0, 0, 0, {0, 0, 0, 0}, 0x0000, 0x0000, 0},
    {0x01, 0x00, 24, 0, 24, 24, {13, 40, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 24, 24, 24, {13, 64, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 48, 24, 24, {13, 88, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 72, 24, 24, {13, 112, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 96, 24, 24, {283, 40, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 120, 24, 24, {283, 64, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 144, 24, 24, {283, 88, 255, 255}, 0x7fe3, 0x011e, 0},
    {0x01, 0x00, 24, 168, 24, 24, {283, 112, 255, 255}, 0x7fe3, 0x011e, 0},
    {0xff, 0x00, 0, 0, 0, 0, {0, 0, 0, 0}, 0x0000, 0x0000, 0},
};

/*
 * The default lighting environments (lighting_load_defaults); 0..47 are
 * zero until an area loads its own (copy_words copies them in).
 */
DATA(0x8007f78c, 0xc00, ".data")
StLightInit lighting_defaults[64] = {
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{0}}},
    {{{{0, 0, -4096}, {0, 0, -3800}, {0, 0, -3800}}, 0},
     {{{900, 900, 900}, {900, 900, 900}, {900, 900, 900}}, 0},
     {60, 60, 60, 0}, -1, -1},
    {{{{0, 0, -4096}, {0, 0, -3800}, {0, 0, -3800}}, 0},
     {{{500, 500, 500}, {500, 500, 500}, {500, 500, 500}}, 0},
     {60, 60, 60, 0}, -1, -1},
    {{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0},
     {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0},
     {60, 60, 60, 0}, -1, -1},
    {{{{0, 0, -4096}, {0, 0, -4096}, {0, 0, -4096}}, 0},
     {{{512, 512, 512}, {512, 512, 512}, {512, 512, 512}}, 0},
     {70, 70, 70, 0}, 32000, 32000},
    {{{{3800, -2800, 1024}, {-3000, -3600, -3400}, {-1300, 2700, 1024}}, 0},
     {{{1024, 1024, 1024}, {1024, 1024, 1024}, {1024, 1024, 1024}}, 0},
     {90, 90, 90, 0}, -1, -1},
    {{{{0, 3800, 0}, {-500, 3600, 0}, {0, 3700, 500}}, 0},
     {{{1200, 1200, 1200}, {1200, 1200, 1200}, {1200, 1200, 1200}}, 0},
     {10, 10, 10, 0}, -1, -1},
    {{{{0, 0, -4096}, {0, 0, -3800}, {0, 0, -3800}}, 0},
     {{{800, 800, 800}, {600, 600, 600}, {500, 500, 500}}, 0},
     {30, 25, 20, 0}, -1268, -1268},
    {{{{3000, -3700, -1500}, {-2000, -3700, -3000}, {-1300, 1000, 1024}}, 0},
     {{{2048, 2048, 2048}, {2048, 2048, 2048}, {2048, 2048, 2048}}, 0},
     {64, 64, 64, 0}, -1, -1},
    {{{{0, 0, -4096}, {0, 0, -3800}, {0, 0, -3800}}, 0},
     {{{3095, 3095, 3095}, {3095, 3095, 3095}, {3095, 3095, 3095}}, 0},
     {60, 60, 60, 0}, -1, -1},
    {{{{3800, -2800, 1024}, {-3000, -3600, -3400}, {-1300, 2700, 1024}}, 0},
     {{{3072, 3072, 3072}, {3072, 3072, 3072}, {3072, 3072, 3072}}, 0},
     {100, 100, 100, 0}, -1, -1},
    {{{{0, 3800, 0}, {0, -3600, 0}, {0, 3700, 0}}, 0},
     {{{3000, 3000, 3000}, {3000, 3000, 3000}, {3000, 3000, 3000}}, 0},
     {0, 0, 0, 0}, -1, -1},
    {{{{3000, 3700, -1500}, {-2000, 3700, -3000}, {-1300, -1000, 1024}}, 0},
     {{{6144, 6144, 6144}, {3072, 3072, 3072}, {3072, 3072, 3072}}, 0},
     {100, 100, 100, 0}, -1, -1},
    {{{{0, 0, -4096}, {0, 0, -3800}, {0, 0, -3800}}, 0},
     {{{800, 800, 800}, {600, 600, 600}, {500, 500, 500}}, 0},
     {80, 80, 80, 0}, -1, -1},
    {{{{2000, 2828, -2828}, {2464, 3864, 1414}, {-4464, 1036, 1414}}, 0},
     {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 0},
     {255, 255, 255, 0}, -1, -1},
    {{{{3000, 3700, -1500}, {-2000, 3700, -3000}, {-1300, -1000, 1024}}, 0},
     {{{2048, 2048, 2048}, {2048, 2048, 2048}, {2048, 2048, 2048}}, 0},
     {100, 100, 100, 0}, -1, -1},
    {{{{0, -682, 0}, {0, -682, 0}, {0, -682, 0}}, 0},
     {{{1024, 1024, 1024}, {1024, 1024, 1024}, {1024, 1024, 1024}}, 0},
     {0, 0, 0, 0}, -1, -1},
};
