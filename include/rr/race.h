#ifndef RR_RACE_H
#define RR_RACE_H

#include "rr/layout.h"

#include "rr/race_types.h"

#include "rr/player_cars.h"
#include "rr/scene_camera_state.h"
void CalcDistanceToPlayer(RaceVehicle *current);
/* The interpolation path requires span != 0; positive blend requires divisor != 0.
 * The target retains the original DIV exception checks for invalid records. */
/* Return a full signed word whose value is normalized to signed16. */
s32 CalcSteerLimit(RaceVehicle *vehicle);

#if defined(mips)
RR_ASSERT_SIZE(RaceVehicle, 0x320);
RR_ASSERT_OFFSET(RaceVehicle, control, 0x1F4);
RR_ASSERT_OFFSET(RaceVehicle, vel30, 0x30);
RR_ASSERT_OFFSET(RaceVehicle, rot50, 0x50);
RR_ASSERT_OFFSET(RaceVehicle, ang80, 0x80);
RR_ASSERT_OFFSET(RaceVehicle, bodyPosition, 0xC8);
RR_ASSERT_OFFSET(RaceVehicle, wheelSteering17C, 0x17C);
RR_ASSERT_OFFSET(RaceVehicle, wheelRotX1A8, 0x1A8);
RR_ASSERT_OFFSET(RaceVehicle, renderAngleZ1AE, 0x1AE);
RR_ASSERT_OFFSET(RaceVehicle, trackNodeF0, 0xF0);
RR_ASSERT_OFFSET(RaceVehicle, height108, 0x108);
RR_ASSERT_OFFSET(RaceVehicle, halfSpan10E, 0x10E);
RR_ASSERT_OFFSET(RaceVehicle, lateralOffset172, 0x172);
RR_ASSERT_OFFSET(RaceVehicle, first, 0x184);
RR_ASSERT_OFFSET(RaceVehicle, second, 0x188);
RR_ASSERT_OFFSET(RaceVehicle, shakeX198, 0x198);
RR_ASSERT_OFFSET(RaceVehicle, steerAngle1A2, 0x1A2);
RR_ASSERT_OFFSET(RaceVehicle, steerInput1BA, 0x1BA);
RR_ASSERT_OFFSET(RaceVehicle, heading1D4, 0x1D4);
RR_ASSERT_OFFSET(RaceVehicle, input, 0x1D8);
RR_ASSERT_OFFSET(RaceVehicle, flags1E4, 0x1E4);
RR_ASSERT_OFFSET(RaceVehicle, trackAngle1EC, 0x1EC);
RR_ASSERT_OFFSET(SteeringState, duration04, 4);

RR_ASSERT_SIZE(RaceVehicleControl, 0x12C);
RR_ASSERT_OFFSET(RaceVehicleControl, record18, 0x18);
RR_ASSERT_OFFSET(RaceVehicleControl, handling1C, 0x1C);
RR_ASSERT_OFFSET(RaceVehicleControl, reserved20, 0x20);
RR_ASSERT_OFFSET(RaceVehicleControl, dynamics94, 0x94);
RR_ASSERT_OFFSET(RaceVehicleControl, laneTimer38, 0x38);
RR_ASSERT_OFFSET(RaceVehicleControl, boostTimer44, 0x44);
RR_ASSERT_OFFSET(RaceVehicleControl, speed4C, 0x4C);
RR_ASSERT_OFFSET(RaceVehicleControl, shiftDir62, 0x62);
RR_ASSERT_OFFSET(RaceVehicleControl, relDist74, 0x74);
RR_ASSERT_OFFSET(RaceVehicleControl, laneMode88, 0x88);
RR_ASSERT_OFFSET(RaceVehicleControl, maxSpeedF8, 0xF8);
RR_ASSERT_OFFSET(RaceVehicleControl, maximum118, 0x118);
RR_ASSERT_OFFSET(RaceVehicleControl, metric12A, 0x12A);
RR_ASSERT_SIZE(RaceStandingEntry0C, 0xC);
RR_ASSERT_SIZE(RaceStandingsTable, 0x78);
#endif
RR_ASSERT_OFFSET(RaceStandingsTable, entries18, 0x18);
RR_ASSERT_OFFSET(RaceStandingsTable, playerSpeed, 0);
RR_ASSERT_OFFSET(RaceStandingsTable, playerDistance, 8);
RR_ASSERT_OFFSET(RaceStandingsTable, closestPlayer, 0x10);

#endif
