#include "rr/garage_garage.h"
void PlaySfx(s32 mode);
void garage_alt_undo_toggle(void)
{
    if(garage_logo_work.words[1]&2){
        PlaySfx(1);
        garage_logo_state.active21=!garage_logo_state.active21;
    }
}
