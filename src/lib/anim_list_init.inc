/*
 * Shared by ST.EXE and VS.EXE: node list initialisation. The two images link
 * byte-identical copies; src/st/anim.c and src/vs/anim_80023764.c include
 * this fragment at their place in each run.
 */

/* Links `count` nodes to their 0x50-byte records (and those to their
 * 0x28-byte ones) and starts the set's list empty. */
ADDRESS_AT("ST", 0x800346dc, 0x84)
ADDRESS_AT("VS", 0x800239d0, 0x84)
void anim_list_init(StAnimSet *set, int count)
{
    StAnimNode *node = set->unk28;
    StAnimRecord *rec = set->unk2C;
    StAnimSubRecord *sub = set->unk30;
    int i;

    set->unk34->count = 0;
    set->unk34->capacity = count;
    set->unk34->nodes = set->unk28;
    for (i = 0; i < count; i++) {
        node->unk0 = 0x80000000;
        node->unkC = -1;
        node->unk4 = rec;
        rec->unk44 = sub;
        node->unk8 = 0xffff;
        node->unkA = 0;
        node->unkB = 0;
        node++;
        rec++;
        sub++;
    }
}
