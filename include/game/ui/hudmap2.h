#ifndef RR_GAME_UI_HUDMAP2_H
#define RR_GAME_UI_HUDMAP2_H

#include "types.h"

extern s32 BgmNumberToCdTrack[];
extern s16 *D_800A1970[];
extern s16 *D_800A19A4[];
extern int CourseBgmChoiceTable2[];
extern int RaceBgmListIndex;
extern s32 OpeningFadeTimer;
extern s32 OpeningMode;
extern s32 D_8010E230;
extern s32 OpeningBgmSelectState;
extern s32 OpeningSlideTimer;
void DrawMapBitmapIso(s32 arg0, u8 *arg1, s32 *arg2);
void SelectReplayBgmTrack(void);
void OpeningInit(s32 arg0, s32 arg1);
void DrawGlyphListSloped();
void AddBrightnessSprite(unsigned char *a0x, int a1, int a2, int a3);

#endif /* RR_GAME_UI_HUDMAP2_H */
