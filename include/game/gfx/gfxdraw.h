#ifndef RR_GAME_GFX_GFXDRAW_H
#define RR_GAME_GFX_GFXDRAW_H

#include "types.h"

void UpdateRaceCamera(int arg0, void *arg1);
void SetupLightMatrices();
int RotTransPersOrigin();
void InitRaceContextBuffers(void);
u8 *AddShadowDiscPrims(s32 a0);

#endif /* RR_GAME_GFX_GFXDRAW_H */
