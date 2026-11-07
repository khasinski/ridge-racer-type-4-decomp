#ifndef RR_GAME_EFFECT_SPFADE_H
#define RR_GAME_EFFECT_SPFADE_H

#include "types.h"

void RampValueToLimit(unsigned short *a0, short *a1, int a2);
void RampValuePairTo4096(unsigned short *a0, unsigned short *a1, short *a2);
void RampValueTo4096(unsigned short *a0, short *a1);
void EmitSpreadSpriteColumns(s32 intensity, s32 scale);
void EmitFadeTile(s32 intensity, s32 scale, u32 pos, s32 width);
void EmitIntroMaskedSprites(int a0, int a1);

#endif /* RR_GAME_EFFECT_SPFADE_H */
