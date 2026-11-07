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

extern u32 D_80112788[10];

extern u32 D_801127B0[4];

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
        over=GpuVramHeight-1<signed_side;
        limit=(u16)GpuVramHeight;
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
