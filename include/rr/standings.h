#ifndef RR_STANDINGS_H
#define RR_STANDINGS_H
#include "types.h"

/* Complete ten-word storage. Standings consume at most the first six active
 * entries, ending at the first negative value. Race setup resets all ten. */
extern s32 LapTimes[10];
extern s32 TotalRaceTime;
extern u16 BestLapIndex;
/* Publish the first minimum entry's original index; an empty list yields0. */
void time_attack_course_select_find_best_lap(void);
#endif
