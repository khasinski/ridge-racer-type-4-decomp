#ifndef RR_OVERLAY662_PIXEL_DISPATCH_H
#define RR_OVERLAY662_PIXEL_DISPATCH_H
#include "rr/overlay662_input.h"
#include "rr/overlay662_image_stroke.h"
#include "rr/overlay662_color_pick.h"
/* Packed log word uses x,y,previous,replacement in bits0..7,8..15,16..23,24..31.
 * The complete four-byte local has no padding and all fields are initialized
 * before the log call. Coordinate sums use the low bytes of the signed halves. */
typedef struct {u8 x,y,previous,replacement;} RrGaragePixelEdit4;
void FloodFill4bpp(u8 *pixel,u32 operation);
/* Only an active hit needs images. active21 remains0..1; the computed byte
 * offset y*32+x/2 fits a complete512-word selected image. Backup may mutate
 * globals and payloads, but the captured pixel address remains valid. */
void garage_func_80124E4C(void);
void garage_func_8012571C(void);
/* Action record52 supplies a code; global input drives this callback and the
 * code is ignored. Preparation may replace scratch34, which then covers512
 * writable words. Brush and mask callbacks must satisfy their own contracts.
 * Mode is reread after the brush before selecting the optional mask action. */
void garage_func_80124974(s32 actionCode);
RR_ASSERT_SIZE(RrGaragePixelEdit4,4);
RR_ASSERT_OFFSET(RrGaragePixelEdit4,x,0);
RR_ASSERT_OFFSET(RrGaragePixelEdit4,y,1);
RR_ASSERT_OFFSET(RrGaragePixelEdit4,previous,2);
RR_ASSERT_OFFSET(RrGaragePixelEdit4,replacement,3);
#endif
