/*
 * Shared by ST.EXE and VS.EXE: node list key lookup, insertion, removal, the
 * node command interpreter and the block runner. The two images link byte-
 * identical copies; src/st/render.c and src/vs/anim_set.c include this
 * fragment at their place in each run.
 */

ADDRESS_AT("ST", 0x800347ac, 0x4c)
ADDRESS_AT("VS", 0x80023a54, 0x4c)
StAnimNode *anim_list_find_key(StAnimList *list, int key)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (key == node->key)
            break;
    }
    if (i == list->count)
        return NULL;
    return node;
}

/* Claims a free node for `key` (reusing a released one first, else
 * appending while below capacity) and resets its record to the identity
 * transform; NULL when the list is full. */
ADDRESS_AT("ST", 0x800347f8, 0x130)
ADDRESS_AT("VS", 0x80023aa0, 0x130)
StAnimNode *anim_list_add(StAnimList *list, int key)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (node->key == -1)
            break;
    }
    if (i >= list->count) {
        if (i >= list->capacity)
            return NULL;
        node = &list->nodes[list->count++];
    }
    node->key = key;
    node->flags = 0;
    node->record->parent = NULL;
    node->record->local_matrix.m[0][0] = 4096;
    node->record->local_matrix.m[0][1] = 0;
    node->record->local_matrix.m[0][2] = 0;
    node->record->local_matrix.m[1][0] = 0;
    node->record->local_matrix.m[1][1] = 4096;
    node->record->local_matrix.m[1][2] = 0;
    node->record->local_matrix.m[2][0] = 0;
    node->record->local_matrix.m[2][1] = 0;
    node->record->local_matrix.m[2][2] = 4096;
    node->record->local_matrix.t[0] = 0;
    node->record->local_matrix.t[1] = 0;
    node->record->local_matrix.t[2] = 0;
    node->part = 0xffff;
    node->kind = 0;
    node->unkB = 0;
    return node;
}

/* Releases the node of `key` (its key stays until it is reused). */
ADDRESS_AT("ST", 0x80034928, 0x64)
ADDRESS_AT("VS", 0x80023bd0, 0x64)
StAnimNode *anim_list_remove(StAnimList *list, int key)
{
    StAnimNode *node = list->nodes;
    int i;

    for (i = 0; i < list->count; i++, node++) {
        if (node->key == key)
            break;
    }
    if (i < list->count) {
        node->part = 0xffff;
        node->kind = 0;
        node->unkB = 0;
        return node;
    }
    return NULL;
}

/* The index of `id` among the first set[1] words of `ids`, or 0xffff. */
ADDRESS_AT("ST", 0x8003498c, 0x44)
ADDRESS_AT("VS", 0x80023c34, 0x44)
u16 anim_id_index(u_long *set, u_long *ids, u_long id)
{
    int count = set[1];
    int i = 0;

    while (count > 0) {
        if (id == *ids)
            break;
        i++;
        count--;
        ids++;
    }
    if (count == 0)
        return 0xffff;
    return i;
}

/*
 * Runs one node command of a skeletal clip block on the node of its key
 * (a scratch node when the key has none) and returns the next command.
 * The command word holds the key (bits 0-15), the opcode (16-19), its
 * flags (20-23) and its length in words (24-31).
 */
ADDRESS_AT("ST", 0x800349d0, 0x498)
ADDRESS_AT("VS", 0x80023c78, 0x498)
u_long *anim_node_command(u_long *cmd, StAnimList *list, u_long *ids, u8 *table, u_long *set)
{
    u_long unused[6];
    StAnimNode scratch_node;
    MATRIX scratch_matrix;
    StAnimSubRecord scratch_sub;
    u_long word = cmd[0];
    long *p = (long *)(cmd + 1);
    int key = word & 0xffff;
    int op = (word >> 16) & 0xf;
    int flags = (word >> 20) & 0xf;
    int length = word >> 24;
    StAnimNode *node;
    MATRIX *m;
    StAnimSubRecord *sub;
    StAnimRecord *rec;

    node = anim_list_find_key(list, key);
    if (node == NULL) {
        node = &scratch_node;
        m = &scratch_matrix;
        sub = &scratch_sub;
    } else {
        rec = node->record;
        sub = rec->transform;
        m = &rec->local_matrix;
        rec->composed = 0;
    }
    switch (op) {
    case 0:
        node->flags = (node->flags & p[0]) | p[1];
        break;
    case 1:
        if (flags & 1) {
            /* relative */
            if (flags & 2) {
                sub->rot.vx += p[0] / 360;
                sub->rot.vy += p[1] / 360;
                sub->rot.vz += p[2] / 360;
                p += 3;
            }
            if (flags & 4) {
                sub->scale.vx = sub->scale.vx * ((short *)p)[0] / 4096;
                sub->scale.vy = sub->scale.vy * ((short *)p)[1] / 4096;
                sub->scale.vz = sub->scale.vz * ((short *)p)[2] / 4096;
                p += 2;
            }
            if (flags & 8) {
                sub->trans.vx += p[0];
                sub->trans.vy += p[1];
                sub->trans.vz += p[2];
            }
            RotMatrix(&sub->rot, m);
            ScaleMatrix(m, &sub->scale);
            TransMatrix(m, &sub->trans);
            node->record->relative_set = 1;
        } else {
            if (flags & 2) {
                sub->rot.vx = p[0] / 360;
                sub->rot.vy = p[1] / 360;
                sub->rot.vz = p[2] / 360;
                p += 3;
                RotMatrix(&sub->rot, m);
            }
            if (flags & 4) {
                sub->scale.vx = ((short *)p)[0];
                sub->scale.vy = ((short *)p)[1];
                sub->scale.vz = ((short *)p)[2];
                p += 2;
                if (!(flags & 2))
                    RotMatrix(&sub->rot, m);
                ScaleMatrix(m, &sub->scale);
            }
            if (flags & 8) {
                sub->trans.vx = p[0];
                sub->trans.vy = p[1];
                sub->trans.vz = p[2];
                TransMatrix(m, &sub->trans);
            }
        }
        break;
    case 2:
        if (set != NULL) {
            node->part = anim_id_index(set, ids, *(u16 *)p);
            node->kind = table[node->part * 4];
        }
        break;
    case 3:
        if (p[0] == 0 || p[0] == 0xffff)
            node->record->parent = NULL;
        else
            node->record->parent = anim_list_find_key(list, p[0])->record;
        break;
    case 8:
        if (list != NULL) {
            switch (flags) {
            case 0:
                if (anim_list_find_key(list, key) == NULL)
                    anim_list_add(list, key);
                break;
            case 1:
                anim_list_remove(list, key);
                break;
            }
        }
        break;
    }
    return cmd + length;
}

/* Runs the block's commands once `frame` has reached it and returns the
 * next block (the same block before then). */
ADDRESS_AT("ST", 0x80034e68, 0xa4)
ADDRESS_AT("VS", 0x80024110, 0xa4)
StAnimBlock *anim_block_run(u_long frame, StAnimBlock *block, StAnimList *list, u_long *ids,
                            u8 *table, u_long *set)
{
    u_long count = block->count;
    u_long *cmd = (u_long *)block;
    u_long i;

    if (frame >= block->frame) {
        cmd = block->cmds;
        for (i = 0; i < count; i++)
            cmd = anim_node_command(cmd, list, ids, table, set);
    }
    return (StAnimBlock *)cmd;
}
