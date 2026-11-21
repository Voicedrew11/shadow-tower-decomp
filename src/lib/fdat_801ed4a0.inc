/*
 * fdat_801ed4a0, the map handler 23 FDAT overlays share (WIP; shared
 * fragment, named by its FDAT_002.BIN address like the common functions).
 * Each of those overlays' map unit includes it at its place in the run; the
 * claim is FDAT_002.BIN's address and the other overlays bind by name. Its
 * switch table is the last entry of the overlay's read-only run, so the
 * including unit's RODATA() claim covers it.
 */

/*
 * After the entry's move and script pick, sets the heading it turns to and
 * its second heading (unk5C, unk5A) from its state, then bends both by the
 * offset of its visible record.
 * The first heading uses a -1024 phase; the second passes unk5A directly.
 */
ADDRESS(0x801ed4a0, 0x244)
void fdat_801ed4a0(void)
{
    Vis212 *e = vis_table_a.unk6138;
    u16 wall;
    int d;
    short x;
    short z;
    short dx;
    short dz;

    if (e->unk1A == 0xf0 || e->unk1A == 0)
        return;
    fdat_801ec6a4();
    fdat_801ecbcc();
    switch (e->unk44) {
    case 8:
        wall = e->unk5E;
        if ((short)wall == 0x7fff) {
            e->unk5A = e->unk5C = (e->unk56 + 2048) & 0xfff;
        } else {
            d = (((e->unk56 + 2048) & 0xfff) - wall + 1024) & 0xfff;
            if (d < 1024 || d > 3072)
                e->unk5A = e->unk5C = (wall + 3072) & 0xfff;
            else
                e->unk5A = e->unk5C = (wall + 1024) & 0xfff;
        }
        break;
    case 2:
    case 4:
    case 16:
    case 32:
        e->unk5A = e->unk5C = e->unk56;
        break;
    default:
        e->unk5A = e->unk5C = e->unk8E;
        break;
    }
    if (e->unk44 != 8)
        e->unk5E = 0x7fff;
    if (vis_records[e->unk26].unk05 != 0 || vis_records[e->unk26].unk06 != 0) {
        x = rcos(e->unk5C - 1024) >> 5;
        z = rsin(e->unk5C - 1024) >> 5;
        dx = x + vis_records[e->unk26].unk05;
        dz = z + vis_records[e->unk26].unk06;
        e->unk5C = (ratan2(dz, dx) + 1024) & 0xfff;
        x = rcos(e->unk5A) >> 5;
        z = rsin(e->unk5A) >> 5;
        dx = x + vis_records[e->unk26].unk05;
        dz = z + vis_records[e->unk26].unk06;
        e->unk5A = ratan2(dz, dx);
    }
}
