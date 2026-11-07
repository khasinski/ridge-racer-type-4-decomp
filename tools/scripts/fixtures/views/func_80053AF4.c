#include "rr/audio_cd_controller.h"

/* legacy typing of shared globals in this part of the unit */
#define CdGetSector ((int (*)())CdGetSector)

#define SoundState (*(char (*)[])&SoundState)

#undef CdGetSector

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x288A]; s16 a, b, c, d; } (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct S (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x2864]; s32 x2864; s32 x2868; s32 x286C; char p1[0x18]; u16 x2888; s16 x288A; s16 x288C; s16 x288E; s16 x2890; } (*))&SoundState)

#undef SoundState

extern int SsSeqStop();

extern void SsSeqClose(int);

extern int PsyqBzero();

void SeqFadeVolume(int a0, int a1) {
    if ((s16)a1 >= 2) {
        u16 v = SoundState.x2888;
        SoundState.x288A = a1;
        SoundState.x288C = 0;
        SoundState.x288E = a0;
        SoundState.x2890 = v;
    } else {
        int s = (s16)a0;
        SeqSetVolume(s);
        SoundState.x288A = 0;
        SoundState.x288C = 0;
        SoundState.x288E = 0;
        SoundState.x2890 = 0;
        if (s == 0) {
            if (SoundState.x2864 >= 0) {
                SsSeqStop((s16)SoundState.x2864);
                SsSeqClose((s16)SoundState.x2864);
                PsyqBzero(&SoundState.x286C, 0x1C);
            }
            SoundState.x2864 = -1;
            SoundState.x2868 = -1;
        }
    }
}
