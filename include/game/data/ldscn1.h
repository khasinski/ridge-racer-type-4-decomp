#ifndef RR_GAME_DATA_LDSCN1_H
#define RR_GAME_DATA_LDSCN1_H

#include "types.h"

extern int LoadDestCursor;
extern signed char *D_800ACBFC;
extern char *TrackNodesEnd;
extern short ModeMenuCursorSaved;
int LoadSelectedModeFileStep(void);
int LoadTimListStep();
int LoadDialogueStep(void);
int LoadSplashAssetsStep(void);

#endif /* RR_GAME_DATA_LDSCN1_H */
