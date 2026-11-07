/* GCC_VERSION: 2.7.2 */
#include "rr/interrupts.h"
#include "psyq/libapi/pad1.h"
#include "psyq/libapi/pad2.h"
#include "rr/spu_tick_control.h"

extern int SysDeqIntRP();
extern volatile short *JOY_DATA_Pad2;

int RemovePatchPad();

int RemovePatchPad() { EnterCriticalSection(); SysDeqIntRP(1, &PadVsyncIntRP); ExitCriticalSection(); return 1; }

int _padVsyncHookHandler(void) {
    volatile int i; int pad[2];
    JOY_DATA_Pad2[5] = 0;
    for (i = 10; --i != -1; ) ;
    return 0;
}

s32 _padVsyncHookCheck(void) {
    RrInterruptRegisters *registers=I_STAT_PadVsync;
    /* ASM_BARRIER: Keep the predicate result in the original return register. */
    register s32 result;
    if(registers->mask&1) {
        if(registers->pending&1) result=1;
        else {
            result=0;
            /* ASM_BARRIER: Preserve the conditional false-result block instead of folding the predicate. */
            __asm__ volatile("" : : "r"(result));
        }
    } else result=0;
    return result;
}
