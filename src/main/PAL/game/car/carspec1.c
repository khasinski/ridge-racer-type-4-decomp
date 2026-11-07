#define RaceContext D_800AC650_prefix
#include "rr/race.h"
#include "game/car/carspec1.h"
#undef RaceContext

extern char ComCarsControl[];
extern RrRaceSpecEntry30 *CarSpecTable;
extern struct { char pad[0x544]; RrRaceSpecEntry30 *p[1]; } RaceContext;

void BlendCarSpec(int a0, int a1, int a2, int a3) {
    RrRaceSpecEntry30 *t3 = &CarSpecTable[a0];
    RrRaceVehicleControl *t2 = (RrRaceVehicleControl *)(ComCarsControl + a1 * sizeof(RrRaceVehicle));
    int lo, v, i, a;
    if (a2 < 0x401) {
        lo = a2 * t2->factor11E;
        v = lo >> 10;
        if (lo < 0) v = (lo + 0x3FF) >> 10;
    } else {
        v = a2 - 0x800;
    }
    a = t3->baseSpeed00;
    t2->maxSpeedF8 = ((((RaceContext.p[a3]->baseSpeed00 - a) * v) / 1024) + a) << 8;
    i = 0;
    do {
        t2->gearRatios100[i] = ((((RaceContext.p[a3]->ratios08[i] - t3->ratios08[i]) * v) / 1024) + t3->ratios08[i]) * 4;
        i++;
    } while (i < 6);
    i = 0;
    do {
        t2->shiftThresholds10C[i] = t3->thresholds14[i] + (((RaceContext.p[a3]->thresholds14[i] - t3->thresholds14[i]) * v) / 1024);
        i++;
    } while (i < 6);
    t2->maximum118 = (unsigned short)t3->maximum20 + (((RaceContext.p[a3]->maximum20 - t3->maximum20) * v) / 1024);
    t2->span11A = (unsigned short)t3->span22 + (((RaceContext.p[a3]->span22 - t3->span22) * v) / 1024);
}
