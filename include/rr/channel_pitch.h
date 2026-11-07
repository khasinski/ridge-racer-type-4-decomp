#ifndef RR_CHANNEL_PITCH_H
#define RR_CHANNEL_PITCH_H
#include "rr/channels.h"
#include "rr/spu.h"

/* Contiguous request at 8011010C: instrument byte, then command/unused/voice.
 * The array view retains halfword addressing relative to offset08. */
typedef struct {
    u8 instrument00, pad01[7];
    s16 fields08[3];
} RrSpuPitchRequest;
extern RrSpuPitchRequest SvmCurTone;
RR_ASSERT_SIZE(RrSpuPitchRequest, 0xE);
RR_ASSERT_OFFSET(RrSpuPitchRequest, fields08, 8);

/* Voice is accepted iff its unsigned low16 is below24. The three selectors
 * compare signed low16 values. Selector status is ignored. Its callback may
 * update the row byte used for the request; pitch calculation may update
 * cache flags, which are read only after storing the returned pitch. */
s32 SsUtChangePitch(s32 voice, s32 bank, s32 program, s32 group,
                 s32 unused, u32 first, u32 second);
#endif
