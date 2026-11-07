#include "rr/garage_garage.h"
void garage_save_undo_copy(void)
{
    s32 i;
    u8 selected=garage_logo_state.active21;
    u8 other;
    u32 *source,*destination;
    garage_logo_state.dirtyD3=1;
    other=!selected;
    source=garage_logo_state.image38[selected];
    destination=garage_logo_state.image38[other];
    for(i=0;i<512;){i++;*destination++=*source++;}
    source=garage_logo_state.image40[selected];
    destination=garage_logo_state.image40[other];
    for(i=0;i<512;){i++;*destination++=*source++;}
    source=garage_logo_state.palette48[selected];
    destination=garage_logo_state.palette48[other];
    for(i=0;i<8;){i++;*destination++=*source++;}
}
