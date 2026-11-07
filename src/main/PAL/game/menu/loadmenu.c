/* RODATA 0x80011588 0x1c */
#include "types.h"
#include "rr/mode_menu_state.h"
#include "m2c_macros.h"
#include "game/data/cardini.h"
#include "game/menu/menustep.h"
#include "game/data/cardini.h"
#include "game/data/savedat.h"
#include "game/menu/loadmenu.h"
#include "game/data/globals.h"

extern short CardLoadStepState;
extern short CardLoadRetryCount;
extern int UpdateBadgeRowAnim();

void SetModeAfterCardLoad(int arg0);
void CardMenuModeUpdate(void);

/* legacy typing of shared globals in this part of the unit */
#define CardTargetFileName (*(char (*)[])&CardTargetFileName)

int CardLoadStep(a0, a1)
short a0;
int a1;
{
    int i;
    int r;
    int q;
    if (a0 > 0) {
        CardLoadStepState = 0;
    }
    switch (CardLoadStepState) {
    case 0:
        CardLoadRetryCount = 0;
        CardLoadStepState = 1;
        break;
    case 1:
        CardLoadStepState = 2;
        break;
    case 2:
        for (i = 0x1D; i >= 0; i--) {
        }
        if (SaveSlotSelect != 0x10) {
            r = CardReadFile((short)a1, 0, CardTargetFileName, SaveBuffer, 1);
        } else {
            r = CardReadFileAt((short)a1, 0, CardTargetFileName, SaveBuffer, 0x80, 0x280);
        }
        if (r == -1) {
            if (CardLoadRetryCount < 3) {
                CardLoadRetryCount = (unsigned short)CardLoadRetryCount + 1;
                break;
            }
            CardLoadStepState = 5;
        } else {
            CardLoadStepState = 3;
        }
        break;
    case 3:
        q = GetHwCardEvent();
        if (q != 0) {
            if (q == 1) {
                if (ApplySaveData(SaveSlotSelect) != 0) {
                    CardLoadStepState = 4;
                } else {
                    CardLoadStepState = 6;
                }
            } else {
                if (CardLoadRetryCount >= 3) {
                    CardLoadStepState = 5;
                } else {
                    CardLoadStepState = 1;
                    CardLoadRetryCount = (unsigned short)CardLoadRetryCount + 1;
                }
            }
        }
        break;
    case 4:
    case 5:
    case 6:
        break;
    }
    return CardLoadStepState;
}

#undef CardTargetFileName

/* legacy typing of shared globals in this part of the unit */
#define ModeId (*(short (*))&ModeId)
#define ModeSubState (*(short (*))&ModeSubState)

void SetModeAfterCardLoad(int arg0) {
    if (arg0 == -1) {
        ModeGroup = 1;
        ModeId = 3;
    } else {
        ModeGroup = 1;
        ModeId = 0xF;
    }
    ModeSubState = 0;
}

#undef ModeId
#undef ModeSubState

