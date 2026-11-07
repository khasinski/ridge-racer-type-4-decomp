#ifndef RR_OVERLAY662_PALETTE_INITIALIZER_H
#define RR_OVERLAY662_PALETTE_INITIALIZER_H
#include "rr/overlay_menu_init.h"
#include "rr/save_progress.h"
#include "rr/course_choice_types.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern s16 garage_D_801272D4,garage_clut_list_count,garage_image_fade_phase,garage_clut_choice,garage_car_index,garage_clut_list_index;
extern s32 garage_D_801272C0,garage_returned_from_logo_editor;
extern u8 garage_car_color_map[25];
extern void *DefaultCarClut,*CarClutData;
void UploadCarClut(void *,s32,s32);
/* Fresh course selector is0..8. Its signed second byte is at most25;
 * positive values select one of25 map slots. Palette bank1 covers25 TIMs.
 * UI and palette services may replace global roots and mutate shared state. */
void garage_init_color_select(void);
#endif
