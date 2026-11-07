#include "types.h"
#include "m2c_macros.h"

extern int PsyqBzero();
extern M2C_UNK ScreenFadeState;
extern M2C_UNK MenuFrameState;

void ClearScreenOverlayState(void) {
    PsyqBzero(&MenuFrameState, 0x20);
    PsyqBzero(&ScreenFadeState, 0xC);
}
