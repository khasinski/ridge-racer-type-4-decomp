#ifndef RR_GARAGE_SCROLL_PANEL_H
#define RR_GARAGE_SCROLL_PANEL_H
#include "rr/animated_sprite_grid.h"
#include "rr/garage_scrollbar.h"
#include "rr/garage_primitives.h"
#include "rr/garage_gradient.h"
/* Capture ordering slot140 before dispatch. Modes46/47 draw the scrollbar,
 * dark track and outline;48..50 emit a black-to-red/green/blue gradient.
 * Other modes emit nothing. The scratch ordering root owns at least141 words
 * even on the default path. The geometry service's fresh cursor feeds the
 * first tile; each emitter's return is published before the next call.
 * Services may replace scratch roots, but all emitters use the captured slot.
 * Packet capacity/alignment and rectangle geometry follow the shared APIs. */
void garage_draw_slider_track(s32 mode);
#endif
