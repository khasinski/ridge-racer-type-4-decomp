#ifndef RR_GAME_GFX_MODEL_H
#define RR_GAME_GFX_MODEL_H

#include "types.h"

int func_8006F1EC();
s32 YawToOctant(void *arg0);
unsigned int *GetSegmentViewTable();
void InitTimTemplate(int *a0);
void RelocateModelPointers(int *a0);
int RegisterModelBank();

#endif /* RR_GAME_GFX_MODEL_H */
