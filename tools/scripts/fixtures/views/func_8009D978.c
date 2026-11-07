/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/commands.h"

s32 _cardSectorTx(void) {
    register s32 result;
    /* ASM_BARRIER: Keep the folding accumulator separate from the returned byte. */
    register s32 checksum __asm__("$3");
    if(CardRetriesLeft) {
        /* ASM_BARRIER: Keep the retry-phase comparison after its independent tick read. */
        __asm__("" : : : "$2");
        switch(CardTxCount) {
        case 2: result=0x52;goto done;
        case 5: goto high;
        case 6: goto low;
        default: result=0;goto done;
        }
    }
    /* ASM_BARRIER: Keep the normal-phase comparison after its independent tick read. */
    __asm__("" : : : "$2");
    switch(CardTxCount) {
    case 2: CardChecksum=0;result=0x57;goto done;
    case 5:
high: result=((u8 *)&CardCmdSector)[1];goto done;
    case 6:
low: result=*(u8 *)&CardCmdSector;goto done;
    case 133: result=CardCmdValue.value;checksum=CardChecksum^result;goto fold;
    case 134: result=CardChecksum;goto done;
    case 135: result=((CardCmdSector>>8)^CardCmdSector)&255;goto done;
    default:
        if(CardTxCount<7) {result=0;goto done;}
        /* ASM_BARRIER: Preserve the separate fresh tick read for the upper bound. */
        __asm__("" : : : "memory");
        if(CardTxCount>=133) {result=0;goto done;}
        /* ASM_BARRIER: Reload the selected tick after both bounds have passed. */
        __asm__("" : : : "memory");
        {
            s32 phase=CardTxCount;
            checksum=CardChecksum;
            result=CardRxTrace[phase+3];
            checksum^=result;
        }
fold:
        CardChecksum=checksum;
    }
done:
    /* ASM_BARRIER: Retain the original shared return entry for all protocol phases. */
    __asm__("" : "+r"(result));
    return result;
}
