/* GCC_VERSION: 2.7.2 */

#include "rr/channels.h"

s32 _SsVmGetSeqLVol(s32 input) {
    s32 channel = input;
    u32 offset = (channel & 0xFF) * sizeof(RrChannelValueRecordB0 *);
    /* ASM_BARRIER: preserve the indexed table-address computation in v1. */
    register RrChannelValueRecordB0 **table __asm__("$3");
    /* ASM_BARRIER: reuse v1 for the loaded channel-record pointer. */
    register RrChannelValueRecordB0 *record __asm__("$3");
    s32 group;
    /* ASM_BARRIER: form the pointer-table offset before materializing its base. */
    __asm__ volatile("" : : "r"(offset) : "$3");
    /* PAL table mapping: _ss_score is 0xF98 bytes below this page. */
    table = (RrChannelValueRecordB0 **)0x80110000U;
    /* ASM_BARRIER: preserve the explicit table base through the indexed load. */
    __asm__("" : "=r"(table) : "0"(table));
    table += (s32)(offset / sizeof(*table));
    record = table[-0xF98 / 4];
    SvmLastChannel = channel;
    /* ASM_BARRIER: record the channel id before decoding its row number. */
    __asm__ volatile("" : "=r"(channel) : "0"(channel) : "memory");
    group = (channel & 0xFF00) >> 8;
    record += group;
    return (s16)record->value58;
}
