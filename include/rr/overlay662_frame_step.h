#ifndef RR_OVERLAY662_FRAME_STEP_H
#define RR_OVERLAY662_FRAME_STEP_H
#include "rr/overlay662_garage.h"
extern u32 PlayTimeFrames;
extern s32 ScreenFrame;
extern u32 *D_800F39A0;
extern RrGarageTimBytes *D_800F399C;
void ovl662_func_8011E070(void);
void ovl662_func_8011E36C(s32 mode);
void ovl662_func_8011B8B4(void);
void ovl662_func_8011B484(void);
void ovl662_func_8011B400(void);
void ovl662_func_8011CF8C(void);
void ovl662_func_8011CD18(void);
/* Increment pulse with PAL word wrap; clock wraps before its upper clamp.
 * After eight services, publish the fresh active0/1 buffer and TIM pointers. */
void ovl662_func_8011AAF4(void);
#endif
