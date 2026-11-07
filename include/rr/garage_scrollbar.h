#ifndef RR_GARAGE_SCROLLBAR_H
#define RR_GARAGE_SCROLLBAR_H
#include "rr/garage_garage.h"
#include "rr/gpu.h"
/* Caller supplies a separate four-halfword rectangle. The zoom cycle is
 *2,4,8,16; more generally2 or4..255 have defined nonzero divisors. Zero mode
 *selects the horizontal bar, every other full signed word the vertical one.
 *Coordinates narrow to PAL halfwords after signed32 geometry arithmetic. */
void garage_get_scrollbar_thumb(s32 mode,RrGpuRectangleFields *rectangle);
#endif
