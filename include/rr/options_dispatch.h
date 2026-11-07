#ifndef RR_OPTIONS_DISPATCH_H
#define RR_OPTIONS_DISPATCH_H
#include "rr/options_calibration.h"
#include "rr/menu_transition.h"
void InitMotionController(void);
void StepCameraScript(void);
void StartAttractDemo();
void options_update_controller_config(s32);
void options_update_audio_menu(void);
/* Dispatch captures the scene and selection at their respective switches.
 * Scene0 increments the fresh value left by initialization, modulo32 bits. */
void options_dispatch_menu(void);
#endif
