#include "rr/overlay662_primitives.h"

RrGpuDrawModePacket *garage_add_draw_mode(u32 *ordering,RrGpuDrawModePacket *packet,u32 x,u32 y)
{
    s32 page;
    page = ((s32)((y & 0x100) << 16) >> 20) | ((x & 0x3FF) >> 6) | ((s16)(y & 0x200) * 4);
    SetDrawMode(packet,0,1,page,0);
    packet->tag.word=(packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering=(*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    return packet+1;
}
