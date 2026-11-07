#include "rr/frame_loop.h"
void garage_init_collection(void);
void UpdateScreenFade(s32 command);
void garage_enter_collection(void)
{
    garage_init_collection();
    ModeGroup=1;
    ModeId=66;
    ModeSubState=0;
    UpdateScreenFade(0x10002);
}
