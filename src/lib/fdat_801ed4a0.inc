/*
 * fdat_801ed4a0, the map handler 21 FDAT overlays share (WIP; shared
 * fragment, named by its FDAT_002.BIN address like the common functions).
 * No unit includes it yet: it is not exact, and its switch table, the last
 * entry of each overlay's read-only run, would diverge (the table's code
 * offsets move with it). An overlay's map unit includes it at its place in
 * the run, with its table in the unit's RODATA() claim, once it is exact.
 */

/*
 * After the entry's move and script pick, sets the heading it turns to and
 * its second heading (unk5C, unk5A) from its state, then bends both by the
 * offset of its visible record.
 * Not exact (8 bytes long): retail keeps e in s1 and the cosine in s0
 * (this build: s0, s1) and adds the record's bytes as z + unk06 after
 * loading unk05 first; this build loads unk06 first and adds it in front.
 */
ADDRESS(0x801ed4a0, 0x244)
void fdat_801ed4a0(void)
{
    Vis212 *e = vis_table_a.unk6138;
    u16 wall;
    int d;
    int x;
    int z;
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
    case 18:
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
        dz = z + vis_records[e->unk26].unk06;
        dx = x + vis_records[e->unk26].unk05;
        e->unk5C = (ratan2(dz, dx) + 1024) & 0xfff;
        x = rcos(e->unk5A - 1024) >> 5;
        z = rsin(e->unk5A - 1024) >> 5;
        dz = z + vis_records[e->unk26].unk06;
        dx = x + vis_records[e->unk26].unk05;
        e->unk5A = ratan2(dz, dx);
    }
}
