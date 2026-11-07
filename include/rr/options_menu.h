#ifndef RR_OPTIONS_MENU_H
#define RR_OPTIONS_MENU_H
#include "rr/menu_arenas.h"
#include "rr/menu_setup.h"
#include "rr/menu_text_state.h"
#include "rr/renderer_setup.h"
#include "rr/options_visual_types.h"
#include "rr/overlay_menu.h"
#include "rr/menu_pulse.h"
#include "rr/frame_environment.h"
typedef struct {u8 reserved00[0x18];s32 mode18;} RrReplayMenuContextPrefix;
extern RrReplayMenuContextPrefix RaceContext;
extern RrOptionsVisualState options_menu_state;
extern s16 MenuFadeTimer;
extern u8 ScreenFadeState[12];
extern s32 options_pad_mode_ready;
void RaceLoadReset(void);
void SeqStartTrack(s32);
void AddScreenFadeTiles(u32 *,s32,u32);
void SetMotionControllerVisible(s32);
void SeqFadeVolume(s32,s32);
void DrawUiRow(s32);
void UpdateScreenFade(s32);
void options_init_controller_config(void);
void options_init_main_menu(void);
void options_update_main_menu(void);
/* Helpers may replace the full text-state root. */
RR_ASSERT_SIZE(RrReplayMenuContextPrefix,0x1C);
RR_ASSERT_SIZE(RrOptionsVisualState,0xA0);
RR_ASSERT_OFFSET(RrOptionsVisualState,selection08,8);
RR_ASSERT_OFFSET(RrOptionsVisualState,exit34,0x34);
#endif
