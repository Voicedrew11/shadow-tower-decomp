/*
 * The first own handler of FDAT_442.BIN, FDAT_452.BIN, FDAT_462.BIN and
 * FDAT_472.BIN: one body at the same address in all four, included by each
 * overlay's unit after its 21 common functions. The scene state it clears
 * (fdat_801ed8c4, named after FDAT_442.BIN's address) is the including
 * unit's first static word.
 */
/* Resets the map scene: the viewer's slots, pitch and the scene state. */
ADDRESS(0x801ed420, 0x34)
void map_enter_shop(void)
{
    DAT_801cd0e4 = 0;
    viewer.state = VIEWER_STATE_SHOP;
    viewer.view_rot.vx = 0;
    viewer.action_timers.halves[0] = -1;
    viewer.action_timers.halves[1] = -1;
    fdat_801ed8c4 = 0;
}
