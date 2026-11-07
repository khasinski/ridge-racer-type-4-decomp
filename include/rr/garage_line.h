#ifndef RR_GARAGE_LINE_H
#define RR_GARAGE_LINE_H
#include "rr/garage_garage.h"
#include "rr/gpu_line.h"
/* The origin is captured before packet writes. DMA links encode low24 address
 * bits; actual memory pointers and the returned array cursor retain full width.
 * Ordering may alias an actual tag.word; all loads/stores retain their order. */
RrGpuLineF2Packet *garage_add_vline(u32 *ordering,RrGpuLineF2Packet *packet,
    u32 x,u32 y0,u32 y1,u32 r,u32 g,u32 b);
#endif
