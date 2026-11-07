#ifndef RR_GARAGE_SCALE_PANEL_H
#define RR_GARAGE_SCALE_PANEL_H
#include "rr/garage_primitives.h"
#include "rr/animated_sprite_grid.h"
#ifndef RR_GARAGE_SCRATCH
#define RR_GARAGE_SCRATCH ((RrGpuScratchPointers*)0x1F800000U)
#endif
/* config1C must be2..255 at both reads. Capture scale and ordering before
 * callbacks; thread each full returned cursor through the shared scratch cell. */
/* Action record51 supplies its word argument(currently0); it is ignored. */
void garage_draw_preview_window(s32 renderArgument);
#endif
