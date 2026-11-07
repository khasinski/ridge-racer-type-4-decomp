#include "rr/overlay_menu.h"
s32 ovl661_func_80117E10(void){
 func_80041320(MenuCursor+38,0,0);
 StepMenuValue(0,2,(char*)&MenuCursor,3,0);
 func_80041320(MenuCursor+38,1,1);
 if(MenuConfirmPressed){PlaySfx(1);return MenuCursor;}
 return -1;
}
