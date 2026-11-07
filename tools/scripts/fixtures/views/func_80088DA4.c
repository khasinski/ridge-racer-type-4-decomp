/* GCC_VERSION: 2.7.2 */

#include "rr/channels.h"

extern u16 _svm_okof1;

extern u16 _svm_okof2;

extern u16 _svm_okon1;

extern u16 _svm_okon2;

extern u8 D_8010F925[];

extern u8 D_8010F90C[];

/* The selected channel belongs to the valid 0..31 control-row range. */
void _SsVmKeyOffNow(s32 unused_mode) {
    s32 selected = SvmVoiceIndex;
    u32 index = selected & 0xFFFF;
    u32 high_mask, low_mask;
    s32 offset;
    if (index < 16) {
        low_mask = 1U << index;
        high_mask = 0;
    } else {
        low_mask = 0;
        index -= 16;
        high_mask = 1U << index;
        index = selected & 0xFFFF;
    }
    offset = index * sizeof(RrChannelControlRow36);
    D_8010F925[offset] = 0;
    {
        u32 low = _svm_okof1;
        u32 high = _svm_okof2;
        *(u16 *)(D_8010F90C + offset) = 0;
        _svm_voice[index].value00 = 0;
        {
            u32 pending = _svm_okon1;
            low |= low_mask;
            _svm_okof1 = low;
            _svm_okon1 = pending & ~low;
        }
        {
            u32 pending = _svm_okon2;
            high |= high_mask;
            _svm_okof2 = high;
            {
                /* ASM_BARRIER: retain the original assembler temporary for the final mask store. */
                register u16 *base __asm__("$1");
                u32 result = pending & ~high;
                /* ASM_BARRIER: finish mask arithmetic and state updates before materializing its address. */
                __asm__ volatile("" : : "r"(result) : "$1", "memory");
                /* PAL mapping: base[-0x706 / 2] is _svm_okon2. */
                base = (u16 *)0x80110000U;
                /* ASM_BARRIER: expose the final halfword store in the return delay slot. */
                __asm__("" : "=r"(base) : "0"(base));
                base[-0x706 / 2] = result;
            }
        }
    }
}
