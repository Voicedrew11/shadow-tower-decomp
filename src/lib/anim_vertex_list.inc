/*
 * Shared by ST.EXE and VS.EXE: adding a count-prefixed vertex list (the
 * count halfword, a pad halfword, then SVECTORs). The two images link
 * byte-identical copies; src/st/render.c and src/vs/anim_vertices.c include
 * this fragment at their place in each run.
 */

/* dst = a + the vertices of the count-prefixed list b. */
ADDRESS_AT("ST", 0x80033bdc, 0x84)
ADDRESS_AT("VS", 0x80027dbc, 0x84)
void vertex_list_add(SVECTOR *dst, SVECTOR *a, short *b)
{
    int count = *b;

    b += 2;
    while (count--) {
        dst->vx = a->vx + b[0];
        dst->vy = a->vy + b[1];
        dst->vz = a->vz + b[2];
        b += 4;
        dst++;
        a++;
    }
}

/* dst += the vertices of the count-prefixed list src. */
ADDRESS_AT("ST", 0x80033c60, 0x74)
ADDRESS_AT("VS", 0x80027e40, 0x74)
void vertex_list_accumulate(SVECTOR *dst, short *src)
{
    int count = *src;

    src += 2;
    while (count--) {
        dst->vx += src[0];
        dst->vy += src[1];
        dst->vz += src[2];
        src += 4;
        dst++;
    }
}
