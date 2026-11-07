#include "rr/menu_screen_exit.h"
void ovl661_func_80114AA0(void)
{
    StopRaceAudioAndRumble();
    SetupRaceAndScene();
    ModeGroup = 1;
    ModeId = 38;
    ModeSubState = 0;
    AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70, 255, 69);
}
