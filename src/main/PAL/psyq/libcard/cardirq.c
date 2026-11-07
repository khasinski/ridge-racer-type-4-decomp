/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/interrupts.h"
#include "psyq/libcard/cardirq.h"
#include "rr/commands.h"

extern char *CardSio0Regs;
extern int EnterCriticalSection();
extern int ExitCriticalSection();
extern int ChangeClearRCnt();
extern int SysEnqIntRP();
extern int SysDeqIntRP();
extern int EnterCriticalSection();
extern int ChangeClearRCnt();
extern int SysDeqIntRP();
extern int ExitCriticalSection();
extern int CardIrqQueueEntry;
extern int SysEnqIntRP();

int _cardVsyncHandler();

/* legacy typing of shared globals in this part of the unit */
#define CardIrqQueueEntry (*(char (*)[])&CardIrqQueueEntry)

void _cardStartCom(void)
{
    RrInterruptRegisters *q;
    *(short *)(CardSio0Regs + 0xA) = 0x40;
    *(short *)(CardSio0Regs + 0xE) = 0x88;
    *(short *)(CardSio0Regs + 8) = 0xD;
    EnterCriticalSection();
    SysDeqIntRP(2, CardIrqQueueEntry);
    SysEnqIntRP(2, CardIrqQueueEntry);
    q = I_STAT_Card;
    q->pending = ~1U;
    *(u32 *)&q->mask = q->mask | 1;
    ChangeClearRCnt(3, 0);
    ExitCriticalSection();
}

#undef CardIrqQueueEntry

void _cardStopCom() { EnterCriticalSection(); ChangeClearRCnt(3, 1); SysDeqIntRP(2, &CardIrqQueueEntry); ExitCriticalSection(); }

s32 _cardChkVsyncIrq(void) {
    RrInterruptRegisters *registers=I_STAT_Card;
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

s32 _cardChkSioIrq(void) {
    RrInterruptRegisters *registers=I_STAT_Card;
    register s32 result;
    if(registers->mask&0x80) {
        if(registers->pending&0x80) result=1;
        else {
            result=0;
            /* ASM_BARRIER: Preserve the conditional false-result block instead of folding the predicate. */
            __asm__ volatile("" : : "r"(result));
        }
    } else result=0;
    return result;
}

/* legacy typing of shared globals in this part of the unit */
#define I_STAT_Card ((unsigned int *)I_STAT_Card)
#define CardSio0Regs ((short *)CardSio0Regs)
#define SysDeqIntRP ((int (*)())SysDeqIntRP)

int _cardVsyncHandler() {
    volatile unsigned int *g;
    short *h;
    if (CardTxCount) {
        g = I_STAT_Card;
        CardCommandStatus = 0x11;
        CardTxCount = 0;
        g[0] = ~0x80;
        g[1] &= ~0x80;
        h = CardSio0Regs;
        h[5] = 0;
        SysDeqIntRP(1, D_800ABDC0);
        CardSio0Regs[5] = 0x40;
        CardSio0Regs[7] = 0x88;
        CardSio0Regs[4] = 0xD;
        CardSio0Regs[5] = 0;
        return 0;
    }
    if (CardCommandStatus & 1) return 0;
    SysDeqIntRP(1, D_800ABDC0);
    SysEnqIntRP(1, D_800ABDC0);
    _cardSioHandler();
    return 0;
}

#undef I_STAT_Card
#undef CardSio0Regs
#undef SysDeqIntRP
