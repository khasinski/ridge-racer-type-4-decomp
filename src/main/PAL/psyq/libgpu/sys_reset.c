/* CC1_FLAGS: -mno-split-addresses */
#include "rr/gpu.h"
#include "rr/gpu_reset.h"
#include "rr/interrupts.h"
extern int D_800ABBF8;
extern char D_801127D0[];
extern char GpuCmdQueue[];
int _reset(a0)
int a0;
{
    int t;
    t = SetIntrMask(0);
    _qout = 0;
    D_800ABBF8 = t;
    _qin = _qout;
    /* Ordinary register views retain the original reset scheduling. */
    switch (a0 & 7) {
    case 5:
    case 0:
        *(u32 *)DMA2_CHCR = 0x401;
        *(u32 *)DPCR |= 0x800;
        *(u32 *)GPU_STATUS = 0;
        GPU_memset(D_801127D0, 0, 0x100);
        GPU_memset(GpuCmdQueue, 0, 0x1800);
        break;
    case 1:
    case 3:
        *(u32 *)DMA2_CHCR = 0x401;
        *(u32 *)DPCR |= 0x800;
        *(u32 *)GPU_STATUS = 0x02000000;
        *(u32 *)GPU_STATUS = 0x01000000;
        break;
    }
    SetIntrMask(D_800ABBF8);
    if ((a0 & 7) == 0) return GPU_CheckVersion(a0);
    return 0;
}
