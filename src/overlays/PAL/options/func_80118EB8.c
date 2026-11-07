#include "rr/overlay665_dispatch.h"
void options_dispatch_menu(void) {
 switch(options_menu_state.scene00) {
 case 0:
  SetDispMask(0);options_init_main_menu();
  options_menu_state.scene00=(s32)((u32)options_menu_state.scene00+1U);
  InitMotionController();SetMotionControllerVisible(1);SeqStartTrack(0);break;
 case 1:options_update_main_menu();break;
 case 2:
  switch(options_menu_state.selection08) {
  case 0:options_update_controller_config(0);break;
  case 1:options_update_controller_config(1);break;
  case 2:options_update_audio_menu();break;
  case 3:options_update_screen_adjust();break;
  case 4:options_update_brightness_test();break;
  case 5:StartAttractDemo(1);break;
  case 6:ModeGroup=1;ModeId=3;ModeSubState=0;break;
  }
  break;
 }
 StepCameraScript();
}
