#ifndef RR_SEQUENCE_QUEUE_H
#define RR_SEQUENCE_QUEUE_H

#include "rr/layout.h"

/* Queue substate inside the existing 800F4DB8 object. Earlier fields are opaque. */
typedef struct {
    u8 pad00[0x19D0];
    s32 active19D0, count19D4;
    s32 timer19D8, timer19DC, mode19E0;
    s16 x19E4, y19E6;
    u16 items19E8[8];
} RrSequenceQueueState;

extern RrSequenceQueueState SoundState;
/* Existing SoundSequenceTable..PanGainTable storage:25 two-pointer rows. */
enum {RR_SEQUENCE_TABLE_COUNT=25};
extern s16 *SoundSequenceTable[RR_SEQUENCE_TABLE_COUNT][2];
extern s32 SoundSequenceSet;

/* The table id must be valid. Each selected list ends at its first nonpositive
 * halfword, or is the pointer sentinel -1. Reset/initialization requires at most
 * seven positive entries; append requires current count in [0,7] and rejects a
 * total >= 8. A success returns object base + 2*new_count, as in the original. */
void *SoundQueueEnqueue(s32 id, s32 restart);

RR_ASSERT_SIZE(RrSequenceQueueState, 0x19F8);
RR_ASSERT_OFFSET(RrSequenceQueueState, active19D0, 0x19D0);
RR_ASSERT_OFFSET(RrSequenceQueueState, count19D4, 0x19D4);
RR_ASSERT_OFFSET(RrSequenceQueueState, items19E8, 0x19E8);

#endif
