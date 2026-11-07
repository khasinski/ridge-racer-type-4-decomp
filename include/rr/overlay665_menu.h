#ifndef RR_OVERLAY665_MENU_H
#define RR_OVERLAY665_MENU_H
#include "rr/menu_arenas.h"
#include "rr/menu_setup.h"
#include "rr/menu_text_state.h"
#include "rr/renderer_setup.h"
#include "rr/overlay665_visual_types.h"
#include "rr/overlay_menu.h"
#include "rr/menu_pulse.h"
#include "rr/frame_environment.h"
typedef struct {u8 reserved00[0x18];s32 mode18;} RrReplayMenuContextPrefix;
extern RrReplayMenuContextPrefix RaceContext;
extern RrOverlay665VisualState ovl665_D_80119768;
extern s16 MenuFadeTimer;
extern u8 ScreenFadeState[12];
extern s32 ovl665_D_80119760;
void RaceLoadReset(void);
void SeqStartTrack(s32);
void AddScreenFadeTiles(u32 *,s32,u32);
void func_8005B2D4(s32);
void SeqFadeVolume(s32,s32);
void DrawUiRow(s32);
void UpdateScreenFade(s32);
void ovl665_func_80116070(void);
void ovl665_func_80114A24(void);
void ovl665_func_80114C4C(void);
/* Helpers may replace the full text-state root. */
RR_ASSERT_SIZE(RrReplayMenuContextPrefix,0x1C);
RR_ASSERT_SIZE(RrOverlay665VisualState,0xA0);
RR_ASSERT_OFFSET(RrOverlay665VisualState,selection08,8);
RR_ASSERT_OFFSET(RrOverlay665VisualState,exit34,0x34);
#endif
