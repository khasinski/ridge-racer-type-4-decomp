/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/commands.h"

s32 _cardSectorRx(s32 input) {
    register s32 original=input;
    /* ASM_BARRIER: Retain the original byte argument and final-marker mask register. */
    register s32 value __asm__("$4");
    /* ASM_BARRIER: Keep error values and shared checksum folding in the ABI result register. */
    register s32 result __asm__("$2");
    /* ASM_BARRIER: Capture the full input before loading the retry flag. */
    __asm__("" : "+r"(original) : : "$2");
    value=original;
    if(CardRetriesLeft) {
        switch(CardRxCount) {
        case 2: {
            /* ASM_BARRIER: Preserve the original byte comparison temporary and branch delay. */
            register u32 byte __asm__("$3")=value&255;
            if(byte==8) {
                CardRetriesLeft=0;
                byte=1;
                CardBytesRemaining=byte;
                result=0;goto done;
            }
            CardBytesRemaining=138;
            result=0;goto done;
        }
        case 3:goto handshake;
        case 8: {
            /* ASM_BARRIER: Preserve the original byte comparison temporary and branch delay. */
            register u32 byte __asm__("$3")=value&255;
            if(byte!=0x5D){result=-8;goto done;}
            CardReadChecksum=0;result=0;goto done;
        }
        case 9:
            if((result=value&255)!=((u8 *)&CardCmdSector)[1]){result=-9;goto done;}
            goto fold_original;
        case 10:
            if((result=value&255)!=*(u8 *)&CardCmdSector){result=-10;goto done;}
fold_original:
            result=CardReadChecksum;
            result^=original;
            CardReadChecksum=result;result=0;goto done;
        case 139:
            if((result=value&255)==CardReadChecksum)goto success;
            result=-139;goto done;
        case 140:
            if((value&255)!=0x47){result=-140;goto done;}
        default:
            if((u32)CardRxCount-11<128)CardReadChecksum^=value;
            result=0;goto done;
        }
    } else {
        switch(CardRxCount) {
        case 2: {
            /* ASM_BARRIER: Preserve the original byte comparison temporary and branch delay. */
            register u32 byte __asm__("$3")=value&255;
            if(byte==8) {
                CardBytesRemaining=1;
                result=0;goto done;
            }
            CardBytesRemaining=136;
            result=0;goto done;
        }
        case 3:
handshake:
            {
                /* ASM_BARRIER: Preserve the original byte comparison temporary and branch delay. */
            register u32 byte __asm__("$3")=value&255;
                if(byte!=0x5A) {result=-3;goto done;}
                goto success;
            }
        case 137: {
            /* ASM_BARRIER: Preserve the original byte comparison temporary and branch delay. */
            register u32 byte __asm__("$3")=value&255;
            if(byte!=0x5D) {result=-137;goto done;}
            goto success;
        }
        case 138: {
            /* ASM_BARRIER: Keep the final marker separate from the masked argument. */
            register s32 magic __asm__("$3");
            value&=255;
            magic=0x47;
            if(value!=magic) {result=-138;goto done;}
            goto success;
        }
        default:result=0;goto done;
        }
    }
success:
    result=0;
done:
    /* ASM_BARRIER: Retain the shared result entry and original validation branch directions. */
    __asm__("" : "+r"(result));
    return result;
}
