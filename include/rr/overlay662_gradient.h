#ifndef RR_OVERLAY662_GRADIENT_H
#define RR_OVERLAY662_GRADIENT_H
#include "rr/overlay662_garage.h"
#include "rr/gouraud_quad.h"
/* Capture origin, retain low16 coordinate/extent results and low8 colors,
 * then prepend the packet to the low24 DMA chain. RGB varies left-to-right;
 * the three unused color-padding bytes survive. Ordering may name a real word
 * in the supplied storage. Packet must belong to an array for the next cursor. */
RrPolyG4Storage *garage_add_gradient_rect(u32 *ordering,RrPolyG4Storage *packet,
    u32 x,u32 y,u32 width,u32 height,u32 r0,u32 g0,u32 b0,
    u32 r1,u32 g1,u32 b1);
#endif
