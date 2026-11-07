#ifndef RR_OVERLAY662_DISTANCE_H
#define RR_OVERLAY662_DISTANCE_H
#include "rr/overlay662_garage.h"
/* Resident signed-word root service. For normal menu coordinates the squared
 * distance is nonnegative and its result is nonnegative. */
s32 CordicSqrt(s32 squared);
/* Both inputs are complete points and may be the same owner. PAL scales X by2
 * and wraps squares/sum at32 bits before calling the root service. The normal
 * root-result contract is nonnegative, giving a positive final divisor. */
s32 ovl662_func_8011DBD8(const RrGaragePoint *a,const RrGaragePoint *b);
#endif
