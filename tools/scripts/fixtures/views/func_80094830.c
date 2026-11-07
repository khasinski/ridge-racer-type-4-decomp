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
        over=GpuVramWidth<signed_side;
        limit=(u16)GpuVramWidth;
        if(over)side=limit;
    }else side=0;
    signed_side=rect->rect.h;raw_side=(u16)rect->rect.h;
    rect->rect.w=side;
    if(signed_side>=0){
        side=raw_side;
        over=GpuVramHeight<signed_side;
        limit=(u16)GpuVramHeight;
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
