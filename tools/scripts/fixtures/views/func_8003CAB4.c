#include "rr/race_types.h"

extern s16 RaceType;

extern void SetupRace();

extern unsigned short GameTypeSceneIds[];

extern unsigned short D_800AD50A;

void SetupRaceAndScene(void) {
    SetupRace();
    D_800AD50A = GameTypeSceneIds[RaceType];
}
