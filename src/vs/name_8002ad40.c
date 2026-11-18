/*
 * VS.EXE: drawing the fighters' names (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>

/*
 * Draws both fighters' names (item names DAT_801624b8[n].slot[DAT_80063338]) in
 * 11-pixel glyphs from the font page at VRAM (640, 0): the first on the
 * left, the second on the right, wrapping to a second line at glyph 49;
 * `bright` sets their green tint.
 */
/* Not exact (size-exact, 8 lines, registers agree): in the glyph loop's pre-
 * header retail has the hoisted constants first (graphics base, 40, 50, 29)
 * and then the four x settings; here the x settings come first. loop.c puts
 * what it hoists directly in front of the loop's start note, so in retail
 * the x settings sit behind that note. Ruled out: an enclosing do-while(0),
 * while or for(;;) around the settings (flow counts it as a loop level and
 * bright moves into a register), a function-level row variable (the 29 is
 * then not hoisted last and takes $t1). */
ADDRESS(0x8002ad40, 0x338)
void func_8002ad40(int bright)
{
    u16 *s;
    int line = 0;
    int n;
    int x0, x1, x2, x3;
    u16 glyph;

    for (n = 0; n < 2;) {
        s = VS_ITEM_NAME(DAT_801624b8[n].slot[DAT_80063338]);
        glyph = *s;
        if (glyph == 0xffff || glyph == 0)
            goto next;

        x3 = 212;
        x2 = 201;
        x1 = 31;
        x0 = 20;
        do {
            POLY_FT4 *p;
            int c;

            if (glyph == 1)
                break;
            c = *s - 2;
            if (c == 47) {
                line = 1;
                x3 = 212;
                x2 = 201;
                x1 = 31;
                x0 = 20;
            }
            p = (POLY_FT4 *)DAT_801624d0.packet->cur;
            DAT_801624d0.packet->cur = (u_long *)(p + 1);
            setPolyFT4(p);
            setSemiTrans(p, 1);
            setUV4(p, c % 18 * 12, c / 18 * 10, c % 18 * 12 + 11, c / 18 * 10,
                   c % 18 * 12, c / 18 * 10 + 10, c % 18 * 12 + 11, c / 18 * 10 + 10);
            if (n == 0) {
                if (line) {
                    int y = 40;

                    setXY4(p, x0, y, x1, y, x0, y + 10, x1, y + 10);
                } else {
                    int y = 29;

                    setXY4(p, x0, y, x1, y, x0, y + 10, x1, y + 10);
                }
            } else {
                if (line) {
                    int y = 40;

                    setXY4(p, x2, y, x3, y, x2, y + 10, x3, y + 10);
                } else {
                    int y = 29;

                    setXY4(p, x2, y, x3, y, x2, y + 10, x3, y + 10);
                }
            }
            p->tpage = GetTPage(0, 1, 640, 0);
            p->clut = GetClut(640, 480);
            setRGB0(p, bright * 3, bright * 3 + 16, bright * 3);
            addPrim(DAT_801624d0.ot_cur + 5, p);
            s++;
            x3 += 11;
            x2 += 11;
            x1 += 11;
            x0 += 11;
            glyph = *s;
        } while (glyph != 0xffff && glyph != 0);
    next:
        n++;
        line = 0;
    }
}
