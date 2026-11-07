#include "rr/audio_loop.h"

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(T (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x286C]; s16 x286C; s16 x286E; s16 x2870; char p1[0x2]; s32 x2874; s32 x2878; } (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct E (*)[])&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SsUtKeyOffV ((void (*)(short))SsUtKeyOffV)

#undef SsUtKeyOffV

/* legacy typing of shared globals in this part of the unit */
#define SoundSlots (*(struct { u32 flags; char pad[1568 - 4]; } (*)[])&SoundSlots)

#undef SoundSlots

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x48]; struct { s32 a, b; char pad[1568 - 8]; } e[1]; } (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundSlots (*(W (*)[])&SoundSlots)

#define SsUtKeyOnV ((int (*)())SsUtKeyOnV)

#undef SoundSlots

#undef SsUtKeyOnV

extern int CdAudioUpdate();

extern int SeqFadeStep();

extern int SoundListenerUpdate();

extern int UpdateSoundSlot();

extern int UpdatePlayerVoices();

extern int UpdateLoopVoices();

extern int UpdateAmbienceVoices();

extern int SoundQueueUpdate();

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
