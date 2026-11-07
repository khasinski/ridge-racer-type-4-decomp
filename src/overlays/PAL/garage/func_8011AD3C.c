#include "rr/overlay662_primitives.h"

RrGpuTilePacket *garage_add_hline_double(u32 *ordering, RrGpuTilePacket *packet, u32 x0, u32 y, u32 x1, u32 r, u32 g, u32 b)
{
    /* ASM_BARRIER: keep the origin X offset in $v1 while computing both endpoints. */
    register u32 baseX __asm__("$3") = (u16)garage_logo_state.halfC8;
    u16 baseY = garage_logo_state.halfCA;
    u16 px0;
    /* ASM_BARRIER: keep the right endpoint in $t2 so the 24-bit mask stays in $t1. */
    register u16 px1 __asm__("$10") = x1;

    packet->tag.bytes.length = 3;
    packet->code = 0x40;
    px0 = baseX + x0;
    baseY = baseY + y;
    px1 = px1 + baseX;
    packet->x = px0;
    packet->y = baseY;
    packet->w = px1;
    packet->h = baseY;
    packet->r = r;
    packet->g = g;
    packet->b = b;
    packet->tag.word = (packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    packet++;

    packet->tag.bytes.length = 3;
    packet->code = 0x40;
    baseY = baseY + 1;
    packet->x = px0;
    packet->y = baseY;
    packet->w = px1;
    packet->h = baseY;
    packet->r = r;
    packet->g = g;
    packet->b = b;
    packet->tag.word = (packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    return packet + 1;
}
