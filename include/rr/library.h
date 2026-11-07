#ifndef RR_LIBRARY_H
#define RR_LIBRARY_H

#include "rr/layout.h"
#include "rr/spu.h"

/* Partial library layouts. Names describe observed sizes, not identified SDK types.
 * Keep the byte-only and halfword views separate: their alignment differs. */

typedef struct {
    char b[16];
    u16 h10, h12;
    char r[16];
} RrLibBlock24Fields;

typedef struct {
    char b[36];
} RrLibBlock24;

typedef struct {
    u8 *c0;
    s32 c4;
    u8 *c8; /* Saved loop cursor, restored by controller 30. */
    s32 cC;
    u8 *p10; /* Stream marker checked before repeat/reset dispatch. */
    u8 b14, b15, b16;
    u8 b17, b18, b19, b1a, b1b, b1c, b1d, b1e, b1f;
    /* b20 is compared as signed; b21 increments modulo 256. */
    s8 b20;
    u8 b21;
    s8 b22[2];
    char p24[2];
    s8 b26;
    u8 b27[0x10];
    u8 b37[0x10];
    char p47[1];
    s16 h48[8];
    u16 h58[4];
    u16 h60[0x10];
    s16 h80;
    char p82[6];
    s32 w88;
    s32 w8c;
    s32 w90;
    s32 w94;
    s32 w98;
    s32 w9c;
    s32 wa0, wa4, wa8, wac;
    char pb0[0];
} RrLibStateB0;

typedef struct {
    u32 value;
} __attribute__((packed)) RrLibPackedWord;

/* Partial layout of the 0x20-byte entries referenced by StRingHeaders. */
typedef struct {
    u16 state;
    u8 pad02[6];
    s32 value08;
    u8 pad0C[4];
    u16 width10,height12;
    u8 pad14[8];
    RrLibPackedWord word1C;
} RrLibQueueRecord32;

extern RrLibQueueRecord32 *StRingHeaders;
void StSetRing(RrLibQueueRecord32 *first, s32 second);
void StSetStream(s32 a, s32 b, s32 c, s32 d, s32 e);
extern s32 StRingReadIndex, StEndFrame, StRingSize;
/* Buffer must contain the configured header table and its 0x7E0-byte payload slots. */
s32 StGetNext(u8 **data, RrLibQueueRecord32 **header);

RR_ASSERT_SIZE(RrLibPackedWord, 4);
RR_ASSERT_SIZE(RrLibQueueRecord32, 0x20);
RR_ASSERT_OFFSET(RrLibQueueRecord32, value08, 8);
RR_ASSERT_OFFSET(RrLibQueueRecord32, width10, 0x10);
RR_ASSERT_OFFSET(RrLibQueueRecord32, height12, 0x12);
RR_ASSERT_OFFSET(RrLibQueueRecord32, word1C, 0x1C);

/* Prefix at D_800ABAC2. Width/height also have assembler aliases
 * GpuVramWidth and GpuVramHeight, used by drawing-area coordinate helpers. */
typedef struct {
    u8 level,mode;
    s16 width,height;
} RrGpuDebugSettings;
extern RrGpuDebugSettings D_800ABAC2;
extern s32 (*GPU_printf)(const char *,...);
RR_ASSERT_SIZE(RrGpuDebugSettings, 6);
RR_ASSERT_OFFSET(RrGpuDebugSettings, width, 2);
RR_ASSERT_OFFSET(RrGpuDebugSettings, height, 4);

/* Observed graphics driver prefix used by DrawOTag dispatch. */
/* PAL addresses are 32 bits; unsigned long also retains native LP64 pointers. */
typedef void (*RrLibDrawOrderingTable)(unsigned long address);
typedef struct {
    u8 reserved00[8];
    s32 (*dispatch)(RrLibDrawOrderingTable, s32, s32, s32);
    u8 reserved0C[12];
    RrLibDrawOrderingTable draw_ordering_table;
} RrLibGraphicsDriverPrefix;

RR_ASSERT_SIZE(RrLibGraphicsDriverPrefix, 0x1C);
RR_ASSERT_OFFSET(RrLibGraphicsDriverPrefix, dispatch, 8);
RR_ASSERT_OFFSET(RrLibGraphicsDriverPrefix, draw_ordering_table, 0x18);

