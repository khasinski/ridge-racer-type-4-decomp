#ifndef RR_TRIG_H
#define RR_TRIG_H

#include "types.h"

/* PAL quarter-wave table and overlapping symbol views used for indexed loads. */
extern s16 SinTableBiasedBaseC00[], SinTableBiasedBase800[], SinTableBiasedBase400[], SinTable[];

/* Signed 32-bit angles use 4096 units per turn; results use 4096 as unity. */
s32 rsin(s32 angle);
s32 rcos(s32 angle);
/* Internal nonnegative sine lookup: angle must be in 0..4095. */
s32 sin_1(s32 angle);

#endif
