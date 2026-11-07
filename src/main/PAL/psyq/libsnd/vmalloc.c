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
    initial=D_8011010F;
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
        initial=D_8011010F;
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

/* PAL scalar state is addressed from page 0x80110000. Pointer-valued bank
 * and tone slots remain full native pointers in the shared declarations. */
void _SsVmDoAllocate(void) {
    register s32 i=0;
    register u16 *initial=&SvmVoiceIndex;
    register u16 *selected;
    register u32 *mask;
    /* ASM_BARRIER: retain the loop shift unit in A3 independently of row scaling. */
    register u32 one asm("$7");
    register u32 scaled;
    /* ASM_BARRIER: reuse V1 for the selected voice only after each preceding value dies. */
    register s32 voice asm("$3");
    /* ASM_BARRIER: stage the control-row byte offset in V0 before the indexed store. */
    register s32 row_offset asm("$2");
    /* ASM_BARRIER: retain the captured halfword or merged result in V1 until publication. */
    register u32 value3 asm("$3");
    /* ASM_BARRIER: materialize each PAL page in AT after preceding values have been consumed. */
    register u8 *store asm("$1");
    s16 program;
    register s16 *selection;
    register s32 at;
    /* ASM_BARRIER: reuse V0 for the selected value, flag loads and clamp comparison. */
    register u32 value asm("$2");
    register s32 offset;
    /* ASM_BARRIER: retain the byte selection base in A3 through the final voice reload. */
    register s8 *tone_selection asm("$7");
    register RrSpuTone32 *tone;
    /* ASM_BARRIER: retain tone flags in A0 through the signed low16 clamp. */
    register u32 flags asm("$4");
    register s32 low;
    register u16 *base;
    register s32 offset2;
    register u8 *load;
    /* ASM_BARRIER: keep a scheduling boundary before copying the live selector pointer. */
    __asm__("" : :);
    selected=initial;
    /* ASM_BARRIER: capture the selector pointer before initializing the shift unit. */
    __asm__("" : : "r"(selected));
    one=1;
    /* ASM_BARRIER: keep the shift unit independent of the constant row multiplier. */
    __asm__("" : "+r"(one) : "r"(selected));
    voice=*selected;
    mask=_svm_envx_hist;
    scaled=(u32)voice*8;
    voice=(s16)voice;
    row_offset=voice*sizeof(RrChannelControlRow36);
    value3=32767;
    /* ASM_BARRIER: finish row offset and sentinel setup before forming the store page. */
    __asm__("" : :);
    store=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the captured PAL page before adding the destination offset. */
    __asm__("" : "+r"(store));
    store+=row_offset;
    *(u16 *)(store - 0x6F8 + RR_FIELD_OFFSET(RrChannelControlRow36, value06))=value3;
    do {
        i++;
        *mask &= ~(one << (s16)*selected);
        mask++;
    } while (i < 16);
    selection=D_80110116;
    program=selection[0];
    if ((s16)((u16)program & 1) > 0) {
        at=((s32)program-1)/2;
        value=_svm_pg[at].value0C;
        offset=(s32)(scaled<<16);
    } else {
        at=(s32)((u32)(u16)program<<16);
        offset=(s32)(scaled<<16);
        at=((at>>16)-1)/2;
        value=_svm_pg[at].value0E;
    }
    offset>>=15;
    store=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the captured PAL page before adding the destination offset. */
    __asm__("" : "+r"(store));
    store+=offset;
    *(u16 *)(store - 0x1E8 + RR_FIELD_OFFSET(RrSpuVolumeCache, voices) + RR_FIELD_OFFSET(RrSpuCachedVolumeEntry, program06))=value;
    voice=selection[1];
    /* ASM_BARRIER: schedule the fresh signed voice load before forming its flag address. */
    __asm__("" : :);
    load=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the independent flag address in the current temporary. */
    __asm__("" : "+r"(load));
    load+=voice;
    value=load[-0x1E8]|8;
    /* ASM_BARRIER: finish flag computation before reusing the assembler temporary. */
    __asm__("" : : : "$1");
    store=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the captured PAL page before adding the destination offset. */
    __asm__("" : "+r"(store));
    store+=voice;
    store[-0x1E8]=value;
    /* ASM_BARRIER: recapture the voice offset after publishing the first dirty flag. */
    __asm__("" : "+r"(scaled) : : "memory");
    tone_selection=D_80110107;
    offset=(s32)(scaled<<16);
    offset2=offset>>15;
    tone=(RrSpuTone32 *)((unsigned long)(tone_selection[0]*16+tone_selection[5])*32UL+(unsigned long)_svm_tn);
    value3=tone->value10;
    /* ASM_BARRIER: schedule tone value10 before forming the cache destination base. */
    __asm__("" : :);
    base=&_svm_sreg_dirty.voices[0].tone08;
    store=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the captured PAL page before adding the destination offset. */
    __asm__("" : "+r"(store));
    store+=offset2;
    *(u16 *)(store - 0x1E8 + RR_FIELD_OFFSET(RrSpuVolumeCache, voices) + RR_FIELD_OFFSET(RrSpuCachedVolumeEntry, tone08))=value3;
    flags=tone->value12;
    value=_svm_damper+(flags&31);
    low=value;
    /* ASM_BARRIER: retain the full sum separately from its signed low16 comparison. */
    __asm__("" : "+r"(value));
    flags&=0xFFE0;
    if ((s16)value >= 32) low=31;
    value3=low|flags;
    /* ASM_BARRIER: retain the cache base before the final indexed halfword store. */
    __asm__("" : "+r"(base));
    load=(u8 *)((unsigned long)base+(unsigned long)offset2);
    *(u16 *)(load+2)=value3;
    voice=*(s16 *)(tone_selection+17);
    /* ASM_BARRIER: schedule the fresh signed voice load before forming its flag address. */
    __asm__("" : :);
    load=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the independent flag address in the current temporary. */
    __asm__("" : "+r"(load));
    load+=voice;
    value=load[-0x1E8]|0x30;
    /* ASM_BARRIER: finish flag computation before reusing the assembler temporary. */
    __asm__("" : : : "$1");
    store=(u8 *)0x80110000U;
    /* ASM_BARRIER: retain the captured PAL page before adding the destination offset. */
    __asm__("" : "+r"(store));
    store+=voice;
    store[-0x1E8]=value;
}
