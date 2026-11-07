#include "game/gfx/fadeset.h"
extern int RaceContext[];
extern short SoundState[];
extern void SsVabClose();

void StartSceneFadeOut(void) {
    RaceContext[0x434 / 4] = 2;
    RaceContext[0x43C / 4] = 0;
    SetParticleBrightFade(0x20, 0x100, 0);
    RaceContext[0x41C / 4] = 3;
    RaceContext[0x420 / 4] = 0;
    if (SoundState[0x32 / 2] >= 0) {
        SsVabClose(SoundState[0x32 / 2]);
        SoundState[0x32 / 2] = -1;
    }
}
