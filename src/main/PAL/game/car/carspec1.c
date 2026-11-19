#include "rr/race.h"
#include "game/car/carspec1.h"

extern RaceVehicle ComCars[];

void BlendCarSpec(int baseIndex, int vehicleIndex, int blend, int targetIndex) {
    RaceSpecEntry30 *baseSpec = &RaceContext.carSpecs540[baseIndex];
    RaceVehicleControl *control = &ComCars[vehicleIndex].control;
    int product, weight, i, baseSpeed;
    if (blend < 0x401) {
        product = blend * control->factor11E;
        weight = product >> 10;
        if (product < 0) weight = (product + 0x3FF) >> 10;
    } else {
        weight = blend - 0x800;
    }
    baseSpeed = baseSpec->baseSpeed00;
    control->maxSpeedF8 = (u32)((((RaceContext.specTargets544[targetIndex]->baseSpeed00 - baseSpeed) * weight) / 1024) + baseSpeed) << 8;
    i = 0;
    do {
        control->gearRatios100[i] = ((((RaceContext.specTargets544[targetIndex]->ratios08[i] - baseSpec->ratios08[i]) * weight) / 1024) + baseSpec->ratios08[i]) * 4;
        i++;
    } while (i < 6);
    i = 0;
    do {
        control->shiftThresholds10C[i] = baseSpec->thresholds14[i] + (((RaceContext.specTargets544[targetIndex]->thresholds14[i] - baseSpec->thresholds14[i]) * weight) / 1024);
        i++;
    } while (i < 6);
    control->maximum118 = (unsigned short)baseSpec->maximum20 + (((RaceContext.specTargets544[targetIndex]->maximum20 - baseSpec->maximum20) * weight) / 1024);
    control->span11A = (unsigned short)baseSpec->span22 + (((RaceContext.specTargets544[targetIndex]->span22 - baseSpec->span22) * weight) / 1024);
}
