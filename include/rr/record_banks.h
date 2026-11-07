#ifndef RR_RECORD_BANKS_H
#define RR_RECORD_BANKS_H
#include "rr/layout.h"

/* Complete snapshot payload. The HUD projects signed map coordinates at20/28;
 * bank management also interprets the signed status/count halfword at1E4. */
typedef struct {
    u32 prefix00[8];
    s32 x20;u32 word24;s32 y28;
    u32 reserved2C[110];
    s16 value1E4;
    u16 pad1E6;
    u32 suffix[78];
} RrBankRecord320;
extern s32 ReplayFrame,ReplayState,SavedRaceState,ReplayLength;
extern s16 NumCars;
extern RrBankRecord320 *CarSnapshots[],*CarTable[];
extern u8 SharedWorkBuffer[];
extern u8 *ReplayInputs[];

/* Mode2 restores the live signed count of records; source/destination records
 * must be identical or disjoint, four-byte-aligned, and their slots accessible.
 * Other modes configure layout1(two banks/four records), layout2(four/four),
 * or the default(one/eight). Arena storage must cover0x60E8 bytes, aligned4.
 * Pointer slots retain native pointer widths; arena byte offsets stay fixed. */
void InitReplayBuffers(s32 layout,s32 mode);
RR_ASSERT_SIZE(RrBankRecord320,0x320);
RR_ASSERT_OFFSET(RrBankRecord320,x20,0x20);
RR_ASSERT_OFFSET(RrBankRecord320,y28,0x28);
RR_ASSERT_OFFSET(RrBankRecord320,value1E4,0x1E4);
#endif
