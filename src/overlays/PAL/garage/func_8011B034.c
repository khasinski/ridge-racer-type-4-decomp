#include "rr/garage_primitives.h"

RrGpuTilePacket *garage_add_tile(u32 *ordering, RrGpuTilePacket *packet, u32 x, u32 y, u32 w, u32 h, u32 r, u32 g, u32 b)
{
    u16 baseX=garage_logo_state.halfC8;
    u16 baseY=garage_logo_state.halfCA;
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