typedef void (*RrLibInterruptCallback)(void);
typedef RrLibInterruptCallback (*RrLibInterruptExchange)(s32, RrLibInterruptCallback);

/* DMA callback table has eight slots (initialization clears all eight).
 * Registration has no bounds check; channel must be in 0..7. */
extern RrLibInterruptCallback InterruptCallbackTable[8];
extern volatile u32 *DICR_VSync;
RrLibInterruptCallback setIntrDMA(s32 channel, RrLibInterruptCallback callback);
RrLibInterruptExchange startIntrDMA(void);
void trapIntrDMA(void);
void DMA_memclr(RrLibInterruptCallback *callbacks, s32 count);

/* Prefix of the callback driver referenced by IntrDriverTablePtr. */
typedef struct {
    RrLibInterruptCallback reserved;
    RrLibInterruptExchange exchange;
    RrLibInterruptExchange install;
    void (*reset)(void);
} RrLibInterruptDriverPrefix;

extern RrLibInterruptDriverPrefix *IntrDriverTablePtr;
extern void ResetCallback(void);
RrLibInterruptCallback DMACallback(s32 channel, RrLibInterruptCallback callback);
RrLibInterruptCallback _SpuDataCallback(RrLibInterruptCallback callback);
RrLibInterruptCallback InterruptCallback(s32 channel, RrLibInterruptCallback callback);

RR_ASSERT_SIZE(RrLibInterruptDriverPrefix, 0x10);
RR_ASSERT_OFFSET(RrLibInterruptDriverPrefix, exchange, 4);
RR_ASSERT_OFFSET(RrLibInterruptDriverPrefix, install, 8);
RR_ASSERT_OFFSET(RrLibInterruptDriverPrefix, reset, 0xC);

typedef void (*RrLibDataCallback)(s32, u8 *);
void CdCommandCallback(void);

typedef void (*RrLibCompletionCallback)(void);
typedef void (*RrLibNotificationCallback)(s32, s32);

extern RrLibDataCallback CD_cbsync, CD_cbready;
extern RrLibNotificationCallback CD_cbread;
extern u32 CD_read_dma_mode;
s32 CdInit(void);
void def_cbsync(s32 selector, u8 *data);
void def_cbready(s32 selector, u8 *data);
void def_cbread(s32 event, s32 argument);

/* Partial state updated by the multi-step library transfer callback. */
typedef struct {
    u8 pad00[8];
    u32 position08;
    u8 pad0C[4];
    u32 step10, remaining14;
    u8 pad18[8];
    u32 completed20, finished24;
    u8 pad28[4];
    RrLibCompletionCallback restore2C, restore30;
    s32 notify_arg34;
} RrLibTransferState38;

RR_ASSERT_SIZE(RrLibTransferState38, 0x38);
RR_ASSERT_OFFSET(RrLibTransferState38, position08, 8);
RR_ASSERT_OFFSET(RrLibTransferState38, step10, 0x10);
RR_ASSERT_OFFSET(RrLibTransferState38, remaining14, 0x14);
RR_ASSERT_OFFSET(RrLibTransferState38, completed20, 0x20);
RR_ASSERT_OFFSET(RrLibTransferState38, finished24, 0x24);
RR_ASSERT_OFFSET(RrLibTransferState38, restore2C, 0x2C);
RR_ASSERT_OFFSET(RrLibTransferState38, restore30, 0x30);
RR_ASSERT_OFFSET(RrLibTransferState38, notify_arg34, 0x34);

/* Named-entry table beginning at D_80110954. A zero key terminates the scan. */
typedef struct {
    s32 key;
    s32 value;
    char name[0x24];
} RrLibNamedEntry;

RR_ASSERT_SIZE(RrLibNamedEntry, 0x2C);
RR_ASSERT_OFFSET(RrLibNamedEntry, name, 8);

/* Prefix of the saved state at D_800A52C8; callback data begins at 0x38. */
typedef struct {
    u16 active;
    s16 dispatching02;
    RrLibInterruptCallback callbacks[11];
    u16 enabled_mask;
    u16 register16;
    s32 register32;
    u8 callback_data[4];
} RrLibSavedState;

