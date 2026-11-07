/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/channels.h"

void _SsVmSeqKeyOff(s32 input) {
    register s32 maximum=spuVmMaxVoice;
    register u32 index;
    s32 wanted;
    u32 one;
    s32 next;
    /* ASM_BARRIER: Read the active count before initializing the saved loop counter. */
    __asm__("" : "+r"(maximum) : : "$16");
    index=0;
    /* ASM_BARRIER: Retain a byte-index computation for the initial loop entry. */
    __asm__("" : "+r"(index));
    if(maximum>0) {
        one=1;
        wanted=(s16)input;
        next=(u8)index;
again: {
            s32 i=next;
            if(!(_snd_vmask & (one << i)) && _svm_voice[i].match10==wanted) {
                SvmVoiceIndex=(u8)index;
                _SsVmKeyOffNow(0);
            }
            index++;
        }
        {
            s32 limit=spuVmMaxVoice;
            s32 test;
            /* ASM_BARRIER: Reload the count before the independent byte comparison. */
            __asm__("" : : "r"(limit) : "$2");
            test=(u8)index;
            /* ASM_BARRIER: Separate the comparison byte from the next body's byte index. */
            __asm__("" : "+r"(test));
            next=(u8)index;
            if(test<limit)goto again;
        }
    }
}
