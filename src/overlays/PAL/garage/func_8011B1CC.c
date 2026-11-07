#include "rr/garage_primitives.h"

RrGpuPolyFT4Packet *garage_add_textured_rect(u32 *ordering, RrGpuPolyFT4Packet *packet, u32 x, u32 y, u32 w, u32 h, u32 u, u32 v, u32 uw, u32 vh, u32 clut, u32 tpage)
{
    u16 baseX = garage_logo_state.halfC8;
    u16 baseY = garage_logo_state.halfCA;
    u16 x1;
    u16 u1;
    u16 v1;

    packet->tag.bytes.length = 9;
    packet->code = 0x2D;
    u1 = u + uw - 1;
    v1 = v + vh - 1;
    baseX += x;
    baseY += y;
    x1 = baseX + w - 1;
    packet->x0 = baseX;
    packet->y0 = baseY;
    packet->x1 = x1;
    packet->y1 = baseY;
    baseY = baseY + h - 1;
    packet->x2 = baseX;
    packet->y2 = baseY;
    packet->x3 = x1;
    packet->y3 = baseY;
    packet->u0 = u;
    packet->v0 = v;
    packet->u1 = u1;
    packet->v1 = v;
    packet->u2 = u;
    packet->v2 = v1;
    packet->u3 = u1;
    packet->v3 = v1;
    packet->clut = clut;
    packet->tpage = tpage;
    packet->tag.word = (packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    return packet + 1;
}
