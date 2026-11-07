#ifndef RR_OPTIONS_AUDIO_OPTIONS_H
#define RR_OPTIONS_AUDIO_OPTIONS_H
#include "rr/options_menu.h"
#include "rr/runtime_settings.h"
extern s32 options_music_volume_setting,options_sfx_volume_setting;
extern s16 MusicSeqVolume;
void SeqSetVolume(s16);
void SetCdMixMode(s32);
void options_init_audio_rows(void);
/* Eight complete UI rows and a full0xB70 frame context are available. The
 * word index stays0..3; its signed low half is edited by width2 input. Mono is
 *0..1 whenever indexed. Services may replace roots and all mutable settings.
 * Timers wrap signed16; layout and scene updates wrap32. */
void options_update_audio_menu(void);
#endif
