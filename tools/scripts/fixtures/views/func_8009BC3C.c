/* CC1_FLAGS: -mno-split-addresses */

#include "rr/commands.h"

#include "rr/interrupts.h"

#include "rr/serial.h"

extern s32 SysDeqIntRP(s32,void *);

s32 _cardSioHandler(void) {
    s32 result;
    RrInterruptRegisters *registers;
    /* ASM_BARRIER: Use the original address temporary for stores in call delay slots. */
    register s32 *page __asm__("$1");
    u32 count=(u32)CardTxCount+1;
    /* ASM_BARRIER: Finish the counter read before materializing its mapped page. */
    __asm__("" : : : "$1");
    page=(s32 *)0x800B0000U;
    /* ASM_BARRIER: Expose the real mapped counter store to the delay-slot scheduler. */
    __asm__("" : "+r"(page));
    page[-0x4218/4]=count;
    result=_cardCmdStep();
    registers=I_STAT_Card;
    registers->pending=~0x80U;
    registers->mask|=0x80;
    if(result!=0) {
        /* ASM_BARRIER: Prepare the event argument in the original branch delay slots. */
        register s32 flag __asm__("$4");
        void *event;
        /* ASM_BARRIER: Keep argument preparation after the zero-result gate. */
        __asm__("" : : : "$2", "$4");
        {
            /* ASM_BARRIER: Keep the success comparison separate from the event argument. */
            register s32 one __asm__("$2")=1;
            /* ASM_BARRIER: Retain the explicit success-comparison value before the branch. */
            __asm__("" : "+r"(one));
            flag=1;
            if(result==one) {
                /* ASM_BARRIER: Capture the retry predicate before publishing the success state. */
                register s32 retry __asm__("$3");
                retry=CardRetriesLeft;
                CardCommandStatus=0x41;
                /* ASM_BARRIER: Retain argument preparation on both retry paths. */
                __asm__ volatile("" : "+r"(flag));
                flag=1;
                if(retry!=0) {
                    _cardCmdReset(CardCommandRequest);
                    CardCommandStatus=2;
                    CardRetriesLeft=(u32)CardRetriesLeft-1;
                    /* ASM_BARRIER: Finish retry bookkeeping before preparing the event argument again. */
                    __asm__("" : : : "$4", "memory");
                    flag=1;
                }
                event=D_800ABDC0;
                /* Ordinary write view preserves the original jump-delay store. */
                *(u16 *)&CardSio0Regs->control=0;
            } else {
                event=D_800ABDC0;
                /* Ordinary write view preserves the original jump-delay store. */
                *(u16 *)&CardSio0Regs->control=0;
                CardCommandStatus=0x21;
            }
        }
        page=(s32 *)0x800B0000U;
        /* ASM_BARRIER: Expose the real mapped counter store to the delay-slot scheduler. */
        __asm__("" : "+r"(page));
        page[-0x4218/4]=0;
        SysDeqIntRP(flag,event);
    }
    return 0;
}
