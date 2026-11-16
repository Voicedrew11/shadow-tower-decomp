/*
 * Shared by ST.EXE and VS.EXE: animation record and list composition. The
 * two images link byte-identical copies; src/st/anim_set.c and
 * src/vs/anim_8002429c.c include this fragment at their place in each run.
 */

/*
 * rec->unk24 = the GTE's current matrix (the parent's) times rec->unk4,
 * left loaded as the GTE rotation and translation.
 */
#define RECORD_COMPOSE(sp, rec)                                   \
    do {                                                          \
        (sp)->unk9C.vx = (rec)->unk4.t[0];                        \
        (sp)->unk9C.vy = (rec)->unk4.t[1];                        \
        (sp)->unk9C.vz = (rec)->unk4.t[2];                        \
        gte_ldv0(&(sp)->unk9C);                                   \
        gte_rt();                                                 \
        gte_stlvnl((rec)->unk24.t);                               \
        gte_SetTransMatrix(&(rec)->unk24);                        \
        gte_ldclmv(&(rec)->unk4);                                 \
        gte_rtir();                                               \
        gte_stclmv(&(rec)->unk24);                                \
        gte_ldclmv((char *)&(rec)->unk4 + 2);                     \
        gte_rtir();                                               \
        gte_stclmv((char *)&(rec)->unk24 + 2);                    \
        gte_ldclmv((char *)&(rec)->unk4 + 4);                     \
        gte_rtir();                                               \
        gte_stclmv((char *)&(rec)->unk24 + 4);                    \
        gte_SetRotMatrix(&(rec)->unk24);                          \
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

    if (rec->unk48 == NULL) {
        gte_SetRotMatrix(SCRATCH_MATRIX(SCRATCHPAD));
        gte_SetTransMatrix(SCRATCH_MATRIX(SCRATCHPAD));
        RECORD_COMPOSE(sp, rec);
    } else if (rec->unk0) {
        gte_SetTransMatrix(&rec->unk24);
        gte_SetRotMatrix(&rec->unk24);
        return;
    } else {
        anim_record_compose(rec->unk48);
        RECORD_COMPOSE(sp, rec);
    }
    rec->unk0 = 1;
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
        if (node->unkC != -1 && node->unk8 != 0xffff)
            anim_record_compose(node->unk4);
        node++;
    }
}
