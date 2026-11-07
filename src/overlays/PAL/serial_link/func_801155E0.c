#include "rr/overlay_screen_exit.h"
void serial_link_exit_race(void)
{
    StopRaceAudioAndRumble();
    SetupRaceAndScene();
    ModeGroup=1;
    ModeId=26;
    ModeSubState=0;
    AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,255,69);
}
