/*
 * creature_step_script, the map handler 19 FDAT overlays share (WIP; shared
 * fragment). Each of those overlays' map unit includes it at its place in
 * the run; the claim is FDAT_002.BIN's address and the other overlays bind
 * by name.
 *
 * It calls the overlay's move picker: creature_pick_move where the overlay has
 * that shared body, else the overlay's own variant, which the including
 * source names by defining FDAT_PICK_MOVE first.
 */
#ifndef FDAT_PICK_MOVE
#define FDAT_PICK_MOVE creature_pick_move
#endif

/*
 * Steps the entry's script frame; when the step leaves the frame range
 * (forwards past 4096, backwards to 0) picks its next move, and runs the
 * current script command.
 */
ADDRESS(0x801ed6e4, 0xb0)
void creature_step_script(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (creature_step_hurt_timer(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        FDAT_PICK_MOVE();
    }
    if (e->unkA8 != NULL) {
        creature_run_script_command(e);
        e->unk1A = 0xf0;
    }
}
