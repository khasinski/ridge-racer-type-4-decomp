#include "rr/menu_pulse.h"
s32 options_func_8011629C(s32 index)
{
    RrMenuItemState5C *item=&PlayerControllers[index];
    s32 result=-1;
    switch(item->type01){
    case 0x41:case 0x73:
        if(item->kind1C){result=3;if(item->flag40==0)result=1;}
        else result=item->type01==0x73;
        break;
    case 0x23:result=2;break;
    case 0xE3:result=3;break;
    }
    return result;
}
