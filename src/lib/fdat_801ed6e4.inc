/*
 * fdat_801ed6e4, the map handler 19 FDAT overlays share (WIP; shared
 * fragment). Each of those overlays' map unit includes it at its place in
 * the run; the claim is FDAT_002.BIN's address and the other overlays bind
 * by name.
 *
 * It calls the overlay's move picker: fdat_801ed4a0 where the overlay has
 * that shared body, else the overlay's own variant, which the including
 * source names by defining FDAT_PICK_MOVE first.
 */
#ifndef FDAT_PICK_MOVE
#define FDAT_PICK_MOVE fdat_801ed4a0
#endif

/*
 * Steps the entry's script frame; when the step leaves the frame range
 * (forwards past 4096, backwards to 0) picks its next move, and runs the
 * current script command.
 */
ADDRESS(0x801ed6e4, 0xb0)
void fdat_801ed6e4(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        FDAT_PICK_MOVE();
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}
