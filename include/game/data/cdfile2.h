#ifndef RR_GAME_DATA_CDFILE2_H
#define RR_GAME_DATA_CDFILE2_H

#include "types.h"

s32 CdFileStep(void);
s32 CdFileReadStart(s32 a0, s32 a1);
s32 CdFileReadRange(s32 a0, s32 a1, s32 a2, s32 a3);
s32 CdFileBytesLoaded(void);
void RelocateOffsetTable(int *a0);
int RequestSceneLoad(int a0);
int SceneLoadStep(void);
int RequestCarLoad(int a0, int a1, int a2, int a3, int a4);
s32 CarLoadStep(void);
int RequestVramImageLoad(void);
int VramImageLoadStep(void);
int SetCarTrackLighting(char *a0_in, int a1_in);

#endif /* RR_GAME_DATA_CDFILE2_H */
