/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define StLastReadyLoc (*(char (*)[])&StLastReadyLoc)

#undef StLastReadyLoc

extern s32 CdReadyCallback();

extern s32 CdDataCallback();

extern s32 CdAltReadyCallback();

extern s32 CdAltDataCallback();

extern s32 EnterCriticalSection();

extern s32 ExitCriticalSection();

extern s32 StUseAltCallbacks;

extern s8 *StCdReg0;

extern s8 *CD_REG3_RING;

void StUnSetRing(void) {
    EnterCriticalSection();
    if (StUseAltCallbacks == 1) {
        CdAltDataCallback(0);
        CdAltReadyCallback(0);
    } else {
        CdDataCallback(0);
        CdReadyCallback(0);
    }
    *StCdReg0 = 0;
    {
        s8 *port = CD_REG3_RING;
        /* ASM_BARRIER: preserve the pointer's load delay before the final call. */
        __asm__ volatile("nop" : : "r"(port));
        *port = 0;
    }
    ExitCriticalSection();
}
