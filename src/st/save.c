/*
 * ST.EXE: new-game defaults and the per-area snapshots of the entry tables
 * in game_progress (WIP module, 0x800559bc..0x800562d4).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/save.h>

#define BIT(i) (1 << ((i) % 32))

/*
 * New-game defaults.
 *
 * The three flags whose masks take two instructions (bits 17, 16 and 15
 * of unk1444) are each cleared twice. The repeat leaves no instruction
 * (cse cannot fold it because the merged mask is not a valid operand,
 * combine can), but flow has already counted the mask's second use, and
 * that is what ranks those three masks ahead of 0x7fffffff in local-alloc
 * (retail: 0x7fffffff in t4, the others in t3, t2, t1).
 */
ADDRESS(0x800559bc, 0x338)
void save_reset_new_game(void)
{
    u8 unused[128];

    fill_words((u32 *)game_progress.unk1448, 0, 12);
    fill_words((u32 *)&inventory, 0, 89);
    fill_words(&game_progress.unk14A8[10], 0, 79);
    game_progress.unk14A8[89] = -1;
    game_progress.unk14A8[90] = -1;
    game_progress.unk14A8[91] = -1;
    lighting.area_mode[0] = 0x11;
    lighting.area_mode[40] = 0x11;
    lighting.area_mode[41] = 0x11;
    lighting.area_mode[4] = 0x11;
    lighting.area_mode[5] = 0x41;
    lighting.area_mode[1] = 0x41;
    lighting.area_mode[6] = 0x41;
    lighting.area_mode[11] = 0x41;
    lighting.area_mode[27] = 0x41;
    lighting.area_mode[12] = 0x41;
    lighting.area_mode[28] = 0x41;
    lighting.area_mode[15] = 0x41;
    lighting.area_mode[33] = 0x41;
    lighting.area_mode[3] = 0x22;
    lighting.area_mode[20] = 0x22;
    lighting.area_mode[24] = 0x22;
    lighting.area_mode[18] = 0x22;
    lighting.area_mode[10] = 0x41;
    lighting.area_mode[21] = 0x41;
    lighting.area_mode[32] = 0x41;
    lighting.area_mode[37] = 0x11;
    lighting.area_mode[9] = 0x22;
    lighting.area_mode[8] = 0x41;
    lighting.area_mode[13] = 0x41;
    lighting.area_mode[38] = 0x41;
    lighting.area_mode[2] = 0x41;
    lighting.area_mode[17] = 0x41;
    lighting.area_mode[25] = 0x41;
    lighting.area_mode[26] = 0x41;
    lighting.area_mode[14] = 0x41;
    lighting.area_mode[23] = 0x41;
    lighting.area_mode[30] = 0x11;
    lighting.area_mode[44] = 0x11;
    lighting.area_mode[45] = 0x11;
    lighting.area_mode[46] = 0x11;
    lighting.area_mode[47] = 0x11;
    game_progress.unk16DC = 0;
    inventory.shop_ids[0] = 268;
    inventory.shop_stock[0] = 3;
    inventory.shop_ids[1] = 270;
    inventory.shop_stock[1] = 5;
    inventory.shop_ids[2] = 313;
    inventory.shop_stock[2] = 1;
    inventory.shop_ids[3] = 312;
    inventory.shop_stock[3] = 1;
    inventory.shop_ids[4] = 275;
    inventory.shop_stock[4] = 1;
    inventory.shop_ids[5] = 284;
    inventory.shop_stock[5] = 30;
    inventory.shop_ids[6] = 285;
    inventory.shop_stock[6] = 20;
    inventory.shop_ids[7] = 286;
    inventory.shop_stock[7] = 10;
    inventory.shop_ids[8] = 302;
    inventory.shop_stock[8] = 2;
    inventory.shop_ids[9] = 18;
    inventory.shop_stock[9] = 1;
    game_progress.unk1444.word &= ~0x60;
    game_progress.unk1444.word &= ~0x18;
    game_progress.unk1444.word &= ~0x6;
    game_progress.unk1440.word &= ~0x80000000;
    game_progress.unk1444.word &= ~0x1;
    game_progress.unk1440.word &= 0x80000000;
    game_progress.unk1444.word &= ~0x20000;
    game_progress.unk1444.word &= ~0x20000;
    game_progress.unk1444.word &= ~0x10000;
    game_progress.unk1444.word &= ~0x10000;
    game_progress.unk1444.word &= ~0x8000;
    game_progress.unk1444.word &= ~0x8000;
    game_progress.unk1444.word &= ~0x4000;
    game_progress.unk1444.word &= ~0x2000;
    game_progress.unk1444.word &= ~0x1000;
    game_progress.unk1444.word &= ~0x800;
    game_progress.unk1444.word &= ~0x400;
    game_progress.unk1444.word &= ~0x200;
    game_progress.unk1444.word &= ~0x100;
    game_progress.unk1444.word &= ~0x80;
    inventory.shop_ids[10] = 28;
    inventory.shop_ids[11] = 88;
    inventory.shop_ids[12] = 243;
    inventory.shop_stock[10] = 1;
    inventory.shop_stock[11] = 1;
    inventory.shop_stock[12] = 1;
    inventory.shop_ids[13] = 133;
    inventory.shop_stock[13] = 1;
    fill_words(game_progress.unk180[0], 0, 48);
    fill_words(game_progress.unk0[0], 0, 96);
    fill_words(game_progress.unk240[0], 0, 384);
    fill_words(game_progress.unk840[0], 0, 384);
    fill_words(game_progress.unkE40[0], 0, 384);
    fill_words((u32 *)game_progress.unk1478, 0, 12);
    fill_words(game_progress.unk14A8, 0, 5);
    fill_words(&game_progress.unk14A8[5], 0, 5);
    fill_words((u32 *)game_progress.unk16E4, 0, 12);
    game_progress.unk16E0 = -1;
}

