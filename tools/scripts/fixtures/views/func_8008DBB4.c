/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "rr_asm.h"

#include "rr/cd.h"

/* legacy typing of shared globals in this part of the unit */
#define CdIntrStatusBytes (*(u8 (*)[2])&CdIntrStatusBytes)

#undef CdIntrStatusBytes

s32 CD_getsector(u8 *a0, u32 a1) {
    volatile unsigned char *p;
    *CD_reg0 = 0;
    *CD_reg3 = 0x80;
    *CD_DELAY = 0x20943;
    /* Ordinary write views retain the original setup and teardown scheduling. */
    *(u32 *)COM_DELAY = 0x1323;
    *CD_dpcr |= 0x8000;
    *DMA3_MADR = a0;
    *DMA3_BCR = a1 | 0x10000;
    p = CD_reg0;
    /* ASM_BARRIER: orig has the load-delay nop BEFORE the loop label; maspsx emits it inside the loop */
    RR_NOP_DEP();
    do {
    } while (!(*p & 0x40));
    *DMA3_CHCR = 0x11000000;
    if (*DMA3_CHCR & 0x01000000) {
        do {
        } while (*DMA3_CHCR & 0x01000000);
    }
    *(u32 *)COM_DELAY = 0x1325;
    return 0;
}
