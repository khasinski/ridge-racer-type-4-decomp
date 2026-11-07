#include "types.h"
#include "game/sound/sndrst2.h"
#include "rr/channels.h"

extern int SetVoiceVolumeScaled();
extern int SsUtKeyOnV();
extern struct { char p0[4]; int f4; char p1[8]; int f10; char p2[4]; short f18; char p3[0xA]; short f24; char p4[0x1996]; unsigned short f19BC, f19BE, f19C0, f19C2; int f19C4; short f19C8; } SoundState;
extern struct { char p0[0x31]; unsigned char f31; short f32; } Options;
extern short RaceType;
extern s16 PanGainTable[];

void SoundLoopSet(int a0, int a1, int a2, int a3);

void UpdateAmbienceVoices(void)
{
    unsigned short s1, s0;
    short t1, t2;
    int v0, v1, a0, a1, i;
    if (SoundState.f10 == 0) {
        s1 = SoundState.f19BC;
        s0 = SoundState.f19C0;
    } else {
        s1 = SoundState.f19BC + SoundState.f19BE;
        s0 = SoundState.f19C0 + SoundState.f19C2;
        if ((short)s1 >= 0x80) s1 = 0x7F;
        if ((short)s0 >= 0x80) s0 = 0x7F;
    }
    if (Options.f32 != 0) {
        v0 = ((short)s1 + (short)s0) / 2;
        s1 = v0;
        s0 = v0;
    }
    if (SoundState.f4 != 0) {
        s0 = 0;
        s1 = 0;
    }
    v1 = (short)s0 * SoundState.f18;
    if (v1 < 0) v1 += 0x7F;
    a0 = (short)s1 * SoundState.f18;
    t1 = v1 >> 7;
    if (a0 < 0) a0 += 0x7F;
    v1 = t1 * Options.f31;
    t2 = a0 >> 7;
    if (v1 < 0) v1 += 0xF;
    v0 = t2 * Options.f31;
    s0 = (unsigned)v1 >> 4;
    if (v0 < 0) v0 += 0xF;
    s1 = (unsigned)v0 >> 4;
    if (RaceType == 5) {
        s0 = 0;
        s1 = 0;
    }
    if ((short)s1 + (short)s0 > 0) {
        i = 0;
        do {
            SetVoiceVolumeScaled(i + 8, (short)s1, (short)s0);
            i++;
        } while (i < 2);
        if (SoundState.f19C4 == 0) {
            SsUtKeyOnV(8, SoundState.f24, SoundState.f19C8, 0, 0x3C, 0, (short)s1, (short)s0);
            SsUtKeyOnV(9, SoundState.f24, SoundState.f19C8, 1, 0x3C, 0, (short)s1, (short)s0);
            SoundState.f19C4 = 1;
        }
    } else if (SoundState.f19C4 != 0) {
        SsUtKeyOffV(8);
        SsUtKeyOffV(9);
        SoundState.f19C4 = 0;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x19F8]; s16 a, b; s32 c, d, e; } (*))&SoundState)

void SoundLoopReset(void) {
    SoundState.a = 0;
    SoundState.b = 0;
    SoundState.c = 0;
    SoundState.d = 0;
    SoundState.e = 0;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SsUtKeyOffV ((int (*)())SsUtKeyOffV)
#define SoundState (*(struct { char p0[0x19F8]; s16 x19F8; s16 x19FA; s32 x19FC; s32 x1A00; s32 x1A04; } (*))&SoundState)

void SoundLoopSet(int a0, int a1, int a2, int a3) {
    int x = PanGainTable[(s16)a3] * (s16)a2;
    int y = PanGainTable[(s16)(0x7F - a3)] * (s16)a2;
    SoundState.x19FC = a1;
    SoundState.x19F8 = y / 16129;
    SoundState.x19FA = x / 16129;
    if (a0 != SoundState.x1A04) {
        SoundState.x1A04 = a0;
        SoundState.x1A00 = 0;
    }
    if (a2 == 0) {
        SsUtKeyOffV(6);
        SoundState.x1A00 = 0;
    }
}

#undef SsUtKeyOffV
#undef SoundState
