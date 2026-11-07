#include "rr/bios_devices.h"
extern s32 DebugPrintStub(const char *format,...);
extern char CardReadErrorMsg[];
s32 _card_sector_write(s32 port,s32 sector,u8 *buffer) {
    u8 readback[128];
    s32 tries=0,count;
    u8 *cursor;
    /* ASM_BARRIER: Keep the readback buffer address across BIOS calls. */
    register u8 *scratch __asm__("$18");
    s32 one;
    u32 checksum=0;
    /* ASM_BARRIER: Keep initial retry zero separate from the checksum accumulator. */
    __asm__("" : "+r"(tries));
    {
        u8 *p=buffer;
        u32 sum;
        s32 count;
        u32 next;
        /* ASM_BARRIER: Initialize checksum independently before preparing the loop count. */
        __asm__("" : "=r"(sum) : "0"(0) : "$4");
        count=126;
        do {
            u32 byte=*p++;
            count--;
            next=sum^byte;
            sum=next;
        } while(count>=0);
        *p=next;
    }
    /* ASM_BARRIER: Prepare the success constant after completing the caller checksum. */
    __asm__("" : : : "$21");
    one=1;
    scratch=readback;
    for(;;) {
        if(tries>=8) return 0;
        _new_card();
        if(_card_write(port,sector,buffer)!=one) return 0;
        while(!(_card_status(port>>4)&1)) {}
        PsyqBzero(scratch,128);
        _new_card();
        if(_card_read(port,sector,scratch)!=one) {
            /* ASM_BARRIER: Form the failure message only after the read-result branch. */
            __asm__("" : : : "$4");
            DebugPrintStub(CardReadErrorMsg);
        } else {
            while(!(_card_status(port>>4)&1)) {}
        }
        cursor=readback;checksum=0;count=126;
        do {checksum^=*cursor++;} while(--count>=0);
        tries++;
        if(buffer[127]==(checksum&255)) return 1;
    }
}
