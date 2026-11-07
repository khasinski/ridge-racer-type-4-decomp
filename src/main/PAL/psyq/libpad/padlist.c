/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/channel_advance.h"
#include "rr/channels.h"

void _padResetChannel();

void _padResetChannel(a0) unsigned char *a0; {
  int i;
  unsigned char *p;
  unsigned char c;
  if (a0[0x49] != 0) {
    p = a0 + 0x5D;
    c = 0xFF;
    i = 5;
    a0[0x49] = 0;
    a0[0x46] = 0;
    *(short *)(a0 + 0xE6) = 0;
    *(int *)(a0 + 0x14) = 0;
    *(int *)(a0 + 0x18) = 0;
    a0[0xE3] = 0;
    a0[0xE4] = 0;
    *(short *)(a0 + 0xE6) = 0;
    a0[0xE9] = 0;
    a0[0xEA] = 0;
    *(int *)a0 = 0;
    *(int *)(a0 + 4) = 0;
    *(int *)(a0 + 8) = 0;
    do {
      *p = c;
      i--;
      p++;
    } while (i >= 0);
  }
}

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

void _padSaveCommand(RrChannelTransferView *channel) {
    u8 command=channel->command37;
    channel->command37=0;
    channel->previous38=command;
}

u32 _padGetTxByte(const RrChannelTransferView *channel) {
    u32 phase=channel->phase45;
    s32 index;
    u32 command=channel->command37;
    const u8 *p;
    u32 result;
    index=(s32)phase-3;
    switch(command) {
    case 0:
        if(index<6 && channel->gates54[index+3]==0) {result=0;goto out;}
        if(index>=channel->length34) {result=0;goto out;}
        p=channel->data28+index;
load:
        result=*p;
        /* ASM_BARRIER: Retain the separate shared-load return for command zero and M. */
        __asm__ volatile("" : : "r"(result) : "memory");
        goto out;
    case 0x4D:
        if(index>=channel->length36) {result=255;goto out;}
        p=channel->data2C+index;
        goto load;
    default:
        if(index>=channel->length36) {result=0;goto out;}
        result=channel->data2C[index];
    }
out:
    return result;
}
