#ifndef RR_AUDIO_LOOP_H
#define RR_AUDIO_LOOP_H

#include "rr/channel_pitch.h"
#include "rr/runtime_settings.h"

typedef struct {
    u8 pad00[4];
    s32 muted04, master08;
    u8 pad0C[12];
    s16 gain18;
    u8 pad1A[12];
    s16 bank26;
    u8 pad28[0x19F8-0x28];
    u16 left19F8, right19FA;
    u32 pitch19FC;
    s32 active1A00;
    s16 program1A04;
    u8 pad1A06[2];
} RrAudioLoopState;

extern RrAudioLoopState SoundState;
extern s16 RaceType;
extern s32 VoiceVolumeScale; /* Alias of SoundState.master08. */
s32 SsUtSetVVol(s32 voice,s32 left,s32 right);
/* Voice uses signed low16. Volume products wrap in32 bits before arithmetic
 * shifting and narrowing to signed16, including full-width master values. */
void SetVoiceVolumeScaled(s32 voice,s32 left,s32 right);
s32 SsUtKeyOnV(s32 voice,s32 bank,s32 program,s32 mode,
                 s32 group,s32 unused,s32 left,s32 right);
/* Captures scaled levels and pitch, then updates channels6/7. Bank/program and
 * active state are re-read after helpers; their return values are ignored. */
void SoundLoopUpdate(void);

RR_ASSERT_SIZE(RrAudioLoopState,0x1A08);
RR_ASSERT_OFFSET(RrAudioLoopState,muted04,4);
RR_ASSERT_OFFSET(RrAudioLoopState,master08,8);
RR_ASSERT_OFFSET(RrAudioLoopState,gain18,0x18);
RR_ASSERT_OFFSET(RrAudioLoopState,bank26,0x26);
RR_ASSERT_OFFSET(RrAudioLoopState,left19F8,0x19F8);
RR_ASSERT_OFFSET(RrAudioLoopState,right19FA,0x19FA);
RR_ASSERT_OFFSET(RrAudioLoopState,pitch19FC,0x19FC);
RR_ASSERT_OFFSET(RrAudioLoopState,active1A00,0x1A00);
RR_ASSERT_OFFSET(RrAudioLoopState,program1A04,0x1A04);
RR_ASSERT_OFFSET(RrAudioSettings,volume31,0x31);
RR_ASSERT_OFFSET(RrAudioSettings,mono32,0x32);

#endif
