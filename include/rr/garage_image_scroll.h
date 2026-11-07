#ifndef RR_GARAGE_IMAGE_SCROLL_H
#define RR_GARAGE_IMAGE_SCROLL_H
#include "types.h"
/* Packed64x64 image:8 unsigned words per row,8 low-first nibbles per word.
 * Both buffers cover512 words and may overlap. Shifts are differences of
 * coordinates0..63, hence x/y lie in[-63,63]. Positive shifts move the image
 * right/down, wrapping at64. The complete input is staged before any output. */
void garage_shift_image_wrap(u32 *source,u32 *destination,s32 x,s32 y);
#endif
