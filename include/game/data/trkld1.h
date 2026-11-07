#ifndef RR_GAME_DATA_TRKLD1_H
#define RR_GAME_DATA_TRKLD1_H

#include "types.h"

extern s32 RaceLoadPhase;
extern s32 RaceLoadStepIndex;
extern int RaceLoadState;
extern int RaceLoadSubState;
extern int RaceLoadCursor;
int LoadCarSpecStep();
int LoadSelectionTimAndModelStep();
int LoadTeamModelsStep();
int LoadVabPairStep(char *a0);
s32 UnloadRaceStepStub(void);
int LoadRaceSoundStep(void);

#endif /* RR_GAME_DATA_TRKLD1_H */
