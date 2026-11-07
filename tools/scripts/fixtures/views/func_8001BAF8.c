#include "rr/sprite_preset.h"

void EmitPresetSprite(s32 intensity,s32 index) {
    s32 value=intensity;
    s32 selected=index;
    s32 clamped;
    /* ASM_BARRIER: captured packet cursor, independent of scratch pointer reloads. */
    register u8 *packet __asm__("$5");
    /* ASM_BARRIER: table address reuses the dead offset temporary. */
    register RrSpritePreset *preset __asm__("$2");
    /* ASM_BARRIER: keep the packed color in the original word temporary. */
    register u32 hi __asm__("$3");
    u32 middle,offset;
    /* ASM_BARRIER: retain the scratch page across packet and ordering writes. */
    register RrGpuScratchPointers *scratch __asm__("$6");
    if(value!=0) {
        if(value>=0) {
            clamped=value<256;
            if(clamped)goto low;
            clamped=255;
        } else {
low:
            clamped=value;
            if(value<0)clamped=0;
        }
        value=clamped;
        scratch=(RrGpuScratchPointers*)0x1F800000U;
        /* ASM_BARRIER: stage the scratch page before packing the RGB command. */
        __asm__("" : "+r"(scratch));
        hi=(u32)value<<16;
        middle=(u32)value<<8;
        middle|=0x66000000U;
        hi|=middle;
        hi|=value;
        offset=(u32)selected*sizeof(RrSpritePreset);
        packet=scratch->cursor;
        *(u32*)(packet+4)=hi;
        preset=(RrSpritePreset*)((u8*)SpritePresets+offset);
        *(u32*)(packet+8)=preset->position;
        *(u32*)(packet+16)=preset->size;
        *(u32*)(packet+12)=preset->uv;
        *(u32*)packet=scratch->order[1];
        packet[3]=4;
        scratch->order[1]=(u32)(unsigned long)packet;
        ((u8*)&scratch->order[1])[3]=0;
        scratch->cursor=packet+sizeof(RrSpritePacket);
    }
}
