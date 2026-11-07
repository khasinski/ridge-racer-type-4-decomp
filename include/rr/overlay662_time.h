#ifndef RR_OVERLAY662_TIME_H
#define RR_OVERLAY662_TIME_H
#include "types.h"

extern u8 garage_D_80126244[6];
/* Input uses 3000 units per second. Negative values produce --:--.
 * The existing separator and terminator are preserved; minutes are not clamped. */
u8 *garage_func_8011883C(s32 value);
#endif
