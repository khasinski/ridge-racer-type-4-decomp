#ifndef RR_OVERLAY667_PACKET_H
#define RR_OVERLAY667_PACKET_H
#include "rr/menu_choice.h"
#include "rr/overlay667_events.h"
/* Twelve-byte serial record: four header bytes followed by two four-byte
 * choice records. XOR covers the entire eight-byte payload. */
typedef struct {
    u8 flags00,checksum01,status02,phase03;
    RrMenuChoice4 first04,second08;
} RrOverlay667SerialPacket;
/* Initialization clears aligned words; serial transfer and payload copy use
 * the byte-aligned record view. Both views belong to the same full owner. */
typedef union {
    RrOverlay667SerialPacket packet;
    u32 words[3];
} RrOverlay667SerialStorage;
typedef union {
    RrOverlay667SerialPacket packets[8];
    u32 words[24];
} RrOverlay667SerialRing;
typedef union {
    RrMenuChoice4 choices[2];
    u32 words[2];
} RrOverlay667ChoicePair;
extern RrOverlay667SerialStorage ovl667_D_80118F80,ovl667_D_80118F10;
extern RrOverlay667ChoicePair LinkLocalInputs;
extern u8 ovl667_D_80118F09,ovl667_D_80118EEF;
extern s32 RaceState,ovl667_D_80118EEC;
/* BIOS B0 service35: write count bytes, returning the transferred count. */
s32 FileWrite(s32 descriptor,const void *buffer,s32 count);
s32 ovl667_func_80114D30(void);
/* Eight stored serial records, from 80118F20 through 80118F7F. Ring indices
 * used for payload access must remain in0..7, including after service calls. */
extern RrOverlay667SerialRing ovl667_D_80118F20;
extern RrOverlay667ChoicePair LinkRemoteInputs,D_800ACBF0;
extern s32 ovl667_D_80118ED0,ovl667_D_80118ED4;
extern s32 ovl667_D_80118EC8,ovl667_D_80118EF4;
extern s32 ovl667_D_80118EF8,ovl667_D_80118EFC;
extern s32 ovl667_D_80118F00,ovl667_D_80118F04,ovl667_D_80118F0C;
extern u8 ovl667_D_80118F08,ovl667_D_80118F8D,ovl667_D_80118EED,ovl667_D_80118EEE;
extern u8 ovl667_D_80118F8C;
extern u8 ovl667_D_80118EF0;
void ovl667_func_80114AF8(unsigned long configuration);
/* An event-ready packet's flags byte must be a valid ring index0..7: the
 * original uses the full byte as the index, without stripping control bits.
 * Serial services may mutate globals; every later ring access stays in0..7. */
s32 ovl667_func_80114E34(void);
/* Receive selection and transmit retry state machines. Counter increments wrap
 * modulo2^32. Payloads and state are reread after the serial service returns.
 * The transmitted phase is the low byte of the little-endian EEC word. */
void ovl667_func_80115068(void);
void ovl667_func_801151E0(void);
RR_ASSERT_SIZE(RrOverlay667SerialPacket,12);
RR_ASSERT_OFFSET(RrOverlay667SerialPacket,first04,4);
RR_ASSERT_OFFSET(RrOverlay667SerialPacket,second08,8);
RR_ASSERT_SIZE(RrOverlay667SerialStorage,12);
RR_ASSERT_SIZE(RrOverlay667SerialRing,96);
RR_ASSERT_SIZE(RrOverlay667ChoicePair,8);
#endif
