#ifndef RR_OVERLAY662_DRAW_AREA_H
#define RR_OVERLAY662_DRAW_AREA_H
#include "rr/overlay662_garage.h"
#include "rr/frame_loop.h"
/* Resident constructor consumes an eight-byte rectangle and emits three words. */
void SetDrawArea(void *packet,void *rectangle);
/* Packet must have three writable words. The head may alias packet words;
 * links use PAL low24 address bits, while the returned cursor stays full-width. */
u8 *garage_add_draw_area(u32 *ordering,u32 *packet,u32 x,u32 y,u16 w,u16 h);
#endif
