#ifndef RR_CHANNELS_H
#define RR_CHANNELS_H

#include "rr/layout.h"

typedef void (*RrChannelUpdateCallback)(void *);

/* Five-byte rows used to select and budget per-command output gates. */
typedef struct {u8 opaque00[2],all_bits02,cost03,opaque04;} RrChannelGateEntry5;

/* Shared channel prefix for readiness and stage dispatch. */
typedef struct RrChannelStatusView {
    u8 pad00[4];
    const RrChannelGateEntry5 *entries04;
    u8 pad08[4];
    s32 reset0C;
    struct RrChannelStatusView *owner10;
    RrChannelUpdateCallback update14;
    u8 pad18[0x24-0x18], argument24, pad25[3];
    u8 *data28;
    u8 *data2C;
    u8 *input30;
    u8 length34,pad35, length36, command37, pad38[4];
    u8 *packet;
    u8 *buffer40;
    u8 pad44[2];
    u8 phase;
    u8 state47, pad48, state49, pad4A[0x57-0x4A], gates57[6], payload5D[6], pad63[0xE4-0x63], argumentE4, padE5;
    u16 pending;
} RrChannelStatusView;

/* Channel records walked by the two-channel reset path. */
typedef struct {
    RrChannelStatusView status;
    u8 responseE8,entry_countE9,padEA[6];
} RrChannelRecordF0;

/* Partial transfer view: pointer fields are native pointers in host fixtures. */
typedef struct {
    u8 pad00[0x28];
    u8 *data28, *data2C;
    u8 pad30[4], length34, pad35, length36, command37, previous38;
    u8 pad39[12], phase45, pad46[14], gates54[9];
} RrChannelTransferView;

/* Accessed payload pointers must cover offsets -3..-1 for phases 0..2. */
u32 _padGetTxByte(const RrChannelTransferView *channel);
void _padSaveCommand(RrChannelTransferView *channel);

RR_ASSERT_SIZE(RrChannelTransferView, 0x60);
RR_ASSERT_OFFSET(RrChannelTransferView, data28, 0x28);
RR_ASSERT_OFFSET(RrChannelTransferView, data2C, 0x2C);
RR_ASSERT_OFFSET(RrChannelTransferView, length34, 0x34);
RR_ASSERT_OFFSET(RrChannelTransferView, command37, 0x37);
RR_ASSERT_OFFSET(RrChannelTransferView, previous38, 0x38);
RR_ASSERT_OFFSET(RrChannelTransferView, phase45, 0x45);
RR_ASSERT_OFFSET(RrChannelTransferView, gates54, 0x54);

/* Partial layout of the 0x36-byte control rows beginning at _svm_voice. */
typedef struct {
    s16 value00;
    u16 age02;
    u16 value04;
    u16 value06;
    u8 pad08[0xE - 8];
    s16 group0E, match10;
    u8 pad12[2];
    s16 match14;
    u8 pad16[2];
    s16 match18;
    s16 priority1A;
    u8 pad1C;
    u8 flags1D;
    u8 pad1E[0x36 - 0x1E];
} RrChannelControlRow36;

/* Partial per-channel record selected through _ss_score (0xB0 stride). */
typedef struct {
    u8 pad00[0x48];
    s16 rampDelta48, rampApplied4A;
    u8 pad4C[0x58 - 0x4C];
    u16 value58, value5A;
    u16 volume5C, volume5E;
    u8 pad60[0x98 - 0x60];
    s32 mode98, rampDuration9C, rampElapsedA0;
    u8 padA4[0xB0 - 0xA4];
} RrChannelValueRecordB0;

extern RrChannelValueRecordB0 *_ss_score[];
extern s16 SvmLastChannel;
extern u16 SvmVoiceIndex;
extern s32 _snd_ev_flag;

