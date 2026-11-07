#ifndef RR_GARAGE_IMAGE_DRAG_H
#define RR_GARAGE_IMAGE_DRAG_H
#include "rr/garage_garage.h"
#include "rr/garage_image_scroll.h"
extern RrGaragePoint garage_shift_start_point;
void garage_set_cursor_bounds(s32 x,s32 y,s32 width,s32 height);
/* Active drag coordinates (scroll origins plus offsets6/7) and saved anchor
 * lie in0..63. active21 selects0/1; image buffers have512 words and palette
 * buffers8 words. Starting a drag backs up both images/palette before shifting
 * the selected layer. Cancel/idle gates need no accessible image buffers. */
void garage_move_layer_drag(void);
#endif
