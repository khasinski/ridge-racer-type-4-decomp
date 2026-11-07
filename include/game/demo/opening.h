#ifndef RR_GAME_DEMO_OPENING_H
#define RR_GAME_DEMO_OPENING_H

#include "types.h"

extern char CdTrackTitleStrings[];
extern s16 *OpeningTitleGlyphLists[];
extern s16 *CourseCardGlyphListsB[];
extern s16 *CourseCardGlyphLists3[];
void ResetOpeningFadeTimer(void);
void EmitProjectedMarkerSprt(int a0, int a1, int a2);
void AddDrawModeTagToOt(void);

#endif /* RR_GAME_DEMO_OPENING_H */
