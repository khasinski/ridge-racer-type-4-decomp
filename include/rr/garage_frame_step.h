#ifndef RR_GARAGE_FRAME_STEP_H
#define RR_GARAGE_FRAME_STEP_H
#include "rr/garage_garage.h"
extern u32 PlayTimeFrames;
extern s32 ScreenFrame;
extern u32 *GarageActiveBuffer;
extern RrGarageTimBytes *GarageLogoTimPtr;
void garage_update_cursor(void);
void garage_update_actions(s32 mode);
void garage_draw_cursor_sprite(void);
void garage_draw_animated_panels(void);
void garage_add_editor_draw_env(void);
void garage_update_editor_overlays(void);
void garage_upload_editor_vram(void);
/* Increment pulse with PAL word wrap; clock wraps before its upper clamp.
 * After eight services, publish the fresh active0/1 buffer and TIM pointers. */
void garage_update_editor_frame(void);
#endif
