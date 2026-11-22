/*
 * VS.EXE: the item list page, its caption bar and detail panel, and the
 * two-tile emblem panel (WIP module). menu_draw_button_prompt is VS's copy of
 * ST.EXE's menu_draw_button_prompt (src/st/render.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/card.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/* The module's initializers (the light grey shared by menu_item_request_draw_list and
 * menu_draw_button_prompt): at most 8 bytes, so .sdata under -G8. */
SDATA(0x80063290, 0x28)

/*
 * The item list page: the cursor bar, the scroll arrows and thumb, the
 * header panel, then eleven rows from the list's first shown row: each
 * item's name (500 for the -2 entry, a plain message) and its icon from
 * the kind its id range gives.
 */
ADDRESS(0x80021cfc, 0x52c)
void menu_item_request_draw_list(void)
{
    RECT unused0;
    CVECTOR unused1 = {96, 96, 96};
    RECT up = {26, 24, 24, 8};
    RECT down = {26, 220, 24, 8};
    RECT thumb = {16, 31, 12, 3};
    CVECTOR dark = {40, 40, 48};
    RECT edge;
    RECT unused2;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;
    u8 kind;

    sp->cursor_rect.x = 48;
    sp->cursor_rect.w = 270;
    sp->cursor_rect.h = 16;
    sp->cursor_rect.y = sp->cursor * 16 + 35;
    ui_draw_rect_outline(&sp->cursor_rect, &sp->cursor_color, 513, 1);
    if (sp->state.menu.half[0] > 0)
        ui_draw_arrow(&up, &sp->cursor_color, 3072);
    if (sp->state.menu.half[0] < sp->state.menu.half[1] - 11)
        ui_draw_arrow(&down, &sp->cursor_color, 1024);
    thumb.y = sp->state.menu.half[0] * 190 / sp->state.menu.half[1] + 31;
    ui_fill_rect(&thumb, &dark, 1);
    ui_add_draw_mode(1, 0, 0);
    edge.x = thumb.x - 2;
    edge.y = thumb.y - 1;
    edge.w = thumb.w;
    edge.h = thumb.h;
    ui_fill_rect(&edge, &dark, 1);
    ui_add_draw_mode(2, 0, 0);
    sp->rect.x = 60;
    sp->rect.y = 12;
    sp->rect.w = 196;
    sp->rect.h = 16;
    sp->tex_pos.vx = 896;
    sp->tex_pos.vy = 448;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 502;
    ui_draw_image(&sp->rect, &sp->tex_pos, &sp->clut_pos, NULL, 3);
    sp->image_rect.x = 50;
    sp->image_rect.y = 35;
    sp->image_rect.w = 196;
    sp->image_rect.h = 16;
    for (i = 0; i < 11; i++) {
        int id = sp->state.menu.list.ids[sp->state.menu.half[0] + i];
        if (id == -2)
            id = 500;
        if (id != -1) {
            if (id != 500) {
                sp->style[0].cd |= 0x80;
                ui_draw_text(VS_ITEM_NAME(id), sp->image_rect.x, sp->image_rect.y + 2, &sp->style[0]);
                sp->style[0].cd &= 0x7f;
            } else {
                ui_draw_text(VS_MESSAGE(500), sp->image_rect.x + 39, sp->image_rect.y + 2,
                              &sp->style[0]);
            }
        }
        sp->image_rect.y += 16;
    }
    sp->image_rect.x = 34;
    sp->image_rect.y = 35;
    sp->image_rect.w = 14;
    sp->image_rect.h = 13;
    for (i = 0; i < 11; i++) {
        int id = sp->state.menu.list.ids[sp->state.menu.half[0] + i];
        if (id < 70)
            kind = 0;
        else if (id < 91)
            kind = 5;
        else if (id < 121)
            kind = 1;
        else if (id < 141)
            kind = 8;
        else if (id < 161)
            kind = 7;
        else if (id < 196)
            kind = 2;
        else if (id < 231)
            kind = 3;
        else if (id < 251)
            kind = 6;
        else if (id < 266)
            kind = 4;
        if (kind >= 5) {
            sp->tex_pos.vx = (kind - 5) * 14 + 640;
            sp->tex_pos.vy = 256;
        } else {
            sp->tex_pos.vx = kind * 14 + 640;
            sp->tex_pos.vy = 240;
        }
        if (id != -1 && id != -2)
            ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
        sp->image_rect.y += 16;
    }
    ui_draw_panel(10, 9, 312, 222);
}

