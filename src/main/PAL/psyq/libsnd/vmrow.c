/* GCC_VERSION: 2.7.2 */
#include "rr/channels.h"
#include "psyq/libsnd/vmrow.h"
#include "game/data/globals.h"

extern u8 SvmVoicePitch[];

void vmNoiseOff(u8 index) {
    s32 offset = index * 54;
    /* ASM_BARRIER: retain the original assembler temporary for the final field store. */
    register u8 *base __asm__("$1");
    ((RrChannelControlRow36 *)((u8 *)_svm_voice + offset))->flags1D = 0;
    ((RrChannelControlRow36 *)((u8 *)_svm_voice + offset))->value00 = 0;
    /* ASM_BARRIER: finish the first two field stores before binding the last address. */
    __asm__ volatile("" : : "r"(offset) : "$1", "memory");
    /* PAL RAM mapping: _svm_voice is base minus 0x6F8. */
    base = (u8 *)0x80110000U;
    /* ASM_BARRIER: expose the indexed field store in the return delay slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base += offset;
    ((RrChannelControlRow36 *)(base - 0x6F8))->value04 = 0;
}

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
    SvmVoiceKeyStatus[offset] = 0;
    {
        u32 low = _svm_okof1;
        u32 high = _svm_okof2;
        *(u16 *)(SvmVoicePitch + offset) = 0;
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
