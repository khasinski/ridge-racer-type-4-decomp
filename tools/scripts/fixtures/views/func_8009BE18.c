/* CC1_FLAGS: -mno-split-addresses */

#include "rr_asm.h"

#include "rr/serial.h"

/* legacy typing of shared globals in this part of the unit */
#define CardSio0Base ((S *)CardSio0Base)

#define I_STAT_CardSel ((int *)I_STAT_CardSel)

#define setRC2wait ((int (*)())setRC2wait)

#undef CardSio0Base

#undef I_STAT_CardSel

#undef setRC2wait

s32 _cardXferByte(s32 value) {
    s32 ready, completed, input;
    RrSerialPortRegisters *initial=CardSio0Base;
    RrSerialPortRegisters *port;
    u32 *interrupt;
    {
        /* ASM_BARRIER: Keep the captured byte in the original zero-extended temporary. */
        register s32 byte __asm__("$2")=initial->data;
        /* ASM_BARRIER: Publish the captured byte in the original branch delay slot. */
        register s32 *page __asm__("$1");
        /* ASM_BARRIER: Capture the incoming byte before saving the outgoing value. */
        __asm__("" : : "r"(byte) : "$16");
        input=value;
        /* ASM_BARRIER: Finish the input capture before forming the saved-byte page. */
        __asm__("" : : : "$1");
        page=(s32 *)0x80110000U;
        /* ASM_BARRIER: Expose the real mapped store to the branch-delay scheduler. */
        __asm__("" : "+r"(page));
        page[0x4700/4]=byte;
    }
    if(input<0) {
        while(!(initial->status&1)) {}
        while(!chkRC2wait()) {}
        {
            /* ASM_BARRIER: Form the complemented byte in the original polling delay slot. */
            register u32 complement __asm__("$2")=~input;
            RrSerialPortRegisters *output=CardSio0Base;
            /* ASM_BARRIER: Preserve the original load gap before the shared success jump. */
            RR_NOP_DEP_ON(output);
            /* Ordinary write view retains the original jump-delay store. */
            *(u8 *)&output->data=complement;
        }
        goto success;
    }
    goto irq_test;
irq_wait:
    if(chkRC2wait()) { ready=0;goto irq_done; }
irq_test:
    if(!(*I_STAT_CardSel&0x80)) goto irq_wait;
    ready=1;
irq_done:
    if(!ready) return (u32)-0x200-(u32)CardSioRxByte;
    setRC2wait(5);
    while(!chkRC2wait()) {}
    {
        s32 limit=60;
        RrSerialPortRegisters *output;
        /* ASM_BARRIER: Prepare the next timer limit before loading the fresh output port. */
        __asm__("" : "+r"(limit));
        output=CardSio0Base;
        /* ASM_BARRIER: Retain the original output-port load gap before launching the timer. */
        RR_NOP_DEP_ON(output);
        /* Ordinary write view retains the original call-delay store. */
        *(u8 *)&output->data=input;
        setRC2wait(limit);
    }
    interrupt=I_STAT_CardSel;
    port=CardSio0Base;
    *interrupt=~0x80U;
    if(port->status&0x80) {
        do {
            if(chkRC2wait()) { completed=0;goto transmit_done; }
        } while(CardSio0Base->status&0x80);
    }
    completed=1;
    CardSio0Base->control|=0x10;
transmit_done:
    if(!completed) return (u32)-0x300-(u32)CardSioRxByte;
success:
    return CardSioRxByte;
}
