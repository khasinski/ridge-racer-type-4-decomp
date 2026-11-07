/* CC1_FLAGS: -mno-split-addresses */
#include "rr/spu.h"
#include "rr/spu_init.h"
extern s32 DebugPrintStub(const char *,...);
extern char D_800116B8[];
extern char StrSpuDmaClearWaitMsg[];
void _spu_FwriteByIO(const u16 *source,u32 size) {
    register RrSpuRegisterBank *port=_spu_RXX;
    /* ASM_BARRIER: Retain the transfer-address halfword while capturing device status. */
    register u32 address __asm__("$3")=_spu_tsa;
    register u32 remaining=size;
    register u32 previous;
    s32 chunk;
    /* ASM_BARRIER: Capture initial status in the consumed byte-count argument register. */
    register u32 status __asm__("$5")=port->spu_status;
    register const u16 *input;
    input=source;
    port->transfer_address=address;
    /* ASM_BARRIER: Publish the transfer address before preparing the saved status mask. */
    __asm__("" : "+r"(status) : : "memory");
    previous=status&0x7FF;
    _spu_Fw1ts();
    while(remaining) {
        s32 written;
        if(remaining<65)chunk=remaining;else chunk=64;
        written=0;
        if(chunk>0) {
            RrSpuRegisterBank *output=_spu_RXX;
            do {
                output->transfer_data=*input++;
                written+=2;
            } while(written<chunk);
        }
        {
            register RrSpuRegisterBank *control=_spu_RXX;
            /* ASM_BARRIER: Keep the old control halfword separate from its command bits. */
            /* ASM_BARRIER: Capture final control before restoring the previous status comparison. */
        register u32 old __asm__("$4")=control->spu_control;
            /* ASM_BARRIER: Form the original control mask in the call-delay temporary. */
            /* ASM_BARRIER: Keep the final mode-clear result separate from the next poll counter. */
        register u32 command __asm__("$2");
            /* ASM_BARRIER: Preserve the original control read and mask operands. */
            __asm__("" : "+r"(old));
            command=old&0xFFCF;
            *(u16 *)&control->spu_control=command|0x10;
            _spu_Fw1ts();
        }
        {
            register u32 count=0;
            if(_spu_RXX->spu_status&0x400) {
                u32 busy;
                /* ASM_BARRIER: Retain zero then increment when entering the bounded busy wait. */
                /* ASM_BARRIER: Retain the final wait initial increment after its entry comparison. */
            __asm__("" : "+r"(count));
                count++;
                do {
                    if(count>0xF00) {
                        /* ASM_BARRIER: Prepare the diagnostic format in its original argument register. */
                        /* ASM_BARRIER: Keep the restore-timeout diagnostic format in its argument register. */
                    register const char *format __asm__("$4")=SpuTimeoutFmt;
                        /* ASM_BARRIER: Prepare the mapped diagnostic reason in its original argument register. */
                        /* ASM_BARRIER: Keep the restore-timeout reason in its argument register. */
                    register const char *reason __asm__("$5");
                        /* ASM_BARRIER: Finish the format address before forming the reason-page address. */
                        /* ASM_BARRIER: Finish the second format before forming its mapped reason page. */
                    __asm__("" : : "r"(format) : "$5");
                        reason=(const char *)0x80010000U;
                        /* ASM_BARRIER: Expose the real PAL ROM page for the diagnostic call delay slot. */
                        /* ASM_BARRIER: Expose the restore-timeout ROM reason in the call delay slot. */
                    __asm__("" : "+r"(reason));
                        DebugPrintStub(format,reason+0x16B8);break;
                    }
                    busy=_spu_RXX->spu_status&0x400;
                    count++;
                } while(busy);
            }
        }
        remaining-=chunk;
        _spu_Fw1ts();
        _spu_Fw1ts();
    }
    port=_spu_RXX;
    {
        /* ASM_BARRIER: Capture final control before restoring the previous status comparison. */
        register u32 old __asm__("$4")=port->spu_control;
        register u32 expected;
        /* ASM_BARRIER: Keep the final mode-clear result separate from the next poll counter. */
        register u32 command __asm__("$3");
        /* ASM_BARRIER: Retain both final captured values before narrowing the saved status. */
        __asm__("" : "+r"(old), "+r"(previous));
        expected=(u16)previous;
        /* ASM_BARRIER: Prepare the saved-status halfword before the control restore store. */
        __asm__("" : "+r"(expected));
        command=old&0xFFCF;
        port->spu_control=command;
        {
        /* ASM_BARRIER: Retain the final bounded-poll counter in its original temporary. */
        register u32 count __asm__("$3")=0;
        if((port->spu_status&0x7FF)!=expected) {
            u32 busy;
            /* ASM_BARRIER: Retain the final wait initial increment after its entry comparison. */
            __asm__("" : "+r"(count));
            count++;
            do {
                if(count>0xF00) {
                    /* ASM_BARRIER: Keep the restore-timeout diagnostic format in its argument register. */
                    register const char *format __asm__("$4")=SpuTimeoutFmt;
                    /* ASM_BARRIER: Keep the restore-timeout reason in its argument register. */
                    register const char *reason __asm__("$5");
                    /* ASM_BARRIER: Finish the second format before forming its mapped reason page. */
                    __asm__("" : : "r"(format) : "$5");
                    reason=(const char *)0x80010000U;
                    /* ASM_BARRIER: Expose the restore-timeout ROM reason in the call delay slot. */
                    __asm__("" : "+r"(reason));
                    DebugPrintStub(format,reason+0x16CC);break;
                }
                busy=(_spu_RXX->spu_status&0x7FF)!=expected;
                count++;
            } while(busy);
        }
    }
}
}
