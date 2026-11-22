/*
 * Shared by ST.EXE and VS.EXE: node list initialisation. The two images link
 * byte-identical copies; src/st/render.c and src/vs/anim_set.c include
 * this fragment at their place in each run.
 */

/* Links `count` nodes to their 0x50-byte records (and those to their
 * 0x28-byte ones) and starts the set's list empty. */
ADDRESS_AT("ST", 0x800346dc, 0x84)
ADDRESS_AT("VS", 0x800239d0, 0x84)
void anim_list_init(StAnimSet *set, int count)
{
    StAnimNode *node = set->nodes;
    StAnimRecord *rec = set->records;
    StAnimSubRecord *sub = set->sub_records;
    int i;

    set->list->count = 0;
    set->list->capacity = count;
    set->list->nodes = set->nodes;
    for (i = 0; i < count; i++) {
        node->flags = 0x80000000;
        node->key = -1;
        node->record = rec;
        rec->transform = sub;
        node->part = 0xffff;
        node->kind = 0;
        node->unkB = 0;
        node++;
        rec++;
        sub++;
    }
}
