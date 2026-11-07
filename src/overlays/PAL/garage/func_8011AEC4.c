#include "rr/garage_primitives.h"

RrGpuLineF4Packet *garage_add_rect_outline(u32 *ordering, RrGpuLineF4Packet *packet, u32 x, u32 y, u32 w, u32 h, u32 r, u32 g, u32 b)
{
    u16 baseX = garage_logo_state.halfC8;
    u16 baseY = garage_logo_state.halfCA;
    u16 xEnd;
    u16 yEnd;

    packet->tag.bytes.length = 6;
    packet->code = 0x4C;
    packet->pad = 0x55555555;
    packet->code &= ~2;
    baseX += x;
    baseY += y;
    xEnd = baseX + w;
    yEnd = baseY + h;
    packet->x0 = baseX;
    packet->y0 = baseY;
    packet->x1 = xEnd - 1;
    packet->y1 = baseY;
    packet->x2 = xEnd - 1;
    packet->y2 = yEnd - 1;
    packet->x3 = baseX;
    packet->y3 = yEnd - 1;
    packet->r = r;
    packet->g = g;
    packet->b = b;
    packet->tag.word = (packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    packet++;

    packet->tag.bytes.length = 6;
    packet->code = 0x4C;
    packet->pad = 0x55555555;
    packet->code &= ~2;
    xEnd -= 2;
    baseY += 1;
    yEnd -= 2;
    packet->x0 = xEnd;
    packet->y0 = baseY;
    packet->x1 = baseX;
    packet->y1 = baseY;
    packet->x2 = baseX;
    packet->y2 = yEnd;
    packet->x3 = xEnd;
    packet->y3 = yEnd;
    packet->r = r;
    packet->g = g;
    packet->b = b;
    packet->tag.word = (packet->tag.word & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    return packet + 1;
}
