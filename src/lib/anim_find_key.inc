/*
 * Shared by ST.EXE and VS.EXE: clip key lookup. The two images link byte-
 * identical copies; src/st/anim.c and src/vs/anim_80027f30.c include this
 * fragment at their place in each run.
 */

/*
 * Finds the key of `clip` that `frame` falls in: stores its index and the
 * position in it (4096ths) and returns it. Past the clip's end it returns
 * the last key at 4096.
 */
ADDRESS_AT("ST", 0x80033cfc, 0xbc)
ADDRESS_AT("VS", 0x80027f30, 0xbc)
StAnimKey *anim_find_key(StAnimFile *file, int clip, int frame, int *key, int *frac)
{
    StAnimKey *k = NULL;
    int i = 0;
    int end = 0;
    int start;
    StAnimClip *c;
    u_long *keys;
    int count;
    u_int length;
    int f;

    c = (StAnimClip *)((u8 *)file + ((u_long *)((u8 *)file + file->clips))[clip]);
    keys = c->keys;
    count = c->count;
    start = end;
    while (count--) {
        k = (StAnimKey *)((u8 *)file + *keys++);
        length = k->length;
        end += length;
        if (frame < end) {
            f = ((frame - start) << 12) / length;
            goto found;
        }
        start = end;
        i++;
    }
    i--;
    f = 4096;
found:
    *key = i;
    *frac = f;
    return k;
}
