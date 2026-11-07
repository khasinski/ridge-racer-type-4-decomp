/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "m2c_macros.h"
#include "rr/draw_environment.h"
#include "rr/gpu.h"
#include "psyq/libgpu/gpudisp.h"
#include "rr/spu_tick.h"

#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

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

typedef char RrDrawEnvironmentAddressFitsPointer[
    sizeof(unsigned long) >= sizeof(void *) ? 1 : -1];

typedef char RrGpuFillAddressFitsPointer[
    sizeof(unsigned long) >= sizeof(void *) ? 1 : -1];

typedef char RrGpuUploadAddressFitsPointer[
    sizeof(unsigned long) >= sizeof(void *) ? 1 : -1];

typedef char RrGpuDownloadAddressFitsPointer[
    sizeof(unsigned long) >= sizeof(void *) ? 1 : -1];

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
extern u32 D_80112788[10];
extern u32 D_801127B0[4];

DISPENV_T *PutDispEnv(DISPENV_T *env);
int GetDispEnv(int a);
u32 GetODE(void);
void SetTexWindow(void *arg0, M2C_UNK arg1);
void SetDrawArea(void *arg0, void *arg1);
void SetDrawOffset(void *arg0, void *arg1);
void SetPriority(void *arg0, s32 arg1, s32 arg2);
void SetDrawStp(char *p, int a);
void SetDrawMode(void *arg0, s32 arg1, s32 arg2, s32 arg3, RrGpuRectangleFields *arg4);
void SetDrawEnv(u32 *arg0, RrFrameEnvironment *arg1);
void SetDrawEnv2(u32 *arg0, RrFrameEnvironment *arg1);
int get_mode(int a0, int a1, int a2);
u32 get_cs(s32 a0, s32 a1);
u32 get_ce(s32 a0, s32 a1);
unsigned get_ofs(int a, int b);
s32 _status(void);
s32 _otc(u8 *address, s32 count);
s32 _clr(RrGpuRectangle *arg0, u32 arg1);
s32 _dws(RrGpuRectangle *arg0,const u32 *arg1);
s32 _drs(RrGpuRectangle *arg0,u32 *arg1);
void _ctl(u32 command);
s32 _getctl(s32 index);
int _cwb(int *a, int n);

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

/* legacy typing of shared globals in this part of the unit */
#define PsyqMemcpy ((int (*)())PsyqMemcpy)

int GetDispEnv(int a) { PsyqMemcpy(a, D_800ABB2C, 0x14); return a; }

#undef PsyqMemcpy

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

u32 GetODE(void) {
    return M2C_FIELD(GpuFuncTable, u32 (**)(), 0x38)() >> 0x1F;
}

#undef GpuFuncTable

void SetTexWindow(void *arg0, M2C_UNK arg1) {
    M2C_FIELD(arg0, s8 *, 3) = 2;
    M2C_FIELD(arg0, s32 *, 4) = get_tw(arg1);
    M2C_FIELD(arg0, s32 *, 8) = 0;
}

void SetDrawArea(void *arg0, void *arg1) {
    M2C_FIELD(arg0, s8 *, 3) = 2;
    M2C_FIELD(arg0, s32 *, 4) = get_cs(M2C_FIELD(arg1, s16 *, 0), M2C_FIELD(arg1, s16 *, 2));
    M2C_FIELD(arg0, s32 *, 8) = get_ce((s16) (((u16) M2C_FIELD(arg1, s16 *, 0) + M2C_FIELD(arg1, u16 *, 4)) - 1), (s16) (((u16) M2C_FIELD(arg1, s16 *, 2) + M2C_FIELD(arg1, u16 *, 6)) - 1));
}

void SetDrawOffset(void *arg0, void *arg1) {
    M2C_FIELD(arg0, s8 *, 3) = 2;
    M2C_FIELD(arg0, s32 *, 4) = get_ofs(M2C_FIELD(arg1, s16 *, 0), M2C_FIELD(arg1, s16 *, 2));
    M2C_FIELD(arg0, s32 *, 8) = 0;
}

