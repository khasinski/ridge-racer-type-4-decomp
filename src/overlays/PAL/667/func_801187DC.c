#include "rr/overlay_menu.h"
s32 ovl667_func_801187DC(void){
 SetMenuTextRowCursor(MenuCursor+39,0,0);
 StepMenuValue(0,2,(char*)&MenuCursor,2,0);
 SetMenuTextRowCursor(MenuCursor+39,1,1);
 if(MenuConfirmPressed){PlaySfx(1);return MenuCursor;}
 return -1;
}
