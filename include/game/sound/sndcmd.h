#ifndef RR_GAME_SOUND_SNDCMD_H
#define RR_GAME_SOUND_SNDCMD_H

#include "types.h"

extern s32 CarSoundMuted;
void SoundQueueUpdate();
void SoundCloseVabsAndReset(void);
void SoundInitForRace(void);

#endif /* RR_GAME_SOUND_SNDCMD_H */
