/*
 * Shared by ST.EXE and VS.EXE: per-vertex sum and count-prefixed vertex list
 * copy. The two images link byte-identical copies; src/st/anim.c and
 * src/vs/anim_80027c64.c include this fragment at their place in each run.
 */

/* dst = a + b, per vertex. */
ADDRESS_AT("ST", 0x80033b00, 0x7c)
ADDRESS_AT("VS", 0x80027ce0, 0x7c)
void vertices_add(SVECTOR *dst, SVECTOR *a, SVECTOR *b, int count)
{
    while (count--) {
        dst->vx = a->vx + b->vx;
        dst->vy = a->vy + b->vy;
        dst->vz = a->vz + b->vz;
        dst++;
        a++;
        b++;
    }
}

/* Copies a count-prefixed vertex list (the count halfword, a pad
 * halfword, then SVECTORs). */
ADDRESS_AT("ST", 0x80033b7c, 0x60)
ADDRESS_AT("VS", 0x80027d5c, 0x60)
void vertex_list_copy(short *dst, short *src)
{
    int count;

    count = *src;
    src += 2;
    *dst = count;
    dst += 2;
    while (count--) {
        dst[0] = *src++;
        dst[1] = *src++;
        dst[2] = *src++;
        src++;
        dst += 4;
    }
}