/* The caption bar: the selected entry's item name. */
ADDRESS(0x80022228, 0xb0)
void menu_arrange_icons_draw_caption(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int id = sp->state.menu.list.ids[sp->state.menu.half[0] * 4 + sp->cursor];

    sp->style[0].cd |= 0x80;
    if (id != -1)
        ui_draw_text(VS_ITEM_NAME(id), 14, 200, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    ui_draw_panel(10, 198, 620, 34);
}

/*
 * The detail panel of the selected entry: the image panels while the CD
 * queue is idle, up to fourteen nonzero part counts of its item record
 * (four columns of four), the count the opponent's save holds, the
 * record's +0x27 value and its weight (+0x24 / 4.54, with tenths).
 */
/* The two captions' x is count_x itself. As a u16 argument it is a zero
 * extension of a variable that has no register and is known to be 342, and
 * reload satisfies that by reading the constant from memory: the halfword
 * 342 in the unit's .sdata (0x800632b0) is reload's own pool entry, not a
 * table of the game's. */
ADDRESS(0x800222d8, 0x4bc)
void menu_item_request_draw_details(int page)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int text_y;
    short x, y;
    int count_x;
    s16 id;
    int name_x;
    int i;
    u8 *count;
    int n, k, j;

    int weight;
    u8 whole;
    u8 tenths;

    x = 334;
    y = 9;
    text_y = 135;
    count_x = 342;
    sp->image_rect.x = x;
    sp->image_rect.y = y;
    sp->image_rect.w = 192;
    sp->image_rect.h = 124;
    id = sp->state.menu.list.ids[(s16)(sp->state.menu.half[0] + sp->cursor)];
    if (id != -1 && id != -2) {
        if ((s16)cd_queue_busy() == 0) {
            sp->tex_pos.vx = 640;
            sp->tex_pos.vy = 0;
            ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
            sp->image_rect.w = 100;
            sp->tex_pos.vx = 832;
            sp->image_rect.x += 192;
            ui_draw_image(&sp->image_rect, &sp->tex_pos, NULL, NULL, 16);
        }
        n = 0;
        k = 2;
        count = vis_table_c.types[id].attributes;
        DAT_8006337a = 1;
        for (i = 0, name_x = count_x; i < 4; i++) {
            int label_x;
            int value_x;

            for (j = 0, label_x = name_x, value_x = name_x + 42; j < 4; j++) {
                if (n == 14)
                    goto done;
                n++;
                if (*count) {
                    ui_draw_text(VS_MESSAGE(200 + k), label_x, text_y + j * 15, &sp->style[0]);
                    ui_draw_number(*count | NUMBER_PLUS | DIGITS(3), value_x, text_y + j * 15, &sp->style[3]);
                }
                count++;
                k++;
            }
            if (n == 14)
                break;
            name_x += 70;
        }
    done:
        DAT_8006337a = 0;
        text_y += 60;
        ui_draw_text(VS_MESSAGE(501), count_x, text_y, &sp->style[0]);
        ui_draw_number(saves[1 - page].versus.item_durability[id] | DIGITS(3), count_x + 70, text_y,
            &sp->style[3]);
        ui_draw_text(VS_MESSAGE(502), count_x + 114, text_y, &sp->style[0]);
        ui_draw_number(vis_table_c.types[id].max_durability | DIGITS(3), count_x + 138, text_y,
            &sp->style[3]);
        weight = vis_table_c.types[id].weight;
        whole = weight * 100 / 454;
        tenths = weight * 1000 / 454 % 10;
        text_y += 15;
        ui_draw_text(VS_MESSAGE(503), count_x, text_y, &sp->style[0]);
        ui_draw_number(whole | DIGITS(3), count_x + 70, text_y, &sp->style[3]);
        ui_draw_glyph(18, count_x + 109, text_y, &sp->style[0]);
        ui_draw_number(tenths | DIGITS(1), count_x + 115, text_y, &sp->style[3]);
        ui_draw_glyph(53, count_x + 132, text_y, &sp->style[0]);
    }
    ui_draw_panel(x, y, 296, 222);
}

/* A two-tile emblem with its caption in a framed panel. */
ADDRESS(0x80022794, 0x144)
void menu_draw_button_prompt(void)
{
    CVECTOR light = {96, 96, 96};
    CVECTOR grey = {80, 80, 80};
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->tex_pos.vx = 896;
    sp->tex_pos.vy = 480;
    sp->clut_pos.vx = 544;
    sp->clut_pos.vy = 503;
    sp->image_rect.w = 32;
    sp->image_rect.h = 16;
    sp->image_rect.x = 353;
    sp->image_rect.y = 168;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, &sp->clut_pos, NULL, 0);
    sp->tex_pos.vx = 904;
    sp->clut_pos.vy = 504;
    sp->image_rect.x = 232;
    sp->image_rect.y = 168;
    ui_draw_image(&sp->image_rect, &sp->tex_pos, &sp->clut_pos, NULL, 0);
    ui_draw_text(VS_MESSAGE(216), 188, 150, &sp->style[0]);
    ui_draw_panel(168, 144, 319, 44);
}
