/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/library.h"
#include "m2c_macros.h"
#include "psyq/libgpu/sys_drain.h"
#include "psyq/libgpu/gpuenv.h"
#include "rr/gpu.h"

extern struct { char p[0x18]; int (*f18)(); } *GpuFuncTable;
extern volatile unsigned *DMA2_CHCR;
extern volatile unsigned *GPU_STATUS;
extern int GpuTimeoutDeadline;
extern int GpuAlarmCount;

int DrawOTag2();
void GPU_memset(s8 *arg0, s8 arg1, s32 arg2);

/* legacy typing of shared globals in this part of the unit */
#define D_800ABAC2 (*(unsigned char (*))&D_800ABAC2)
#define GPU_printf (*(int (*(*))())&GPU_printf)

int DrawOTag2(a0) int a0; {
    if (D_800ABAC2 >= 2) GPU_printf(DrawOTagFmt, a0);
    GpuTimeoutDeadline = VSync(-1) + 0xF0;
    GpuAlarmCount = 0;
    while ((*DMA2_CHCR & 0x1000000) || (*GPU_STATUS & 0x4000000) == 0) {
        if (get_alarm() != 0) return -1;
    }
    DMACallback(2, GPU_DmaDoneRequeue);
    GpuFuncTable->f18(a0);
    return 0;
}

#undef D_800ABAC2
#undef GPU_printf

void GPU_DmaDoneRequeue(void) {
    DMACallback(2,_exeque);
}

void GPU_memset(s8 *arg0, s8 arg1, s32 arg2) {
    s32 var_v0;
    s8 *var_a0;

    var_a0 = arg0;
    var_v0 = arg2 - 1;
    if (arg2 != 0) {
        do {
            *var_a0 = arg1;
            var_v0 -= 1;
            var_a0 += 1;
        } while (var_v0 != -1);
    }
}
