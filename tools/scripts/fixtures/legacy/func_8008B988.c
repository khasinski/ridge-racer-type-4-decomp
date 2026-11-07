/* CC1_FLAGS: -mno-split-addresses */
#include "rr/library.h"

extern RrLibSavedState IntrEnv;
extern u16 *I_MASK;
extern s32 *DPCR_Intr;
extern void HookEntryInt(void *data);
extern void ExitCriticalSection(void);

RrLibSavedState *restartIntr(void) {
    RrLibSavedState *state = &IntrEnv;
    if (state->active != 0) return 0;
    HookEntryInt(state->callback_data);
    state->active = 1;
    *I_MASK = state->register16;
    {
        s32 *destination = DPCR_Intr;
        s32 value = state->register32;
        /* ASM_BARRIER: retain the original load-delay nop before the call/store pair. */
        __asm__ volatile("nop");
        *destination = value;
    }
    ExitCriticalSection();
    return state;
}
