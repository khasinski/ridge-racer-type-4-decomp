#ifndef RR_MENU_TRANSITION_H
#define RR_MENU_TRANSITION_H

#include "rr/badge_rows.h"

/* Script operations return completion; scene-entry callbacks take no arguments.
 * Script bytes are signed: negative values select the next scene. The current
 * script position is reloaded after operations, which may replace the script. */
typedef s32 (*RrMenuScriptOperation)(void);
typedef void (*RrMenuSceneEntry)(void);
extern RrMenuScriptOperation MenuScriptOps[];
extern RrMenuSceneEntry MenuSceneEntries[];
extern s8 *MenuScripts[];
extern u16 SceneId;
extern s32 ModeFrame;
extern s16 ModeId;
extern s8 MenuScriptStep;
extern u16 MenuScriptId,ModeGroup;
extern s16 RaceType;
extern s8 LoadState;
extern s16 D_800F4D8A;
extern u16 MenuDispMode;
int UpdateLinkHandshake();
s32 OpeningScriptStep(s32);
void StartSceneFadeOut();
s32 IntroScriptStep(void);
void SetDispMask(s32);
void UpdateMenuScript(void);

#endif