/* Reset uses the global selector; the caller-supplied mode is unused. */
extern void _SsVmKeyOffNow(s32 unused_mode);
extern void vmNoiseOff(u8 index);
extern RrChannelControlRow36 _svm_voice[];
extern u32 _snd_vmask;
/* Uses the valid active-channel count established by SsSetReservedVoice. */
s32 _SsVmKeyOff(s32 match10, s32 match18, s32 match14, u32 group0E);
/* Reset all unblocked rows matching the signed low16 match10 value.
 * Uses the active-channel count1..24 established by SsSetReservedVoice. */
void _SsVmSeqKeyOff(s32 group);
extern s32 SsUtKeyOffV(s32 selected);

/* Active channel count, accepted values are 1 through 24 after byte wrapping. */
extern s8 spuVmMaxVoice;
extern s32 SsSetReservedVoice(u32 count);

typedef s32 (*RrChannelStatusCallback)(void *);

/* Command record used by the two-byte channel mode request and its handlers. */
typedef struct {
    u8 pad00[0x14];
    RrChannelUpdateCallback update;
    RrChannelStatusCallback complete;
    u8 pad1C[0x2C-0x1C];
    u8 *data2C;
    u8 pad30[6], length36, command37, pad38[0x46-0x38];
    u8 phase, pad47[0x51-0x47], first, second, equal;
    u8 pad54[0x5D-0x54], payload[6], pad63[0xE4-0x63], argument;
} RrChannelPatchCommand;

typedef void *(*RrChannelRecordCallback)(s32 selector);
extern RrChannelRecordCallback PadPortToChannelPtr;
void PadSetMainMode(s32 selector, s32 first, s32 second);
s32 _padSetMainMode(RrChannelPatchCommand *channel, s32 first, s32 second);
void _padMainModeUpdate(void *channel);
s32 _padMainModeComplete(void *channel);

RR_ASSERT_SIZE(RrChannelPatchCommand, 0xE8);
RR_ASSERT_OFFSET(RrChannelPatchCommand, update, 0x14);
RR_ASSERT_OFFSET(RrChannelPatchCommand, complete, 0x18);
RR_ASSERT_OFFSET(RrChannelPatchCommand, data2C, 0x2C);
RR_ASSERT_OFFSET(RrChannelPatchCommand, length36, 0x36);
RR_ASSERT_OFFSET(RrChannelPatchCommand, command37, 0x37);
RR_ASSERT_OFFSET(RrChannelPatchCommand, phase, 0x46);
RR_ASSERT_OFFSET(RrChannelPatchCommand, first, 0x51);
RR_ASSERT_OFFSET(RrChannelPatchCommand, second, 0x52);
RR_ASSERT_OFFSET(RrChannelPatchCommand, equal, 0x53);
RR_ASSERT_OFFSET(RrChannelPatchCommand, payload, 0x5D);
RR_ASSERT_OFFSET(RrChannelPatchCommand, argument, 0xE4);
extern RrChannelRecordF0 *PadChannelRecords;
extern RrChannelUpdateCallback PadResetChannelFn;
extern s32 PadComActive;
void _padStartCom(void);

typedef void (*RrChannelErrorCallback)(s32 status);
extern RrChannelErrorCallback PadAdvanceChannelFn;
extern s32 D_800ABCD0, PadFirstChannel, PadLastChannel;
extern s32 PadActiveChannel, PadRecvStage, D_800ABCB0;
extern s32 PadPortTimeoutCounts[2];
s32 _padInitSioMode(RrChannelRecordF0 *channel);
/* Entry stage must be in 0..4; the handler sees its incremented value. */
typedef s32 (*RrChannelStageCallback)(RrChannelRecordF0 *channel);
extern RrChannelStageCallback PadRecvStageTable[5];
void _padSioStageStep(RrChannelRecordF0 *channel);
s32 _padIntPad(void);

extern RrChannelStatusCallback PadChannelDispatchPtr, PadChannelBusyFn;
extern RrChannelUpdateCallback PadPacketDoneCallback;

