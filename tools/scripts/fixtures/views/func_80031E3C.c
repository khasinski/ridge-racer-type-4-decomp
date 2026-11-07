#include "rr/selection.h"

/* legacy typing of shared globals in this part of the unit */
#define UpdateSelectionRank ((int (*)())UpdateSelectionRank)

#undef UpdateSelectionRank

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((Struct800F3AFC *)BadgeRows)

#define Dialogue ((Struct800FF68C *)Dialogue)

#undef BadgeRows

#undef Dialogue

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows (*(s32 (*))&BadgeRows)

#define Dialogue ((void *)Dialogue)

#undef BadgeRows

#undef Dialogue

/* legacy typing of shared globals in this part of the unit */
#define RankEntries (*(struct E (*)[])&RankEntries)

#define RankTable (*(struct E *(*)[])&RankTable)

#undef RankEntries

#undef RankTable

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((void *)BadgeRows)

#undef BadgeRows

/* legacy typing of shared globals in this part of the unit */
#define SelectionState (*(w3g2S (*))&SelectionState)

#define ModeSubState (*(unsigned short (*))&ModeSubState)

#undef SelectionState

#undef ModeSubState

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((void *)BadgeRows)

#undef BadgeRows

void InitSelectionRows(void) {
    s32 a2 = 0;
    s32 a1;
    RrSelectionRow *a0;
    RrSelectionRow *v1;
    RrSelectionRows *b;
    unsigned char *page;
    const u8 *choices, *variants;
    const RrSelectionConfigView *config;
    u8 t;
    s32 x;
    unsigned char *stage;
    /* ASM_BARRIER: Preserve the indexed byte offset in the address temporary. */
    register u32 offset __asm__("$2");
    u32 attribute;
    page=(unsigned char *)0x800F0000U;
    /* ASM_BARRIER: Materialize the state page after the initial row index. */
    __asm__("" : "+r"(page) : "r"(a2));
    b=(RrSelectionRows *)(page+0x4D98);
    /* ASM_BARRIER: Retain the full frame pointer separately from its page. */
    __asm__("" : "+r"(b));
    stage=(unsigned char *)0x800A0000U;
    /* ASM_BARRIER: Expose the selection-table page before adding its signed offset. */
    __asm__("" : "+r"(stage));
    choices=stage-0xBDC;
    stage=(unsigned char *)0x800A0000U;
    /* ASM_BARRIER: Keep the separate variant-table page materialization. */
    __asm__("" : "+r"(stage));
    variants=stage-0xBE0;
    stage=(unsigned char *)0x800B0000U;
    /* ASM_BARRIER: Expose the configuration page before adding its signed offset. */
    __asm__("" : "+r"(stage));
    config=(const RrSelectionConfigView *)(stage-0x2B28);
    /* ASM_BARRIER: Form lookup bases before initializing the byte offset. */
    __asm__("" : : "r"(choices), "r"(variants), "r"(config) : "$5");
    a1=RR_FIELD_OFFSET(RrSelectionRows,rows);
    do {
        x = page[0x4D98];
        offset=x*8;
        offset=a2+offset;
        t=*(unsigned char *)((unsigned long)offset+(unsigned long)choices);
        a0 = (RrSelectionRow *)((unsigned long)a1+(unsigned long)b);
        a0->selection = t;
        if (t == page[0x4D98]) {
            a0->variant = b->variant;
        } else {
            a0->variant = *(unsigned char *)((unsigned long)a0->selection+(unsigned long)variants);
            /* ASM_BARRIER: Retain the original row-pointer recomputation after the variant store. */
            __asm__("" : "+r"(a0));
            a0 = (RrSelectionRow *)((unsigned long)a1+(unsigned long)b);
        }
        if (a0->selection != page[0x4D98]) {
            if (a0->variant == b->variant) {
                a0->variant = *(unsigned char *)((unsigned long)page[0x4D98]+(unsigned long)variants);
            }
        }
        v1 = (RrSelectionRow *)((unsigned long)a1+(unsigned long)b);
        /* ASM_BARRIER: Keep the current row and index before publishing its attribute. */
        __asm__("" : : "r"(v1), "r"(a2));
        attribute=config->parameter;
        /* ASM_BARRIER: Read the row attribute before advancing the loop index. */
        __asm__("" : "+r"(attribute) : "r"(a2));
        a2 += 1;
        v1->parameter = attribute;
        a1 += sizeof(RrSelectionRow);
    } while (a2 < 7);
}
