/* GCC_VERSION: 2.7.2 */
#include "rr/gpu.h"
#include "rr/timing.h"
#include "rr/interrupts.h"

extern s32 DebugPrintStub(const char *format,...);
extern char GpuTimeoutFmt[];

void set_alarm(void) {
    GpuTimeoutDeadline = (u32)VSync(-1) + 0xF0;
    GpuAlarmCount = 0;
}

int get_alarm(void) {
    s32 old,result;
    s32 saved;
    if(GpuTimeoutDeadline>=VSync(-1)) {
        s32 *counter=&GpuAlarmCount;
        {
            s32 tick=*counter;
            old=tick;
            tick=(u32)tick+1;
            *counter=tick;
        }
        if(old<=0xF0000) goto ok;
    }
    {
        volatile u32 *gpu=GPU_STATUS;
        const char *format;
        u32 head;
        format=GpuTimeoutFmt;
        *gpu;
        /* ASM_BARRIER: Keep the discarded status read before reading queue indices. */
        __asm__("" : : : "memory");
        head=_qin;
        /* ASM_BARRIER: Read the queue head before preparing the remaining diagnostic operands. */
        __asm__("" : "=r"(head) : "0"(head));
        DebugPrintStub(format,(head-(u32)_qout)&63,
            *gpu,*DMA2_CHCR,*DMA2_MADR);
    }
    saved=SetIntrMask(0);
    _qout=0;
    SavedIntrMask=saved;
    _qin=_qout;
    *DMA2_CHCR=0x401;
    *DPCR|=0x800;
    *GPU_STATUS=0x02000000;
    *GPU_STATUS=0x01000000;
    SetIntrMask(SavedIntrMask);
    result=-1;
    goto done;
ok:
    result=0;
done:
    return result;
}

s32 GPU_CheckVersion(s32 a) {
    *GPU_STATUS = 0x10000007;
    if ((*GPU_DATA & 0xFFFFFF) != 2) {
        *GPU_DATA = (*GPU_STATUS & 0x3FFF) | 0xE1001000;
        *GPU_DATA;
        return 0;
    }
    if (!(a & 8)) return 1;
    /* Ordinary final publication exposes the original branch-delay store. */
    *(u32 *)GPU_STATUS = 0x09000001;
    return 2;
}
