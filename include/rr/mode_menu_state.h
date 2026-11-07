#ifndef RR_MODE_MENU_STATE_H
#define RR_MODE_MENU_STATE_H

#include "rr/menu_setup.h"
#include "rr/frame_environment.h"
#include "rr/driver_control.h"

extern char CardSaveFileName[], CardTargetFileName[24];
extern s32 SelectionPlayer, ScreenFadeLayout, D_800F4CA8, D_801272F0[3];
extern s16 MenuPlayerIndex, MenuStateIndex, SaveSlotSelect, ModeGroup;
extern s16 SaveSlotIndex, RaceType, LinkState, CardSlot, MenuResultCode;
extern s16 D_8010948C, D_8010948E, D_80109478[3];
extern u16 ModeId, D_800F4D8A;
extern RrMenuSettings2C SaveSlotSelections[3], SaveSlotSelectionsBackup[3];

void CopyStringClearTail(char *destination, char *source);
int UpdateLinkHandshake();
void SeqStartTrack(s32 mode);
s32 LoadDialogueTableStep(void);
void UpdateScreenFade(s32 layout);

/* The initial choice index is0..2. The name terminates within23 bytes and
 * the text destination has24 bytes. Badge helpers may replace the live root;
 * each replacement covers two complete RrBadgeRow1C records. Both settings
 * arrays contain three complete records. Only the full word result1 from
 * the readiness service completes state1. */
void func_8007ABCC(void);

#endif
