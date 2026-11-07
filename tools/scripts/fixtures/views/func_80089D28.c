/* GCC_VERSION: 2.7.2 */

#include "rr/channels.h"

s32 _SsVmGetSeqVol(s32 input, u16 *left, u16 *right) {
    s32 channel = input;
    u32 offset = (channel & 0xFF) * sizeof(RrChannelValueRecordB0 *);
    /* ASM_BARRIER: preserve the indexed table-address computation in v1. */
    register RrChannelValueRecordB0 **table __asm__("$3");
    RrChannelValueRecordB0 *record;
    s32 group;
    s16 *last;
    /* ASM_BARRIER: form the pointer-table offset before materializing its base. */
    __asm__ volatile("" : : "r"(offset) : "$3");
    /* PAL table mapping: _ss_score is 0xF98 bytes below this page. */
    table = (RrChannelValueRecordB0 **)0x80110000U;
    /* ASM_BARRIER: preserve the explicit table base through the indexed load. */
    __asm__("" : "=r"(table) : "0"(table));
    table += (s32)(offset / sizeof(*table));
    record = table[-0xF98 / 4];
    last = &SvmLastChannel;
    *last = channel;
    
    group = (channel & 0xFF00) >> 8;
    record += group;
    *left = record->value58;
    *right = record->value5A;
    return *last;
}
