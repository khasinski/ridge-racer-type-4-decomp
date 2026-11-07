#include "rr/cd_read_request.h"
#include "rr/cd.h"

extern s32 VSync(s32);
extern u8 CdStatus(void);
extern s32 CdMode(void);
extern u8 *CdLastPos(void);
extern void CdFlush(void);
extern RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback);
extern RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback);
extern RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback);
extern s32 CdPosToInt(void *);
extern s32 PsyqPuts(const char *, ...);
extern const char StrCdReadShellOpen[], StrCdReadRetry[];
extern volatile u32 DsSavedCdMode;
extern void cb_read(s32, u8 *);
extern void cb_data(void);

s32 cd_read_retry(s32 restart) {
    u8 mode_byte;
    register u32 requested;
    register RrLibReadRequestState *state;
    /* ASM_BARRIER: keep the captured mode address in s1 and preserve the raw
     * restart argument in s2 through the command and status helpers. */
    register volatile u32 *mode_slot __asm__("$17");
    register s32 retry __asm__("$18") = restart;
    register volatile RrLibReadRequestState *shell_state;
    register volatile RrLibReadRequestState *failed_state;
    u32 timestamp;
    s32 completed;
    register RrLibCompletionCallback callback;
    /* ASM_BARRIER: a0/a1 stage independent command/null-parameter values;
     * neither bound value is used across a call that can overwrite it. */
    register s32 command __asm__("$4");
    register const u8 *parameters __asm__("$5");

    CdSyncCallback(0);
    CdReadyCallback(0);
    if (CD_read_dma_mode & 1) {
        CdDataCallback(0);
    }
    if (CdStatus() & 0x10) {
        if (!(VSync(-1) & 63)) {
            PsyqPuts(StrCdReadShellOpen);
        }
        CdControlF(1, 0);
        timestamp = VSync(-1);
        shell_state = &CdReadState;
        shell_state->start1C = timestamp;
        shell_state->remaining14 = -1;
        return shell_state->remaining14;
    }
    if (retry != 0) {
        /* ASM_BARRIER: keep retry-only diagnostic address formation in this
         * branch instead of copying it into its predecessor's delay slot. */
        __asm__("" : );
        PsyqPuts(StrCdReadRetry);
        command = 9;
        parameters = 0;
        /* ASM_BARRIER: stage the null parameter after the command value. */
        __asm__("" : "+r"(parameters) : "r"(command));
        CdControl(command, parameters, 0);
        if (!CdControl(2, CdLastPos(), 0)) {
            goto failed;
        }
    }
    CdFlush();
    /* ASM_BARRIER: form the captured mode address after reset returns. */
    __asm__("" : : : "$17");
    mode_slot = &DsSavedCdMode;
    /* ASM_BARRIER: preserve the complete address before the mode load. */
    __asm__("" : "+r"(mode_slot));
    requested = *mode_slot;
    mode_byte = requested;
    requested &= 255;
    if (requested != CdMode() || retry != 0) {
        if (!CdControl(14, &mode_byte, 0)) {
failed:
            /* ASM_BARRIER: retain one shared failure block with fresh state
             * publication and reload, rather than duplicating address setup. */
            __asm__("" : );
            failed_state = &CdReadState;
            failed_state->remaining14 = -1;
            return failed_state->remaining14;
        }
    }
    completed = CdPosToInt(CdLastPos());
    /* ASM_BARRIER: the success-state address is formed after decoding. */
    __asm__("" : : : "$16");
    callback = (RrLibCompletionCallback)cb_read;
    state = &CdReadState;
    /* ASM_BARRIER: stage the callback address before the state address, then
     * publish the decoded sector in the callback setter's delay slot. */
    __asm__("" : "+r"(state) : "r"(callback));
    state->completed20 = completed;
    CdReadyCallback(callback);
    command = 6;
    if (CD_read_dma_mode & 1) {
        CdDataCallback(cb_data);
        command = 6;
    }
    state->position08 = state->buffer04;
    CdControlF(command, 0);
    state->remaining14 = state->sectors00;
    ((volatile RrLibReadRequestState *)state)->retry18 = VSync(-1);
    return ((volatile RrLibReadRequestState *)state)->remaining14;
}
