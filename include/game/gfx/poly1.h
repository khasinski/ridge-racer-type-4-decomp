#ifndef RR_GAME_GFX_POLY1_H
#define RR_GAME_GFX_POLY1_H

#include "types.h"

void InitLineG2TrailPool();
void ConfigureTrackObjectParticles(int a0, int a1, int *a2, int a3);
void UpdateTrackObjectParticles(char *a0, int a1, int a2);
void ApplySegmentOctantValue(void);
void SetSegmentOctantTable(s32 arg0);
int GetOctantTableValue(void);
void SetOctantLookupTable(s32 arg0);
void SplashLogoInit(void);

#endif /* RR_GAME_GFX_POLY1_H */
