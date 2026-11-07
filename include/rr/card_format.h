#ifndef RR_CARD_FORMAT_H
#define RR_CARD_FORMAT_H

#include "rr/bios_devices.h"

/* Byte-aligned wire views preserve unaligned card-buffer loads/stores. */
typedef struct {u32 value;} __attribute__((packed)) RrCardWireWord;
typedef union {u8 bytes[32];RrCardWireWord words[8];} RrCardDirectoryWire;
typedef union {u8 bytes[4];RrCardWireWord word;} RrCardBrokenWire;
typedef struct {u32 state,size;u16 next;u8 remainder[22];} RrCardDirectoryFields;
typedef union {
    RrCardDirectoryFields fields;
    RrCardDirectoryWire wire;
} RrCardDirectoryRecord;
typedef union {s32 value;RrCardBrokenWire wire;} RrCardBrokenRecord;
typedef union {
    RrCardDirectoryWire directory;
    RrCardBrokenWire broken;
    u8 bytes[128];
} RrCardFormatSector;

extern RrCardDirectoryRecord CardFormatDirRecords[15];
extern RrCardBrokenRecord CardFormatBrokenRecords[20];
extern RrCardFormatSector CardFormatSectorBuf;
/* Initialize directory sectors1..15, broken-sector records16..35 and MC header0.
 * Return1 only if all36 writes return1; stop immediately on any other status.
 * Clear/copy operations occur before each write, so failure preserves that
 * sector's prepared buffer and all prior in-memory changes. Port remains32bit. */
s32 _card_format(s32 port);

typedef char RrCardAddressWordMustHoldPointer[
    sizeof(unsigned long)>=sizeof(void*)?1:-1];
RR_ASSERT_SIZE(RrCardWireWord,4);
RR_ASSERT_SIZE(RrCardDirectoryWire,32);
RR_ASSERT_SIZE(RrCardBrokenWire,4);
RR_ASSERT_SIZE(RrCardDirectoryRecord,32);
RR_ASSERT_OFFSET(RrCardDirectoryFields,size,4);
RR_ASSERT_OFFSET(RrCardDirectoryFields,next,8);
RR_ASSERT_SIZE(RrCardBrokenRecord,4);
RR_ASSERT_SIZE(RrCardFormatSector,128);

#endif
