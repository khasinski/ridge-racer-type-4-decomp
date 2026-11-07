#ifndef RR_OVERLAY662_AVAILABLE_CARS_H
#define RR_OVERLAY662_AVAILABLE_CARS_H
#include "rr/menu_arenas.h"
#include "rr/menu_setup.h"
#include "rr/menu_selection.h"
#include "rr/save_progress.h"
#include "rr/menu_glyph.h"
#include "rr/course_choice_types.h"
/* Four-byte available-car slots retain their unused upper halfword. Twenty
 * slots occupy27348..27398;the following storage is the image-fade phase. */
typedef struct {s16 value;u16 reserved;} RrGarageAvailableCar;
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern s16 garage_color_preview_state,garage_car_list_count,garage_car_index;
extern s16 MenuCursor,MenuItemCount,ModeId,ModeSubState;
extern u16 ModeGroup;
extern s16 garage_car_status[20];
extern RrGarageAvailableCar garage_collected_cars[20];
void SetupRenderer(s32);void InitPreviewImageRows(void);s32 PackChoiceGlyph(void *);
/* Resource base owns at least0x2D000 bytes for the initial arena offset.
 * Services may replace full arena/UI pointers;progress group bytes remain
 *0..3 whenever the next progress word is selected. Each of20 low mask bits
 * determines a status and appends a car. Eight readable choices in group1
 * identify saved cars after byte2 values below39 reduce modulo20.
 * The final glyph decode reads slot0 even when no cars were appended,so it
 * remains a readable initialized record. Count is0..20. Final UI and arena
 * first pointer both capture the fresh screen root after the last decoder. */
void garage_init_car_list(void);
RR_ASSERT_SIZE(RrGarageAvailableCar,4);
RR_ASSERT_OFFSET(RrGarageAvailableCar,reserved,2);
#endif