void SetPriority(void *arg0, s32 arg1, s32 arg2) {
    s32 var_v1;

    M2C_FIELD(arg0, s8 *, 3) = 2;
    var_v1 = 0xE6000000;
    if (arg1 != 0) {
        var_v1 = 0xE6000002;
    }
    M2C_FIELD(arg0, s32 *, 4) = (s32) (var_v1 | (arg2 != 0));
    M2C_FIELD(arg0, s32 *, 8) = 0;
}

void SetDrawStp(char *p, int a) { p[3] = 2; ((int *)p)[1] = a ? 0xE6000001 : 0xE6000000; ((int *)p)[2] = 0; }

void SetDrawMode(void *arg0, s32 arg1, s32 arg2, s32 arg3, RrGpuRectangleFields *arg4) {
    M2C_FIELD(arg0, s8 *, 3) = 2;
    M2C_FIELD(arg0, s32 *, 4) = get_mode(arg1, arg2, arg3 & 0xFFFF);
    M2C_FIELD(arg0, s32 *, 8) = get_tw(arg4);
}

void SetDrawEnv(u32 *arg0, RrFrameEnvironment *arg1) {
    register RrFrameEnvironment *env=arg1;
    register u32 *output=arg0;
    /* ASM_BARRIER: retain this arithmetic or packet-address phase. */
    register s32 high asm("$2");
    RrGpuRectangle rect;
    /* ASM_BARRIER: retain this arithmetic or packet-address phase. */
    register u32 color_offset asm("$6"),xy_offset asm("$7"),size_offset;
    register u32 blue,green;
    s32 word_count;
    /* ASM_BARRIER: retain this arithmetic or packet-address phase. */
    register s32 signed_side asm("$3");
    register s32 over;
    /* ASM_BARRIER: retain this arithmetic or packet-address phase. */
    register s32 raw_limit asm("$3");
    register s32 raw_side;
    /* ASM_BARRIER: retain this arithmetic or packet-address phase. */
    register s32 width asm("$2");
    /* ASM_BARRIER: retain this arithmetic or packet-address phase. */
    register s32 height asm("$2");

    output[1] = get_cs(env->clip.x, env->clip.y);
    output[2] = get_ce((s16) ((((u16)env->clip.w) + (u16) env->clip.x) - 1), (s16) (((u16) env->clip.y + ((u16)env->clip.h)) - 1));
    output[3] = get_ofs(env->offset_x, env->offset_y);
    output[4] = get_mode(env->draw_display, env->dither, ((u16)env->page));
    output[5] = get_tw(&env->window);
    output[6] = 0xE6000000;
    word_count = 7;
    if (env->rgb24 != 0) {
        rect.rect.x = (u16) env->clip.x;
        rect.rect.y = (u16) env->clip.y;
        raw_side = ((u16)env->clip.w);
        rect.rect.w = raw_side;
        rect.rect.h = ((u16)env->clip.h);
        /* ASM_BARRIER: finish the four halfword copies before sign extraction. */
        __asm__("" : : "m"(rect.rect.h));
        high=(s32)((u32)raw_side<<16);
        signed_side=high>>16;
        width=0;
        if(signed_side>=0) {
            over=(D_800ABAC4-1)<signed_side;
            raw_limit=(u16)D_800ABAC4;
            if(over)width=raw_limit-1;
            else width=raw_side;
        }
        rect.rect.w = width;
        signed_side=rect.rect.h;
        raw_side=(u16)rect.rect.h;
        if(signed_side>=0) {
            over=(D_800ABAC6-1)<signed_side;
            raw_limit=(u16)D_800ABAC6;
            if(over)height=raw_limit-1;
            else height=raw_side;
        }else height=0;
        color_offset=(u32)word_count++*4U;
        xy_offset=(u32)word_count++*4U;
        size_offset=(u32)word_count++*4U;

        rect.rect.h = height;
        rect.rect.x -= (u16) env->offset_x;
        /* ASM_BARRIER: prepare the color destination before the Y/command phase. */
        __asm__("" : : "m"(output[color_offset/4U]));
        rect.rect.y -= (u16) env->offset_y;
        blue=(u32)env->b<<16;
        green=((u32)env->g<<8)|0x60000000U;

        output[color_offset/4U] = blue | green | env->r;
        output[xy_offset/4U] = rect.words[0];
        output[size_offset/4U] = rect.words[1];
    }
    ((u8*)output)[3] = (s8) (word_count - 1);
}

