/*
 * Shared by ST.EXE and VS.EXE: animation record and list composition. The
 * two images link byte-identical copies; src/st/anim_set.c and
 * src/vs/anim_set.c include this fragment at their place in each run.
 */

/*
 * rec->composed_matrix = the GTE's current matrix (the parent's) times rec->local_matrix,
 * left loaded as the GTE rotation and translation.
 */
#define RECORD_COMPOSE(sp, rec)                                   \
    do {                                                          \
        (sp)->node_trans.vx = (rec)->local_matrix.t[0];                        \
        (sp)->node_trans.vy = (rec)->local_matrix.t[1];                        \
        (sp)->node_trans.vz = (rec)->local_matrix.t[2];                        \
        gte_ldv0(&(sp)->node_trans);                                   \
        gte_rt();                                                 \
        gte_stlvnl((rec)->composed_matrix.t);                               \
        gte_SetTransMatrix(&(rec)->composed_matrix);                        \
        gte_ldclmv(&(rec)->local_matrix);                                 \
        gte_rtir();                                               \
        gte_stclmv(&(rec)->composed_matrix);                                \
        gte_ldclmv((char *)&(rec)->local_matrix + 2);                     \
        gte_rtir();                                               \
        gte_stclmv((char *)&(rec)->composed_matrix + 2);                    \
        gte_ldclmv((char *)&(rec)->local_matrix + 4);                     \
        gte_rtir();                                               \
        gte_stclmv((char *)&(rec)->composed_matrix + 4);                    \
        gte_SetRotMatrix(&(rec)->composed_matrix);                          \
    } while (0)

/*
 * Composes the record's matrix with its parents' (composing them first
 * unless already done for this pose; a root one with the scratchpad's
 * root transform) and leaves it loaded on the GTE.
 */
ADDRESS_AT("ST", 0x80035260, 0x3a4)
ADDRESS_AT("VS", 0x8002429c, 0x3a4)
void anim_record_compose(StAnimRecord *rec)
{
    ScratchPad *sp = SCRATCHPAD;

    if (rec->parent == NULL) {
        gte_SetRotMatrix(SCRATCH_MATRIX(SCRATCHPAD));
        gte_SetTransMatrix(SCRATCH_MATRIX(SCRATCHPAD));
        RECORD_COMPOSE(sp, rec);
    } else if (rec->composed) {
        gte_SetTransMatrix(&rec->composed_matrix);
        gte_SetRotMatrix(&rec->composed_matrix);
        return;
    } else {
        anim_record_compose(rec->parent);
        RECORD_COMPOSE(sp, rec);
    }
    rec->composed = 1;
}

/* Composes the matrices of every node of the scratchpad's node list that
 * is in use and has an id. */
ADDRESS_AT("ST", 0x80035604, 0x98)
ADDRESS_AT("VS", 0x80024640, 0x98)
void anim_list_compose(void)
{
    StAnimList *list = SCRATCHPAD->list.anims;
    int i = list->count;
    StAnimNode *node = list->nodes;

    while (i--) {
        if (node->key != -1 && node->part != 0xffff)
            anim_record_compose(node->record);
        node++;
    }
}
