#include "rr/menu_transition.h"

/* legacy typing of shared globals in this part of the unit */
#define SaveProgress (*(char (*)[])&SaveProgress)

#undef SaveProgress

extern s16 ModeMenuCursorSaved;

extern s16 MenuResultCode;

void SetNextModeByGameType(void){
 switch(ModeMenuCursorSaved){
 case 0:SceneId=0x105;break;
 case 1:default:SceneId=0x131;break;
 case 2:SceneId=0x137;break;
 case 3:SceneId=0x127;break;
 case 6:SceneId=0x201;break;
 case 4:SceneId=0x149;break;
 case 5:MenuResultCode=0;SceneId=0x147;break;
 case 7:SceneId=0x14E;break;
 case 8:SceneId=0x13B;break;
 }
}