void SetDrawEnv2(u32 *arg0, RrFrameEnvironment *arg1) {
    register RrFrameEnvironment *env=arg1;
    register u32 *output=arg0;
    /* ASM_BARRIER: retain the original width sign-extraction register. */
    register s32 high asm("$2");
    RrGpuRectangle rect;
    /* ASM_BARRIER: retain packet-offset and RGB arithmetic registers. */
    register u32 color_offset,xy_offset asm("$7"),size_offset;
    register u32 blue asm("$2"),green asm("$3");
    s32 word_count;
    /* ASM_BARRIER: retain signed/unsigned clamp phases in their registers. */
    register s32 signed_side asm("$3");
    register s32 over;
    /* ASM_BARRIER: retain the unsigned limit reload register. */
    register s32 raw_limit asm("$3");
    register s32 raw_side;
    /* ASM_BARRIER: retain the width/height result registers. */
    register s32 width asm("$2");
    register s32 height asm("$2");

    output[1] = get_cs(env->clip.x, env->clip.y);
    output[2] = get_ce((s16) ((((u16)env->clip.w) + (u16) env->clip.x) - 1), (s16) (((u16) env->clip.y + ((u16)env->clip.h)) - 1));
    output[3] = get_ofs(env->offset_x, env->offset_y);
    output[4] = get_mode(env->draw_display, env->dither, ((u16)env->page));
    output[5] = get_tw(&env->window);
    output[6] = 0xE6000000;
    word_count = 7;
    if (env->rgb24 != 0) {
        rect.rect.x = (u16) env->clip.x;
        rect.rect.y = (u16) env->clip.y;
        raw_side = ((u16)env->clip.w);
        rect.rect.w = raw_side;
        rect.rect.h = ((u16)env->clip.h);
        /* ASM_BARRIER: finish the four halfword copies before sign extraction. */
        __asm__("" : : "m"(((u8*)&rect.rect.h)[0]));
        high=(s32)((u32)raw_side<<16);
        signed_side=high>>16;
        width=0;
        if(signed_side>=0) {
            over=(D_800ABAC4-1)<signed_side;
            raw_limit=(u16)D_800ABAC4;
            if(over)width=raw_limit-1;
            else width=raw_side;
        }
        rect.rect.w = width;
        signed_side=rect.rect.h;
        raw_side=(u16)rect.rect.h;
        if(signed_side>=0) {
            over=(D_800ABAC6-1)<signed_side;
            raw_limit=(u16)D_800ABAC6;
            if(over)height=raw_limit-1;
            else height=raw_side;
        }else height=0;
        rect.rect.h=height;
        color_offset=(u32)word_count*4U;
        if(((u16)rect.rect.x&63U)||((u16)rect.rect.w&63U)) {
            word_count++;
            xy_offset=(u32)word_count++*4U;
            size_offset=(u32)word_count++*4U;
            rect.rect.x-=(u16)env->offset_x;
            /* ASM_BARRIER: prepare the color address before Y/command arithmetic. */
            __asm__("" : : "m"((*(u8 *)((unsigned long)color_offset+(unsigned long)output))));
            {
                register u32 y=(u16)rect.rect.y;
                register u32 offset_y=(u16)env->offset_y;
                register u32 command;
                command=0x60000000U;
                rect.rect.y=y-offset_y;
                blue=(u32)env->b<<16;
                green=((u32)env->g<<8)|command;
                (*(u32 *)((unsigned long)color_offset+(unsigned long)output))=blue|green|env->r;
                output[xy_offset/4U]=rect.words[0];
                output[size_offset/4U]=rect.words[1];
            }
        } else {
            register u32 fast_color,fast_xy,fast_size;
            /* Modulo32 byte offset: masking before the shift also keeps this
             * preparation independent of the extended branch's offset. */
            fast_color=((u32)word_count++&0x3FFFFFFFU)<<2;
            fast_xy=(u32)word_count++*4U;
            fast_size=(u32)word_count++*4U;
            /* ASM_BARRIER: prepare the fast color address before its command. */
            __asm__("" : : "m"((*(u8 *)((unsigned long)fast_color+(unsigned long)output))));
            {
                register u32 command=0x02000000U;
                blue=(u32)env->b<<16;
                green=((u32)env->g<<8)|command;
                (*(u32 *)((unsigned long)fast_color+(unsigned long)output))=blue|green|env->r;
                output[fast_xy/4U]=rect.words[0];
                output[fast_size/4U]=rect.words[1];
            }
        }
    }
    ((u8*)output)[3] = (s8) (word_count - 1);
}

