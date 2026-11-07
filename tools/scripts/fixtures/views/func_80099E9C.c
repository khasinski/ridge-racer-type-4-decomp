/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/channel_advance.h"

#include "rr/channels.h"

void _padAdvanceChannel(s32 command)
{
    /* ASM_BARRIER: retain the incoming command in a1 until first-pass dispatch. */
    register s32 mode __asm__("$5") = command;
    register RrChannelRecordF0 *base = PadChannelRecordArray;
    register s32 skip = -9;
    register s32 *flags = PadChannelPendingPackets;
    register s32 current;
    /* ASM_BARRIER: retain the control pointer in v0 until its halfword store. */
    register u16 *control __asm__("$2");
    register s32 status;
    RrChannelRecordF0 *channel;
    do {
        channel = (RrChannelRecordF0 *)((long)(PadActiveChannel * sizeof(RrChannelRecordF0)) + (unsigned long)base);
        if (mode != skip) {
            if (mode == 0) {
                s32 *slot = (s32 *)((long)(PadActiveChannel * sizeof(s32)) + (unsigned long)flags);
                /* ASM_BARRIER: preserve a literal zero store after command dispatch. */
                __asm__("" : "+r"(mode));
                *slot = 0;
            } else {
                _dirFailAuto((u8 *)channel);
                _padSaveCommand((RrChannelTransferView *)channel);
            }
        }
        control = PadListJoyRegs;
        current = PadActiveChannel;
        PadRecvStage = 0;
        control[5] = 0;
        /* ASM_BARRIER: finish control publication before reading the live bound. */
        __asm__("" : : : "memory");
        {
            register s32 bound = PadLastChannel;
            current++;
            PadActiveChannel = current;
            if (bound >= current) {
                status = _padInitSioMode((RrChannelRecordF0 *)((long)(current * sizeof(RrChannelRecordF0)) + (unsigned long)base));
                mode = 0xFFFF;
            } else {
                status = 1;
                mode = 0xFFFF;
            }
        }
    } while (!status);
}
