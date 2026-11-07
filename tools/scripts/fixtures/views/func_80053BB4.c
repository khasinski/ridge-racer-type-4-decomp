#include "types.h"

extern int SsSeqOpen();

extern int SsSetMarkCallback();

extern int SeqMarkCallback();

extern int ReverbReset();

extern int SetReverbDepthBoth();

extern int SsSeqPlay();

extern int SeqSetVolume();

extern int PsyqBzero();

extern int RestartMotionSequence();

extern struct { s32 a, b, c; } D_800A0CD8[];

extern struct {
    char pad[0x24]; s16 h[1];
    char p2[0x2858 - 0x26]; s32 w[1];
    char p3[0x2864 - 0x285C]; s32 x2864, x2868, x286C;
    char p4[0x287C - 0x2870]; s32 x287C;
} SoundState;

void SeqStartTrack(int a0) {
    int arr[2] = {0x6C, 0x7F};
    if (SoundState.x2868 != a0) {
        SeqMuteVolume();
        SoundState.x2868 = a0;
        SoundState.x2864 = (s16)SsSeqOpen(SoundState.w[a0], SoundState.h[a0 + 2]);
        SsSetMarkCallback((s16)SoundState.x2864, 0, SeqMarkCallback);
        ReverbReset();
        SetReverbDepthBoth(0x32);
        SsSeqPlay((s16)SoundState.x2864, 1, 0);
        SeqSetVolume((s16)arr[a0]);
        PsyqBzero(&SoundState.x286C, 0x1C);
        SoundState.x287C = D_800A0CD8[a0].a;
        RestartMotionSequence();
    }
}