int get_mode(int a0, int a1, int a2) {
    int v1;
    register int v0 asm("$2");
    v1 = 0xE1000000;
    if (a1 != 0) {
        v1 = 0xE1000200;
    }
    v0 = a2 & 0x9FF;
    if (a0 != 0) {
        v0 |= 0x400;
    }
    return v1 | v0;
}

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
        c = D_800ABAC4 - 1 < coordinate;
        /* ASM_BARRIER: Preserve the separate unsigned limit reload after the signed comparison. */
        __asm__ volatile("" : "=r"(c) : "0"(c) : "memory");
        t = *(u16 *)&D_800ABAC4;
        if (c) v = t - 1; else v = a0;
    } else v=0;
    a0=v;
    shifted=(s32)((u32)a1<<16);
    coordinate=shifted>>16;
    if (coordinate >= 0) {
        c = D_800ABAC6 - 1 < coordinate;
        /* ASM_BARRIER: Preserve the separate unsigned limit reload after the signed comparison. */
        __asm__ volatile("" : "=r"(c) : "0"(c) : "memory");
        t = *(u16 *)&D_800ABAC6;
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

u32 get_ce(s32 a0, s32 a1) {
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
        c = D_800ABAC4 - 1 < coordinate;
        /* ASM_BARRIER: Preserve the separate unsigned limit reload after the signed comparison. */
        __asm__ volatile("" : "=r"(c) : "0"(c) : "memory");
        t = *(u16 *)&D_800ABAC4;
        if (c) v = t - 1; else v = a0;
    } else v=0;
    a0=v;
    shifted=(s32)((u32)a1<<16);
    coordinate=shifted>>16;
    if (coordinate >= 0) {
        c = D_800ABAC6 - 1 < coordinate;
        /* ASM_BARRIER: Preserve the separate unsigned limit reload after the signed comparison. */
        __asm__ volatile("" : "=r"(c) : "0"(c) : "memory");
        t = *(u16 *)&D_800ABAC6;
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
    a0=(s32)0xE4000000U;
    v |= a0;
    return u | v;
}

unsigned get_ofs(int a, int b) { return 0xE5000000 | (b & 0x7FF) << 11 | (a & 0x7FF); }

u32 get_tw(RrGpuRectangleFields *a0)
{
  int v[4];
  int new_var;
  unsigned r;
  if (a0 == 0)
  {
    r = 0;
  }
  else
  {
    v[0] = ((u8 *)&a0->x)[0] >> 3;
    v[2] = ((-a0->w) & 0xFF) >> 3;
    v[1] = ((u8 *)&a0->y)[0] >> 3;
    v[3] = ((-a0->h) & 0xFF) >> 3;
    r = (((v[1] << 15) | ((v[0] << 10) | (new_var = 0xE2000000))) | (v[3] << 5)) | v[2];
  }
  return r;
}

/* legacy typing of shared globals in this part of the unit */
#define GPU_STATUS ((s32 *)GPU_STATUS)

s32 _status(void) {
    return *GPU_STATUS;
}

#undef GPU_STATUS

s32 _otc(u8 *address, s32 count) {
    u32 offset;
    u32 *configuration=DPCR;
    *configuration |= 0x08000000;
    *DMA6_CHCR = 0;
    offset = (u32)count * 4;
    offset -= 4;
    address=(u8 *)((unsigned long)address+(long)(s32)offset);
    *DMA6_MADR = address;
    *DMA6_BCR = count;
    /* The final control store occupies the call delay slot in the original. */
    *(u32 *)DMA6_CHCR = 0x11000002;
    set_alarm();
    if (*DMA6_CHCR & 0x01000000) {
        /* ASM_BARRIER: Retire the first mask and prevent hoisting the loop mask across the busy gate. */
        __asm__("" : : : "$3", "$17");
        {
            u32 mask = 0x01000000;
            do {
                if (get_alarm() != 0) return -1;
            } while (*DMA6_CHCR & mask);
        }
    }
    return count;
}

s32 _clr(RrGpuRectangle *arg0, u32 arg1) {
    register RrGpuRectangle *rect=arg0;
    register u32 color;
    /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
    register s32 signed_side asm("$3");
    register u32 raw_side;
    /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
    register s32 over,width asm("$2");
    /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
    register s32 limit asm("$3"),height asm("$3");
    signed_side=rect->rect.w;raw_side=(u16)rect->rect.w;
    color=arg1;
    if(signed_side>=0){
        over=D_800ABAC4-1<signed_side;
        limit=(u16)D_800ABAC4;
        if(over)width=limit-1;else width=raw_side;
    }else width=0;
    signed_side=rect->rect.h;raw_side=(u16)rect->rect.h;
    rect->rect.w=width;
    if(signed_side>=0){
        over=D_800ABAC6-1<signed_side;
        limit=(u16)D_800ABAC6;
        if(over)height=limit-1;else height=raw_side;
    }else height=0;
    rect->rect.h=height;
    if(((u16)rect->rect.x&63)||((u16)rect->rect.w&63)){
        register u32 mask=0xFFFFFFU;
        register u32 full_area=0xE4FFFFFFU;
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 tail_tag asm("$7")=0x03FF0000U;
        register u32 *packet=D_80112788;
        u32 *secondary=D_801127B0;
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 link asm("$2");
        register u32 e3,e5;
        register u32 command;
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 fill asm("$2");
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register volatile u32 *status_ptr asm("$2");
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 status,high asm("$2");
        register u32 xy;
        /* ASM_BARRIER: prepare the primary packet before completing the secondary header. */
        __asm__("" : : "r"(tail_tag),"r"(packet));
        /* The DMA link is a24-bit wire address; other pointers stay full width. */
        link=(unsigned long)secondary&mask;
        link|=0x08000000U;
        e3=0xE3000000U;e5=0xE5000000U;
        /* ASM_BARRIER: prepare both restore opcodes before storing the primary link. */
        __asm__("" : : "r"(e3));
        packet[0]=link;
        fill=0xE6000000U;
        command=color&mask;
        command|=0x60000000U;
        packet[4]=fill;
        status_ptr=GPU_STATUS;
        packet[1]=e3;packet[2]=full_area;packet[3]=e5;
        status=*status_ptr;
        high=(color>>31)<<10;
        packet[6]=command;
        high|=0xE1000000U;
        status&=0x7FF;
        status|=high;
        packet[5]=status;
        xy=rect->words[0];
        tail_tag|=0xFFFFU;
        packet[7]=xy;
        secondary[0]=tail_tag;
        packet[8]=rect->words[1];
        secondary[1] = (s32) (_param(3) | e3);
        secondary[2] = (s32) (_param(4) | 0xE4000000);
        secondary[3] = (s32) (_param(5) | e5);
    }else{
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 header asm("$2")=0x05FFFFFFU;
        register u32 mask=0xFFFFFFU;
        register u32 *packet=D_80112788;
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 encoded asm("$3");
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 textured asm("$5");
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 e1 asm("$3");
        /* ASM_BARRIER: retain the original clamp, packet or command register phase. */
        register u32 fill asm("$2");
        register volatile u32 *status_ptr;
        register u32 status,high;
        packet[0]=header;
        fill=0xE6000000U;
        packet[1]=fill;
        encoded=color&mask;
        textured=0x02000000U;
        /* ASM_BARRIER: prepare the fast fill opcode before reading GPU status. */
        __asm__("" : : "r"(textured));
        encoded|=textured;
        status_ptr=GPU_STATUS;
        status=*status_ptr;
        high=(color>>31)<<10;
        packet[3]=encoded;
        e1=0xE1000000U;
        high|=e1;
        status&=0x7FF;
        status|=high;
        packet[2]=status;
        packet[4]=rect->words[0];
        packet[5]=rect->words[1];
    }
    _cwc((unsigned long)D_80112788);
    {
        register s32 result=0;
        /* ASM_BARRIER: form the zero return value before restoring saved registers. */
        __asm__("" : "+r"(result));
        return result;
    }
}

/* The small CPU tail uses compiler-fenced stores so GCC can place the final
 * write in a branch delay slot. The per-iteration and final memory fences
 * make every port write visible and ordered before subsequent accesses. */
s32 _dws(RrGpuRectangle *arg0,const u32 *arg1) {
    register RrGpuRectangle *rect=arg0;
    register const u32 *cursor;
    register s32 signed_side;
    /* ASM_BARRIER: retain the raw halfword reload register. */
    register u32 raw_side asm("$3");
    register u32 side;
    register s32 over;
    register u32 limit;
    /* ASM_BARRIER: retain the adjusted word-count arithmetic register. */
    register s32 count,blocks,remainder,adjusted asm("$3");
    /* ASM_BARRIER: retain the preliminary count and captured quotient phases. */
    register s32 preliminary asm("$16");
    register s32 quotient asm("$3");
    register s32 scaled;
    register volatile u32 *status_ptr;
    register u32 status;
    register s32 mode;
    u32 command;
    cursor=arg1;
    /* ASM_BARRIER: prepare both input pointers before starting the deadline. */
    __asm__("" : "+r"(cursor) : "r"(rect));
    set_alarm();
    signed_side=rect->rect.w;raw_side=(u16)rect->rect.w;
    mode=0;
    if(signed_side>=0){
        side=raw_side;
        over=D_800ABAC4<signed_side;
        limit=(u16)D_800ABAC4;
        if(over)side=limit;
    }else side=0;
    signed_side=rect->rect.h;raw_side=(u16)rect->rect.h;
    rect->rect.w=side;
    if(signed_side>=0){
        side=raw_side;
        over=D_800ABAC6<signed_side;
        limit=(u16)D_800ABAC6;
        if(over)side=limit;
    }else side=0;
    adjusted=rect->rect.w*(s16)side+1;
    rect->rect.h=side;
    adjusted+=(u32)adjusted>>31;
    count=adjusted>>1;
    preliminary=adjusted>>5;
    if(count<=0)return -1;
    quotient=preliminary;
    /* ASM_BARRIER: capture the block quotient before forming the CPU tail. */
    __asm__("" : "+r"(quotient));
    scaled=quotient*16;
    remainder=count-scaled;
    status_ptr=GPU_STATUS;
    blocks=quotient;
    status=*status_ptr;
    if(!(status&0x04000000U)){
        register u32 mask;
        /* ASM_BARRIER: materialize a fresh mask after the first status test. */
        __asm__("" : : : "$3");
        mask=0x04000000U;
        do {
            if(get_alarm())return -1;
        }while(!(*GPU_STATUS&mask));
    }
    command=0xA0000000U;
    *GPU_STATUS=0x04000000U;
    *GPU_DATA=0x01000000U;
    {
        register volatile u32 *port=GPU_DATA;
        if(mode)command=0xB0000000U;
        *port=command;
    }
    *GPU_DATA=rect->words[0];
    *GPU_DATA=rect->words[1];
    remainder--;
    if(remainder!=-1){
        register s32 sentinel;
        /* ASM_BARRIER: form the loop sentinel independently of the return value. */
        __asm__("" : : : "$2");
        sentinel=-1;
        do {
            register u32 word;
            /* ASM_BARRIER: expose each preceding port write before new input loads. */
            __asm__ volatile("" : : : "memory");
            word=*cursor++;
            remainder--;
            *(u32 *)GPU_DATA=word;
        }while(remainder!=sentinel);
    }
    /* ASM_BARRIER: expose the final port write before configuring DMA. */
    __asm__ volatile("" : : : "memory");
    {
        register u32 dma_mode=0x04000000U;
        if(blocks){
            /* ASM_BARRIER: retain DMA register-address and word-count phases. */
            register volatile u32 *port asm("$2")=GPU_STATUS;
            register u32 start;
            register volatile u32 *length_port;
            register u32 length asm("$2");
            dma_mode|=2U;
            *port=dma_mode;
            port=DMA2_MADR;
            start=0x01000000U;
            /* DMA hardware consumes a32-bit bus-address word; keep CPU pointers full. */
            *port=(u32)(unsigned long)cursor;
            length=(u32)blocks<<16;
            length_port=DMA2_BCR;
            length|=16U;
            *length_port=length;
            port=DMA2_CHCR;
            start|=0x201U;
            *port=start;
        }
    }
    return 0;
}

s32 _drs(RrGpuRectangle *arg0,u32 *arg1) {
    register RrGpuRectangle *rect=arg0;
    register u32 *cursor;
    register s32 signed_side;
    /* ASM_BARRIER: retain the raw halfword reload register. */
    register u32 raw_side asm("$3");
    register u32 side;
    register s32 over;
    register u32 limit;
    /* ASM_BARRIER: retain the adjusted word-count arithmetic register. */
    register s32 count,blocks,remainder,adjusted asm("$3");
    /* ASM_BARRIER: retain the preliminary count and captured quotient phases. */
    register s32 preliminary asm("$16");
    register s32 quotient asm("$3");
    register s32 scaled;
    register volatile u32 *status_ptr;
    register u32 status;
    cursor=arg1;
    /* ASM_BARRIER: prepare rectangle and output pointers before starting the deadline. */
    __asm__("" : "+r"(cursor) : "r"(rect));
    set_alarm();
    signed_side=rect->rect.w;raw_side=(u16)rect->rect.w;
    if(signed_side>=0){
        side=raw_side;
        over=D_800ABAC4<signed_side;
        limit=(u16)D_800ABAC4;
        if(over)side=limit;
    }else side=0;
    signed_side=rect->rect.h;raw_side=(u16)rect->rect.h;
    rect->rect.w=side;
    if(signed_side>=0){
        side=raw_side;
        over=D_800ABAC6<signed_side;
        limit=(u16)D_800ABAC6;
        if(over)side=limit;
    }else side=0;
    adjusted=rect->rect.w*(s16)side+1;
    rect->rect.h=side;
    adjusted+=(u32)adjusted>>31;
    count=adjusted>>1;
    preliminary=adjusted>>5;
    if(count<=0)return -1;
    quotient=preliminary;
    /* ASM_BARRIER: capture the block quotient before forming the CPU tail. */
    __asm__("" : "+r"(quotient));
    scaled=quotient*16;
    remainder=count-scaled;
    status_ptr=GPU_STATUS;
    blocks=quotient;
    status=*status_ptr;
    if(!(status&0x04000000U)){
        register u32 mask;
        /* ASM_BARRIER: materialize a fresh mask after the first status test. */
        __asm__("" : : : "$3");
        mask=0x04000000U;
        do {
            if(get_alarm())return -1;
        }while(!(*GPU_STATUS&mask));
    }
    *GPU_STATUS=0x04000000U;
    *GPU_DATA=0x01000000U;
    *GPU_DATA=0xC0000000U;
    *GPU_DATA=rect->words[0];
    *GPU_DATA=rect->words[1];
    if(!(*GPU_STATUS&0x08000000U)) {
        u32 mask;
        /* ASM_BARRIER: materialize the receive mask after its first status test. */
        __asm__("" : : : "$3");
        mask=0x08000000U;
        do {
            if(get_alarm())return -1;
        }while(!(*GPU_STATUS&mask));
    }
    {
        register s32 empty;
        remainder--;
        empty=-1;
        /* ASM_BARRIER: keep entry and loop sentinels in separate arithmetic phases. */
        __asm__("" : "+r"(empty));
        if(remainder!=empty){
            register s32 sentinel;
            sentinel=-1;
            do {
                u32 word=*GPU_DATA;
                remainder--;
                *cursor++=word;
            }while(remainder!=sentinel);
        }
    }
    {
        register u32 dma_mode=0x04000000U;
        if(blocks){
            /* ASM_BARRIER: retain DMA register-address and word-count phases. */
            register volatile u32 *port asm("$2")=GPU_STATUS;
            register u32 start;
            register volatile u32 *length_port;
            register u32 length asm("$2");
            dma_mode|=3U;
            *port=dma_mode;
            port=DMA2_MADR;
            start=0x01000000U;
            /* DMA hardware consumes a 32-bit bus address word; keep CPU pointers full. */
            *port=(u32)(unsigned long)cursor;
            length=(u32)blocks<<16;
            length_port=DMA2_BCR;
            length|=16U;
            *length_port=length;
            port=DMA2_CHCR;
            start|=0x200U;
            *port=start;
        }
    }
    return 0;
}

/* legacy typing of shared globals in this part of the unit */
#define GPU_STATUS ((u32 *)GPU_STATUS)

void _ctl(u32 command) {
    /* ASM_BARRIER: reuse v0 for the port pointer and then the command index. */
    register u32 index __asm__("$2");
    /* ASM_BARRIER: retain the original assembler temporary for the history address. */
    register u8 *base __asm__("$1");
    {
        /* ASM_BARRIER: keep the command-port pointer in v0 until the word is written. */
        register u32 *port __asm__("$2") = GPU_STATUS;
        *port = command;
    }
    index = command >> 24;
    /* ASM_BARRIER: finish the port write before binding the history address. */
    __asm__ volatile("" : "=r"(index) : "0"(index) : "$1", "memory");
    /* PAL RAM mapping: D_801127D0 is base plus 0x27D0. */
    base = (u8 *)0x80110000U;
    /* ASM_BARRIER: retain the base-plus-index byte store in the return delay slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base += index;
    base[0x27D0] = command;
}

#undef GPU_STATUS

s32 _getctl(s32 index) {
    /* PAL RAM mapping: D_801127D0 is 0x27D0 bytes into this page. */
    u8 *base = (u8 *)0x80110000U;
    /* ASM_BARRIER: retain the base register through the indexed byte load. */
    __asm__("" : "=r"(base) : "0"(base));
    base += index;
    return base[0x27D0];
}

/* legacy typing of shared globals in this part of the unit */
#define GPU_DATA ((int *)GPU_DATA)
#define GPU_STATUS ((int *)GPU_STATUS)

int _cwb(int *a, int n) {
    int i = n - 1;
    *GPU_STATUS = 0x04000000;
    if (n) {
        do { int v = *a; a++; i--; *GPU_DATA = v; } while (i != -1);
    }
    return 0;
}

#undef GPU_DATA
#undef GPU_STATUS

void _cwc(unsigned long address) {
    *GPU_STATUS=0x04000002;
    *DMA2_MADR=(u32)address;
    *DMA2_BCR=0;
    *(u32 *)DMA2_CHCR=0x01000401;
}

s32 _param(s32 a) {
    *GPU_STATUS = a | 0x10000000;
    return *GPU_DATA & 0xFFFFFF;
}
