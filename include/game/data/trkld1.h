#ifndef RR_GAME_DATA_TRKLD1_H
#define RR_GAME_DATA_TRKLD1_H

#include "types.h"

extern s32 RaceLoadPhase;
extern s32 RaceLoadStepIndex;
extern int RaceLoadState;
extern int RaceLoadSubState;
extern int RaceLoadCursor;
int LoadCarSpecStep();
int func_8004D7D8();
int func_8004D994();
int LoadVabPairStep(char *a0);
s32 func_8004E184(void);
int LoadRaceSoundStep(void);

#endif /* RR_GAME_DATA_TRKLD1_H */
