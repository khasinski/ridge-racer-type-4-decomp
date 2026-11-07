#include "rr/overlay_screen_exit.h"
void ovl667_func_801155E0(void)
{
    StopRaceAudioAndRumble();
    SetupRaceAndScene();
    ModeGroup=1;
    ModeId=26;
    D_800F4D8A=0;
    AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,255,69);
}
