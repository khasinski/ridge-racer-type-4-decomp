#ifndef RR_STEERING_H
#define RR_STEERING_H
#include "rr/race_types.h"

/* Active node comparisons require a positive track count. Curve construction
 * requires duration / 2 != 0 and fixed-point products representable in s32. */
void UpdateComCarLaneChange(RrRaceVehicle *vehicle);
s32 AngleDiffAbs(s32 first, s32 second);
void BuildRateCurve(RrCurve24 *record, s32 displacement, s32 duration, s32 initialRate);
s32 EvalRateCurve(const RrCurve24 *record, s32 time, s32 *output);
extern s32 TrackNodeCount;
extern s16 CourseListIndex, RaceType;

RR_ASSERT_SIZE(RrCurve24, 0x18);
RR_ASSERT_OFFSET(RrCurve24, duration, 4);
RR_ASSERT_OFFSET(RrCurve24, initialRate, 8);
RR_ASSERT_OFFSET(RrCurve24, firstAcceleration, 0x10);
RR_ASSERT_OFFSET(RrCurve24, secondAcceleration, 0x14);
RR_ASSERT_SIZE(RrSteeringState, 0x64);
RR_ASSERT_OFFSET(RrSteeringState, primary34, 0x34);
RR_ASSERT_OFFSET(RrSteeringState, secondary4C, 0x4C);
RR_ASSERT_SIZE(RrRaceVehicle, 0x320);
RR_ASSERT_OFFSET(RrRaceVehicle, lateralMotion178, 0x178);
RR_ASSERT_OFFSET(RrRaceVehicle, steeringLock1B8, 0x1B8);
RR_ASSERT_OFFSET(RrRaceVehicle, steering288, 0x288);
RR_ASSERT_OFFSET(RrRaceVehicle, steeringEnabled318, 0x318);
RR_ASSERT_SIZE(RrRaceVehicleControl, 0x12C);
RR_ASSERT_OFFSET(RrRaceVehicleControl, steering94, 0x94);
RR_ASSERT_OFFSET(RrRaceVehicleControl, steeringEnabled124, 0x124);
#endif
