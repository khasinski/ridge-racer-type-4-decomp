#include "rr/garage_pixels.h"
void garage_blend_image_nibbles(const u32 *source,u32 *destination)
{
    s32 i;
    for(i=0;i<512;i++){
        u32 value=*source;
        u32 mask=0xffffffffU;
        if(value&0xf0000000U)mask=0x0fffffffU;
        if(value&0x0f000000U)mask&=0xf0ffffffU;
        if(value&0x00f00000U)mask&=0xff0fffffU;
        if(value&0x000f0000U)mask&=0xfff0ffffU;
        if(value&0x0000f000U)mask&=0xffff0fffU;
        if(value&0x00000f00U)mask&=0xfffff0ffU;
        if(value&0x000000f0U)mask&=0xffffff0fU;
        if(value&0x0000000fU)mask&=0xfffffff0U;
        *destination &= mask;
        *destination |= *source;
        source++;destination++;
    }
}
