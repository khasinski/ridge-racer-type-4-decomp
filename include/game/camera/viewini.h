#ifndef RR_GAME_CAMERA_VIEWINI_H
#define RR_GAME_CAMERA_VIEWINI_H

#include "types.h"

void InitViewState(void);
void BuildViewSegmentObjectList(void);
void AddUniqueViewSegmentRecords();
int SetViewClipMask();

#endif /* RR_GAME_CAMERA_VIEWINI_H */
