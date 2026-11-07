#include "rr/overlay662_gradient.h"

RrPolyG4Storage *garage_add_gradient_rect(u32 *ordering, RrPolyG4Storage *packet, u32 x, u32 y, u32 w, u32 h, u32 r0, u32 g0, u32 b0, u32 r1, u32 g1, u32 b1) {
    u16 baseX = garage_logo_state.halfC8;
    u16 baseY = garage_logo_state.halfCA;
    u16 x1;
    packet->packet.len = 8;
    packet->packet.code = 0x38;
    baseX += x;
    baseY += y;
    x1 = baseX + w - 1;
    packet->packet.x0 = baseX;
    packet->packet.y0 = baseY;
    packet->packet.x1 = x1;
    packet->packet.y1 = baseY;
    baseY = baseY + h - 1;
    packet->packet.x2 = baseX;
    packet->packet.y2 = baseY;
    packet->packet.x3 = x1;
    packet->packet.y3 = baseY;
    packet->packet.r0 = r0;
    packet->packet.g0 = g0;
    packet->packet.b0 = b0;
    packet->packet.r1 = r1;
    packet->packet.g1 = g1;
    packet->packet.b1 = b1;
    packet->packet.r2 = r0;
    packet->packet.g2 = g0;
    packet->packet.b2 = b0;
    packet->packet.r3 = r1;
    packet->packet.g3 = g1;
    packet->packet.b3 = b1;
    packet->words[0] = (packet->words[0] & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)packet & 0xFFFFFF);
    return packet + 1;
}
