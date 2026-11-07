/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/library.h"

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#undef GpuDebugLevel

#undef GPU_printf

extern void _exeque(void);

void GPU_DmaDoneRequeue(void) {
    DMACallback(2,_exeque);
}
