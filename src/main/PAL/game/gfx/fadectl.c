#include "game/gfx/fadeset.h"
#include "game/sound/sndvab.h"
#include "game/gfx/fadectl.h"
#include "game/gfx/fadespr.h"

#include "rr/scene_camera_state.h"

void ResetDemoFadeState(void) {
    int v, w, x;
    RaceContext.fade3E0.rippleMode54 = 0;
    RaceContext.fade3E0.rippleIntensity60 = 0;
    v = RaceContext.fade3E0.opacityMode28;
    if (v == 1 || (v == 0 && RaceContext.fade3E0.opacity0C == 0)) {
        if (RaceContext.fade3E0.particlesEnabled14 == 0) {
            SetParticleBrightFade(1, 0, 0x100);
            RaceContext.fade3E0.particlesEnabled14 = 1;
        }
    }
    w = RaceContext.fade3E0.opacityMode28;
    if (w == 2 || (w == 0 && RaceContext.fade3E0.opacity0C == 0xFF)) {
        if (RaceContext.fade3E0.particlesEnabled14) {
            SetParticleBrightFade(0, 0, 0);
            RaceContext.fade3E0.particlesEnabled14 = 0;
        }
    }
    RaceContext.fade3E0.spriteY20 = 0x8C;
    RaceContext.fade3E0.spriteIntensity24 = 0x100;
    RaceContext.fade3E0.spriteMode18 = 0;
    RaceContext.fade3E0.linesEnabled30 = 1;
    RaceContext.fade3E0.linesProgress32 = 0x10;
    RaceContext.fade3E0.sprite3Progress3A = 0x10;
    RaceContext.fade3E0.sprite2Progress36 = 0x10;
    RaceContext.fade3E0.sprite3Enabled38 = 1;
    RaceContext.fade3E0.sprite2Enabled34 = 1;
    RaceContext.fade3E0.frame04 = 0xBB8;
    x = RaceContext.fade3E0.soundPlayed50;
    RaceContext.fade3E0.cycleFrame44 = 0;
    RaceContext.fade3E0.mode3C = 1;
    if (x == 0) {
        PlaySfxBank2(RaceContext.fade3E0.sound4C);
        RaceContext.fade3E0.soundPlayed50 = 1;
    }
}
