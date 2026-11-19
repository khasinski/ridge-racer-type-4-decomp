#include "rr/race.h"
#include "game/car/carspec3.h"
#include "game/car/carspec1.h"
#include "game/car/carspec2.h"
#include "rr/audio_loop.h"

void ApplyRubberBandSpec(RaceVehicle *a0) {
    int s0 = 0;
    RaceSpecEntry30 *s2;
    RaceVehicleControl *s3;
    int a, b;
    int lo;
    int tt;
    short *a1;
    short *a2;
    int k;
    int w;
    int v1;
    s2 = RaceContext.carSpecs540 + (a0->flags1E4 & 0xF) - 1;
    s3 = &a0->control;
    if (RaceContext.playerCount14 == 1) {
        s0 = CalcRubberBandFactor(a0, s0);
        a = (a0->flags1E4 & 0xF) - 1;
        BlendCarSpec(a, a, s0, 0);
    } else if (RaceContext.playerCount14 == 2) {
        BlendCarSpec(0, 2, CalcRubberBandFactor(a0, 0), 0);
        s0 = CalcRubberBandFactor(a0, 1);
        BlendCarSpec(1, 3, s0, 0);
        AverageCarSpec((a0->flags1E4 & 0xF) - 1, 2, 3);
    }
    b = s3->boostTimer44;
    if (b > 0) {
        if (GameModeType == 1) {
            lo = (b << 10) / __builtin_abs(s3->divisor11C);
            k = 1;
            if (s0 < lo) s0 = lo;
            a2 = (short *)s2 + 1;
            a1 = (short *)s3 + 1;
            do {
                v1 = s2->ratios08[0];
                tt = ((RaceSpecEntry30 *)a2)->ratios08[0];
                ((RaceVehicleControl *)a1)->gearRatios100[0] = ((((v1 - tt) * s0) / 1024) + tt) * 4;
                a2++;
                k++;
                a1++;
            } while (k < 6);
            v1 = s2->altSpeed28;
            w = s2->baseSpeed00;
            s3->maxSpeedF8 = ((((v1 - w) * lo) / 1024) + w) << 8;
        }
    }
}
