/* CC1_FLAGS: -mno-split-addresses */

#include "rr/channel_voice_choice.h"

#include "rr/spu_voice_cache.h"

s32 _SsVmAlloc(void) {
    register u32 chosen;
    /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
    register u32 best_gate asm("$12");
    register s32 ties;
    /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
    register u32 best_age asm("$8");
    /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
    register u32 best asm("$9");
    register u32 index;
    register s32 priority;
    register s32 limit;
    register u32 reserved;
    register u32 one;
    register s32 offset;
    register s32 signed_priority;
    /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
    register u32 raw_priority asm("$4");
    /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
    register s32 cutoff asm("$5");
    register u32 gate;
    register u32 age;
    register u32 initial;
    register s32 count;
    register u32 ix;
    register s32 tmp;
    register u32 ties_test;
    register s32 loop_test;
    s32 signed_age;
    chosen=99;
    best_gate=65535;
    ties=0;
    /* ASM_BARRIER: initialize independent candidate accumulators before scanning. */
    __asm__("" : "+r"(ties), "+r"(chosen));
    best_age=0;
    best=99;
    /* ASM_BARRIER: retain the independent initial age accumulator. */
    __asm__("" : "+r"(best_age));
    index=0;
    /* ASM_BARRIER: keep the byte loop comparison separate from the next byte index. */
    __asm__("" : "+r"(index));
    initial=SvmCurTonePriority;
    count=spuVmMaxVoice;
    /* ASM_BARRIER: capture the priority byte and active count before narrowing priority. */
    __asm__("" : : : "memory");
    priority=(s8)initial;
    if(count>0) {
        one=1;
        reserved=_snd_vmask;
        limit=count;
        ix=(u8)index;
        do {
            if(!(reserved&(one<<ix))) {
                tmp=ix*(sizeof(RrChannelControlRow36)/2);
                offset=tmp*2;
                if(( {
                    register u8 *page=(u8 *)0x80110000U;
                    /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                    __asm__("" : "+r"(page));
                    page+=( offset );
                    *( s8 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> flags1D );
                }
                ) ==0 && ( {
                    register u8 *page=(u8 *)0x80110000U;
                    /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                    __asm__("" : "+r"(page));
                    page+=( offset );
                    *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> value06 );
                }
                ) ==0) {
                    chosen=index;
                    break;
                }
                offset=(u8)index*sizeof(RrChannelControlRow36);
                cutoff=(u16)priority;
                signed_priority= ( {
                    /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                    register u8 *page asm("$6")=(u8 *)0x80110000U;
                    /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                    __asm__("" : "+r"(page));
                    page+=( offset );
                    *( s16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> priority1A );
                }
                );
                /* ASM_BARRIER: retain the scheduling boundary between field capture and the next comparison. */
                __asm__("" : :);
                raw_priority= ( {
                    /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                    register u8 *page asm("$4")=(u8 *)0x80110000U;
                    /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                    __asm__("" : "+r"(page));
                    page+=( offset );
                    *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> priority1A );
                }
                );
                if(signed_priority<cutoff) {
                    priority=raw_priority;
                    best=index;
                    best_gate= ( {
                        /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                        register u8 *page asm("$12")=(u8 *)0x80110000U;
                        /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                        __asm__("" : "+r"(page));
                        page+=( offset );
                        *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> value06 );
                    }
                    );
                    /* ASM_BARRIER: retain the scheduling boundary between field capture and the next comparison. */
                    __asm__("" : :);
                    best_age= ( {
                        /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                        register u8 *page asm("$8")=(u8 *)0x80110000U;
                        /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                        __asm__("" : "+r"(page));
                        page+=( offset );
                        *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 );
                    }
                    );
                    ties=1;
                }
                else if(signed_priority==cutoff) {
                    raw_priority=(u16)best_gate;
                    gate= ( {
                        /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                        register u8 *page asm("$6")=(u8 *)0x80110000U;
                        /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                        __asm__("" : "+r"(page));
                        page+=( offset );
                        *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> value06 );
                    }
                    );
                    ties++;
                    if((u16)gate<raw_priority) {
                        best_age= ( {
                            /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                            register u8 *page asm("$8")=(u8 *)0x80110000U;
                            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                            __asm__("" : "+r"(page));
                            page+=( offset );
                            *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 );
                        }
                        );
                        best_gate=gate;
                        best=index;
                    }
                    else if((u16)gate==raw_priority) {
                        /* ASM_BARRIER: retain the scheduling boundary between field capture and the next comparison. */
                        __asm__("" : :);
                        signed_age= ( {
                            register u8 *page=(u8 *)0x80110000U;
                            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                            __asm__("" : "+r"(page));
                            page+=( offset );
                            *( s16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 );
                        }
                        );
                        /* ASM_BARRIER: retain the scheduling boundary between field capture and the next comparison. */
                        __asm__("" : :);
                        age= ( {
                            /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                            register u8 *page asm("$1")=(u8 *)0x80110000U;
                            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                            __asm__("" : "+r"(page));
                            page+=( offset );
                            *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 );
                        }
                        );
                        if((s32)best_age<signed_age) {
                            best_age=age;
                            best=index;
                        }
                    }
                }
            }
            index++;
            loop_test=(u8)index;
            /* ASM_BARRIER: keep the byte loop comparison separate from the next byte index. */
            __asm__("" : "+r"(index));
            ix=(u8)index;
        }
        while(loop_test<limit);
    }
    if((u8)chosen==99) {
        ties_test=(u8)ties;
        chosen=best;
        if(!ties_test)chosen=(u8)spuVmMaxVoice;
    }
    count=spuVmMaxVoice;
    if((u8)chosen<count) {
        if(count>0) {
            register u32 one2;
            /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
            register u32 mask asm("$5");
            register s32 count2;
            index=0;
            /* ASM_BARRIER: keep the byte loop comparison separate from the next byte index. */
            __asm__("" : "+r"(index));
            one2=1;
            mask=_snd_vmask;
            count2=count;
            ix=(u8)index;
            do {
                if(!(mask&(one2<<ix))) {
                    /* ASM_BARRIER: retain the captured selection accumulator in its original temporary. */
                    register s32 off asm("$2")=ix*sizeof(RrChannelControlRow36);
                    register u32 v= ( {
                        register u8 *page=(u8 *)0x80110000U;
                        /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                        __asm__("" : "+r"(page));
                        page+=( off );
                        *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 );
                    }
                    );
                    v++;
                    do {
                        /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
                        register u8 *page asm("$1")=(u8 *)0x80110000U;
                        /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
                        __asm__("" : "+r"(page));
                        page+=( off );
                        *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 )=( v );
                    }
                    while(0);
                }
                index++;
                loop_test=(u8)index;
                /* ASM_BARRIER: keep the byte loop comparison separate from the next byte index. */
                __asm__("" : "+r"(index));
                ix=(u8)index;
            }
            while(loop_test<count2);
        }
        offset=(u8)chosen*sizeof(RrChannelControlRow36);
        do {
            /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
            register u8 *page asm("$1")=(u8 *)0x80110000U;
            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
            __asm__("" : "+r"(page));
            page+=( offset );
            *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> age02 )=( 0 );
        }
        while(0);
        initial=SvmCurTonePriority;
        do {
            /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
            register u8 *page asm("$1")=(u8 *)0x80110000U;
            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
            __asm__("" : "+r"(page));
            page+=( offset );
            *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> pad1E[12] )=( 0 );
        }
        while(0);
        do {
            /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
            register u8 *page asm("$1")=(u8 *)0x80110000U;
            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
            __asm__("" : "+r"(page));
            page+=( offset );
            *( u16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> pad1E[0] )=( 0 );
        }
        while(0);
        initial=(s8)initial;
        do {
            /* ASM_BARRIER: retain this field page in its original temporary until the row access. */
            register u8 *page asm("$1")=(u8 *)0x80110000U;
            /* ASM_BARRIER: capture the PAL row page before applying its index and field offset. */
            __asm__("" : "+r"(page));
            page+=( offset );
            *( s16 *)((u8 *)&((RrChannelControlRow36 *)(page-0x6F8))-> priority1A )=( initial );
        }
        while(0);
    }
    return (u8)chosen;
}
