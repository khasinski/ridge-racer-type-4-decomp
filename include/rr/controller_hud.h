#ifndef RR_CONTROLLER_HUD_H
#define RR_CONTROLLER_HUD_H
#include "rr/sprite_emit.h"
#include "rr/badge_rows.h"
#include "rr/animated_sprite_grid.h"
#include "rr/tables.h"
#include "rr/record_banks.h"
#include "rr/frame_storage_types.h"
typedef union {s16 half[6];s32 word[3];} RrHudTriangle;
RR_ASSERT_SIZE(RrHudTriangle,12);
extern RrMenuItemState5C PlayerControllers[2];
extern u32 ScreenFrame;
extern s16 D_8009EE68[2];
extern u8 D_8009EE5C[6],D_8009EE64[3];
extern s16 D_800AD508;
extern RrUiUvRect D_8009EE58;
s32 GetControllerWarning(s32 player);
/* Layout1 selects port0 at Y112; layout2 selects port1 at Y240.
 * Other layouts select port0 at Y240. Status2 permits the warning and
 * optional steering needle. Button0x100 toggles that port's needle. */
void DrawSteeringIndicator(u32 *ordering,s32 layout);
/* Layout0..2: layout0 uses the first frame ordering table; other layouts use the second.
 * Layout1 shifts the Y origin by128 pixels. Both full frame arenas and
 * count record-pointer slots must be accessible. Nonpositive counts skip
 * record markers; bit0x10 selects visible map records. Captures the frame
 * head, then reads fresh record fields/counts after each emitter callback.
 * CPU pointers remain full width; coordinates narrow only in GPU packets. */
void DrawRaceMinimap(s32 layout);
#ifndef RR_HUD_SCRATCH
#define RR_HUD_SCRATCH ((RrGpuScratchPointers*)0x1F800000U)
#endif
#endif
