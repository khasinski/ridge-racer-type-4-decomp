/* CC1_FLAGS: -mno-split-addresses */

#include "rr/spu_tone_update.h"

s32 SsUtSetVagAtr(s32 a0, s32 a1, s32 a2, const u8 *a3)
{
    /* Byte-address arithmetic retains full native pointers; on PAL unsigned long is 32-bit. */
    int o;
    /* ASM_BARRIER: keep the incoming source in a3 until its saved-register capture. */
    register const u8 *incoming __asm__("$7")=a3;
    s32 bank=a0;
    /* ASM_BARRIER: retain the PAL flag page in the original address register. */
    register u8 *page __asm__("$3");
    unsigned char *s;
    /* ASM_BARRIER: stage the incoming bank before its signed low-halfword conversion. */
    __asm__("" : "+r"(bank) : : "$16");
    bank = (u32)bank << 16;
    bank >>= 16;
    /* ASM_BARRIER: materialize the flag page after the bank value. */
    __asm__("" : : "r"(bank) : "$3");
    page=(u8*)0x80110000U;
    /* ASM_BARRIER: use v1 for the actual PAL flag page and signed bank indexing. */
    __asm__("" : "+r"(page));
    /* ASM_BARRIER: keep the source pointer live through the bank-state read. */
    __asm__("" : "+r"(incoming) : : "memory");
    a3=incoming;
    if (*(u8*)(page+bank+0x120) == 1) {
        /* The selector result is ignored; its current tone pointer remains live. */
        _SsVmVSetUp(bank, (s16)a1);
        o = (s16)((u32)a2 + (u32)(s32)(s8)SvmCurVabId.value * 16U) * 32;
        ((u8 *)(o + (unsigned long)_svm_tn))[0] = a3[0];
        ((u8 *)(o + (unsigned long)_svm_tn))[1] = a3[1];
        ((u8 *)(o + (unsigned long)_svm_tn))[2] = a3[2];
        ((u8 *)(o + (unsigned long)_svm_tn))[3] = a3[3];
        ((u8 *)(o + (unsigned long)_svm_tn))[4] = a3[4];
        ((u8 *)(o + (unsigned long)_svm_tn))[5] = a3[5];
        ((u8 *)(o + (unsigned long)_svm_tn))[7] = a3[7];
        ((u8 *)(o + (unsigned long)_svm_tn))[6] = a3[6];
        ((u8 *)(o + (unsigned long)_svm_tn))[8] = a3[8];
        ((u8 *)(o + (unsigned long)_svm_tn))[9] = a3[9];
        ((u8 *)(o + (unsigned long)_svm_tn))[10] = a3[10];
        ((u8 *)(o + (unsigned long)_svm_tn))[11] = a3[11];
        ((u8 *)(o + (unsigned long)_svm_tn))[12] = a3[12];
        ((u8 *)(o + (unsigned long)_svm_tn))[13] = a3[13];
        s = (u8 *)(o + (unsigned long)_svm_tn);
        *(u16 *)(s + 16) = *(const u16 *)(a3 + 16);
        *(u16 *)(s + 18) = *(const u16 *)(a3 + 18);
        *(u16 *)(s + 20) = *(const u16 *)(a3 + 20);
        *(u16 *)(s + 22) = *(const u16 *)(a3 + 22);
        return 0;
    }
    return -1;
}
