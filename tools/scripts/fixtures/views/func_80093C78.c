/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "rr/draw_environment.h"

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
            over=(GpuVramWidth-1)<signed_side;
            raw_limit=(u16)GpuVramWidth;
            if(over)width=raw_limit-1;
            else width=raw_side;
        }
        rect.rect.w = width;
        signed_side=rect.rect.h;
        raw_side=(u16)rect.rect.h;
        if(signed_side>=0) {
            over=(GpuVramHeight-1)<signed_side;
            raw_limit=(u16)GpuVramHeight;
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
