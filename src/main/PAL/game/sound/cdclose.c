/* RODATA 0x80011160 0x14 */
#include "types.h"
#include "game/sound/cdclose.h"
#include "rr/cd.h"
#include "rr/menu_overlay_runtime.h"

extern struct {
    char pad0[0x2670];
    int f2670;
    char pad1[0x2678 - 0x2674];
    int f2678;
    char pad2[0x2688 - 0x267C];
    int f2688;
    char pad3[0x2694 - 0x268C];
    int f2694;
    char pad4[0x269C - 0x2698];
    int f269C;
    char pad5[0x26B8 - 0x26A0];
    int f26B8;
} SoundState;
extern int CdAudioSetVolume();
extern s32 CdAudioSubStep;

void CdAudioPauseStep();
void CdAudioResumeStep(void);
void CdAudioLoopStep(void);

void CdAudioPauseStep()
{
    int r;
    switch (CdAudioFadeState) {
    case 0:
        SoundState.f269C = 2;
        CdAudioSetVolume(0);
        SoundState.f2678 = SoundState.f2678 + 1;
    case 1:
        if (CdSync(1, 0) == 0) break;
        SoundState.f2678 = SoundState.f2678 + 1;
    case 2:
        if (CdControl(9, 0, 0) == 0) break;
        goto inc;
    case 3:
        r = CdSync(1, 0);
        if (r == 2) {
    inc:
            SoundState.f2678 = SoundState.f2678 + 1;
        } else if (r == 5) {
            CdAudioFadeState = 1;
        }
        break;
    case 4:
        SoundState.f2670 = -1;
        SoundState.f2678 = 0;
        if (SoundState.f26B8 == 1) SoundState.f2694 = SoundState.f2688;
        break;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x2664]; s32 x2664; char p1[0x8]; s32 x2670; s32 x2674; char p2[0x1C]; s32 x2694; char p3[0x8]; s32 x26A0; s32 x26A4; char p4[0x14]; s32 x26BC; char p5[0xE]; u16 x26CE; } (*))&SoundState)

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

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x2664]; s32 x2664; char p1[0x4]; s32 x266C; char p2[0x4]; s32 x2674; char p3[0xC]; s32 x2684; char p4[0x10]; s32 x2698; char p5[0x8]; s32 x26A4; } (*))&SoundState)

void CdAudioLoopStep(void) {
    switch (SoundState.x26A4) {
    case 0:
        SoundState.x2674 = 0;
        SoundState.x26A4 = 1;
        SoundState.x2664 = SoundState.x2684;
        SoundState.x266C = SoundState.x2698;
        break;
    case 1:
        if (SoundState.x2674 == 0) {
            CdAudioBeginPlay(1);
            SoundState.x26A4 = 2;
        }
        break;
    }
}

#undef SoundState
