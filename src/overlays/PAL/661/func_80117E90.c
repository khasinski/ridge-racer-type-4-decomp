#include "rr/menu_text_rows.h"

extern s32 ModeFrame, TotalRaceTime, MenuConfirmPressed;
extern u16 D_800F4D8A;
extern s16 ModeGroup, ModeId, MenuFadeTimer, MenuCursor, MenuScriptId;
extern s16 ovl661_D_80118410[2];
extern s16 ovl661_D_80118418[2];
extern char ovl661_D_80118420[2][12];
extern s16 ovl661_D_80118448[2];
extern char ovl661_D_801148EC[];
extern char ovl661_D_801148F8[];
extern u8 *CurrentDrawBuf;

void SetMenuFrameOptions(s32, s32, s32);
void PlaySfx(s32);
void CopyStringClearTail(char *, char *);
void CopyStringPadSpaces(char *, char *, s32);
void FormatDecimalDigits(char *, s32, s32);
void AddScreenFadeTiles(u32 *, s32, u32);
void ResetScreenCounters(void);
void InitMenuArenas(void);
void UpdateScreenFade(s32);
void func_8003662C(s32);
void UpdateMenuTextRow(s32);
void func_800411D4(void);
s32 LoadResultsSceneStep(void);
void SeqStartTrack(s32);
s32 ovl661_func_80117E10(s32);
void ovl661_func_80115598(s32);

void ovl661_func_80117E90(void) {
    s32 group;
    s32 i;
    s32 player;
    s32 ready;

    ModeFrame++;
    switch (D_800F4D8A) {
    case 0:
        if (TotalRaceTime == -1) {
            group = 1;
            if (MenuTextRows[11].signedBytes[2] >= 2) {
                D_800F4D8A = 3;
            }
            break;
        }
        group = 0;
        if (MenuTextRows[3].signedBytes[3] >= 17) {
            SetMenuFrameOptions(1, 1, group);
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                D_800F4D8A = 3;
                SetMenuFrameOptions(group, 1, group);
            }
        }
        FormatDecimalDigits(MenuTextRows[4].marquee.at28 + 8, ovl661_D_80118410[0] + 1, 0);
        CopyStringPadSpaces(MenuTextRows[5].marquee.at28 + 1, ovl661_D_80118420[ovl661_D_80118410[0]], 9);
        CopyStringClearTail(MenuTextRows[5].marquee.at28 + 10, ovl661_D_801148EC);
        for (i = 0; i < 2; i++) {
            player = ovl661_D_80118410[i];
            CopyStringPadSpaces(MenuTextRows[7 + i].marquee.at28 + 5, ovl661_D_80118420[player], 9);
            CopyStringClearTail(MenuTextRows[7 + i].marquee.at28 + 14, ovl661_D_801148F8);
            FormatDecimalDigits(MenuTextRows[7 + i].marquee.at28 + 18, ovl661_D_80118418[player], 1);
        }
        break;
    case 3:
        group = 3;
        if (MenuTextRows[39].signedBytes[3] >= 17) {
            SetMenuFrameOptions(1, 1, 0);
            if (ovl661_func_80117E10(0) >= 0) {
                ResetScreenCounters();
                InitMenuArenas();
                MenuFadeTimer = 64;
                D_800F4D8A = MenuCursor + 4;
                SetMenuFrameOptions(0, 1, 0);
            }
        }
        break;
    case 4:
        MenuFadeTimer--;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        group = 3;
        if (MenuFadeTimer <= 0) {
            MenuScriptId = 0;
            ovl661_D_80118448[0] = -1;
            ModeGroup = 1;
            ModeId = 35;
            D_800F4D8A = 0;
        }
        break;
    case 5:
        MenuFadeTimer -= 2;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        group = 3;
        if (MenuFadeTimer <= 0) {
            group = -1;
            D_800F4D8A = 8;
        }
        break;
    case 6:
        MenuFadeTimer--;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        group = -1;
        if (MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 3;
            D_800F4D8A = 0;
        }
        break;
    case 8:
        ready = LoadResultsSceneStep();
        if (ready == 1) {
            SeqStartTrack(0);
            ovl661_func_80115598(0);
            ModeGroup = ready;
            ModeId = 56;
            D_800F4D8A = 0;
        }
        return;
    }
    UpdateScreenFade(0x1000C);
    func_8003662C(ModeFrame);
    if (group >= 0) {
        for (i = 0; i < 11; i++) {
            UpdateMenuTextRow(i + group * 11);
        }
    }
    if ((u8)(MenuTextRows[0].bytes[2] - 3) < 5 || (u8)(MenuTextRows[11].bytes[2] - 3) < 5) {
        func_800411D4();
    }
}
