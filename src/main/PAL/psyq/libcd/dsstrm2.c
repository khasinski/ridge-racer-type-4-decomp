/* CC1_FLAGS: -mno-split-addresses */
#include "psyq/libcd/dsstrm23.h"
#include "psyq/libcd/dsmedia2.h"
#include "psyq/libcd/dsstrm2.h"
#include "psyq/libcd/dsstrm32.h"
extern int StCurFrameNo;
extern int StDataReadyFunc;
extern int StRingFullFunc;
void StSetStream(int a, int b, int c, int d, int e) {
    StSetMask(1);
    StRingBase = 0;
    StDataReadyFunc = d;
    StRgb24 = a & 1;
    StExpectedChannel = 0;
    D_801102E0 = 0;
    StNextSectorOffset = 0;
    StCurFrameNo = 0;
    StRingFullFunc = e;
    /* ASM_BARRIER: keeps epilogue stack loads (lw ra/s*) after the final global stores; sched2 otherwise hoists/sinks them */
    __asm__ volatile ("" : : : "memory");
}