/* IRQ selector must be in the callback-table range 0..10. */
RrLibInterruptCallback setIntr(s32 selector, RrLibInterruptCallback callback);

RR_ASSERT_SIZE(RrLibSavedState, 0x3C);
RR_ASSERT_OFFSET(RrLibSavedState, dispatching02, 2);
RR_ASSERT_OFFSET(RrLibSavedState, callbacks, 4);
RR_ASSERT_OFFSET(RrLibSavedState, enabled_mask, 0x30);
RR_ASSERT_OFFSET(RrLibSavedState, register16, 0x32);
RR_ASSERT_OFFSET(RrLibSavedState, register32, 0x34);
RR_ASSERT_OFFSET(RrLibSavedState, callback_data, 0x38);

/* Bank reset requires a low-halfword bank index in the 32-entry table (0..31). */
void _SsClose(s32 bank);
void SsSeqClose(s32 bank);
void SsSepClose(s32 bank);

/* Callback entry points use full argument registers; selectors narrow inside. */
typedef void (*RrLibControllerCallback)(s32 bank, s32 row, s32 value);
typedef void (*RrLibControllerResetCallback)(s32 bank, s32 row);
extern RrLibControllerCallback SsCtrlHandlerCC6, SsCtrlHandlerCC7, SsCtrlHandlerCC10, SsCtrlHandlerCC11;
extern RrLibControllerCallback SsCtrlHandlerCC64, SsCtrlHandlerCC98, SsCtrlHandlerCC99, SsCtrlHandlerCC100;
extern RrLibControllerCallback SsCtrlHandlerCC101, SsCtrlHandlerCC91;
extern RrLibControllerCallback _SsMarkCallback[][16];
extern RrLibControllerResetCallback SsCtrlHandlerCC121;
extern void _SsContMainVol(s32 bank, s32 row, s32 value);
void _SsContPanpot(s32 bank, s32 row, s32 value);
extern void _SsContDamper(s32 bank, s32 row, s32 value);
void _SsContExternal(s32 bank, s32 row, s32 value);
void SsUtSetReverbDepth(s32 left, s32 right);
void _SsContNrpn1(s32 bank, s32 row, s32 value);
void _SsContNrpn2(s32 bank, s32 row, s32 value);
void _SsContExpression(s32 bank, s32 row, s32 value);
extern void _SsContRpn1(s32 bank, s32 row, s32 value);
extern void _SsContRpn2(s32 bank, s32 row, s32 value);
extern void _SsContResetAll(s32 bank, s32 row);
extern void _SsSetPitchBend(s32 bank, s32 row);
extern void _SsSetControlChange(s32 bank, s32 row, s32 control);
extern void _SsSetProgramChange(s32 bank, s32 row, s32 value);
extern s32 _SsReadDeltaValue(s32 bank, s32 row);
extern void _SsVmSetVol(s32 channel, s32 device, s32 slot, u32 pitch, u32 value);

RR_ASSERT_SIZE(RrLibBlock24Fields, 0x24);
RR_ASSERT_OFFSET(RrLibBlock24Fields, h10, 0x10);
RR_ASSERT_OFFSET(RrLibBlock24Fields, h12, 0x12);
RR_ASSERT_SIZE(RrLibBlock24, 0x24);
RR_ASSERT_SIZE(RrLibStateB0, 0xb0);
RR_ASSERT_OFFSET(RrLibStateB0, c8, 8);
RR_ASSERT_OFFSET(RrLibStateB0, p10, 0x10);
RR_ASSERT_OFFSET(RrLibStateB0, b20, 0x20);
RR_ASSERT_OFFSET(RrLibStateB0, b21, 0x21);
RR_ASSERT_OFFSET(RrLibStateB0, h48, 0x48);
RR_ASSERT_OFFSET(RrLibStateB0, w88, 0x88);
RR_ASSERT_OFFSET(RrLibStateB0, w94, 0x94);
RR_ASSERT_OFFSET(RrLibStateB0, w98, 0x98);
RR_ASSERT_OFFSET(RrLibStateB0, wac, 0xac);

s32 SsUtSetReverbType(s32 value);

#endif
