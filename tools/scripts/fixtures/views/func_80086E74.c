/* CC1_FLAGS: -mno-split-addresses */

#include "rr/channel_voice_choice.h"

#include "rr/spu_voice_cache.h"

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
    selection=SvmCurVagIndex;
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
    tone_selection=SvmCurProgIndex;
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
