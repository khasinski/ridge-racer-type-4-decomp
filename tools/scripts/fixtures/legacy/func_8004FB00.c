#include "types.h"
extern int SlotKeyOffAll();
extern int SetPlayerVoice();
extern int SetLoopVoiceVolumes();
extern int SetSoundChannelVolLR();
extern int SoundLoopSet();
extern int CdAudioMute();
extern int SoundReverbOffAll();
extern int ClearSoundLoopActiveFlag();
extern int SoundPaused;
struct E { char pad[0x38]; u32 flags; char r[1568 - 0x3C]; };
extern struct E SoundState[];
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
