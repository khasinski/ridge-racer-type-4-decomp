#include "types.h"
#include "rr/audio_loop.h"
#include "m2c_macros.h"
#include "game/sound/fxramp1.h"
#include "game/sound/sndbgm3.h"
#include "game/sound/sndchan.h"
#include "game/sound/sndcmd.h"
#include "game/sound/sndctrl.h"
#include "game/sound/sndmain.h"
#include "game/sound/sndrst2.h"
#include "game/sound/sndvol2.h"
#include "game/data/trkld6.h"
#include "game/sound/sndslot.h"
#include "rr/memory.h"
#include "rr/menu_overlay_runtime.h"

typedef struct { short pad[0xC]; short o; short n; short c; short to; short from; } T;

struct E { char pad[0x38]; u32 flags; char p[0xC]; s32 a, b, c; char r[1568 - 0x54]; };

struct S { int a; int b; int c; int d; char pad[0x610]; };

typedef struct {
    int f0;
    short h4;
    short pad6;
    int f8;
    int fC;
    int pad10;
    int f14;
    char pad18[0x28 - 0x18];
    short h28;
    short h2A;
    char pad2C[0x5DC - 0x2C];
    short h5DC[(0x620 - 0x5DC) / 2];
} W;

extern int CdAudioUpdate();
extern struct S SoundSlots[];

void SoundMasterFadeStep(void);
void SoundUpdate(void);
void SetVoiceVolumeScaled(s32 a0, int a1, int a2);
void SlotStart(int a0);
void SlotSetPause(int a0, int a1);
void SlotSetLevelProgram(int a0, int a1, int a2);
void SlotKeyOn();

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(T (*))&SoundState)

void SoundMasterFadeStep(void) {
    T *p = &SoundState;
    int n = p->n;
    int v = 0x80;
    int c;
    short c1;
    if (n > 0) {
        c = (unsigned short)p->c;
        if (p->c < n) {
            n = ((p->to - p->from) << 12) / n;
            c1 = c + 1;
            p->c = c1;
            v = (unsigned short)p->from + n * c1 / 4096;
        } else {
            p->n = 0;
            SoundStopAll(n, p, c);
            return;
        }
    }
    p->o = v;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x286C]; s16 x286C; s16 x286E; s16 x2870; char p1[0x2]; s32 x2874; s32 x2878; } (*))&SoundState)

void SoundUpdate(void) {
    int i;
    u32 *p;
    int a;
    if (SoundState.x2878 != SoundState.x2874) {
        a = SoundState.x2874 - 1;
        SoundState.x286C = 1;
        if ((a & 1) == 0) SoundState.x286E = 1;
        if ((a & 3) == 0) SoundState.x2870 = 1;
    } else {
        SoundState.x286C = 0;
        SoundState.x286E = 0;
        SoundState.x2870 = 0;
    }
    i = 0;
    SoundState.x2878 = SoundState.x2874;
    CdAudioUpdate();
    SeqFadeStep();
    SoundMasterFadeStep();
    SoundListenerUpdate();
    for (p = (u32 *)((char *)&SoundState + 0x38); i < 4; i++, p += 0x188) {
        if (*p & 1) UpdateSoundSlot(i);
    }
    UpdatePlayerVoices();
    UpdateLoopVoices();
    UpdateAmbienceVoices();
    SoundQueueUpdate();
    SoundLoopUpdate();
}

#undef SoundState

void SetVoiceVolumeScaled(s32 a0, int a1, int a2) {
    int v = VoiceVolumeScale;
    SsUtSetVVol((s16)a0, (s16)((s32)((u32)a1 * (u32)v) >> 7), (s16)((s32)((u32)a2 * (u32)v) >> 7));
}

void SlotsStartForRace(void) {
    M2C_UNK var_a0;

    SlotStart(0);
    if (LinkRaceSoundFlag == 0) {
        if (D_800ACBC8 >= 0) {
            SlotStart(2);
            var_a0 = 3;
            goto block_4;
        }
    } else {
        var_a0 = 1;
block_4:
        SlotStart(var_a0);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct E (*)[])&SoundState)

void SlotStart(int a0) {
    struct E *base = SoundState;
    struct E *e = base + a0;
    e->flags |= 1;
    SlotKeyOn(a0);
    e->c = e->b;
    SlotSetPause(a0, 0);
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SsUtKeyOffV ((void (*)(short))SsUtKeyOffV)

void SlotKeyOffAll(int arg0) {
 int i; int s0; struct S *t = &SoundSlots[arg0]; s0 = t->c; i = 0; while (i < t->d) { SsUtKeyOffV(s0); i++; s0++; }
 t->a = 0;
}

#undef SsUtKeyOffV

/* legacy typing of shared globals in this part of the unit */
#define SoundSlots (*(struct { u32 flags; char pad[1568 - 4]; } (*)[])&SoundSlots)

void SlotSetPause(int a0, int a1) {
    u32 *p = &SoundSlots[a0].flags;
    if (a1 == 1) *p |= 4;
    else *p &= ~4;
}

#undef SoundSlots

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x48]; struct { s32 a, b; char pad[1568 - 8]; } e[1]; } (*))&SoundState)

void SlotSetLevelProgram(int a0, int a1, int a2) {
    SoundState.e[a2].a = a0;
    SoundState.e[a2].b = a1;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundSlots (*(W (*)[])&SoundSlots)
#define SsUtKeyOnV ((int (*)())SsUtKeyOnV)

void SlotKeyOn(a0)
int a0;
{
    int s[4];
    W *w = &SoundSlots[a0];
    unsigned short *q;
    int i;
    int k;
    int t;
    PsyqMemset(s, 0, 0x10);
    s[3] = 2;
    k = w->f8;
    i = 0;
    if (w->fC > 0) {
        q = (unsigned short *)((int)s + a0 * 4);
        do {
            SsUtKeyOnV((short)k, w->h4, w->h5DC[w->f14 * 5 + i], 0, (short)(*q + 0x3C), 0, 0, 0);
            k++;
            i++;
        } while (i < w->fC);
    }
    t = *(unsigned short *)&s[a0];
    w->h2A = 0;
    w->h28 = t + 0x3C;
}

#undef SoundSlots
#undef SsUtKeyOnV
