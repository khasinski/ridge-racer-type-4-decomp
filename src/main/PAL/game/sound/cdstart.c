#include "types.h"
#include "m2c_macros.h"
#include "rr/movie_playback_stream.h"
#include "rr/cd.h"

#define D SoundState

typedef struct {
    char pad0[0x2664];
    int f2664;
    char pad1[0x2670 - 0x2668];
    int f2670;
    char pad2[0x2678 - 0x2674];
    int f2678;
    char pad3[0x2694 - 0x267C];
    int f2694;
    char pad4[0x269C - 0x2698];
    int f269C;
    char pad5[0x26A4 - 0x26A0];
    int f26A4;
    char pad6[0x26B0 - 0x26A8];
    int f26B0;
    int f26B4;
    int f26B8;
    char pad7[0x26C4 - 0x26BC];
    unsigned short h26C4;
    char pad8[0x26CE - 0x26C6];
    unsigned short h26CE;
} S;

extern struct { char p0[0x2670]; s32 x2670; char p1[0x4]; s32 x2678; char p2[0x20]; s32 x269C; s32 x26A0; char p3[0x4]; s32 x26A8; char p4[0x4]; s32 x26B0; s32 x26B4; char p5[0x4]; s32 x26BC; u16 x26C0; char p6[0xC]; u16 x26CE; } SoundState;
extern int CdDiskReady();

void CdAudioBeginPlay(int a0);
void CdAudioMute(void);
void CdAudioSetVolume(u16 a0);
void CdAudioSetVolumeRaw(s32 arg0);
void CdAudioFade(u16 a0, int a1);
void CdAudioRequestPause();
void CdAudioSetFilter(int a0);
int CdAudioFilterPending(void);
void CdAudioFilterStep(void);

void CdAudioBeginPlay(int a0) {
    D.x2670 = 1;
    D.x26BC = 1;
    if (a0 == 1) D.x26A0 = a0;
    else D.x26A0 = 0;
    D.x2678 = 0;
    D.x26A8 = 0;
    D.x26B0 = 0;
    D.x26B4 = 0;
    if (D.x269C == 2) CdAudioSetVolume(D.x26CE);
    else CdAudioFade(D.x26C0, 0);
    CdAudioSetVolumeRaw(0);
}

void CdAudioMute(void) {
    CdAudioFade(0, 0);
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x26C4]; u16 v; } (*))&SoundState)

void CdAudioSetVolume(u16 a0) {
    s16 s;
    if (SoundState.v != a0) {
        SoundState.v = (u8)a0;
        s = (SoundState.v * OptionsMusicVolume) / 16;
        SsSetSerialVol(0, s, s);
    }
}

#undef SoundState

void CdAudioSetVolumeRaw(s32 arg0) {
    s32 var_a0;
    u32 temp_a1;

    var_a0 = (arg0 & 0xFF) * OptionsMusicVolume;
    if (var_a0 < 0) {
        var_a0 += 0xF;
    }
    temp_a1 = (u32) var_a0 >> 4;
    SsSetSerialVol(0, temp_a1, temp_a1);
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x2670]; s32 x2670; char p1[0x4]; s32 x2678; char p2[0x20]; s32 x269C; char p3[0x8]; s32 x26A8; char p4[0x4]; s32 x26B0; s32 x26B4; char p5[0xC]; u16 x26C4; s16 x26C6; s16 x26C8; u16 x26CA; u16 x26CC; u16 x26CE; } (*))&SoundState)

void CdAudioFade(u16 a0, int a1) {
    if (a1 >= 2) {
        u16 v = SoundState.x26C4;
        SoundState.x26C6 = a1;
        SoundState.x26C8 = 0;
        SoundState.x26CA = a0;
        SoundState.x26CC = v;
    } else {
        CdAudioSetVolume(a0);
        SoundState.x26C6 = 0;
        SoundState.x26C8 = 0;
        SoundState.x26CA = 0;
        SoundState.x26CC = 0;
        if (a0 == 0) {
            int r = SoundState.x269C;
            if (r != -1) {
                if (SoundState.x2670 != 1) {
                    SoundState.x2670 = 4;
                    SoundState.x2678 = 0;
                    SoundState.x26A8 = 0;
                    if (r == 2) CdAudioSetVolume(SoundState.x26CE);
                } else {
                    if (SoundState.x26B0 < 0x20) {
                        SoundState.x26B4 = 2;
                    } else {
                        SoundState.x2670 = 4;
                        SoundState.x2678 = 0;
                        SoundState.x26A8 = 0;
                    }
                }
            }
        }
    }
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(S (*))&SoundState)

void CdAudioRequestPause()
{
    switch (SoundState.f269C) {
    case 0:
    case 1:
        if (SoundState.f2670 != 2) {
            SoundState.f2670 = 2;
            SoundState.f2678 = 0;
            SoundState.h26CE = SoundState.h26C4;
        }
        if (SoundState.f269C == 1) {
            SoundState.f26B8 = SoundState.f269C;
        } else {
            SoundState.f26B8 = 0;
            SoundState.f2694 = SoundState.f2664;
        }
        break;
    case 2:
        if (SoundState.f2670 != 3) {
            SoundState.f2670 = 3;
            SoundState.f26A4 = 0;
            SoundState.f2678 = 0;
        }
        break;
    }
    if (SoundState.f2670 == 1) {
        if (SoundState.f26B0 < 0x20) {
            SoundState.f26B4 = SoundState.f2670;
            return;
        }
        SoundState.f2670 = 2;
        SoundState.f2678 = 0;
        SoundState.h26CE = SoundState.h26C4;
    }
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x266C]; s32 a, b, c, d; } (*))&SoundState)

void CdAudioSetFilter(int a0) {
    SoundState.a = a0;
    SoundState.b = 6;
    SoundState.d = 0;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x2674]; s32 a, b; char p[4]; s32 c; } (*))&SoundState)

int CdAudioFilterPending(void) {
    if (SoundState.a != 0 || SoundState.b != 0) return 1;
    return SoundState.c;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x266C]; int a; int b; int pp; int c; int d; int e; char pad2[0x14]; int g; } (*))&SoundState)

void CdAudioFilterStep(void) {
    char sp[8];
    switch (SoundState.c) {
    case 0:
        sp[0] = 1;
        sp[1] = SoundState.a;
        if (CdControl(0xD, sp, 0) != 0) {
            SoundState.c++;
            return;
        }
        if (CdDiskReady(1) == 0x10) {
            SoundState.d = 1;
            SoundState.e = -3;
            SoundState.b = -1;
            SoundState.c = 0;
            return;
        }
        return;
    case 1:
        SoundState.b = -1;
        SoundState.c = 0;
        SoundState.e = 0;
        SoundState.g = SoundState.a;
        break;
    }
}

#undef SoundState
