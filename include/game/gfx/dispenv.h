#ifndef RR_GAME_GFX_DISPENV_H
#define RR_GAME_GFX_DISPENV_H

#include "types.h"

int func_80043C14();
void GetTrackLaneWorldPos();
void SetCamAnglesRotateNegZ(unsigned short *, int, int *);
void RotateNegZByEulerZYX(unsigned short *, int, int *);
short LerpAngleWrap4096(short *p, int t);

#endif /* RR_GAME_GFX_DISPENV_H */
