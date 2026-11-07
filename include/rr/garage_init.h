#ifndef RR_GARAGE_INIT_H
#define RR_GARAGE_INIT_H
#include "rr/garage_garage.h"

/* Opaque saved word: resident code copies its 32 bits to/from buffer offset0x280. */
extern u32 PlayTimeFrames;
void garage_init_editor_state(void);
#endif
