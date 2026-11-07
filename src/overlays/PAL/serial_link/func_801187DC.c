#include "rr/overlay_menu.h"
s32 serial_link_update_retry_choice(void){
 SetMenuTextRowCursor(MenuCursor+39,0,0);
 StepMenuValue(0,2,(char*)&MenuCursor,2,0);
 SetMenuTextRowCursor(MenuCursor+39,1,1);
 if(MenuConfirmPressed){PlaySfx(1);return MenuCursor;}
 return -1;
}
