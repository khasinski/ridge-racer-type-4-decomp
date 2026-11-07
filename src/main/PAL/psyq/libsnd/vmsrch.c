/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channels.h"
int _SsVmKeyOff(s32 a,s32 b,s32 c,u32 group) {
    register s32 maximum=spuVmMaxVoice;
    register u32 index;
    register s32 count;
    /* ASM_BARRIER: Capture the active count before initializing the saved loop index. */
    __asm__("" : "+r"(maximum) : : "$16");
    index=0;
    /* ASM_BARRIER: Initialize the match count independently of the loop index. */
    __asm__("" : "+r"(index));
    count=0;
    if(maximum>0) {
        register u32 one=1;
        register u32 selector=(u16)group;
        register s32 third,first,second;
        /* ASM_BARRIER: Sign-extend each caller word through the original short-lived temporary. */
        register s32 raw __asm__("$2");
        raw=(u32)c<<16;third=raw>>16;
        raw=(u32)a<<16;first=raw>>16;
        raw=(u32)b<<16;second=raw>>16;
        do {
            u32 i=(u8)index;
            u32 offset;
            if(_snd_vmask&(one<<i))goto next;
            {
                register u32 half=i*27;
                offset=half*2;
            }
            {
                /* ASM_BARRIER: Form this independent field address in the original temporary. */
                register u8 *page __asm__("$2");
                /* ASM_BARRIER: Keep the next field page out of the preceding branch delay slot. */
                __asm__("" : : : "$2");
                page=(u8 *)0x80110000U;
                /* ASM_BARRIER: Expose the real PAL control-row page to the field load. */
                __asm__("" : "+r"(page));
                if(((RrChannelControlRow36 *)(page+offset-0x6F8))->group0E!=selector)goto next;
            }
            {
                /* ASM_BARRIER: Form this independent field address in the original temporary. */
                register u8 *page __asm__("$2");
                /* ASM_BARRIER: Keep the next field page out of the preceding branch delay slot. */
                __asm__("" : : : "$2");
                page=(u8 *)0x80110000U;
                /* ASM_BARRIER: Expose the real PAL control-row page to the field load. */
                __asm__("" : "+r"(page));
                if(((RrChannelControlRow36 *)(page+offset-0x6F8))->match14!=third)goto next;
            }
            {
                /* ASM_BARRIER: Form this independent field address in the original temporary. */
                register u8 *page __asm__("$2");
                /* ASM_BARRIER: Keep the next field page out of the preceding branch delay slot. */
                __asm__("" : : : "$2");
                page=(u8 *)0x80110000U;
                /* ASM_BARRIER: Expose the real PAL control-row page to the field load. */
                __asm__("" : "+r"(page));
                if(((RrChannelControlRow36 *)(page+offset-0x6F8))->match10!=first)goto next;
            }
            {
                /* ASM_BARRIER: Form this independent field address in the original temporary. */
                register u8 *page __asm__("$2");
                /* ASM_BARRIER: Keep the next field page out of the preceding branch delay slot. */
                __asm__("" : : : "$2");
                page=(u8 *)0x80110000U;
                /* ASM_BARRIER: Expose the real PAL control-row page to the field load. */
                __asm__("" : "+r"(page));
                if(((RrChannelControlRow36 *)(page+offset-0x6F8))->match18!=second)goto next;
            }
            {
            /* ASM_BARRIER: Prepare the sentinel in the last match-comparison delay slot. */
            register s32 marker __asm__("$2")=255;
            if(((RrChannelControlRow36 *)((u8 *)_svm_voice+offset))->value00==marker) {count++;vmNoiseOff(i);}
            else {SvmVoiceIndex=(u8)index;_SsVmKeyOffNow(0);count++;}
            }
next:;
            index++;
        } while((u8)index<spuVmMaxVoice);
    }
    return count;
}
