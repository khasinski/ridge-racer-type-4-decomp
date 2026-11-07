#ifndef RR_CD_READ_REQUEST_H
#define RR_CD_READ_REQUEST_H
#include "rr/library.h"

/* Input-side completion fields in the same PAL record as the older partial
 * RrLibTransferState38 view. Buffer and callback slots retain native pointers. */
typedef struct {
    RrLibCompletionCallback saved28;
    RrLibCompletionCallback volatile saved2C;
    RrLibCompletionCallback saved30;
    u8 *notify34; /* Callback result buffer, saved without address narrowing. */
} RrLibReadRestoreTail;
typedef struct {
    s32 sectors00;
    u8 *buffer04;
    u8 *position08;
    volatile u32 mode0C;
    u32 step10;
    s32 remaining14;
    u32 retry18;
    volatile u32 start1C;
    u32 completed20;
    s32 finished24;
    RrLibReadRestoreTail restore;
} RrLibReadRequestState;
extern RrLibReadRequestState CdReadState;
extern volatile s32 CdReadStatePending;
extern RrLibCompletionCallback CdReadSavedSyncCallback;

/* Waits for pending completion with unsigned elapsed ticks (timeout at121),
 * restores its saved callback on timeout, configures the request, and saves
 * displaced callbacks. Returns whether the begin helper reports >0.
 * Buffer and callback values are passed/stored without pointer truncation. */
s32 CdRead(s32 sectors,u8 *buffer,u32 mode);
/* Clears active callbacks, optionally repositions, captures the requested mode,
 * then starts transfer. Shell-open/command failure publishes remaining=-1.
 * Successful return reloads remaining after the clock helper completes. */
s32 cd_read_retry(s32 restart);
RR_ASSERT_SIZE(RrLibReadRestoreTail, 0x10);
RR_ASSERT_OFFSET(RrLibReadRestoreTail, saved2C, 4);
RR_ASSERT_OFFSET(RrLibReadRestoreTail, saved30, 8);
RR_ASSERT_OFFSET(RrLibReadRestoreTail, notify34, 0xC);
RR_ASSERT_SIZE(RrLibReadRequestState, 0x38);
RR_ASSERT_OFFSET(RrLibReadRequestState, buffer04, 4);
RR_ASSERT_OFFSET(RrLibReadRequestState, mode0C, 0xC);
RR_ASSERT_OFFSET(RrLibReadRequestState, step10, 0x10);
RR_ASSERT_OFFSET(RrLibReadRequestState, start1C, 0x1C);
RR_ASSERT_OFFSET(RrLibReadRequestState, finished24, 0x24);
RR_ASSERT_OFFSET(RrLibReadRequestState, restore, 0x28);
#endif
