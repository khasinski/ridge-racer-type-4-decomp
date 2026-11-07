#include "types.h"
#include "game/sound/sndbgm3.h"
#include "rr/channels.h"
#include "game/gfx/color.h"

struct EE { s32 a, b, c; s16 d, e; s32 f; };

struct EE_1 { s32 a, b, c; s16 d[2]; s32 f; };

extern struct {
    char pad[0x26]; s16 h; char p1[0x1994 - 0x28]; s32 f1;
    struct EE e[2];
} SoundState;
extern char Options[];
extern int SetVoiceVolumeScaled();
extern int SsUtKeyOnV();

void SetLoopVoiceVolumes(int a0, int a1, int a2);
void SetSoundChannelVolLR(int a0, s16 a1, s16 a2);

void func_80051248(int a0) {
    int k, i;
    if (a0 == 0) {
        k = 0xB;
        SoundState.f1 = 1;
        SoundState.e[0].f = 0;
    } else {
        k = 0xE;
        SoundState.f1 = 1;
        SoundState.e[0].f = 1;
    }
    for (i = 0; i < 2; i++) {
        SoundState.e[i].e = 0;
        SoundState.e[i].d = 0;
        SoundState.e[i].a = 0;
        SoundState.e[i].b = SoundState.h;
        SoundState.e[i].c = k + i;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x1998]; struct EE_1 e[2]; } (*))&SoundState)

void SetLoopVoiceVolumes(int a0, int a1, int a2) {
    int i;
    if (SoundMonoFlag == 0) {
        SoundState.e[a0].d[0] = a1;
        SoundState.e[a0].d[1] = a2;
    } else {
        SoundState.e[a0].d[0] = (a1 + a2) / 2;
        SoundState.e[a0].d[1] = (a1 + a2) / 2;
    }
    for (i = 0; i < 2; i++) {
        if (SoundState.e[a0].d[i] < 2) SoundState.e[a0].d[i] = 0;
    }
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(char (*)[])&SoundState)

void UpdateLoopVoices() {
    int i;
    register char *s2 asm("$18"); /* ASM_BARRIER: pin */
    char *s4;
    char *s5;
    int s0, s1, a, b;
    i = 0;
    s5 = Options;
    s4 = SoundState;
    s2 = s4;
    for (i = 0; i < 2; i++, s2 += 0x14) {
        if (*(int *)(s2 + 0x1994) == 0) continue;
        s0 = *(short *)(s2 + 0x19A4);
        s1 = *(short *)(s2 + 0x19A6);
        if (*(int *)(s4 + 4) != 0) {
            s1 = 0;
            s0 = 0;
        }
        a = *(short *)(s4 + 0x18);
        s1 = (s1 * a) / 128;
        s0 = (s0 * a) / 128;
        b = *(unsigned char *)(s5 + 0x31);
        s1 = (s1 * b) / 16;
        s0 = (s0 * b) / 16;
        if (s0 + s1 > 0) {
            SetVoiceVolumeScaled(*(int *)(s2 + 0x19A0), s0, s1);
            if (*(int *)(s2 + 0x1998) == 0) {
                SsUtKeyOnV(*(short *)(s2 + 0x19A0), *(short *)(s2 + 0x199C), 7, 0, 0x3C, 0, (short)s0, (short)s1);
                *(int *)(s2 + 0x1998) = 1;
            }
        } else if (*(int *)(s2 + 0x1998) != 0) {
            SsUtKeyOffV(*(short *)(s2 + 0x19A0));
            *(int *)(s2 + 0x1998) = 0;
        }
    }
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x19BC]; s16 a[2]; s16 b[2]; s32 c, d; } (*))&SoundState)

void ResetSoundChannelVolLR(void) {
    int i;
    for (i = 0; i < 2; i++) {
        SoundState.a[i] = 0;
        SoundState.b[i] = 0;
    }
    SoundState.c = 0;
    SoundState.d = 5;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x19BC]; s16 a[2]; s16 b[2]; } (*))&SoundState)

void SetSoundChannelVolLR(int a0, s16 a1, s16 a2) {
    SoundState.a[a0] = a1;
    SoundState.b[a0] = a2;
}

#undef SoundState
