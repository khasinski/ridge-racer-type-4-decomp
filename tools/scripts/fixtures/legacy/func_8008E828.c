#include "rr/cd_read_callback.h"
#include "rr/cd.h"

extern s32 VSync(s32);
extern RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback);
extern RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback);
extern RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback);
extern s32 CdGetSector2(u8 *,u32);
extern void CdDataSync(s32);
extern s32 CdPosToInt(void *);
extern s32 PsyqPuts(const char *,...);
extern const char CdReadSectorErrorMsg[];
extern void cb_data(void),cd_read_restore_sync(void);

void cb_read(s32 event,u8 *data) {
    u8 header[12];
    register volatile RrLibReadRequestState *initial=&CdReadState;
    register volatile RrLibReadRequestState *state;
    register volatile RrLibReadRequestState *transfer;
    register volatile RrLibReadRequestState *failed;
    register u32 *completed;
    register volatile RrCdPositionView *position;
    register volatile RrCdPositionView *deferred;
    RrCdDataNotify notify;
    s32 now;
    register u8 *notify_page;

    /* ASM_BARRIER: materialize the initial full address before saving data. */
    __asm__("" : "+r"(initial));
    initial->restore.notify34=data;
    if ((u8)event==1) {
        if (initial->remaining14>0) {
            if (initial->step10==512) {
                /* ASM_BARRIER: retain flags address formation inside this arm. */
                __asm__("" : );
                if (CD_read_dma_mode&1) {
                    CdDataCallback(0);
                    CdGetSector2(header,3);
                    CdDataSync(0);
                    CdDataCallback(cb_data);
                } else {
                    CdGetSector(header,3);
                }
                now=CdPosToInt(header);
                completed=&CdReadExpectedPos;
                /* ASM_BARRIER: capture the complete counter address before load. */
                __asm__("" : "+r"(completed));
                if ((u32)now!=*completed) {
                    PsyqPuts(CdReadSectorErrorMsg);
                    failed=(RrLibReadRequestState *)((u8 *)completed-
                                                     RR_READ_COMPLETED_OWNER_OFFSET);
                    failed->remaining14=-1;
                }
            }
            /* ASM_BARRIER: reload flags at the common transfer join. */
            __asm__("" : );
            if (CD_read_dma_mode&1) {
                /* ASM_BARRIER: keep the deferred address inside its branch. */
                __asm__("" : );
                deferred=&CdReadStatePosition;
                /* ASM_BARRIER: form the complete window before loading arguments. */
                __asm__("" : "+r"(deferred));
                CdGetSector2(deferred->position08,deferred->step10);
            } else {
                /* ASM_BARRIER: keep the immediate address inside its branch. */
                __asm__("" : );
                position=&CdReadStatePosition;
                /* ASM_BARRIER: retain one captured window across the transfer. */
                __asm__("" : "+r"(position));
                CdGetSector(position->position08,position->step10);
                transfer=(RrLibReadRequestState *)((u8 *)position-
                                                   RR_READ_POSITION_OWNER_OFFSET);
                transfer->position08=position->position08+(position->step10<<2);
                transfer->remaining14=(s32)((u32)transfer->remaining14-1U);
                /* The object is ordinary; qualify the load for a fresh value,
                 * then allow its publication in the unconditional jump delay. */
                ((RrLibReadRequestState *)transfer)->completed20=transfer->completed20+1U;
            }
        }
    } else {
        initial->remaining14=-1;
    }
    now=VSync(-1);
    state=&CdReadState;
    /* ASM_BARRIER: form the complete state address after the clock returns. */
    __asm__("" : "+r"(state));
    state->retry18=now;
    if (state->remaining14<0) {
        cd_read_retry(1);
    }
    now=VSync(-1);
    if ((s32)(state->start1C+1200U)<now) {
        state->remaining14=-1;
    }
    if (state->remaining14==0 ||
        (now=VSync(-1),(s32)(state->start1C+1200U)<now)) {
        CdReadyCallback(state->restore.saved2C);
        if (CD_read_dma_mode&1) {
            CdDataCallback(state->restore.saved30);
        }
        CdSyncCallback(cd_read_restore_sync);
        CdControlF(9,0);
        /* PAL page+0x63F0 contains the data notification function pointer. */
        notify_page=(u8 *)0x800A0000U;
        notify=*(RrCdDataNotify *)(notify_page+0x63F0);
        ((RrLibReadRequestState *)state)->finished24=1;
        if (notify) {
            notify(state->remaining14==0?2:5,data);
        }
    }
}
