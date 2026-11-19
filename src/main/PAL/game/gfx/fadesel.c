#include "types.h"
#include "game/gfx/fadesel.h"
#include "game/gfx/fadespr.h"

#include "rr/scene_camera_state.h"

void SelectDemoFadePreset(int preset)
{
    RaceContext.fade3E0.spriteIntensity24 = 0;
    RaceContext.fade3E0.spriteMode18 = 0;
    RaceContext.fade3E0.rippleMode54 = 1;
    RaceContext.fade3E0.rippleFrame5C = 0;
    RaceContext.fade3E0.rippleIntensity60 = 0;
    switch (preset) {
    case 0:
        RaceContext.fade3E0.ripplePreset58 = 0xB;
        break;
    case 1:
        RaceContext.fade3E0.ripplePreset58 = 8;
        break;
    case 2:
        RaceContext.fade3E0.ripplePreset58 = 0xC;
        break;
    }
    RaceContext.fade3E0.particlesEnabled14 = 1;
    RaceContext.fade3E0.frame04 = 0xBB8;
    RaceContext.fade3E0.mode3C = 2;
}

void EnableDemoFadeSkip(void) {
    RaceContext.fade3E0.scaleMode48 = 1;
}
