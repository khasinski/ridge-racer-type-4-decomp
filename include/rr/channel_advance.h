#ifndef RR_CHANNEL_ADVANCE_H
#define RR_CHANNEL_ADVANCE_H
#include "rr/channels.h"

extern RrChannelRecordF0 PadChannelRecordArray[];
extern s32 PadChannelPendingPackets[];
extern u16 *D_800ABD00;
void _dirFailAuto(u8 *channel);
/* The current index must select accessible record/flag storage and must not
 * overflow when incremented. Handlers may replace the index, bound and control
 * pointer. Mode -9 skips only the first cleanup; mode 0 clears its flag instead.
 * Subsequent passes use 65535, stopping at the bound or a nonzero ready result. */
void _padAdvanceChannel(s32 mode);
#endif
