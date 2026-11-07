#include "rr/frame_loop.h"
void ovl662_func_80116148(void);
void UpdateScreenFade(s32 command);
void ovl662_func_801161B0(void)
{
    ovl662_func_80116148();
    ModeGroup=1;
    ModeId=66;
    D_800F4D8A=0;
    UpdateScreenFade(0x10002);
}
