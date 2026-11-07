#ifndef RR_OVERLAY667_COURSE_MENU_H
#define RR_OVERLAY667_COURSE_MENU_H
#include "rr/overlay_menu.h"
#include "rr/menu_selection.h"
#include "rr/frame_environment.h"
#include "rr/menu_arenas.h"
#include "rr/tables.h"
#include "rr/driver_control.h"
#include "game/race/racegrid.h"
extern RrMenuItemState5C PlayerControllers[2];
extern s16 MenuFadeTimer,LinkRoleSlot,LinkState,D_8010C0B8,D_8010C0C8,ModeId;
extern u16 D_800F4D8A,ModeGroup;
extern s32 D_8010C078,NumLaps,MenuActivePlayerPlusOne,ModeFrame,ScreenFrame;
s32 ExitOnLinkLost(void);
int UpdateLinkHandshake();
s32 LoadTrackNodesStep(void);
void ResetScreenCounters(void);
void DrawCourseMapPreview(s32,s32);
void DrawCourseInfoPanel(s32,s32);
void SetMenuFrameOptions(s32,s32,s32);
void PlaySfx(s32);
void SeqFadeVolume(s32,s32);
void SeqMuteVolume(void);
void DrawUiRow(s32);
void AddScreenFadeTiles(u32 *,s32,u32);
void UpdateScreenFade(s32);
void ovl667_func_801161A8(s32);
/* At least25 complete UI rows are accessible. Menu selection is0..2 at
 * indexed accesses; the live highlight count is0..8. LinkRoleSlot is0..1
 * before scene8 reset,so initializer161A8 receives a valid player0 or2. Services may replace
 * roots,selections,counts and resources. Both counters wrap32 and the timer
 * wraps signed16. Driver/loader results are compared as complete32-bit words. */
void ovl667_func_80117FA8(void);
#endif