/* Records area `area`'s entry states: live table A entries in state 3 or 5,
 * empty table C slots 32..63, and per table B entry: free, unk4F 0xff,
 * unk0C 0xff. */
ADDRESS(0x80055cf4, 0x2a8)
void area_save_snapshot(int area)
{
    Creature *a = vis_table_a.entries;
    GroundItem *c = vis_table_c.entries;
    AreaObject *b = vis_table_b.entries;
    int i;
    int n;

    n = 2;
    for (i = 0; i < n; i++)
        game_progress.unk0[area][i] = 0;
    for (i = 0; i < 64; i++, a++) {
        if (a->spawn_chance != 0xff && (a->state == 3 || a->state == 5))
            game_progress.unk0[area][i / 32] |= BIT(i);
    }
    n = 1;
    for (i = 0; i < n; i++)
        game_progress.unk180[area][i] = 0;
    for (i = 0; i < 32; i++, c++) {
        if (c->state == 0)
            game_progress.unk180[area][i / 32] |= BIT(i);
    }
    n = 8;
    for (i = 0; i < n; i++) {
        game_progress.unk240[area][i] = 0;
        game_progress.unk840[area][i] = 0;
        game_progress.unkE40[area][i] = 0;
    }
    for (i = 0; i < 240; i++, b++) {
        if (b->type == 0xffff) {
            game_progress.unk240[area][i / 32] |= BIT(i);
        } else {
            if (b->params.door.lock == 0xff)
                game_progress.unk840[area][i / 32] |= BIT(i);
            if (b->state == 0xff)
                game_progress.unkE40[area][i / 32] |= BIT(i);
        }
    }
}

/* Restores what area_save_snapshot recorded for area `area`. */
ADDRESS(0x80055f9c, 0x338)
void area_restore_snapshot(int area)
{
    Creature *a = vis_table_a.entries;
    GroundItem *c = vis_table_c.entries;
    AreaObject *b = vis_table_b.entries;
    int i;
    u16 n;
    u8 t;

    for (i = 0; i < 64; i++, a++) {
        if (game_progress.unk0[area][i / 32] & BIT(i))
            a->state = 3;
    }
    for (i = 0; i < 32; i++, c++) {
        if (game_progress.unk180[area][i / 32] & BIT(i))
            c->state = 0;
    }
    for (i = 0; i < 240; b++, i++) {
        if (game_progress.unk240[area][i / 32] & BIT(i)) {
            b->type = 0xffff;
        } else {
            switch (b->kind) {
            default:
                if (game_progress.unk840[area][i / 32] & BIT(i))
                    b->params.door.lock = 255;
                if (game_progress.unkE40[area][i / 32] & BIT(i))
                    b->state = 255;
                break;
            case 0:
            case 1:
                if (game_progress.unk840[area][i / 32] & BIT(i))
                    b->params.door.lock = 255;
                if (game_progress.unkE40[area][i / 32] & BIT(i)) {
                    n = b->params.door.n_blocks;
                    while (n--) {
                        t = vis_records[b->params.door.block_b + n].part;
                        vis_records[b->params.door.block_b + n].part =
                            vis_records[b->params.door.block_a + n].part;
                        vis_records[b->params.door.block_a + n].part = t;
                    }
                    if (b->type == 58)
                        b->visibility = 2;
                    else if (b->kind == 1)
                        b->visibility = 1;
                    b->state = 255;
                    b->anim_frame = 4095;
                }
                break;
            case 52:
                if (game_progress.unk840[area][i / 32] & BIT(i))
                    b->params.door.lock = 255;
                if (game_progress.unkE40[area][i / 32] & BIT(i)) {
                    b->state = 255;
                    b->anim_frame = 0;
                    b->anim = 1;
                }
                break;
            }
        }
    }
    vis_lists.floor_kinds_off &= ~0x20;
    vis_lists.floor_kinds_off &= ~0x10;
    vis_lists.floor_kinds_off &= ~0x8;
    vis_lists.floor_kinds_off &= ~0x4;
    vis_lists.floor_kinds_off &= ~0x2;
    vis_lists.floor_kinds_off &= ~0x1;
}
