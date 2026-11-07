/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channel_reset.h"
s32 SsUtKeyOff(s32 voice,s32 bank,s32 program,s32 instrument,s32 group_input) {
    s32 offset, value, locked=_snd_ev_flag;
    /* ASM_BARRIER: keep the fifth raw argument in t0 until its signed conversion. */
    register s32 group __asm__("$8")=group_input;
    s32 program_shift;
    s32 instrument_shift;
    /* ASM_BARRIER: stage the signed group value in the comparison register. */
    register s32 group_shift __asm__("$2");
    /* ASM_BARRIER: use the assembler temporary only after both indexed stores. */
    register u8 *lock_page __asm__("$1");
    /* ASM_BARRIER: retain successful return zero through the final row stores. */
    register s32 result __asm__("$2");
    /* ASM_BARRIER: preserve incoming a0 until the first row-index calculation. */
    register s32 incoming __asm__("$4")=voice;
    s32 marker;
    /* ASM_BARRIER: sign-extend the initial voice in v1 before its row offset. */
    register s32 initial_voice __asm__("$3");
    /* ASM_BARRIER: reuse v1 for each field page only after its previous read dies. */
    register u8 *page __asm__("$3");
    if(locked!=1) {
        _snd_ev_flag=1;
        if((u16)voice<24) {
            initial_voice=(s16)incoming;
            offset=initial_voice*sizeof(RrChannelControlRow36);
            /* ASM_BARRIER: consume the row offset before materializing its page. */
            __asm__("" : : "r"(offset) : "$3");
            value=({
                page=(u8*)0x80110000U;
                /* ASM_BARRIER: retain the actual PAL field page for indexed access. */
                __asm__("" : "+r"(page));
                *(s16*)(page+offset-0x6E0);
            });
            if(value!=(s16)bank)goto reset;
            program_shift=(u32)program<<16;
            /* ASM_BARRIER: stage program sign-extension before its field page. */
            __asm__("" : : "r"(program_shift) : "$3");
            value=({
                page=(u8*)0x80110000U;
                /* ASM_BARRIER: retain the actual PAL field page for indexed access. */
                __asm__("" : "+r"(page));
                *(s16*)(page+offset-0x6E4);
            });
            if(value!=(program_shift>>16))goto reset;
            instrument_shift=(u32)instrument<<16;
            /* ASM_BARRIER: stage instrument sign-extension before its field page. */
            __asm__("" : : "r"(instrument_shift) : "$3");
            value=({
                page=(u8*)0x80110000U;
                /* ASM_BARRIER: retain the actual PAL field page for indexed access. */
                __asm__("" : "+r"(page));
                *(s16*)(page+offset-0x6E2);
            });
            if(value!=(instrument_shift>>16))goto reset;
            group_shift=(u32)group<<16;
            value=({
                page=(u8*)0x80110000U;
                /* ASM_BARRIER: retain the actual PAL field page for indexed access. */
                __asm__("" : "+r"(page));
                *(s16*)(page+offset-0x6EA);
            });
            if(value!=(group_shift>>16))goto reset;
            marker=255;
            /* ASM_BARRIER: prepare kind255 before replacing its comparison page. */
            __asm__("" : : "r"(marker) : "$3");
            if(({
                page=(u8*)0x80110000U;
                /* ASM_BARRIER: retain the actual PAL field page for indexed access. */
                __asm__("" : "+r"(page));
                *(s16*)(page+offset-0x6F8);
            })==marker) {
                vmNoiseOff((u8)voice);
            }else{
reset:
                SvmVoiceIndex=voice;
                _SsVmKeyOffNow(0);
            }
            result=0;
            *(s16*)(_svm_voice[(s16)voice].pad1E+12)=0;
            *(s16*)_svm_voice[(s16)voice].pad1E=0;
            /* ASM_BARRIER: finish both row stores before claiming at for unlock. */
            __asm__("" : : : "$1","memory");
            lock_page=(u8*)0x80110000U;
            /* ASM_BARRIER: retain the real PAL page for the unlock displacement. */
            __asm__("" : "+r"(lock_page));
            *(s32*)(lock_page-0xFA4)=0;
            return result;
        }
        _snd_ev_flag=0;
    }
    return -1;
}
