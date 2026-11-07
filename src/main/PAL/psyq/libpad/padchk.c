/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channel_gates.h"
void _padAllocGates(void *argument) {
    RrChannelRecordF0 *record=argument;
    register s32 i,j,found,mask;
    s32 value;
    register s32 one;
    register s32 bound;
    register s32 offset;
    /* ASM_BARRIER: retain the clamped input length in T1 for both scans. */
    register s32 length asm("$9");
    /* ASM_BARRIER: use A0 for gate stores after the input scan is finished. */
    register u8 *mapping,*data,*gates asm("$4");
    PsyqBzero(record->status.gates57,6);
    if(record->status.pending && record->status.data28) {
        length=6;
        if(((volatile RrChannelStatusView *)&record->status)->length34<7)length=((volatile RrChannelStatusView *)&record->status)->length34;
        i=0;
        if(record->entry_countE9) {
            one=1;
            /* ASM_BARRIER: keep the row index distinct from the zero byte offset. */
            __asm__("" : "+r"(i));
            offset=0;
            do {
                found=0;
                mask=1;
                if(((const RrChannelGateEntry5 *)((const u8 *)((unsigned long)offset+(unsigned long)record->status.entries04)))->all_bits02)mask=255;
                mapping=record->status.payload5D;
                data=record->status.data28;
                j=0;
                if(length) {
scan:
                    if(*mapping==i) {
                        if(!(*data&mask))goto miss;
                        goto matched;
                    } else {
miss:
                        mapping++;j++;data++;
                        if(j<length)goto scan;
                    }
                }
scan_done:
                if(found) {
                    value=(s32)((u32)D_800ABCB0+((const RrChannelGateEntry5 *)((const u8 *)((unsigned long)offset+(unsigned long)record->status.entries04)))->cost03);
                    if(value<61) {
                        /* ASM_BARRIER: stage the mapped budget address in AT after acceptance. */
                        register s32 *budget_page asm("$1");
                        __asm__("" : : : "$1");
                        budget_page=(s32 *)0x800B0000U;
                        /* ASM_BARRIER: expose the final store independently of address formation. */
                        __asm__("" : "+r"(budget_page));
                        budget_page[-0x4350/4]=value;
                        goto reserved;
matched:
                        found=1;goto scan_done;
                    } else found=0;
reserved:
                    /* ASM_BARRIER: share the accepted/rejected continuation before marking gates. */
                    __asm__("" : "+r"(found));
                    if(found) {
                        mapping=record->status.payload5D;
                        gates=record->status.gates57;
                        j=0;
                        if(length) {
                            do {
                                s32 selected=*mapping;
                                mapping++;
                                if(selected==i)*gates=one;
                                j++;gates++;
                            } while(j<length);
                        }
                    }
                }
                bound=record->entry_countE9;
                /* ASM_BARRIER: capture the live row bound before incrementing the index. */
                __asm__("" : "+r"(bound));
                i++;offset+=5;
            } while(i<bound);
        }
        goto out;
    }
    value=record->responseE8;
    if(((u32)value-4U)<2U || value==7) {
        if(!record->status.pending && record->status.length34>=2) {
            data=record->status.data28;
            if((data[0]&0xC0)==0x40 && (data[1]&1) && (s32)((u32)D_800ABCB0+10U)<61) {
                register s32 enable=1;
                record->status.gates57[1]=enable;
                record->status.gates57[0]=enable;
                {
                    s32 total=(s32)((u32)D_800ABCB0+10U);
                    /* ASM_BARRIER: finish the wrapped increment before forming its AT address. */
                    register s32 *tail_page asm("$1");
                    __asm__("" : : "r"(total) : "$1");
                    tail_page=(s32 *)0x800B0000U;
                    /* ASM_BARRIER: retain an independent mapped store at the common exit. */
                    __asm__("" : "+r"(tail_page));
                    tail_page[-0x4350/4]=total;
                }
            }
            goto out;
        }
    }
    if(((volatile RrChannelRecordF0 *)record)->responseE8==3) {record->status.gates57[0]=1;goto out;}
    if(!record->status.pending) {
        register s32 fill=1;
        j=5;
        /* ASM_BARRIER: keep the reverse count separate from the byte-view base bias. */
        __asm__("" : "+r"(j));
        gates=(u8 *)record+5;
        do {gates[RR_FIELD_OFFSET(RrChannelRecordF0,status.gates57)]=fill;j--;gates--;}while(j>=0);
    }
out:
    /* ASM_BARRIER: end short V0 values at the common void return. */
    __asm__("" : : : "$2");
}
