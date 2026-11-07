#ifndef RR_GARAGE_TIME_H
#define RR_GARAGE_TIME_H
#include "types.h"

extern u8 garage_time_text[6];
/* Input uses 3000 units per second. Negative values produce --:--.
 * The existing separator and terminator are preserved; minutes are not clamped. */
u8 *garage_format_time_mm_ss(s32 value);
#endif
