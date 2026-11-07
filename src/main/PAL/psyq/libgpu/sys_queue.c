/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "psyq/libgpu/sys_drain.h"
#include "rr/gpu.h"
#include "rr/interrupts.h"
struct GpuSys {
    unsigned char pad0;
    unsigned char enabled;
    unsigned char pad2[6];
    int unk8;
    int unkC;
};

struct GpuQueue {
    int (*op)();
    int *p0;
    int p1;
    int buf[21];
};

typedef int (*GpuOpFunc)();

extern int DMACallback(int, int (*)());
extern struct GpuSys GpuSysVersion;
extern volatile unsigned int *GPU_STATUS;
extern volatile unsigned int *DMA2_CHCR;
extern volatile int _qin;
extern int GpuQueueSavedIntrMask;
extern volatile struct GpuQueue GpuCmdQueue[];

void _addque(int a, int b, int c);
int _addque2(GpuOpFunc op, int *p0, int len, int p1);

void _addque(int a, int b, int c) { _addque2(a, b, 0, c); }

int _addque2(GpuOpFunc op, int *p0, int len, int p1) {
    int i;
    struct GpuSys *sys;

    set_alarm();
    while (((_qin + 1) & 0x3F) == _qout) {
        if (get_alarm() != 0) {
            return -1;
        }
        _exeque();
    }
    GpuQueueSavedIntrMask = SetIntrMask(0);
    sys = &GpuSysVersion;
    sys->unk8 = 1;
    if (sys->enabled == 0 || (_qin == _qout && (*DMA2_CHCR & 0x01000000) == 0 && sys->unkC == 0)) {
        while ((*GPU_STATUS & 0x04000000) == 0)
;
        op(p0, p1);
        SetIntrMask(GpuQueueSavedIntrMask);
        return 0;
    }
    DMACallback(2, _exeque);
    if (len != 0) {
        for (i = 0; i < len / 4; i++) {
            GpuCmdQueue[_qin].buf[i] = p0[i];
        }
        GpuCmdQueue[_qin].p0 = GpuCmdQueue[_qin].buf;
    } else {
        GpuCmdQueue[_qin].p0 = p0;
    }
    GpuCmdQueue[_qin].p1 = p1;
    GpuCmdQueue[_qin].op = op;
    _qin = (_qin + 1) & 0x3F;
    SetIntrMask(GpuQueueSavedIntrMask);
    _exeque();
    return (_qin - _qout) & 0x3F;
}
