#include "types.h"
#include "game/sound/sndchan.h"
#include "rr/channel_pitch.h"

typedef struct {
    u32 flags00;
    s16 bank04;
    u16 reserved06;
    s32 voice08;
    s32 count0C;
    s32 level10;
    s32 program14;
    s32 previous18;
    s32 reserved1C;
    s16 level20;
    s16 gain22;
    s16 pan24;
    s16 reserved26;
    s16 group28;
    s16 reserved2A;
    s16 spatial2C;
    s16 reserved2E;
    s32 filter30;
    u8 reserved34[0x5DC - 0x34];
    u16 programs5DC[2][5];
    u16 duration5F0;
    u16 gain5F2;
    u16 spread5F4;
    u16 reserved5F6;
    struct {
        u16 coarse;
        u16 fine;
    } pitch5F8[10];
} RrSlot620;

struct T { s32 k[9]; s32 v[9]; };

struct E { char pad[0x74]; struct T t[2][10]; char r[12]; };

extern RrSlot620 SoundSlots[];
extern u16 SlotPitchOffsetTable[];
extern s16 PanGainTable[];
extern struct {
    char pad[0x31];
    u8 volume31;
    s16 mono32;
} Options;
extern s16 SoundGain;
extern int SsUtKeyOnV();
extern void SetVoiceVolumeScaled();
extern struct E SoundState[];

int SlotInterpCurve(int a0, int x, int a2, int a3);

void UpdateSoundSlot(int a0) {
    RrSlot620 *s0 = &SoundSlots[a0];
    u16 fp = 0;
    int s3, i;
    u16 sp20;
    int sp28;
    int s5;
    int v1;

    if (s0->program14 != s0->previous18) {
        s3 = s0->voice08;
        for (i = 0; i < s0->count0C; i++, s3++) {
            if (s0->programs5DC[0][i] != s0->programs5DC[1][i]) {
                SsUtKeyOnV((s16)s3, s0->bank04, (s16)s0->programs5DC[s0->program14][i], 0, s0->group28, 0, 0, 0);
            }
        }
        s0->previous18 = s0->program14;
    }

    if ((s0->flags00 & 4) == 0) {
        sp28 = s0->level20;
    } else {
        sp28 = 0;
    }

    s5 = (s0->level10 * 0x2800) / s0->duration5F0;
    s3 = s0->voice08;
    if (s5 > 0x2800) {
        s5 = 0x2800;
    }

    v1 = 0x11A800 / (0x2350 - s0->filter30);
    v1 -= 0x4D;
    if (v1 < 0) {
        v1 = 0;
    }
    if (v1 >= 0x120) {
        v1 = 0x11F;
    }
    sp20 = SlotPitchOffsetTable[v1];

    for (i = 0; i < s0->count0C; i++, s3++) {
        int s6 = SlotInterpCurve(i * 2, s5, s0->program14, a0);
        int a1 = SlotInterpCurve(i * 2 + 1, s5, s0->program14, a0) * sp28;
        int a2;
        if (a1 < 0) {
            a1 += 0x7F;
        }
        a1 >>= 7;
        a2 = (PanGainTable[s0->pan24] * a1) / 16129;
        a1 = (PanGainTable[(s16)(0x7F - s0->pan24)] * a1) / 16129;

        if (a0 >= 2 && i == 2) {
            int t = PanGainTable[s0->spatial2C];
            fp = s0->spread5F4;
            a2 = (t * a2) / 16129;
            a1 = (t * a1) / 16129;
            a2 = -a2;
            a1 = -a1;
        }

        a2 = (a2 * s0->level20 * s0->gain22) / 16384;
        a1 = (a1 * s0->level20 * s0->gain22) / 16384;
        a2 = (a2 * s0->gain5F2) / 128;
        a1 = (a1 * s0->gain5F2) / 128;

        if (Options.mono32 != 0) {
            a2 = (a2 + a1) / 2;
            a1 = a2;
        }

        a1 = (a1 * SoundGain) / 128;
        a2 = (a2 * SoundGain) / 128;
        a2 = (a2 * Options.volume31) / 16;
        a1 = (a1 * Options.volume31) / 16;

        SetVoiceVolumeScaled(s3, a1, a2);

        {
            int off = (s0->program14 * 5 + i) * 4;
            RrSlot620 *p = (RrSlot620 *)((char *)s0 + off);
            s16 v;
            int t1;
            s16 t0;
            int c;
            v = (p->pitch5F8[0].fine * s6) / 128;
            v += sp20;
            v += fp;
            t0 = v % 128;
            t1 = v / 128;
            c = p->pitch5F8[0].coarse + t1;
            SsUtChangePitch((s16)s3, s0->bank04, (s16)s0->programs5DC[s0->program14][i], s0->group28, 0, (s16)c, t0);
        }
    }
}

int SlotInterpCurve(int a0, int x, int a2, int a3) {
    int i, lo, vlo, vhi, klo, khi;
    for (i = 1; i < 9; i++) {
        if (x < SoundState[a3].t[a2][a0].k[i]) break;
    }
    lo = i - 1;
    vlo = SoundState[a3].t[a2][a0].v[lo];
    vhi = SoundState[a3].t[a2][a0].v[i];
    klo = SoundState[a3].t[a2][a0].k[lo];
    khi = SoundState[a3].t[a2][a0].k[i];
    return (vhi - vlo) * (x - klo) / (khi - klo) + vlo;
}
