/*
 * Shared by ST.EXE and VS.EXE: VRAM block move and roll. The two images link
 * byte-identical copies; src/st/effect_spawn.c and
 * src/vs/effect_spawn_run.c include this fragment at their place in each
 * run.
 */

ADDRESS_AT("ST", 0x80038438, 0x2c)
ADDRESS_AT("VS", 0x80033470, 0x2c)
void vram_move_to(RECT *rect, DVECTOR *pos)
{
    MoveImage(rect, pos->vx, pos->vy);
}

/*
 * Copies the VRAM rectangle `src` to `dst` rolled down by *step rows (64
 * columns to the right when `right`), wrapping its bottom rows round to
 * the top; resets *step once it reaches the height. A step of 0 copies it
 * unrolled.
 */
ADDRESS_AT("ST", 0x80038464, 0x14c)
ADDRESS_AT("VS", 0x8003349c, 0x14c)
void vram_roll(u8 right, u8 *step, RECT *src, DVECTOR *dst)
{
    RECT part;
    RECT rect;
    DVECTOR pos;

    pos = *dst;
    rect = *src;

    if (right) {
        rect.x += 64;
        pos.vx += 64;
    }
    if (*step == 0)
        vram_move_to(&rect, &pos);
    else if (*step >= rect.h)
        *step = 0;
    part.x = pos.vx;
    part.y = pos.vy;
    pos.vx = rect.x;
    pos.vy = rect.y + *step;
    part.w = rect.w;
    part.h = rect.h - *step;
    MoveImage(&part, pos.vx, pos.vy);
    if (*step != 0) {
        pos.vy = rect.y;
        part.y = pos.vy + part.h;
        part.h = *step;
        MoveImage(&part, pos.vx, pos.vy);
    }
}
