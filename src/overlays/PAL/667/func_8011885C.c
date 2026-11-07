#include "rr/menu_text_rows.h"

extern s32 ModeFrame, TotalRaceTime, MenuConfirmPressed, SelectionPlayer;
extern u16 D_800F4D8A;
extern s16 ModeGroup, ModeId, MenuFadeTimer, MenuCursor, D_800AD508;
extern s16 LinkState, D_8010C068, PlayerCount;
extern s16 D_8010C070[4];
extern s16 D_8010C080[4];
extern char D_8010C088[4][12];
extern u8 *CurrentDrawBuf;

s32 ExitOnLinkLost(void);
s32 UpdateLinkHandshake(s32);
void BeginLinkSync(void);
void SetMenuFrameOptions(s32, s32, s32);
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
s32 ovl667_func_801187DC(s32);
void ovl667_func_801161A8(s32);

void ovl667_func_8011885C(void) {
    s32 pressed;
    s32 group;
    s32 i;
    s32 result;
    s32 player;

    ModeFrame++;
    if (ExitOnLinkLost() != 0) {
        return;
    }
    pressed = UpdateLinkHandshake(2);
    if ((u32)ModeFrame >= 0x08000000) {
        ModeFrame = 3;
    }
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
        if (MenuTextRows[3].signedBytes[3] >= 16) {
            SetMenuFrameOptions(1, 1, group);
            if (MenuConfirmPressed != 0) {
                D_800F4D8A = 3;
                SetMenuFrameOptions(group, 1, group);
            }
        }
        FormatDecimalDigits(MenuTextRows[4].marquee.at28 + 8, D_8010C070[0] + 1, 0);
        CopyStringPadSpaces(MenuTextRows[5].marquee.at28 + 1, D_8010C088[D_8010C070[0]], 9);
        /* The original literal occupies 12 bytes (zero word before "WIN "). */
        CopyStringClearTail(MenuTextRows[5].marquee.at28 + 10, "WON !!!\0\0\0");
        for (i = 0; i < 4; i++) {
            if (i < PlayerCount) {
                player = D_8010C070[i];
                CopyStringPadSpaces(MenuTextRows[7 + i].marquee.at28 + 5, D_8010C088[player], 9);
                CopyStringClearTail(MenuTextRows[7 + i].marquee.at28 + 14, "WIN ");
                FormatDecimalDigits(MenuTextRows[7 + i].marquee.at28 + 18, D_8010C080[player], 1);
            } else {
                MenuTextRows[7 + i].marquee.at28[0] = 0;
            }
        }
        break;
    case 3:
        group = 3;
        if (MenuTextRows[39].signedBytes[3] >= 17) {
            SetMenuFrameOptions(1, 1, 0);
            if (ovl667_func_801187DC(0) >= 0) {
                ResetScreenCounters();
                InitMenuArenas();
                MenuFadeTimer = 64;
                D_800F4D8A = MenuCursor + 5;
                BeginLinkSync();
                SetMenuFrameOptions(0, 1, 0);
            }
        }
        break;
    case 4:
        group = -1;
        if (pressed == 0 && MenuFadeTimer == 64) {
            break;
        }
        MenuFadeTimer--;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            D_8010C068 = 1;
            ModeGroup = 1;
            ModeId = 41;
            D_800F4D8A = 0;
            LinkState = 0;
        }
        break;
    case 5:
        group = 3;
        if (pressed == 0 && MenuFadeTimer == 64) {
            CopyStringClearTail(MenuTextRows[38].marquee.at28, "*PLEASE WAIT! ");
            break;
        }
        MenuFadeTimer -= 2;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            group = -1;
            D_800F4D8A = 8;
        }
        break;
    case 6:
        group = -1;
        MenuFadeTimer--;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 3;
            D_800F4D8A = 0;
        }
        break;
    case 8:
        result = LoadResultsSceneStep();
        if (result != 1) {
            return;
        }
        SeqStartTrack(0);
        LinkState = 0;
        SelectionPlayer = (result - D_800AD508) * 2;
        ovl667_func_801161A8(SelectionPlayer);
        ModeGroup = result;
        ModeId = 60;
        D_800F4D8A = 0;
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
