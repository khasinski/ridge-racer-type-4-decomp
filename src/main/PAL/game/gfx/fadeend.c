#include "game/gfx/fadeset.h"
#include "rr/scene_camera_state.h"
#include "rr/audio_state_types.h"
#include "psyq/libsnd/vmcfg.h"

void StartDemoFadeOutAndCloseVab(void) {
    RaceContext.fade3E0.rippleMode54 = 2;
    RaceContext.fade3E0.rippleFrame5C = 0;
    SetParticleBrightFade(0x20, 0x100, 0);
    RaceContext.fade3E0.mode3C = 3;
    RaceContext.fade3E0.closeFrame40 = 0;
    if (SoundState.banks24.ids[7] >= 0) {
        SsVabClose(SoundState.banks24.ids[7]);
        SoundState.banks24.ids[7] = -1;
    }
}
