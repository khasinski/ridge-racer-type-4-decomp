#include "rr/menu_transition.h"

/* legacy typing of shared globals in this part of the unit */
#define SaveProgress (*(char (*)[])&SaveProgress)

#undef SaveProgress

extern s16 ModeMenuCursorSaved;

extern s16 D_800FFC18;

void SetNextModeByGameType(void){
 switch(ModeMenuCursorSaved){
 case 0:D_800AD50A=0x105;break;
 case 1:default:D_800AD50A=0x131;break;
 case 2:D_800AD50A=0x137;break;
 case 3:D_800AD50A=0x127;break;
 case 6:D_800AD50A=0x201;break;
 case 4:D_800AD50A=0x149;break;
 case 5:D_800FFC18=0;D_800AD50A=0x147;break;
 case 7:D_800AD50A=0x14E;break;
 case 8:D_800AD50A=0x13B;break;
 }
}
