/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/library.h"

extern int DmaChannelRegsBase;

extern char DmaBusErrorFmt[];

extern char DmaMadrDebugFmt[];

extern void DebugPrintStub();

void trapIntrDMA(void) {
    unsigned int s1;
    int i;
    RrLibInterruptCallback *bp;
    unsigned int one, m;
    s1 = (*DICR_VSync >> 24) & 0x7F;
    if (s1 != 0) {
        one = 1;
        m = 0xFFFFFF;
        bp = InterruptCallbackTable;
        do {
            i = 0;
            if (s1 != 0) {
                for (; s1 != 0; i++, s1 >>= 1) {
                    if (i >= 7) break;
                    if (s1 & 1) {
                        *DICR_VSync &= (one << (i + 24)) | m;
                        if (bp[i] != 0) bp[i]();
                    }
                }
            }
            s1 = (*DICR_VSync >> 24) & 0x7F;
        } while (s1 != 0);
    }
    if ((*DICR_VSync & 0xFF000000) == 0x80000000 || (*DICR_VSync & 0x8000)) {
        DebugPrintStub(DmaBusErrorFmt, *DICR_VSync);
        i = 0;
        do {
            DebugPrintStub(DmaMadrDebugFmt, i, *(int *)(i * 16 + DmaChannelRegsBase));
            i++;
        } while (i < 7);
    }
}
