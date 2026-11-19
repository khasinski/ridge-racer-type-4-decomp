#include "rr/scene_camera_state.h"
#include "game/sound/sndque.h"
#include "rr/audio_state_types.h"
#include "game/car/carspec2.h"
#include "rr/audio_loop.h"

typedef struct {
    short x, y;
} CurvePt;

extern RaceVehicle ComCars[];
extern CurvePt RubberBandCurve[];
extern short BonusCarUnlockMask;
extern int ReplayState;
extern int RaceState;

void AverageCarSpec(int destinationIndex, int leftIndex, int rightIndex);
int CalcRubberBandFactor(RaceVehicle *a0, int a1);

void AverageCarSpec(int destinationIndex, int leftIndex, int rightIndex) {
    int i;
    RaceVehicleControl *destination = &ComCars[destinationIndex].control;
    RaceVehicleControl *left = &ComCars[leftIndex].control;
    RaceVehicleControl *right = &ComCars[rightIndex].control;
    destination->maxSpeedF8 = (left->maxSpeedF8 + right->maxSpeedF8) / 2;
    for (i = 0; i < 6; i++) {
        destination->gearRatios100[i] = ((u16)left->gearRatios100[i] + (u16)right->gearRatios100[i]) >> 1;
    }
    for (i = 0; i < 6; i++) {
        destination->shiftThresholds10C[i] = (left->shiftThresholds10C[i] + right->shiftThresholds10C[i]) >> 1;
    }
    destination->maximum118 = (left->maximum118 + right->maximum118) / 2;
    destination->span11A = (left->span11A + right->span11A) / 2;
}

int CalcRubberBandFactor(RaceVehicle *a0, int a1) {
    int s1 = 0;
    RaceStandingsTable *s2 = RaceContext.standings;
    RaceVehicleControl *t0 = &a0->control;
    int d;
    int i;
    int lo;
    int b;
    int v2;
    int v1;
    int v0;

    if (s2->playerDistance[a1] >= RubberBandCurve[0].x) {
        for (i = 1; i < 7; i++) {
            if (s2->playerDistance[a1] < RubberBandCurve[i].x) {
                break;
            }
        }
        if (i < 4) {
            d = s2->playerDistance[a1] - RubberBandCurve[i].x;
            lo = ((RubberBandCurve[i].y - RubberBandCurve[i - 1].y) * d) / (RubberBandCurve[i].x - RubberBandCurve[i - 1].x);
            s1 = RubberBandCurve[i].y + lo;
            d = t0->lateralSpeed58 - s2->playerSpeed[a1];
            if (d < 0) {
                d = 0;
            }
            if (d > a0->input) {
                d = a0->input;
            }
            if (a0->input > 0) {
                d = (d << 10) / a0->input;
            }
            if (d >= 0x100) {
                d = 0x400 - (((d - 0x100) * 4) / 3);
                s1 = (s1 * d) / 1024;
            }
            i = t0->boostTimer44;
            if (i > 0) {
                s1 = (s1 * (0x400 - ((i << 10) / __builtin_abs(t0->divisor11C)))) / 1024;
            }
        } else if (i < 7) {
            int lo2;
            int denom;
            d = s2->playerDistance[a1] - RubberBandCurve[i].x;
            denom = RubberBandCurve[i].x - RubberBandCurve[i - 1].x;
            lo2 = ((RubberBandCurve[i].y - RubberBandCurve[i - 1].y) * d) / denom;
            /* Note: extend denom past div so local_alloc does not combine denom with lo2 */
            /* ASM_BARRIER: extend denom past div so local_alloc does not combine denom with lo2 */
            __asm__ ("" : : "r"(denom));
            s1 = RubberBandCurve[i].y + lo2;
            s1 += 0x800;
        }
        if (GameModeType == 1 || GameModeType == 6) {
            if (RaceContext.standings->entries18[0].unk00 == 0 && RaceContext.standings->entries18[1].unk00 == (s16)a0->flags1E4 && s2->playerDistance[a1] > BonusCarUnlockMask) {
                s1 = 0xC00;
            }
        }
    }
    if (ReplayState != 2 && RaceState < 4) {
        if (s2->playerDistance[a1] < 0x7D0) {
            if (SoundState.unk1A0C[((s16)a0->flags1E4 & 0xF) - 1] == 0) {
                if (SoundState.unk1A28 == 0) {
                    v0 = PsyqRand() & 1;
                    SoundState.unk1A2C = v0;
                    lo = v0 + 0xD;
                    SoundQueueEnqueue(lo, 0);
                }
                SoundState.unk1A0C[((s16)a0->flags1E4 & 0xF) - 1] = 1;
                SoundState.unk1A28 = 0x4B;
            }
        }
        if (s2->playerDistance[a1] >= 0x1771) {
            if (SoundState.unk1A0C[((s16)a0->flags1E4 & 0xF) - 1] != 0) {
                SoundState.unk1A0C[((s16)a0->flags1E4 & 0xF) - 1] = 0;
            }
        }
        if (RaceContext.standings->entries18[0].unk00 == (s16)a0->flags1E4 && RaceContext.standings->entries18[1].unk00 == 0) {
            if (s2->playerDistance[a1] >= -0x5DB) {
                if (SoundState.unk1A08 == 0) {
                    if (SoundState.unk1A28 == 0) {
                        v0 = PsyqRand() & 1;
                        SoundState.unk1A30 = v0;
                        SoundQueueEnqueue(v0 + 0xB, 0);
                    }
                    SoundState.unk1A08 = 1;
                    SoundState.unk1A28 = 0x4B;
                }
            }
            if (s2->playerDistance[a1] < -0x1388) {
                if (SoundState.unk1A08 != 0) {
                    SoundState.unk1A08 = 0;
                }
            }
        }
    }
    return s1;
}
