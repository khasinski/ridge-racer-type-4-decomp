/* CC1_FLAGS: -mno-split-addresses */

#include "rr/card_format.h"

s32 _card_format(s32 arg0){
    register s32 port=arg0;
    register s32 i=0;
    RrCardFormatSector *scratch=&D_80114328;
    RrCardDirectoryRecord *base=CardFormatDirRecords;
    register s32 state=0xA0,last=0xFFFF,success=1;
    register s32 next;
    s32 status;
    s32 result;
    register RrCardDirectoryRecord *entry;
    /* ASM_BARRIER: retain the byte-aligned directory view for this copy phase. */
    register RrCardDirectoryWire *wire asm("$17");
    register u32 offset;
    RrCardBrokenRecord *slot;

    do {
        PsyqBzero(scratch,128);
        offset=(u32)i*32U;
        entry=(RrCardDirectoryRecord*)((unsigned long)offset+(unsigned long)base);
        PsyqBzero(entry,32);
        wire=&entry->wire;

        CardFormatDirRecords[i].fields.state=state;
        CardFormatDirRecords[i].fields.size=0;
        CardFormatDirRecords[i].fields.next=last;
        {
        register u32 a,b,c,d;
        a=wire->words[0].value;
        b=wire->words[1].value;
        c=wire->words[2].value;
        d=wire->words[3].value;

        scratch->directory.words[0].value=a;
        scratch->directory.words[1].value=b;
        scratch->directory.words[2].value=c;
        scratch->directory.words[3].value=d;
        a=wire->words[4].value;
        b=wire->words[5].value;
        c=wire->words[6].value;
        d=wire->words[7].value;

        scratch->directory.words[4].value=a;
        scratch->directory.words[5].value=b;
        scratch->directory.words[6].value=c;
        scratch->directory.words[7].value=d;
        /* ASM_BARRIER: preserve this clear/copy/write preparation boundary. */
        __asm__("" : : "r"(scratch));
        }
        {
        register s32 write_port=port;

        next=i+1;
        if(_card_sector_write(write_port,next,scratch->bytes)!=success){result=0;goto done;}
        }
        i=next;
    }while(i<15);
    {
    register RrCardFormatSector *second;
    register s32 invalid;
    register s32 one;
    register RrCardWireWord *word;
    u32 tail;
    i=0;
    invalid=-1;second=scratch;one=1;
    slot=D_801142D8;

    do {
        slot->value=invalid;
        /* ASM_BARRIER: preserve this clear/copy/write preparation boundary. */
        __asm__("" : : "m"(slot->wire.bytes[0]));
        PsyqBzero(second,128);
        word=&slot->wire.word;

        slot=(RrCardBrokenRecord*)word;
        tail=word->value;
        second->broken.word.value=tail;
        /* ASM_BARRIER: preserve this clear/copy/write preparation boundary. */
        __asm__("" : : "m"(second->broken.bytes[0]));
        status=_card_sector_write(port,i+16,second->bytes);
        i++;
        if(status!=one){result=0;goto done;}
        slot++;
    }while(i<20);
    PsyqBzero(second,128);
    second->bytes[0]='M';second->bytes[1]='C';
    result=_card_sector_write(port,0,second->bytes)==1;
}
done:
    return result;
}
