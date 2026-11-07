/* RODATA 0x80011178 0x14 */
#include "types.h"
#include "game/sound/cdclose.h"
#include "game/sound/cdstop.h"
#include "rr/cd.h"
#include "rr/bios_devices.h"

extern struct {
    char pad0[0x2670];
    int f2670;
    char pad1[0x2678 - 0x2674];
    int f2678;
    char pad2[0x269C - 0x267C];
    int f269C;
    char pad3[0x26BC - 0x26A0];
    int f26BC;
    char pad4[0x26D0 - 0x26C0];
    char blk[0x1C];
} SoundState;
extern int CdAudioSetVolumeRaw();

void CdAudioStopStep();

void CdAudioStopStep()
{
    int r;
    switch (CdAudioFadeState) {
    case 0:
        SoundState.f269C = 4;
        SoundState.f26BC = 0;
        CdAudioSetVolumeRaw(0);
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
        SoundState.f269C = -1;
        SoundState.f2670 = -1;
        SoundState.f2678 = 0;
        PsyqBzero(SoundState.blk, 0x1C);
        break;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x269C]; s32 a; char pad2[0x50]; s32 b; } (*))&SoundState)

int GetCdAudioPosition(void) {
    if (SoundState.a == 1) return SoundState.b;
    return -1;
}

#undef SoundState
