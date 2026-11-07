#include "rr/menu_text_rows.h"

extern s32 ModeFrame, TotalRaceTime, MenuConfirmPressed, SelectionPlayer;
extern u16 ModeSubState;
extern s16 ModeGroup, ModeId, MenuFadeTimer, MenuCursor, LinkRoleSlot;
extern s16 LinkState, LinkResultsExitFlag, PlayerCount;
extern s16 D_8010C070[4];
extern s16 PlayerWinCounts[4];
extern char PlayerNames[4][12];
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
void DrawResultsTitleBanner(s32);
void UpdateMenuTextRow(s32);
void EmitMenuRowTransitionGrid(void);
s32 LoadResultsSceneStep(void);
void SeqStartTrack(s32);
s32 serial_link_func_801187DC(s32);
void serial_link_func_801161A8(s32);

void serial_link_func_8011885C(void) {
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
    switch (ModeSubState) {
    case 0:
        if (TotalRaceTime == -1) {
            group = 1;
            if (MenuTextRows[11].signedBytes[2] >= 2) {
                ModeSubState = 3;
            }
            break;
        }
        group = 0;
        if (MenuTextRows[3].signedBytes[3] >= 16) {
            SetMenuFrameOptions(1, 1, group);
            if (MenuConfirmPressed != 0) {
                ModeSubState = 3;
                SetMenuFrameOptions(group, 1, group);
            }
        }
        FormatDecimalDigits(MenuTextRows[4].marquee.at28 + 8, D_8010C070[0] + 1, 0);
        CopyStringPadSpaces(MenuTextRows[5].marquee.at28 + 1, PlayerNames[D_8010C070[0]], 9);
        /* The original literal occupies 12 bytes (zero word before "WIN "). */
        CopyStringClearTail(MenuTextRows[5].marquee.at28 + 10, "WON !!!\0\0\0");
        for (i = 0; i < 4; i++) {
            if (i < PlayerCount) {
                player = D_8010C070[i];
                CopyStringPadSpaces(MenuTextRows[7 + i].marquee.at28 + 5, PlayerNames[player], 9);
                CopyStringClearTail(MenuTextRows[7 + i].marquee.at28 + 14, "WIN ");
                FormatDecimalDigits(MenuTextRows[7 + i].marquee.at28 + 18, PlayerWinCounts[player], 1);
            } else {
                MenuTextRows[7 + i].marquee.at28[0] = 0;
            }
        }
        break;
    case 3:
        group = 3;
        if (MenuTextRows[39].signedBytes[3] >= 17) {
            SetMenuFrameOptions(1, 1, 0);
            if (serial_link_func_801187DC(0) >= 0) {
                ResetScreenCounters();
                InitMenuArenas();
                MenuFadeTimer = 64;
                ModeSubState = MenuCursor + 5;
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
            LinkResultsExitFlag = 1;
            ModeGroup = 1;
            ModeId = 41;
            ModeSubState = 0;
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
            ModeSubState = 8;
        }
        break;
    case 6:
        group = -1;
        MenuFadeTimer--;
        AddScreenFadeTiles((u32 *)(CurrentDrawBuf + 0x70), (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 3;
            ModeSubState = 0;
        }
        break;
    case 8:
        result = LoadResultsSceneStep();
        if (result != 1) {
            return;
        }
        SeqStartTrack(0);
        LinkState = 0;
        SelectionPlayer = (result - LinkRoleSlot) * 2;
        serial_link_func_801161A8(SelectionPlayer);
        ModeGroup = result;
        ModeId = 60;
        ModeSubState = 0;
        return;
    }
    UpdateScreenFade(0x1000C);
    DrawResultsTitleBanner(ModeFrame);
    if (group >= 0) {
        for (i = 0; i < 11; i++) {
            UpdateMenuTextRow(i + group * 11);
        }
    }
    if ((u8)(MenuTextRows[0].bytes[2] - 3) < 5 || (u8)(MenuTextRows[11].bytes[2] - 3) < 5) {
        EmitMenuRowTransitionGrid();
    }
}
