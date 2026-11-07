#ifndef RR_OVERLAY665_AUDIO_OPTIONS_H
#define RR_OVERLAY665_AUDIO_OPTIONS_H
#include "rr/overlay665_menu.h"
#include "rr/runtime_settings.h"
extern s32 options_D_80119754,options_D_80119758;
extern s16 MusicSeqVolume;
void SeqSetVolume(s16);
void SetCdMixMode(s32);
void options_func_80114FD8(void);
/* Eight complete UI rows and a full0xB70 frame context are available. The
 * word index stays0..3; its signed low half is edited by width2 input. Mono is
 *0..1 whenever indexed. Services may replace roots and all mutable settings.
 * Timers wrap signed16; layout and scene updates wrap32. */
void options_func_801151A4(void);
#endif
