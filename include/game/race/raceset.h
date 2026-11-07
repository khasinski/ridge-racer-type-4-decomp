#ifndef RR_GAME_RACE_RACESET_H
#define RR_GAME_RACE_RACESET_H

#include "types.h"

extern int CourseBestTotalTimes[];
extern int CourseBestTotal[2][8];
extern s16 D_8010C104;
extern s32 D_8010C108;
extern s32 D_8010C10C;
extern s32 SplitDeltaTimer;
void UpdateRaceRanking(void);
void SetupRaceAndScene(void);

#endif /* RR_GAME_RACE_RACESET_H */
