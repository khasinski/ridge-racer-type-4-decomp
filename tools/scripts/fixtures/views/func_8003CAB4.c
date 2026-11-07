#include "rr/race_types.h"

extern s16 RaceType;

extern void SetupRace();

extern unsigned short GameTypeSceneIds[];

extern unsigned short SceneId;

void SetupRaceAndScene(void) {
    SetupRace();
    SceneId = GameTypeSceneIds[RaceType];
}
