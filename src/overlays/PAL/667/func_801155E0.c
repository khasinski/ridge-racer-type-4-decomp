#include "rr/overlay_screen_exit.h"
void ovl667_func_801155E0(void)
{
    StopRaceAudioAndRumble();
    SetupRaceAndScene();
    ModeGroup=1;
    ModeId=26;
    ModeSubState=0;
    AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,255,69);
}
