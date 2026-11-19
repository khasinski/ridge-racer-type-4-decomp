#include "game/gfx/fadectl.h"
#include "game/gfx/fadeset.h"
#include "game/gfx/fadein.h"
#include "game/gfx/fadespr.h"

#include "rr/scene_camera_state.h"

void StartDemoFadeIn(int a0) {
    if (RaceContext.fade3E0.mode3C == 0) {
        ResetDemoFadeState();
    }
    RaceContext.fade3E0.ripplePreset58 = a0 + 2;
    RaceContext.fade3E0.rippleMode54 = 1;
    RaceContext.fade3E0.rippleFrame5C = 0;
    if (RaceContext.fade3E0.opacityMode28 == 1 || (RaceContext.fade3E0.opacityMode28 == 0 && RaceContext.fade3E0.opacity0C == 0xFF)) {
        RaceContext.fade3E0.opacityMode28 = 2;
        RaceContext.fade3E0.alternate00 = 1;
        SetParticleBrightFade(0x20, 0, 0x100);
        RaceContext.fade3E0.particlesEnabled14 = 1;
    }
    RaceContext.fade3E0.spriteMode18 = 3;
    RaceContext.fade3E0.mode3C = 2;
}
