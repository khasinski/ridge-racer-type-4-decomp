/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/library.h"
#include "rr/audio_cd_controller.h"

extern int DmaChannelRegsBase;
extern char DmaBusErrorFmt[];
extern char DmaMadrDebugFmt[];

RrLibInterruptCallback setIntrDMA(s32 channel,RrLibInterruptCallback callback);
void DMA_memclr(RrLibInterruptCallback *callbacks, s32 count);

RrLibInterruptExchange startIntrDMA(void) {
    DMA_memclr(InterruptCallbackTable, 8);
    /* The ordinary write view permits the original call-delay store. */
    *(u32 *)DICR_VSync = 0;
    InterruptCallback(3, trapIntrDMA);
    return setIntrDMA;
}

void trapIntrDMA(void) {
    unsigned int s1;
    int i;
    RrLibInterruptCallback *bp;
    unsigned int one, m;
    s1 = (*DICR_VSync >> 24) & 0x7F;
    if (s1 != 0) {
        one = 1;
        m = 0xFFFFFF;
        bp = InterruptCallbackTable;
        do {
            i = 0;
            if (s1 != 0) {
                for (; s1 != 0; i++, s1 >>= 1) {
                    if (i >= 7) break;
                    if (s1 & 1) {
                        *DICR_VSync &= (one << (i + 24)) | m;
                        if (bp[i] != 0) bp[i]();
                    }
                }
            }
            s1 = (*DICR_VSync >> 24) & 0x7F;
        } while (s1 != 0);
    }
    if ((*DICR_VSync & 0xFF000000) == 0x80000000 || (*DICR_VSync & 0x8000)) {
        DebugPrintStub(DmaBusErrorFmt, *DICR_VSync);
        i = 0;
        do {
            DebugPrintStub(DmaMadrDebugFmt, i, *(int *)(i * 16 + DmaChannelRegsBase));
            i++;
        } while (i < 7);
    }
}

RrLibInterruptCallback setIntrDMA(s32 channel,RrLibInterruptCallback callback) {
    s32 i=channel;
    RrLibInterruptCallback *entry=InterruptCallbackTable;
    /* ASM_BARRIER: Retain the previous callback for the common return. */
    register RrLibInterruptCallback old __asm__("$7");
    RrLibInterruptCallback value;
    /* ASM_BARRIER: Keep the device word base in the consumed argument register. */
    register volatile u32 *control __asm__("$5");
    RrLibInterruptCallback result;
    entry=(RrLibInterruptCallback *)((unsigned long)entry+(u32)i*sizeof(*entry));
    old=*entry;
    value=callback;
    result=old;
    if(value!=old) {
        u32 mask;
        mask=0xFF0000;
        if(value) {
            u32 word,bit;
            s32 shift;
            control=DICR_VSync;
            mask|=65535;
            *entry=value;
            word=*control;
            shift=i+16;
            word&=mask;
            bit=(1U<<shift)|0x800000;
            *control=word|bit;
        } else {
            u32 word;
            s32 shift;
            control=DICR_VSync;
            mask|=65535;
            *entry=0;
            word=*control;
            shift=i+16;
            word&=mask;
            word|=0x800000;
            *control=word&~(1U<<shift);
        }
        result=old;
        /* ASM_BARRIER: Keep the changed-callback return separate from the equality gate. */
        __asm__("" : "+r"(result));
    }
    return result;
}

void DMA_memclr(RrLibInterruptCallback *callbacks, s32 count) {
    u32 remaining=(u32)count-1;
    if(count) {
        do {
            *callbacks=0;
            remaining--;
            callbacks++;
        } while(remaining!=0xFFFFFFFFU);
    }
}
