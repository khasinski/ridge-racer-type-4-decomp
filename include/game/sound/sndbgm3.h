#ifndef RR_GAME_SOUND_SNDBGM3_H
#define RR_GAME_SOUND_SNDBGM3_H

#include "types.h"

void func_80051248(int a0);
void func_800512C4(int a0, int a1, int a2);
void UpdateLoopVoices();
void ResetSoundChannelVolLR(void);
void SetSoundChannelVolLR(int a0, s16 a1, s16 a2);

#endif /* RR_GAME_SOUND_SNDBGM3_H */
