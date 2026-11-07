/* RODATA 0x80011118 0xc */
#include "types.h"
#include "m2c_macros.h"
#include "game/sound/sndbgm3.h"
#include "game/sound/sndevt.h"
#include "game/sound/sndloop2.h"
#include "game/sound/sndown.h"
#include "game/sound/sndrst2.h"
#include "game/sound/sndslot.h"
#include "game/sound/sndvab.h"
#include "psyq/libcd/s_2.h"
#include "psyq/libsnd/ssmvol.h"
#include "psyq/libsnd/utrev3.h"
#include "psyq/libsnd/vmsel.h"
#include "psyq/libsnd/vmvinit.h"
#include "rr/library.h"
#include "rr/options_audio_options.h"

struct E { char pad[0x38]; u32 flags; char r[1568 - 0x3C]; };

extern int SsVabClose();
extern struct {
    char pad00[0x24];
    short h24[0x1A];
    char pad0[0x1A08-0x24-0x34];
    int f1a08;
    int a1a0c[7];
    char pad2[0x1A2C - 0x1A28];
    int f1a2c;
    int f1a30;
    char pad3[0x2864 - 0x1A34];
    int f2864;
    int f2868;
} SoundState;
extern int CdAudioRequestPause();
extern s32 SoundPaused;
extern int CdAudioMute();
extern void CdAudioFade(int, short);

void SoundReset(int a0, int a1);
void SoundReverbOffAll(void);
void SoundPause(void);
void SoundResume(void);
void SoundStopAll(void);
void SoundMasterVolumeReset(void);
void SoundSetMasterVolume(short arg0, short arg1);

void SoundReset(int a0, int a1) {
    int a[3] = { 2, 3, 7 };
    int i;
    int *p;
    char *g;
    SsUtAllKeyOff(0);
    if (SoundState.f2864 >= 0) {
        SsSeqStop((short)SoundState.f2864);
        SsSeqClose((short)SoundState.f2864);
        SoundState.f2864 = -1;
    }
    for (i = 0; i < 3; i++) {
        if (SoundState.h24[a[i]] >= 0) {
            SsVabClose(SoundState.h24[a[i]]);
            SoundState.h24[a[i]] = -1;
        }
    }
    SoundMasterVolumeReset();
    ResetPlayerVoices();
    InitLoopVoices(a0);
    ResetSoundChannelVolLR();
    SoundQueueClear();
    SoundLoopReset();
    ReverbReset();
    if (a0 == 0) {
        SoundListenerInit(a1 & 0xFFFF, 0);
    } else {
        SoundListenerInit(a1 & 0xFFFF, 1);
    }
    SetSoundActiveFlag();
    SsUtAllKeyOff(0);
    SoundState.f1a08 = 0;
    i = 6;
    do {
        SoundState.a1a0c[i] = 1;
        i--;
    } while (i >= 0);
    SoundState.f1a30 = 0;
    SoundState.f1a2c = 0;
    SoundState.f2868 = -1;
}

void SoundReverbOffAll(void) {
    SsUtAllKeyOff(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
}

void SoundPause(void) {
    s32 var_a0;
    s32 var_s0;

    SoundPaused = 1;
    CdAudioRequestPause();
    var_s0 = 0;
    var_a0 = 0;
    do {
        SlotSetPause(var_a0, 1);
        var_s0 += 1;
        var_a0 = var_s0;
    } while (var_s0 < 4);
}

void SoundResume(void) {
    s32 var_a0;
    s32 var_s0;

    SoundPaused = 0;
    CdAudioRequestPause();
    var_s0 = 0;
    var_a0 = 0;
    do {
        SlotSetPause(var_a0, 0);
        var_s0 += 1;
        var_a0 = var_s0;
    } while (var_s0 < 4);
}

void SetCdMixMode(int a0) {
    u8 b[4];
    u8 *p;
    if (a0 == 0) {
        SsSetStereo();
        p = b;
        b[0] = 0x80; b[1] = 0; b[2] = 0x80; b[3] = 0;
    } else {
        SsSetMono();
        p = b;
        b[0] = 0x64; b[1] = 0x64; b[2] = 0x64; b[3] = 0x64;
    }
    CdMix(p);
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct E (*)[])&SoundState)

void SoundStopAll(void) {
    int i;
    struct E *p;
    for (i = 0, p = SoundState; i < 4; i++, p++) {
        if (p->flags & 1) SlotKeyOffAll(i);
    }
    for (i = 0; i < 2; i++) SetPlayerVoice(0, 0, 0, i);
    for (i = 0; i < 2; i++) SetLoopVoiceVolumes(i, 0, 0);
    SetSoundChannelVolLR(0, 0, 0);
    SoundLoopSet(5, 0x1E00, 0, 0x40);
    CdAudioMute();
    SoundReverbOffAll();
    ClearSoundLoopActiveFlag();
    SoundPaused = 1;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x18]; s16 a, b, c, d, e, f; } (*))&SoundState)

void SoundMasterVolumeReset(void) {
    SoundState.a = 0x80;
    SoundState.b = 0;
    SoundState.c = 0;
    SoundState.d = 0;
    SoundState.e = 0;
    SoundState.f = 0;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x18]; unsigned short a; short b; short c; unsigned short d; unsigned short e; } (*))&SoundState)

void SoundSetMasterVolume(short arg0, short arg1) {
    if (arg1 >= 2) {
        SoundState.b = arg1;
        SoundState.c = 0;
        SoundState.d = arg0;
        SoundState.e = SoundState.a;
    } else {
        SoundState.a = arg0;
        SoundState.b = 0;
        SoundState.c = 0;
        SoundState.d = 0;
        SoundState.e = 0;
        if (arg0 == 0) {
            SoundStopAll();
        }
    }
    CdAudioFade(arg0 & 0xFFFF, arg1);
}

#undef SoundState
