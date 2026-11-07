#include "rr/overlay662_garage.h"
s32 garage_is_layer_empty(void)
{
    u32 mode=garage_logo_state.layer23;
    u32 selected=garage_logo_state.active21;
    u32 *cursor;
    s32 i;
    if(mode==0)
        cursor=garage_logo_state.image38[selected];
    else
        cursor=garage_logo_state.image40[selected];
    for(i=0;i<512;cursor++){i++;if(*cursor)return 0;}
    return 1;
}
