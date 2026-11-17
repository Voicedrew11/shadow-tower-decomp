/*
 * FDAT_142.BIN: its second handler, not claimed (WIP): it reads the scene
 * statics of src/fdat_142/map_801ed4a0.c, and the delinker cannot yet pair
 * fdat_801edafc's low with one of its three luis (two dead in delay
 * slots), so the unit cannot take it. Include it there once it can.
 */
/*
 * Runs the area: the viewer standing on record 0x1f2 or 0x63 picks the
 * scene words; entry 48 asleep until it wakes (scene word 3 armed), then
 * leaves once the area fade is done, keeping the viewer's status 0x188 at
 * 30; record 0x10e with both words set sends entry 49 away and frees
 * entries 5..7.
 */
ADDRESS(0x801ed54c, 0x26c)
void func_801ed54c(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e49 = &vis_table_a.entries[49];
    Vis212 *e5 = &vis_table_a.entries[5];
    Vis212 *e6 = &vis_table_a.entries[6];
    Vis212 *e7 = &vis_table_a.entries[7];

    if (fdat_801edaf8 == 0) {
        if (viewer.unk178 == 0x1f2) {
            fdat_801edaf4 = 1;
        } else if (viewer.unk178 == 0x63) {
            fdat_801edaf4 = 0;
            fdat_801edaf8 = 1;
        }
    }
    if (fdat_801edaf4 == 1 && fdat_801edaf8 == 1)
        e48->unk00 = 0;
    if (fdat_801edaf4 == 0) {
        if (fdat_801edaf8 == 1)
            e49->unk00 = 0;
        if (fdat_801edaf4 == 0 && e48->unk05 == 1) {
            fdat_801edafc = 1;
            viewer.unk218 = 5;
        }
    }
    if (fdat_801edafc == 1 && area_state.unk18 == -1 && (short)area_state.unk2 >= 2)
        fdat_801edafc = 2;
    if (fdat_801edafc == 2) {
        fdat_801ed388(e48, 0, 0);
        fdat_801ecf44(e48);
        fdat_801edafc = 0xff;
        viewer.unk218 = 0;
    }
    if (fdat_801edafc == 0xff && !(viewer.unk2C & 0x800)) {
        viewer.unk188 = 30;
        viewer.unk182 = 0;
    }
    if (fdat_801edb00 == 0 && viewer.unk178 == 0x10e && fdat_801edaf4 == 1
        && fdat_801edaf8 == 1) {
        fdat_801ed388(e49, 0, 0);
        fdat_801ed388(e49, 0, 0);
        fdat_801ecf44(e49);
        fdat_801ed410(e5);
        fdat_801ed410(e6);
        fdat_801ed410(e7);
        fdat_801edb00 = 0xff;
    }
}
