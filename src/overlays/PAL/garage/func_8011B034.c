#include "rr/overlay662_primitives.h"

RrGpuTilePacket *garage_func_8011B034(u32 *ordering, RrGpuTilePacket *packet, u32 x, u32 y, u32 w, u32 h, u32 r, u32 g, u32 b)
{
    u16 baseX=garage_D_801276A8.halfC8;
    u16 baseY=garage_D_801276A8.halfCA;
    packet->tag.bytes.length=3;
    packet->code=0x60;
    packet->x=baseX+x;
    packet->y=baseY+y;
    packet->w=w;
    packet->h=h;
    packet->r=r;packet->g=g;packet->b=b;
    packet->tag.word=(packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering=(*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    return packet+1;
}
