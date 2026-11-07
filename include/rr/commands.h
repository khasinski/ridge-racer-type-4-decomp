#ifndef RR_COMMANDS_H
#define RR_COMMANDS_H

#include "rr/layout.h"

typedef s32 (*RrCommandReadCallback)(void);
typedef s32 (*RrCommandWriteCallback)(s32);

/* Command 2 retains a buffer here; other commands use an integer parameter. */
typedef union {
    s32 value;
    u8 *buffer;
} RrCommandParameter;

/* Prefix beginning at CardCmdBuffer. Callback signatures follow their call sites. */
typedef struct {
    u8 *buffer;
    void *first_result; /* Byte buffer or word result, depending on the command. */
    u32 *second_result;
    s32 parameter24;
    s32 count28;
    RrCommandParameter parameter2C;
    s32 result30;
    RrCommandReadCallback read;
    RrCommandWriteCallback write;
} RrCommandContextPrefix;

extern s32 CardRetriesLeft;
extern u8 CardSioIrqQueueEntry[];
/* Received and transmitted command bytes, indexed by CardRxCount/F4. */
extern u8 CardRxTrace[], CardTxTrace[];
s32 _cardSioHandler(void);
s32 _cardCmdStep(void);
s32 _cardCmdReset(s32 request);

extern s32 CardCommandStatus, CardTxCount, CardCommandRequest, CardReceiveIndex, CardRxCount;
extern u8 *CardCmdBuffer;
extern void *CardCmdResultPtrA;
extern u32 *CardCmdResultPtrB;
extern s32 CardCmdSector, CardVarRxLength, CardBytesRemaining, CardCurrentCommand;
extern RrCommandParameter CardCmdValue;
extern RrCommandReadCallback CardCommandReader;
extern RrCommandWriteCallback CardWriteHandler;

/* Command 11 block transfer and its checksum-producing byte reader. */
extern s32 CardChecksum;
extern u8 CardReadChecksum;
s32 _cardSectorBegin(s32 request, s32 block, s32 mode);
s32 _cardSectorTx(void);
s32 _cardSectorRx(s32 value);

/* Command 101 response: handshake, then two words assembled high byte first. */
s32 _cardGetIdRx(s32 value);

static __inline__ s32 rr_command_begin(s32 request) {
    if (!(CardCommandStatus & 1)) return 0;
    CardReceiveIndex = 0;
    CardRxCount = 0;
    CardBytesRemaining = 0;
    CardTxCount = 0;
    CardCommandRequest = request;
    return 1;
}

RR_ASSERT_SIZE(RrCommandParameter, 4);
RR_ASSERT_SIZE(RrCommandContextPrefix, 0x24);
RR_ASSERT_OFFSET(RrCommandContextPrefix, first_result, 4);
RR_ASSERT_OFFSET(RrCommandContextPrefix, second_result, 8);
RR_ASSERT_OFFSET(RrCommandContextPrefix, parameter24, 0xC);
RR_ASSERT_OFFSET(RrCommandContextPrefix, count28, 0x10);
RR_ASSERT_OFFSET(RrCommandContextPrefix, parameter2C, 0x14);
RR_ASSERT_OFFSET(RrCommandContextPrefix, result30, 0x18);
RR_ASSERT_OFFSET(RrCommandContextPrefix, read, 0x1C);
RR_ASSERT_OFFSET(RrCommandContextPrefix, write, 0x20);

#endif
