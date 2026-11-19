#ifndef RR_RACE_SPEC_TYPES_H
#define RR_RACE_SPEC_TYPES_H
#include "rr/layout.h"

/* 0x30-byte performance interpolation row (CarSpecTable / context table). */
typedef struct {
    s32 baseSpeed00, gearCount04;
    unsigned short ratios08[6];
    unsigned short thresholds14[6];
    s16 maximum20, span22, divisor24, factor26;
    s32 altSpeed28;
    s16 steering2C, pad2E;
} RaceSpecEntry30;

RR_ASSERT_SIZE(RaceSpecEntry30, 0x30);
RR_ASSERT_OFFSET(RaceSpecEntry30, thresholds14, 0x14);
RR_ASSERT_OFFSET(RaceSpecEntry30, maximum20, 0x20);
RR_ASSERT_OFFSET(RaceSpecEntry30, altSpeed28, 0x28);
#endif
