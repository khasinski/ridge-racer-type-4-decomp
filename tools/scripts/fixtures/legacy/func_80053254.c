#include "types.h"
extern int CdAudioSetVolume();extern int CdAudioBeginPlay();extern s32 CdAudioSubStep;
extern struct { char p0[0x2664]; s32 x2664; char p1[0x8]; s32 x2670; s32 x2674; char p2[0x1C]; s32 x2694; char p3[0x8]; s32 x26A0; s32 x26A4; char p4[0x14]; s32 x26BC; char p5[0xE]; u16 x26CE; } SoundState;
void CdAudioResumeStep(void) {
    switch (SoundState.x26A4) {
    case 0:
        CdAudioSetVolume(SoundState.x26CE);
        SoundState.x2664 = SoundState.x2694;
        SoundState.x2674 = 0;
        SoundState.x26A4++;
        break;
    case 1:
        if (SoundState.x2674 == 0) {
            if (SoundState.x26BC == 1) CdAudioBeginPlay(SoundState.x26A0);
            else SoundState.x2670 = -1;
            CdAudioSubStep = 0;
        }
        break;
    }
}
