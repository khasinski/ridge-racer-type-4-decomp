/* CC1_FLAGS: -fno-strength-reduce */

#include "rr/animated_sprite_grid.h"

#define G IntroSceneState

#define H BadgeRows

#define K DialogueState

#define SPP (*(struct P **)0x1F800000)

#define SPO (*(struct P **)0x1F800004)

#define SPAD ((int *)0x1F800000)

#define CURSOR (scratch->cursor)

#define ORDER (order_page->order)

void EmitSpriteGrid(u32 intensity,RrSpriteGridClock *state) {
    s32 row;
    /* ASM_BARRIER: retain the horizontal position counter in a3. */
    register s32 x __asm__("$7");
    s32 phase;
    s32 index;
    u32 hi;
    u32 middle;
    u32 size;
    u32 uv;
    s32 limit;
    s32 length;
    s32 quotient;
    /* ASM_BARRIER: materialize the packed row step only after each sprite row. */
    register u32 row_step __asm__("$2");
    u32 mode_command;
    u32 y;
    /* ASM_BARRIER: keep the raw/color word in its original temporary. */
    register u32 color __asm__("$9")=intensity;
    RrGpuScratchPointers *order_page;
    RrGpuScratchPointers *scratch;
    u8 *packet;
    u8 *fields;
    hi=color<<16;
    middle=color<<8;
    middle|=0x66000000U;
    hi|=middle;
    color|=hi;
    /* ASM_BARRIER: finish color packing before initializing loop constants. */
    __asm__("" : "+r"(color) : : "$15");
    row=0;
    /* ASM_BARRIER: keep the row counter distinct from later coordinate zero. */
    __asm__("" : "+r"(row));
    limit=15;size=0x180010;uv=0x2F29A400U;
    order_page=(RrGpuScratchPointers*)0x1F800000U;
    /* ASM_BARRIER: stage the scratch page before the packed-y constant. */
    __asm__("" : "+r"(order_page) : : "$8");
    length=4;
    y=0xB00000;
    packet=*(u8**)0x1F800000U;
    state->tick0E++;
    phase=state->tick0E;
    /* ASM_BARRIER: capture tick phase before starting the horizontal counter. */
    __asm__("" : "+r"(phase) : : "$7");
    x=0;
    do {
        fields=packet+3;
        do {
            quotient=phase;
            if(phase<0)quotient=phase+3;
            index=(quotient>>2)&15;
            if(index==limit)index=0;
            *(u32*)(fields+5)=y|(x+172);
            *(u32*)(fields+1)=color;
            *(u32*)(fields+13)=size;
            *(u32*)(fields+9)=index*16+uv;
            phase+=5;x+=26;
            *(u32*)packet=*ORDER;
            fields[0]=length;
            fields+=sizeof(RrSpritePacket);
            *ORDER=(u32)(unsigned long)packet;
            ((u8*)ORDER)[3]=0;
            packet+=sizeof(RrSpritePacket);
        }while(x<130);
        row_step=0x300000;
        y+=row_step;
        row+=48;x=0;
    }while(row<144);
    mode_command=0xE100022A;
    scratch=(RrGpuScratchPointers*)0x1F800000U;
    CURSOR=packet;
    *(u32*)(packet+4)=mode_command;
    *(u32*)CURSOR=*scratch->order;
    CURSOR[3]=1;
    *scratch->order=(u32)(unsigned long)CURSOR;
    ((u8*)scratch->order)[3]=0;
    CURSOR+=sizeof(RrGpuSingleCommandPacket);
}
