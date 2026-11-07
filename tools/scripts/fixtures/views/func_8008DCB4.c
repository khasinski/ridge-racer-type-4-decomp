/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "rr/cd.h"

/* legacy typing of shared globals in this part of the unit */
#define CdIntrStatusBytes (*(u8 (*)[2])&CdIntrStatusBytes)

#undef CdIntrStatusBytes

s32 CD_getsector2(u8 *a, u32 b) {
    volatile int x;
    *CD_reg0 = 0;
    *CD_reg3 = 0x80;
    *CD_DELAY = 0x21020843;
    *COM_DELAY = 0x1325;
    *CD_dpcr |= 0x8000;
    *DMA3_MADR = a;
    *DMA3_BCR = b | 0x10000;
    while (!(*CD_reg0 & 0x40)) {
    }
    *DMA3_CHCR = 0x11400100;
    x = *DMA3_CHCR;
    return 0;
}
