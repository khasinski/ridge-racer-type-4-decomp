#include "types.h"
#include "m2c_macros.h"
#include "rr/record_banks.h"
#include "game/data/ldscn1.h"
#include "game/menu/menu13.h"
#include "game/data/bankrec.h"

extern int CarRecordPtrs[];
extern s32 TrackLightTable;

void InitReplayBuffers(s32 layout,s32 mode);

void SetupDialogueTable(a) char *a; {
    int i;
    unsigned short *p;
    void **q;
    DialoguePageCounts = a + 0x50;
    i = 0;
    q = DialogueLinePtrs;
    p = (unsigned short *)a;
    do { *q++ = a + 0x1B8 + *p++ * 2; i++; } while (i < 0x28);
    DialogueLoadIndexTable = a + 0x78;
}

void SetCarRecordPtr(a, i) int a; int i; { CarRecordPtrs[i] = a; }

void SetLightTable(s32 arg0) {
    TrackLightTable = arg0;
}

void InitReplayBuffers(s32 layout,s32 mode) {
    /* ASM_BARRIER: keep each loop's index in the consumed mode register. */
    register s32 i __asm__("$5");
    register u32 factor;
    register u8 *base;
    /* ASM_BARRIER: finish each shared arena address before copying its base. */
    register u8 *loaded __asm__("$2");
    register u8 *bankbase;
    register RrBankRecord320 **source_slots;
    register u32 offset;
    register RrBankRecord320 **bank_source_slots;
    register u8 **bank_slots;
    register u32 bank_offset;
    RrBankRecord320 **restore_source,**restore_dest;
    s32 bound;
    register u8 *read_page;
    u8 *bound_page;

    ReplayFrame=0;
    ReplayState=mode;
    if (mode==2) {
        /* PAL mapping: page-0x3A64 is the live signed count NumCars. */
        read_page=(u8 *)0x800B0000U;
        /* ASM_BARRIER: capture the count page before testing and retaining it. */
        __asm__("" : "+r"(read_page));
        i=0;
        if (*(s16 *)(read_page-0x3A64)>0) {
            bound_page=read_page;
            restore_dest=CarTable;
            restore_source=CarSnapshots;
            do {
                **restore_dest=**restore_source;
                if ((*restore_dest)->value1E4>=17) {
                    (*restore_dest)->value1E4&=255;
                }
                restore_dest++;
                bound=*(s16 *)(bound_page-0x3A64);
                /* ASM_BARRIER: reload the limit before incrementing the index. */
                __asm__("" : "+r"(bound));
                i++;
                restore_source++;
            } while (i<bound);
        }
    } else {
        SavedRaceState=0;
        i=0;
        if (layout==1) {
            loaded=SharedWorkBuffer;
            base=loaded;
            source_slots=CarSnapshots;
            offset=0x5468;
            ReplayLength=2700;
            ReplayInputs[0]=base;
            ReplayInputs[1]=base+0x2A34;
            do {
                *source_slots=(RrBankRecord320 *)((unsigned long)offset+
                                                (unsigned long)base);
                source_slots++;
                i++;
                offset+=0x320;
            } while (i<4);
        } else if (layout==2) {
            ReplayLength=900;
            /* ASM_BARRIER: republish zero after this branch's metadata store;
             * the previous fixed-register index value is dead at this point. */
            __asm__("" : : : "$5");
            i=0;
            factor=901;
            bankbase=SharedWorkBuffer;
            bank_source_slots=CarSnapshots;
            bank_offset=0x3850;
            bank_slots=ReplayInputs;
            do {
                register u32 scaled=(u32)i*4U;
                u32 product;
                product=factor*scaled;
                *bank_slots=(u8 *)((unsigned long)product+(unsigned long)bankbase);
                *bank_source_slots=(RrBankRecord320 *)((unsigned long)bank_offset+
                                                     (unsigned long)bankbase);
                bank_source_slots++;
                bank_offset+=0x320;
                i++;
                bank_slots++;
            } while (i<4);
        } else {
            loaded=SharedWorkBuffer;
            base=loaded;
            source_slots=CarSnapshots;
            offset=0x3844;
            ReplayLength=3600;
            ReplayInputs[0]=base;
            do {
                *source_slots=(RrBankRecord320 *)((unsigned long)offset+
                                                (unsigned long)base);
                source_slots++;
                i++;
                offset+=0x320;
            } while (i<8);
        }
    }
}
