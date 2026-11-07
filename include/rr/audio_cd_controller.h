#ifndef RR_AUDIO_CD_CONTROLLER_H
#define RR_AUDIO_CD_CONTROLLER_H
#include "rr/audio_state_types.h"
#include "rr/cd.h"
#include "game/sound/cdclose.h"
#include "game/sound/cdstart.h"
extern RrAudioState SoundState;
extern char XaRepeatMsg[];
/* Physical alias: request26B4. */
extern s32 CdAudioStopRequest;
s32 DebugPrintStub(const char*,...);
void CdAudioMute(void);void CdAudioSetVolume(u16);void CdAudioSetVolumeRaw(s32);void CdAudioRequestPause(void);
void CdAudioFadeStep(void);void CdAudioPlayStep(void);void CdAudioPauseStep(void);void CdAudioStopStep(void);
s32 CdLastCom(void);void CdFlush(void);s32 CdPosToInt(const u8*);s32 CdDiskReady(s32);
/* CD parameters use a two-byte filter and three defined location bytes.
 * Active TOC indices address one of16 banks and256 complete8-byte entries.
 * Services may mutate controller fields; counters/products wrap32 bits.
 * All software pointers remain full width, including automatic CD buffers. */
void CdAudioUpdate(void);
#endif
