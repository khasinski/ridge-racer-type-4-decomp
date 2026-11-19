#ifndef RR_GAME_GFX_FADESPR_H
#define RR_GAME_GFX_FADESPR_H

#include "types.h"
#include "rr/demo_fade_types.h"

void InitOptionsDefaults(void);
void InitDemoFadeContext(int sound);
void UpdateFadeTint(int intensity);
void EmitFadeSprite(s32 index, s32 x, s32 y, s32 intensity);


#endif /* RR_GAME_GFX_FADESPR_H */
