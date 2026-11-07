#ifndef RR_GARAGE_PIXELS_H
#define RR_GARAGE_PIXELS_H
#include "types.h"

/* Blend 512 packed words. Zero nibbles preserve the destination.
 * The forward operation permits overlapping buffers and reloads each source
 * word after masking the destination. */
void garage_blend_image_nibbles(const u32 *source,u32 *destination);
#endif
