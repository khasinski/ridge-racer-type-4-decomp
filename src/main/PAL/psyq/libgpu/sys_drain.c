/* CC1_FLAGS: -mno-split-addresses */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "psyq/libgpu/sys_drain.h"
#include "rr/interrupts.h"
#include "rr/gpu.h"
extern int DMACallback(int, int);

struct GpuSys {
    unsigned char pad0;
    unsigned char enabled;
    unsigned char pad2[6];
    int unk8;
    void (*unkC)();
};

struct GpuQueue {
    void (*op)();
    int p0;
    int p1;
    int buf[21];
};

extern struct GpuSys D_800ABAC0;
extern int GpuDrawSyncCallback;
extern volatile unsigned int *GPU_STATUS;
extern volatile unsigned int *DMA2_CHCR;
extern volatile int _qin;
extern int GpuSavedIntrMask;
extern volatile struct GpuQueue GpuCmdQueue[];

int _exeque(void) {
    int m0, m1;
    if (*DMA2_CHCR & 0x01000000) {
        return 1;
    }
    GpuSavedIntrMask = SetIntrMask(0);
    while (_qin != _qout && (m0 = 0x01000000, (*DMA2_CHCR & m0) == 0)) {
        if (((_qout + 1) & 0x3F) == _qin && GpuDrawSyncCallback == 0) {
            DMACallback(2, 0);
        }
        while ((m1 = 0x04000000, (*GPU_STATUS & m1) == 0))
;
        GpuCmdQueue[_qout].op(GpuCmdQueue[_qout].p0, GpuCmdQueue[_qout].p1);
        _qout = (_qout + 1) & 0x3F;
    }
    SetIntrMask(GpuSavedIntrMask);
    if (_qin == _qout && (*DMA2_CHCR & 0x01000000) == 0) {
        void (*fn)();
        if (D_800ABAC0.unk8 != 0 && (fn = D_800ABAC0.unkC) != 0) {
            struct GpuSys *sys = &D_800ABAC0;
            *(volatile int *)&sys->unk8 = 0;
            fn();
        }
    }
    return (_qin - _qout) & 0x3F;
}
