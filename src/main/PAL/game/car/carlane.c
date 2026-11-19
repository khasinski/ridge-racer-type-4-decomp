/* RODATA 0x80010fec 0x28 */
#include "types.h"
#include "game/car/grid.h"
#include "game/car/carbound.h"
#include "game/car/carlane.h"
#include "rr/audio_loop.h"
#include "rr/menu_choice.h"
#include "rr/menu_setup.h"
#include "rr/track_projection.h"
#include "rr/bios_devices.h"
#include "game/course/trkclip.h"
#include "rr/cd_poll.h"

#include "rr/scene_camera_state.h"
extern RaceVehicle ComCars[];
extern u16 CarDraftSlipstreamFactor;


void InitComCars(s32 count) {
    s32 i, j, k;
    RaceVehicleControl *sub;
    RaceSpecEntry30 *spec;
    s32 tmp;
    RaceVehicle *car;

    car = ComCars;
    RaceContext.comCarCount10 = count;
    PsyqBzero(car, sizeof(RaceVehicle[11]));
    for (i = 0; i < count; i++) {
        sub = &car->control;
        spec = &RaceContext.carSpecs540[i];
        ResetCarState(car);
        car->flags1E4 = i + 1;
        for (j = 0; j < 8; j++) {
            if (RaceContext.startOrder00[j] == i + 1) {
                k = j;
                break;
            }
        }
        car->position20.words[0] = RaceContext.startGrid53C->positions10[k].x;
        car->position20.words[1] = 0;
        car->position20.words[2] = RaceContext.startGrid53C->positions10[k].z;
        car->posScaled00[0] = (u32)car->position20.words[0] << 8;
        car->posScaled00[2] = (u32)car->position20.words[2] << 8;
        car->trackNodeF0 = RaceContext.startGrid53C->trackNode00;
        car->rot50 = 0;
        car->rot58 = 0;
        car->rot54 = RaceContext.startGrid53C->headings50[k];
        car->heading1D4 = car->rot54;
        car->trackNodeF0 = FindTrackNode(car->position20.words, car->trackNodeF0);
        AssignCarModelSlot(car);
        UpdateCarTrackProjection(car);
        sub->laneTimer38 = 10;
        sub->laneStart3A = car->lateralOffset172;
        sub->maxSpeedF8 = spec->baseSpeed00 << 8;
        sub->gearCountFC = spec->gearCount04;
        for (j = 0; j < 6; j++) {
            sub->gearRatios100[j] = spec->ratios08[j] << 2;
        }
        for (j = 0; j < 6; j++) {
            sub->shiftThresholds10C[j] = spec->thresholds14[j];
        }
        sub->maximum118 = spec->maximum20;
        sub->span11A = spec->span22;
        sub->divisor11C = spec->divisor24;
        sub->boostTimer44 = sub->divisor11C < 0 ? -sub->divisor11C : sub->divisor11C;
        sub->factor11E = spec->factor26;
        sub->altSpeed120 = spec->altSpeed28;
        sub->steeringEnabled124 = spec->steering2C;
        sub->laneCooldown40 = (i * 25 + 50) / 2;
        sub->metric12A = 0;
        car++;
    }
    switch ((s16)GameModeType) {
    case 6:
        RaceContext.playerCount14 = 1;
        RaceContext.specTargets544[0] = &RaceContext.carSpecs540[SelectedMakerIndex + 7];
        RaceContext.specTargets544[1] = 0;
        break;
    case 1:
        RaceContext.playerCount14 = 1;
        RaceContext.specTargets544[0] = &RaceContext.carSpecs540[SelectedTeamIndex + 7];
        RaceContext.specTargets544[1] = 0;
        break;
    case 2:
    case 3:
    case 4:
        RaceContext.playerCount14 = 2;
        RaceContext.specTargets544[0] = &RaceContext.carSpecs540[7];
        RaceContext.specTargets544[1] = &RaceContext.carSpecs540[7];
        break;
    case 5:
    default:
        RaceContext.playerCount14 = 0;
        RaceContext.specTargets544[0] = &RaceContext.carSpecs540[SelectedTeamIndex + 7];
        break;
    }
    if ((s16)GameModeType == 1) {
        for (i = 0; i < count; i++) {
            sub = &ComCars[i].control;
            sub->metric12A = RaceContext.specTargets544[0]->divisor24;
            tmp = sub->divisor11C * RaceContext.specTargets544[0]->steering2C;
            if (tmp < 0) {
                tmp += 0x3FF;
            }
            sub->divisor11C = tmp >> 10;
            sub->boostTimer44 = sub->divisor11C < 0 ? -sub->divisor11C : sub->divisor11C;
        }
        CarDraftSlipstreamFactor = RaceContext.specTargets544[0]->factor26;
    } else {
        CarDraftSlipstreamFactor = 0x320;
    }
    DebugPrintStub("InitComCar OK!\n", i);
}
