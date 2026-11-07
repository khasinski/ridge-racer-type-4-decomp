#ifndef RR_OVERLAY662_IMAGE_DRAG_H
#define RR_OVERLAY662_IMAGE_DRAG_H
#include "rr/overlay662_garage.h"
#include "rr/overlay662_image_scroll.h"
extern RrGaragePoint garage_D_801277AC;
void garage_func_8011E7C0(s32 x,s32 y,s32 width,s32 height);
/* Active drag coordinates (scroll origins plus offsets6/7) and saved anchor
 * lie in0..63. active21 selects0/1; image buffers have512 words and palette
 * buffers8 words. Starting a drag backs up both images/palette before shifting
 * the selected layer. Cancel/idle gates need no accessible image buffers. */
void garage_func_801251AC(void);
#endif
