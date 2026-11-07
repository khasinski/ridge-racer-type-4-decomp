#ifndef RR_GARAGE_INPUT_H
#define RR_GARAGE_INPUT_H
#include "rr/garage_garage.h"
#include "rr/tables.h"
#include "rr/memory.h"
extern RrMenuItemState5C PlayerControllers[];
extern s16 garage_analog_cursor_remainder[2];
/* Analog movement with per-axis fractional accumulation outside112..144.
 * Vertical movement is doubled before its contribution is subtracted from
 * the accumulator. Neutral axes clear their accumulator. */
void garage_read_analog_motion(RrGaragePoint *output);
extern u32 garage_analog_up_repeat, garage_analog_down_repeat;
extern u32 garage_analog_left_repeat, garage_analog_right_repeat;
extern u32 garage_dpad_up_repeat, garage_dpad_down_repeat;
extern u32 garage_dpad_left_repeat, garage_dpad_right_repeat;
/* Digital movement: Mask0x8 selects the faster step; later opposite directions win. */
void garage_read_dpad_motion(RrGaragePoint *output);
/* Direction repeat events at counter1 and40; values47 and above wrap to40.
 * Active directions write the accumulated mask even when no event fires;
 * neutral directions reset their counter and preserve the caller's output. */
void garage_read_analog_repeat(u32 *output);
void garage_read_dpad_repeat(u32 *output);
#endif