extern s32 _padChannelDispatch(void *channel);
void _padCmdParaMode(void *channel, s32 value);
void _padCmd45(void *channel);
void _padCmd4C(void *channel, s32 value);
void _padCmd46(void *channel, s32 value);
void _padCmd47(void *channel, s32 value);
void _padCmd4B(void *channel);
void _padSendAtLoadInfo(u8 *channel);
extern s32 _padChannelBusy(void *channel);
extern void _padPacketDone(void *channel);

RR_ASSERT_SIZE(RrChannelGateEntry5,5);
RR_ASSERT_OFFSET(RrChannelGateEntry5,all_bits02,2);
RR_ASSERT_OFFSET(RrChannelGateEntry5,cost03,3);
RR_ASSERT_OFFSET(RrChannelStatusView,entries04,4);
RR_ASSERT_OFFSET(RrChannelStatusView,data28,0x28);
RR_ASSERT_OFFSET(RrChannelStatusView,length34,0x34);
RR_ASSERT_OFFSET(RrChannelStatusView,gates57,0x57);
RR_ASSERT_OFFSET(RrChannelRecordF0,entry_countE9,0xE9);
RR_ASSERT_OFFSET(RrChannelStatusView, reset0C, 0x0C);
RR_ASSERT_OFFSET(RrChannelStatusView, owner10, 0x10);
RR_ASSERT_OFFSET(RrChannelStatusView, input30, 0x30);
RR_ASSERT_OFFSET(RrChannelStatusView, buffer40, 0x40);
RR_ASSERT_OFFSET(RrChannelStatusView, payload5D, 0x5D);
RR_ASSERT_SIZE(RrChannelRecordF0, 0xF0);
RR_ASSERT_OFFSET(RrChannelRecordF0, status, 0);
RR_ASSERT_SIZE(RrChannelStatusView, 0xE8);
RR_ASSERT_OFFSET(RrChannelStatusView, update14, 0x14);
RR_ASSERT_OFFSET(RrChannelStatusView, argument24, 0x24);
RR_ASSERT_OFFSET(RrChannelStatusView, data2C, 0x2C);
RR_ASSERT_OFFSET(RrChannelStatusView, length36, 0x36);
RR_ASSERT_OFFSET(RrChannelStatusView, command37, 0x37);
RR_ASSERT_OFFSET(RrChannelStatusView, packet, 0x3C);
RR_ASSERT_OFFSET(RrChannelStatusView, state47, 0x47);
RR_ASSERT_OFFSET(RrChannelStatusView, state49, 0x49);
RR_ASSERT_OFFSET(RrChannelStatusView, argumentE4, 0xE4);
RR_ASSERT_OFFSET(RrChannelRecordF0, responseE8, 0xE8);
RR_ASSERT_OFFSET(RrChannelStatusView, phase, 0x46);
RR_ASSERT_OFFSET(RrChannelStatusView, pending, 0xE6);
RR_ASSERT_SIZE(RrChannelControlRow36, 0x36);
RR_ASSERT_OFFSET(RrChannelControlRow36, value04, 4);
RR_ASSERT_OFFSET(RrChannelControlRow36, group0E, 0xE);
RR_ASSERT_OFFSET(RrChannelControlRow36, match10, 0x10);
RR_ASSERT_OFFSET(RrChannelControlRow36, match14, 0x14);
RR_ASSERT_OFFSET(RrChannelControlRow36, match18, 0x18);
RR_ASSERT_OFFSET(RrChannelControlRow36, flags1D, 0x1D);
RR_ASSERT_SIZE(RrChannelValueRecordB0, 0xB0);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, value58, 0x58);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, value5A, 0x5A);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, mode98, 0x98);

RR_ASSERT_OFFSET(RrChannelValueRecordB0, rampDelta48, 0x48);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, rampApplied4A, 0x4A);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, volume5C, 0x5C);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, volume5E, 0x5E);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, rampDuration9C, 0x9C);
RR_ASSERT_OFFSET(RrChannelValueRecordB0, rampElapsedA0, 0xA0);

#endif
