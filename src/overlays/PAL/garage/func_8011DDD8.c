#include "rr/overlay662_garage.h"
s32 garage_func_8011DDD8(s32 mode)
{
    if(mode){
        garage_D_80127668.words[2]++;
        if(garage_D_80127668.words[2]>=47)garage_D_80127668.words[2]=40;
        if(garage_D_80127668.words[2]==1 || garage_D_80127668.words[2]==40)return 1;return 0;
    }else{
        garage_D_80127668.words[3]++;
        if(garage_D_80127668.words[3]>=47)garage_D_80127668.words[3]=40;
        if(garage_D_80127668.words[3]==1 || garage_D_80127668.words[3]==40)return 1;return 0;
    }
}
