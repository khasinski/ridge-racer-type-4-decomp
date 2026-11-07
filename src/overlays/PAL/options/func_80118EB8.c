#include "rr/overlay665_dispatch.h"
void options_func_80118EB8(void) {
 switch(options_D_80119768.scene00) {
 case 0:
  SetDispMask(0);options_func_80114A24();
  options_D_80119768.scene00=(s32)((u32)options_D_80119768.scene00+1U);
  InitMotionController();SetMotionControllerVisible(1);SeqStartTrack(0);break;
 case 1:options_func_80114C4C();break;
 case 2:
  switch(options_D_80119768.selection08) {
  case 0:options_func_80116D10(0);break;
  case 1:options_func_80116D10(1);break;
  case 2:options_func_801151A4();break;
  case 3:options_func_80115820();break;
  case 4:options_func_80115CFC();break;
  case 5:StartAttractDemo(1);break;
  case 6:ModeGroup=1;ModeId=3;ModeSubState=0;break;
  }
  break;
 }
 StepCameraScript();
}
