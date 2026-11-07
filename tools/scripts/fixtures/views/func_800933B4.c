/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "rr/gpu_reset.h"

#include "rr/library.h"

#include "rr/draw_environment.h"

/* legacy typing of shared globals in this part of the unit */
#define GetClut ((int (*)())GetClut)

#undef GetClut

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(u8 (*)[])&GpuDebugLevel)

#define GPU_printf (*(void (*(*))(const char *, s32, s32, s32))&GPU_printf)

#undef GpuDebugLevel

#undef GPU_printf

/* legacy typing of shared globals in this part of the unit */
#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuFuncTable ((int *)GpuFuncTable)

#define DMACallback ((int (*)())DMACallback)

#undef GPU_printf

#undef GpuFuncTable

#undef DMACallback

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(u8 (*))&GpuDebugLevel)

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

#define GPU_printf (*(void (*(*))())&GPU_printf)

#undef GpuDebugLevel

#undef GPU_printf

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

#define GPU_printf (*(void (*(*))())&GPU_printf)

#define GpuFuncTable (*(struct { char pad[0x10]; int (*f)(); } *(*))&GpuFuncTable)

#define GPU_memset ((int (*)())GPU_memset)

#undef GpuDebugLevel

#undef GPU_printf

#undef GpuFuncTable

#undef GPU_memset

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

#define GPU_printf (*(M2C_UNK (*(*))(M2C_UNK *, s32))&GPU_printf)

#define GpuDebugLevel (*(u8 (*))&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuMovePayload (*(B (*))&GpuMovePayload)

#define GpuFuncTable ((int *)GpuFuncTable)

#undef GpuMovePayload

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((RrLibGraphicsDriverPrefix *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((int *)GpuFuncTable)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

extern int PsyqMemcpy();

extern char PutDrawEnvFmt[];

int PutDrawEnv(int *a0) {
    unsigned char *p = &GpuDebugLevel;
    if (*p >= 2) {
        GPU_printf(PutDrawEnvFmt, a0);
    }
    SetDrawEnv2((u32 *)(a0 + 7), (RrFrameEnvironment *)a0);
    a0[7] |= 0xFFFFFF;
    ((int (*)())GpuFuncTable[2])(GpuFuncTable[6], a0 + 7, 0x40, 0);
    PsyqMemcpy(p + 0xE, a0, 0x5C);
    return (int)a0;
}
