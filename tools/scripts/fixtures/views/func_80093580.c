/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "types.h"

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

typedef struct {
    s16 x, y, w, h;
} RECT16;

typedef struct {
    volatile u16 x, y, w, h;
} URECT16;

typedef struct {
    RECT16 disp;
    RECT16 screen;
    u8 isinter;
    u8 isrgb24;
    u8 pad0;
    u8 pad1;
} DISPENV_T;

typedef struct {
    u16 x0, x1;
} HRANGE;

extern u8 GetVideoMode(void);

extern void PsyqMemcpy(void *, void *, s32);

extern char StrPutDispEnvFmt[];

extern struct {
    char pad[0x10];
    void (*f10)(u32);
} *GpuFuncTable;

extern void (*GPU_printf)(const char *, ...);

extern u8 D_800ABAC2[];

extern u8 D_800ABAC3;

extern char D_800ABB2C[];

extern URECT16 D_800ABB34;

extern HRANGE DispHRangeTable[2][5];

extern u8 DispDotClockDivisors[5];

DISPENV_T *PutDispEnv(DISPENV_T *env) {
    u32 mode;
    /* ASM_BARRIER: keep p in $s2 so env gets $s1 */
    register u8 *p asm("$18");
    URECT16 *d;
    s32 y0, y1;

    p = D_800ABAC2;
    mode = 0x08000000;
    if (*p >= 2) {
        GPU_printf(StrPutDispEnvFmt, env);
    }
    {
        /* ASM_BARRIER: keep 0x05000000 in $v0 so disp.y shift gets $v1 */
        register u32 cmd5 asm("$2") = 0x05000000;
        GpuFuncTable->f10((((u16)env->disp.y & 0x3FF) << 10) | (((u16)env->disp.x & 0x3FF) | cmd5));
    }
    d = (URECT16 *)(p + 0x6A);
    if (*(s32 *)(p + 0x7A) != *(s32 *)&env->isinter ||
        (s16)*(volatile u16 *)(p + 0x6A) != env->disp.x ||
        (s16)d->y != env->disp.y ||
        (s16)d->w != env->disp.w ||
        (s16)d->h != env->disp.h) {
        if ((env->pad0 = GetVideoMode()) == 1) {
            mode |= 0x08;
        }
        if (env->isrgb24) {
            mode |= 0x10;
        }
        if (env->isinter) {
            mode |= 0x20;
        }
        if (D_800ABAC3) {
            mode |= 0x80;
        }
        if (env->disp.w >= 0x119) {
            if (env->disp.w < 0x161) {
                mode |= 0x01;
            } else if (env->disp.w < 0x191) {
                mode |= 0x40;
            } else if (env->disp.w < 0x231) {
                mode |= 0x02;
            } else {
                mode |= 0x03;
            }
        }
        {
            s32 h = env->disp.h;
            s32 v = env->pad0 ? (h < 0x121) : (h < 0x101);
            if (!v) {
                mode |= 0x24;
            }
        }
        GpuFuncTable->f10(mode);
        env->pad0 = 8;
    }
    if ((s16)D_800ABB34.x != env->screen.x ||
        (s16)D_800ABB34.y != env->screen.y ||
        (s16)D_800ABB34.w != env->screen.w ||
        (s16)D_800ABB34.h != env->screen.h ||
        env->pad0 == 8) {
        s32 x0;
        /* ASM_BARRIER: keep x1 in $v1 */
        register s32 x1 asm("$3");
        /* ASM_BARRIER: keep dx in $v0 */
        register s32 dx asm("$2");
        /* ASM_BARRIER: keep w in $a1 */
        register s32 w asm("$5");
        /* ASM_BARRIER: keep i in $a2 */
        register s32 i asm("$6");

        env->pad0 = GetVideoMode();
        y0 = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        y1 = env->screen.h ? y0 + env->screen.h : y0 + 0xF0;
        if (env->disp.w < 0x119) {
            i = 0;
        } else if (env->disp.w < 0x161) {
            i = 1;
        } else if (env->disp.w < 0x191) {
            i = 2;
        } else if (env->disp.w < 0x231) {
            i = 3;
        } else {
            i = 4;
        }
        x0 = DispHRangeTable[env->pad0][i].x0 + env->screen.x * DispDotClockDivisors[i];
        dx = DispHRangeTable[env->pad0][i].x1 - DispHRangeTable[env->pad0][i].x0;
        w = env->screen.w;
        if (w) {
            dx = (dx * w) >> 8;
        }
        x1 = x0 + dx;
        /* ASM_BARRIER: force reload of env->pad0 after computing x1 */
        __asm__ volatile("" : : : "memory");
        if (env->pad0) {
            x0 = CLAMP(x0, 0x21C, 0xC94);
            x1 = CLAMP(x1, x0 + DispDotClockDivisors[i] * 4, 0xCBC);
            y0 = CLAMP(y0, 0x13, 0x12F);
            y1 = CLAMP(y1, y0 + 2, 0x131);
        } else {
            x0 = CLAMP(x0, 0x1F4, 0xCB2);
            x1 = CLAMP(x1, x0 + DispDotClockDivisors[i] * 4, 0xCDA);
            y0 = CLAMP(y0, 0x10, 0x101);
            y1 = CLAMP(y1, y0 + 2, 0x102);
        }
        GpuFuncTable->f10(0x06000000 | ((x1 & 0xFFF) << 12) | (x0 & 0xFFF));
        /* ASM_BARRIER: compute y1 shift in $v1 so 0x07000000 gets $v0 after y0 mask */
        x1 = (y1 & 0x3FF) << 10;
        GpuFuncTable->f10(0x07000000 | x1 | (y0 & 0x3FF));
    }
    PsyqMemcpy(D_800ABB2C, env, 0x14);
    return env;
}
