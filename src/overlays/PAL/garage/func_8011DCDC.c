#include "rr/overlay662_actions.h"
void garage_dispatch_direction_input(u32 flags){
 if(flags)PlaySfx(0);
 if(flags&1)garage_logo_actions[garage_logo_state.current60]->callback(1);
 else if(flags&2)garage_logo_actions[garage_logo_state.current60]->callback(2);
 if(flags&4)garage_logo_actions[garage_logo_state.current60]->callback(4);
 else if(flags&8)garage_logo_actions[garage_logo_state.current60]->callback(8);
}
