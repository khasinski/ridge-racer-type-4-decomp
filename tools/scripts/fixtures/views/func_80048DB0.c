#include "rr/steering.h"

#define CONTROL_VOL(field) (*(volatile __typeof__(((RrRaceVehicle *)0)->field) *)(controlBase + (RR_FIELD_OFFSET(RrRaceVehicle,field) - RR_RACE_CONTROL_OFFSET)))

#define CONTROL(field) (*(__typeof__(((RrRaceVehicle *)0)->field) *)(controlBase + (RR_FIELD_OFFSET(RrRaceVehicle,field) - RR_RACE_CONTROL_OFFSET)))

void UpdateComCarLaneChange(RrRaceVehicle *vehicle) {
    s32 stepOutput;
    register s32 workingStep __asm__("$2");
    s16 mode;
    s16 nextTimer;
    s32 timer; s32 currentNode; s32 trackCount;
    s32 phase;
    s32 duration;
    s32 delta;
    s32 targetNode;
    s32 correction;
    s32 smoothedMotion;
    s32 headingDelta;
    s32 angleDifference;
    s32 angle;
    s32 quarterDuration;
    s32 wrappedHeading;
    s32 motion;
    s32 adjustedAngle;
    register s32 lateralStep __asm__("$4");
    s32 nodeDelta;
    register s32 limit __asm__("$4");
    register s32 normalizedMotion __asm__("$2");
    s32 lateral;
    s32 adjustedDifference;
    s32 gain;
    s32 shortRange;
    s32 adjustedDuration;
    s32 nodeOffset;
    s32 turnStep;
    u32 wrappedOffset;
    u8 *controlBase;

    controlBase = (u8 *)vehicle + RR_RACE_CONTROL_OFFSET;
    if (CONTROL(steeringEnabled318) == 0) {
        vehicle->rot54 = (s32) vehicle->heading1D4;
        return;
    }
    mode = CONTROL(steering288.mode00);
    switch (mode) {
    case 0:
        vehicle->rot54 = (s32) vehicle->heading1D4;
        break;
    case 1:
        EvalRateCurve(&vehicle->steering288.primary34, CONTROL(steering288.timer02), &stepOutput);
        workingStep=stepOutput;
        angle = (s32)((u32)CONTROL(steering288.angle0C) - (u32)workingStep);
        CONTROL(steering288.angle0C) = angle; asm("" : : "m"(CONTROL(steering288.angle0C)));
        adjustedAngle = angle; asm("" : : "r"(angle), "r"(adjustedAngle));
        CONTROL(steering288.delta10) = workingStep;
        CONTROL(steering288.remaining08) = (s32)((u32)CONTROL(steering288.remaining08) - (u32)workingStep);
        if (adjustedAngle < 0) {
            adjustedAngle += 0xFFF;
        }
        workingStep = adjustedAngle >> 12;  vehicle->rot54 = workingStep;
        if (CONTROL(steering288.phase28) == 0) {
            delta = CONTROL(steering288.delta10);
            lateralStep = delta >> 3;
            if (delta < 0) {
                delta += 7;  lateralStep = delta >> 3;
            }
             vehicle->lateralMotion178 = lateralStep;
            duration = CONTROL(steering288.duration04); timer=CONTROL(steering288.timer02);
            adjustedDuration = duration;
            if (duration < 0) {
                adjustedDuration = duration + 3;
            }
            quarterDuration = adjustedDuration >> 2;
            if (timer >= quarterDuration) {
                normalizedMotion = lateralStep;
                if (normalizedMotion < 0) {
                    normalizedMotion += 0xFF;
                }
                BuildRateCurve(&vehicle->steering288.secondary4C, (normalizedMotion >> 8) * 2, duration - quarterDuration, 0);
                CONTROL(steering288.lateral24) = (s32) ((s32)((u32)(vehicle->lateralMotion178 / 256) << 12));
                CONTROL(steering288.phase28) = (s32) mode;
            }
        }
        phase = CONTROL(steering288.phase28);
        if (phase > 0) {
            EvalRateCurve(&CONTROL(steering288.secondary4C), phase - 1, &stepOutput);
            workingStep=CONTROL(steering288.lateral24);  lateral = (s32)((u32)workingStep - (u32)stepOutput);
            CONTROL(steering288.lateral24) = lateral;
            if (lateral < 0) {
                lateral += 0xF;
            }
            vehicle->lateralMotion178 = (s32) (lateral >> 4);
            CONTROL(steering288.phase28) = (s32) ((u32)CONTROL(steering288.phase28) + 1U);
        }
        nextTimer = (u16) CONTROL(steering288.timer02) + 1;
        CONTROL(steering288.timer02) = nextTimer;
        if (nextTimer >= CONTROL(steering288.duration04)) {
            CONTROL(steering288.mode00) = 2;
        }
        break;
    case 2:
        CONTROL(steering288.lateral24) = (s32) ((s32) CONTROL(steering288.lateral24) / 16);
        currentNode = vehicle->trackNodeF0;
        targetNode = CONTROL(steering288.targetNode18);
        nodeOffset = (s32)((u32)currentNode - (u32)targetNode);
        wrappedOffset = (u32)targetNode - (u32)currentNode;
        if (nodeOffset >= 0) {
            wrappedOffset = (u32) nodeOffset >> 0x1F;
        } else {
            nodeOffset = (s32) (wrappedOffset >> 0x1F);
        }
        CONTROL(steering288.midpoint1C) = (s32)((u32)currentNode - (u32)((s32)(wrappedOffset + (u32)nodeOffset) >> 1));
        CONTROL(steering288.mode00) = 3;
        /* fallthrough */
    case 3:
        if (CONTROL(steering288.midpoint1C) < vehicle->trackNodeF0) {
            vehicle->lateralMotion178 = (s32) CONTROL(steering288.lateral24);
        } else {
            correction = (s32)(((u32)CONTROL(steering288.lateral24) - (u32)vehicle->lateralMotion178) * 3U) / 4;
            CONTROL(steering288.lateral24) = correction;
            smoothedMotion = (s32)((u32)correction + (u32)vehicle->lateralMotion178);
            CONTROL(steering288.lateral24) = smoothedMotion;
            vehicle->lateralMotion178 = smoothedMotion;
        }
        trackCount = TrackNodeCount; nodeDelta = (s32)((u32)CONTROL(steering288.targetNode18) - (u32)vehicle->trackNodeF0);  nodeDelta %= trackCount;
        if ((trackCount / 2) < nodeDelta) {
            nodeDelta -= trackCount;
        }
        if (nodeDelta >= 0) {
            headingDelta = (s32)((u32)vehicle->rot54 - (u32)(s32)vehicle->heading1D4);
            wrappedHeading = headingDelta & 0xFFF;
            stepOutput = wrappedHeading;
            if (headingDelta & 0x800) {
                stepOutput = wrappedHeading | 0xF000; /* Original 32-bit OR: no sign extension. */
            }
            if (stepOutput != 0) {
                turnStep = stepOutput >> 2;
                if (stepOutput < 0) {
                    turnStep = (s32) (stepOutput + 3) >> 2;
                }
                stepOutput = turnStep;
                if (turnStep != 0) goto applyTurn;
            }
            CONTROL(steering288.mode00) = 0;
            goto tail;
applyTurn:
            vehicle->rot54 = (s32)((u32)vehicle->rot54 - (u32)turnStep);
        }
        break;
    }
tail:
    if (vehicle->steeringLock1B8 == 0) {
        angleDifference = AngleDiffAbs(vehicle->rot54, vehicle->heading1D4);
        if (angleDifference < 0x201) {
            adjustedDifference = angleDifference;
            if (angleDifference < 0) {
                adjustedDifference = angleDifference + 7;
            }
            gain = (adjustedDifference >> 3) + 0x40;
        } else {
            gain = 0x7F;
        }
        CONTROL_VOL(steering288.gain30) = gain;
        CONTROL_VOL(steering288.gain30) = (s32) ((s32)((u32)CONTROL_VOL(steering288.gain30) * 96U) / 128);
if (RaceType == 6 && CourseListIndex == 0) {
            shortRange = angleDifference < 0x201;
            CONTROL_VOL(steering288.gain30) = 0;
        } else {
            shortRange = angleDifference < 0x201;
        }
        if (shortRange) {
            CONTROL(steering288.range2C) = (s32)((u32)angleDifference * 3U + 0x1800U);
        } else {
            CONTROL(steering288.range2C) = 0x1E00;
        }
    } else {
        CONTROL_VOL(steering288.gain30) = 0;
        CONTROL(steering288.range2C) = 0;
    }
    limit = -0xF000;
    motion = vehicle->lateralMotion178;
    if ((motion < limit) || (limit = 0xF000, ((motion > limit) != 0))) {
        vehicle->lateralMotion178 = limit;
    }
}
