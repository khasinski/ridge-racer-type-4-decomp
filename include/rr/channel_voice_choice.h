#ifndef RR_CHANNEL_VOICE_CHOICE_H
#define RR_CHANNEL_VOICE_CHOICE_H
#include "rr/channels.h"

extern u8 SvmCurTonePriority;
/* Active count is 0..24, as established by its two writers. Reserved voices
 * are skipped. A free row wins immediately; otherwise priority, gate and age
 * comparisons retain the original mixed signed16/unsigned16 conversions.
 * The result is the chosen low byte: the active count when no candidate was
 * accepted, or the retained sentinel 99 for an unresolved tie. On success,
 * unreserved ages increment modulo16 bits and the selected row is reset. */
s32 _SsVmAlloc(void);

RR_ASSERT_OFFSET(RrChannelControlRow36, age02, 2);
RR_ASSERT_OFFSET(RrChannelControlRow36, priority1A, 0x1A);
#endif
