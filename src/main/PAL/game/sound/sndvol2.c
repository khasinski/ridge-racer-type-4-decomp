#include "types.h"
#include "game/sound/sndvol2.h"
#include "rr/channels.h"
#include "rr/channel_pitch.h"

typedef struct {
    char pad[0x22];
    s16 gain22;
    s16 pan24;
    char pad2[0x620 - 0x26];
} Slot;

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} Entry;

typedef struct {
    s32 a;
    s32 b;
    s32 c;
} TableEntry;

extern struct {
    s32 unk0;
    s32 unk4;
    char pad8[0x10];
    s16 unk18;
    char pad1A[0x38 - 0x1A];
    Slot slots[4];
    char pad18B8[0x18C2 - 0x18B8];
    u16 unk18C2;
    char pad18C4[0x1990 - 0x18C4];
    s32 unk1990;
} SoundState;

extern Entry D_800F6714[2];
extern const TableEntry PlayerVoiceParamTable[];
extern s16 PanGainTable[];
extern struct {
    char pad[0x31];
    u8 vol31;
    s16 mono32;
} Options;

extern int SsUtKeyOnV();
extern int SetVoiceVolumeScaled();

void UpdatePlayerVoices(void) {
    int i;
    Entry *s0;
    Slot *s2;
    int s6, s7, fp;
    int a1, a2, v0;

    for (i = 0; i < 2; i++) {
        s0 = &D_800F6714[i];
        if (s0->unkC != s0->unk8) {
            if (s0->unk8 == 0) {
                SsUtKeyOffV((s16)s0->unk4);
            } else if (s0->unk8 >= 0) {
                if (s0->unk8 < 4) {
                    SsUtKeyOnV((s16)s0->unk4, (s16)s0->unk0, (s16)PlayerVoiceParamTable[s0->unk8 - 1].a, 0, 0x3C, 0, 0, 0);
                }
            }
            s0->unkC = s0->unk8;
        }
        if (s0->unk8 != 0) {
            s6 = PlayerVoiceParamTable[s0->unk8 - 1].a;
            s7 = s0->unk10 >> 7;
            fp = s0->unk10 & 0x7F;
            a1 = (s0->unk14 * PlayerVoiceParamTable[s0->unk8 - 1].c) / 128;
            switch (SoundState.unk18C2) {
            case 0:
                switch (i) {
                case 0:
                    s2 = &SoundState.slots[0];
                    break;
                case 1:
                    s2 = &SoundState.slots[SoundState.unk1990];
                    break;
                }
                break;
            case 1:
                s2 = &SoundState.slots[i];
                break;
            }
            a2 = (PanGainTable[s2->pan24] * a1) / 16129;
            a1 = (PanGainTable[(s16)(0x7F - s2->pan24)] * a1) / 16129;
            a2 = a2 * s2->gain22;
            if (a2 < 0) a2 += 0x7F;
            a1 = a1 * s2->gain22;
            a2 >>= 7;
            if (a1 < 0) a1 += 0x7F;
            a1 >>= 7;
            if (Options.mono32 != 0) {
                a2 = (a1 + a2) / 2;
                a1 = a2;
            }
            v0 = a2 * SoundState.unk18;
            if (v0 < 0) v0 += 0x7F;
            a1 = a1 * SoundState.unk18;
            a2 = v0 >> 7;
            if (a1 < 0) a1 += 0x7F;
            a1 >>= 7;
            v0 = a2 * Options.vol31;
            if (v0 < 0) v0 += 0xF;
            a1 = a1 * Options.vol31;
            a2 = v0 >> 4;
            if (a1 < 0) a1 += 0xF;
            a1 >>= 4;
            if (SoundState.unk4 != 0) {
                a2 = 0;
                a1 = 0;
            }
            SetVoiceVolumeScaled(s0->unk4, a1, a2);
            SsUtChangePitch((s16)s0->unk4, (s16)s0->unk0, (s16)s6, 0x3C, 0, (s16)s7, fp);
        }
    }
}
