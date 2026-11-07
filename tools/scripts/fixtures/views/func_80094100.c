/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "rr/gpu.h"

#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

/* legacy typing of shared globals in this part of the unit */
#define PsyqMemcpy ((int (*)())PsyqMemcpy)

#undef PsyqMemcpy

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GPU_STATUS ((s32 *)GPU_STATUS)

#undef GPU_STATUS

/* legacy typing of shared globals in this part of the unit */
#define GPU_STATUS ((u32 *)GPU_STATUS)

#undef GPU_STATUS

/* legacy typing of shared globals in this part of the unit */
#define GPU_DATA ((int *)GPU_DATA)

#define GPU_STATUS ((int *)GPU_STATUS)

#undef GPU_DATA

#undef GPU_STATUS

u32 get_cs(s32 a0, s32 a1) {
    /* ASM_BARRIER: Preserve the original coordinate result register. */
    register s32 v __asm__("$2");
    /* ASM_BARRIER: Preserve the original clamped y value register. */
    register s32 w __asm__("$5");
    /* ASM_BARRIER: Preserve the original packed y value register. */
    register s32 u __asm__("$3");
    s32 c;
    s32 t;
    s32 coordinate;
    register s32 shifted=(s32)((u32)a0<<16);
    coordinate=shifted>>16;
    if (coordinate >= 0) {
        c = GpuVramWidth - 1 < coordinate;
        /* ASM_BARRIER: Preserve the separate unsigned limit reload after the signed comparison. */
        __asm__ volatile("" : "=r"(c) : "0"(c) : "memory");
        t = *(u16 *)&GpuVramWidth;
        if (c) v = t - 1; else v = a0;
    } else v=0;
    a0=v;
    shifted=(s32)((u32)a1<<16);
    coordinate=shifted>>16;
    if (coordinate >= 0) {
        c = GpuVramHeight - 1 < coordinate;
        /* ASM_BARRIER: Preserve the separate unsigned limit reload after the signed comparison. */
        __asm__ volatile("" : "=r"(c) : "0"(c) : "memory");
        t = *(u16 *)&GpuVramHeight;
        u = a1 & 0x3FF;
        if (c) {
            /* ASM_BARRIER: Preserve the original y value on the unclamped path. */
            __asm__("" : : : "$5");
            w = t - 1;
            goto L;
        }
    } else {
        w = 0;
L:
        u = w & 0x3FF;
    }
    u <<= 10;
    v=a0 & 0x3FF;
    a0=(s32)0xE3000000U;
    v |= a0;
    return u | v;
}
