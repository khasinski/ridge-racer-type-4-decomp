#include "game/gfx/fadectl.h"
#include "game/gfx/fadeset.h"
#include "game/gfx/fadein.h"
extern int RaceContext[];

void StartSceneFadeIn(int a0) {
    if (RaceContext[0x41C / 4] == 0) {
        func_8005C7A4();
    }
    RaceContext[0x438 / 4] = a0 + 2;
    RaceContext[0x434 / 4] = 1;
    RaceContext[0x43C / 4] = 0;
    if (RaceContext[0x408 / 4] != 1) {
        if (RaceContext[0x408 / 4] == 0) {
            if (RaceContext[0x3EC / 4] == 0xFF) {
                goto b5;
            }
        }
    } else {
b5:
        RaceContext[0x408 / 4] = 2;
        RaceContext[0x3E0 / 4] = 1;
        SetParticleBrightFade(0x20, 0, 0x100);
        RaceContext[0x3F4 / 4] = 1;
    }
    RaceContext[0x3F8 / 4] = 3;
    RaceContext[0x41C / 4] = 2;
}
