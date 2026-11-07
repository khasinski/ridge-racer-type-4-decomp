#include "rr/spu_tick_control.h"
void setIntrVSync(s32 slot,RrSpuTickCallback callback)
{
    RrSpuTickCallback *base=VSyncCallbackTable;
    RrSpuTickCallback *p=base+slot;
    if(callback!=*p) *p=callback;
}
