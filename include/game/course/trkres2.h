#ifndef RR_GAME_COURSE_TRKRES2_H
#define RR_GAME_COURSE_TRKRES2_H

#include "types.h"
#include "game/car/grid.h"
#include "rr/race_types.h"

/* Align an address or size in the native machine-word domain. */
unsigned long AlignUp4(unsigned long value);
void SetCarSpecTable(RaceSpecEntry30 *specs);
void SetTrackStartNode(RaceStartGrid60 *startGrid);
void SetTrackWorkspace(u8 *workspace);


#endif /* RR_GAME_COURSE_TRKRES2_H */
