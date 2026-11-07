#ifndef RR_OVERLAY662_PALETTE_INITIALIZER_H
#define RR_OVERLAY662_PALETTE_INITIALIZER_H
#include "rr/overlay_menu_init.h"
#include "rr/save_progress.h"
#include "rr/course_choice_types.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern s16 ovl662_D_801272D4,ovl662_D_8012739A,ovl662_D_80127398,ovl662_D_801272E2,ovl662_D_801272E4,ovl662_D_801272BE;
extern s32 ovl662_D_801272C0,ovl662_D_801272E8;
extern u8 ovl662_D_801272A0[25];
extern void *DefaultCarClut,*CarClutData;
void UploadCarClut(void *,s32,s32);
/* Fresh course selector is0..8. Its signed second byte is at most25;
 * positive values select one of25 map slots. Palette bank1 covers25 TIMs.
 * UI and palette services may replace global roots and mutate shared state. */
void ovl662_func_80119B44(void);
#endif
