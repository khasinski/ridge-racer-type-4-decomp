#include "rr/overlay662_garage.h"
s32 garage_step_button_repeat(s32 mode)
{
    if(mode){
        garage_logo_work.words[2]++;
        if(garage_logo_work.words[2]>=47)garage_logo_work.words[2]=40;
        if(garage_logo_work.words[2]==1 || garage_logo_work.words[2]==40)return 1;return 0;
    }else{
        garage_logo_work.words[3]++;
        if(garage_logo_work.words[3]>=47)garage_logo_work.words[3]=40;
        if(garage_logo_work.words[3]==1 || garage_logo_work.words[3]==40)return 1;return 0;
    }
}