void CardMenuModeEnter(void) {
    unsigned long clearAddress;
    u16 clockLow;
    s32 selection;
    u16 modeRange;
    s16 bank;
    s32 ready;
    s32 i;
    s32 defaultChoice;
    s32 layout;
    u16 phase;

    if (RaceType == 4) {
        UpdateLinkHandshake(2);
        if (LinkState == -1) {
            _sio_control(1, 1, 0);
        }
    }
    phase = ModeSubState;
    switch (phase) {
    case 0:
        SetDispMask(0);
        SetupDrawDispEnvs(0x20, 0x20, 0x20, 2);
        ModeFrame = 0;
        SaveSlotIndex = 0;
        MenuChoiceValues[MenuChoiceIndex] = 3;
        /* ASM_BARRIER: finish the choice write before the clock read. */
        __asm__("" : : : "memory");
        __builtin_memcpy(&clockLow, &SelectionPlayer, sizeof(clockLow));
        bank = clockLow & 1;
        MenuPlayerIndex = bank;
        CardSlot = bank;
        ClearScreenOverlayState();
        i = 23;
        /* ASM_BARRIER: retain the initialized byte counter before indexing. */
        __asm__("" : "+r"(i));
        clearAddress = (unsigned long)CardTargetFileName;
        /* ASM_BARRIER: retain the real buffer address before its byte offset. */
        __asm__("" : "+r"(clearAddress));
        clearAddress += i;
        do {
            *(char *)clearAddress = 0;
            i--;
            clearAddress--;
        } while (i >= 0);
        CopyStringClearTail(CardTargetFileName,CardSaveFileName);
        selection = MenuResultCode;
        /* ASM_BARRIER: keep the signed snapshot separate from the range read. */
        __asm__("" : "+r"(selection));
        modeRange = (u16)MenuResultCode;
        modeRange = (u16)(modeRange - 6);
        if (selection == 0) {
            InitUiRow(0x22, 0xC, 0x40, 0x44, 0x20, 0, 0, 1, 0, 0x66);
            InitUiRow(0x23, 0xC, 0x68, 0x44, 0x20, 0, 0, 1, 0, 0x65);
            InitUiRow(0x24, 0xC, 0x90, 0x44, 0x20, 0, 0, 1, 0, 0x64);
            InitUiRow(0x25, 0xC, 0xB8, 0x44, 0x20, 0, 0, 1, 0, 0x55);
            MenuStateIndex = 0;
            SaveSlotSelect = 0;
            ScreenFadeLayout = 0x1001A;
            SeqStartTrack(0);
        } else if (modeRange < 2U) {
            if (selection == 6) {
                InitUiRow(0x22, 0xC, 0x40, 0x44, 0x20, 0, 0, 1, 0, 0x66);
                InitUiRow(0x23, 0xC, 0x68, 0x44, 0x20, 0, 0, 1, 0, 0x65);
                InitUiRow(0x24, 0xC, 0x90, 0x44, 0x20, 0, 0, 1, 0, 0x55);
                layout = 0x1001A;
            } else {
                InitUiRow(0x22, 0xC, 0x40, 0x44, 0x20, 0, 0, 1, 0, 0x66);
                InitUiRow(0x23, 0xC, 0x68, 0x44, 0x20, 0, 0, 1, 0, 0x55);
                layout = 0x10012;
            }
            ScreenFadeLayout = layout;
            /* ASM_BARRIER: publish the layout before the badge status. */
            __asm__("" : : : "memory");
            SaveSlotSelect = 0x10;
            MenuStateIndex = 0x29;
            InitBadgeRow(1, 0x80, 0x58, 0xA8, 0x80, 0, 0xE02);
            BadgeRows[1].half02 = 1;
            D_801272F0[0] = -1;
            D_801272F0[1] = -1;
            D_801272F0[2] = -1;
        } else {
            if (selection == 1 || selection == 3 || selection == 5) {
                InitUiRow(0x22, 0xC, 0x40, 0x44, 0x20, 0, 0, 1, 0, 0x66);
                InitUiRow(0x23, 0xC, 0x68, 0x44, 0x20, 0, 0, 1, 0, 0x55);
                ScreenFadeLayout = 0x10012;
                if (MenuResultCode == 3) {
                    MenuStateIndex = 0x17;
                } else {
                    MenuStateIndex = 0x20;
                }
                InitBadgeRow(1, 0x80, 0x58, 0xA8, 0x80, 0, 0x702);
                BadgeRows[1].half02 = 1;
                SaveSlotSelect = (s16) (u16) MenuResultCode;
            } else if (selection == 2) {
                ScreenFadeLayout = 0x10016;
                InitUiRow(0x22, 0xC, 0x40, 0x44, 0x20, 0, 0, 1, 0, 0x65);
                InitUiRow(0x23, 0xC, 0x68, 0x44, 0x20, 0, 0, 1, 0, 0x55);
                MenuStateIndex = 0x21;
                SaveSlotSelect = selection;
            } else {
                ScreenFadeLayout = 0x1001A;
            }
        }
        InitUiRow(0x26, 0x80, 0xE0, 0x1C, 0x20, 0, 0, 1, 0, 0x68);
        InitUiRow(0x27, 0x80, 0x108, 0x1C, 0x20, 0, 0, 1, 0, 0x69);
        InitBadgeRow(0, 0xC, 0x130, 0x128, 0x58, 0, 0);
        BadgeRows[0].half02 = 1;
        ResetScreenCounters();
        defaultChoice = 3;
        /* ASM_BARRIER: keep the initialized fill value apart from the count. */
        __asm__("" : "+r"(defaultChoice));
        DialogueContext = 3;
        MenuChoiceIndex = 0;
        for (i = 2; i >= 0; i--) {
            MenuChoiceValues[i] = defaultChoice;
        }
        D_8010948C = 0;
        for (i = 0; i < 3; i++) {
            SaveSlotSelectionsBackup[i] = SaveSlotSelections[i];
        }
        ModeSubState = 1;
        break;
    case 1:
        ModeFrame = (s32)((u32)ModeFrame + 1U);
        if (ModeId == phase) {
            SetDispMask(1);
        }
        ready = LoadDialogueTableStep();
        if (ready == phase) {
            SetDispMask(1);
            ModeGroup = ready;
            ModeId = 0x48;
            ModeSubState = 0;
        }
        break;
    }
    UpdateScreenFade(ScreenFadeLayout);
    SetMenuFrameOptions(0, 1, 1);
}

/* legacy typing of shared globals in this part of the unit */
#define UpdateScreenFade ((int (*)())UpdateScreenFade)
#define SetMenuFrameOptions ((int (*)())SetMenuFrameOptions)
#define UpdateLinkHandshake ((int (*)())UpdateLinkHandshake)
#define _sio_control ((int (*)())_sio_control)

void CardMenuModeUpdate(void) {
    ModeFrame += 1;
    if (RaceType == 4) {
        UpdateLinkHandshake(2);
        if (LinkState == -1) {
            _sio_control(1, 1, 0);
        }
    }
    DispatchCardMenuStep();
    UpdateBadgeRowAnim(0);
    if (MenuResultCode != 3) {
        if (MenuResultCode != 6) {
            if (MenuResultCode == 7) {
                goto block_6;
            }
        } else {
            goto block_6;
        }
    } else {
block_6:
        UpdateBadgeRowAnim(1);
    }
    UpdateScreenFade(ScreenFadeLayout);
    SetMenuFrameOptions(1, 1, 1);
}

#undef UpdateScreenFade
#undef SetMenuFrameOptions
#undef UpdateLinkHandshake
#undef _sio_control
