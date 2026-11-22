/*
 * Shared by ST.EXE and VS.EXE: node list lookup by id. The two images link
 * byte-identical copies; src/st/render.c and src/vs/anim_set.c include
 * this fragment at their place in each run.
 */

ADDRESS_AT("ST", 0x80034760, 0x4c)
ADDRESS_AT("VS", 0x80025670, 0x4c)
StAnimNode *anim_list_find_id(StAnimList *list, int id)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (id == node->unk8)
            break;
    }
    if (i == list->count)
        return NULL;
    return node;
}
