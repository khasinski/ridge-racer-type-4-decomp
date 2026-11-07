/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/spu.h"

s32 _SsVmVSetUp(s32 bank,s32 program) {
    /* ASM_BARRIER: Retain full selectors until publishing their low bytes. */
    register s32 raw_bank __asm__("$7")=bank;
    /* ASM_BARRIER: Retain full selectors until publishing their low bytes. */
    register s32 raw_program __asm__("$8")=program;
    /* ASM_BARRIER: Retain the narrowed bank in the consumed argument register. */
    register s32 i;
    /* ASM_BARRIER: Retain the narrowed program while publishing pointer globals. */
    register s32 j;
    /* ASM_BARRIER: Retain the selected bank-data pointer. */
    register u8 *data;
    /* ASM_BARRIER: Retain the selected tone-data pointer. */
    register u8 *tones;
    RrSpuProgram *programs;
    /* ASM_BARRIER: Reuse the consumed bank register for selection state. */
    register RrSpuSelectionPrefix *state;
    unsigned long offset;
    /* ASM_BARRIER: Retain the common return value before the epilogue. */
    register s32 result __asm__("$2");
    if((u16)raw_bank>=16)goto fail;
    {
        /* ASM_BARRIER: Sign-extend the bank through the original temporary. */
        register s32 shifted=(u32)bank<<16;
        i=shifted>>16;
    }
    /* ASM_BARRIER: Finish narrowing the bank before forming its flag-page address. */
    __asm__("" : "+r"(i) : : "$3");
    {
        /* ASM_BARRIER: Keep the indexed flag address in its original temporary. */
        register unsigned long page __asm__("$3")=0x80110000U;
        /* ASM_BARRIER: Expose the actual mapped PAL table page to the indexed load. */
        __asm__("" : "+r"(page));
        page=page+(u32)i;
        if(*(u8 *)(page+0x120)!=1) {result=-1;goto out;}
    }
    j=(s16)program;
    if(j<kMaxPrograms)goto publish;
fail:
    result=-1;
    goto out;
publish:
    offset=i*sizeof(u8 *);
    /* ASM_BARRIER: Prepare the native-width table offset before the first table page. */
    __asm__("" : "+r"(offset) : : "$3");
    {
        /* ASM_BARRIER: Load bank data before preparing the program-table address. */
        register u8 **page __asm__("$3")=(u8 **)0x80110000U;
        /* ASM_BARRIER: Expose the actual mapped PAL table page to the indexed load. */
        __asm__("" : "+r"(page));
        data=*(u8 **)((u8 *)page+offset+0x60);
        /* ASM_BARRIER: Capture bank data before loading the second table pointer. */
        __asm__("" : "+r"(data) : : "$5");
    }
    {
        /* ASM_BARRIER: Keep the program-table address in the original argument register. */
        register RrSpuProgram **page __asm__("$5")=(RrSpuProgram **)0x80110000U;
        /* ASM_BARRIER: Expose the actual mapped PAL table page to the indexed load. */
        __asm__("" : "+r"(page));
        programs=*(RrSpuProgram **)((u8 *)page+offset+0x20);
    }
    tones=*(u8 **)((u8 *)_svm_vab_tn+offset);
    state=&SvmCurVabId;
    state->bank=raw_bank;
    state->program=raw_program;
    _svm_tn=tones;
    /* ASM_BARRIER: Publish tone data before the other selected pointers. */
    __asm__("" : : : "memory");
    _svm_vh=data;
    _svm_pg=programs;
    state->value=programs[j].value08;
    result=0;
out:
    /* ASM_BARRIER: Materialize the return value before the shared return instruction. */
    __asm__("" : : "r"(result));
    return result;
}
