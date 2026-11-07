#ifndef RR_GAME_RACE_RACESET_H
#define RR_GAME_RACE_RACESET_H

#include "types.h"

extern int CourseBestTotalTimes[];
extern int CourseBestTotal[2][8];
extern s16 SplitDeltaIndex;
extern s32 ShownSplitTime;
extern s32 LatchedBestSplitTime;
extern s32 SplitDeltaTimer;
void UpdateRaceRanking(void);
void SetupRaceAndScene(void);

#endif /* RR_GAME_RACE_RACESET_H */
