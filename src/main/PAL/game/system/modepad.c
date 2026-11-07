/* RODATA 0x800104f4 0x1c */
#include "types.h"
#include "game/system/modeslot.h"
#include "m2c_macros.h"
#include "game/race/racegrid.h"
#include "game/race/ranking.h"
#include "rr/audio_loop.h"
#include "game/data/ldscn1.h"
#include "game/data/globals.h"

extern int IntroScriptStep();
extern int DrawUiRow();
extern int PlayerControllers[];
extern char *UiRows;
extern short MenuCursor;
extern short D_800AD564;
extern int MenuConfirmPressed;
extern int D_800F3AA4;
extern short MenuScriptId;
extern int UiRowCount;
extern int D_800F4CA8;
extern short MenuDispMode;
extern void StepMenuValue();
extern void PlaySfx();
extern unsigned _sio_control();

void func_8002F0E0(void);
void DrawModeSelectRows();
int ModeSelectMenuStep(void);

void func_8002F0E0(void) {
    IntroScriptStep();
}

void DrawModeSelectRows() {
    int n = D_800F4CD0 + IsLinkPeerPresent();
    int i = 0;
    if (n > 0) {
        do {
            if (i == 2) {
                if (PlayerControllers[0x6C / 4] == -1) *(short *)(UiRows + 0x68) = 1;
                else *(short *)(UiRows + 0x68) = 0;
            }
            DrawUiRow(i);
            i++;
        } while (i < n);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define UiRows ((W7C0_E *)UiRows)

int ModeSelectMenuStep(void)
{
    int s1, s2, i;
    int v;

    s1 = D_800F4CD0 + IsLinkPeerPresent();
    if (MenuCursor == D_800F4CD0) {
        if (s1 == MenuCursor) {
            UiRows[s1].b0C = 0;
            MenuCursor = 0;
        }
    }
    UiRows[MenuCursor].b0C = 0;
    StepMenuValue(0, 2, &MenuCursor, s1, 0);
    StepMenuValue(3, 4, &D_800AD564, 5, 0);
    if (MenuCursor == 2 && D_800F3AA4 == -1)
        UiRows[2].b0C = 4;
    else
        UiRows[MenuCursor].b0C = 2;
    s2 = 0;
    if (MenuConfirmPressed != 0) {
        if (UiRows[MenuCursor].s20 == 1) {
            PlaySfx(4);
            return s2;
        }
        PlaySfx(1);
        s2 = 1;
        if (MenuCursor == D_800F4CD0)
            func_80034E14();
        else
            _sio_control(1, 1, 0);
        D_800F4CA8 = 2;
        MenuScriptId = 8;
        PendingModeId = 0x123;
        ModeMenuCursorSaved = MenuCursor;
        v = (short)MenuCursor;
        switch (v) {
        case 0:
            RaceType = 1;
            break;
        case 1:
            RaceType = 0;
            MenuDispMode = 0;
            break;
        case 2:
            RestoreSaveData();
            RaceType = 2;
            MenuDispMode = 0;
            break;
        case 3:
        case 4:
            RaceType = 0;
            break;
        case 5:
            D_800F4CA8 = 3;
            break;
        case 6:
            break;
        default:
            if (v == D_800F4CD0) {
                RestoreSaveData();
                RaceType = 4;
                ModeMenuCursorSaved = 8;
                PendingModeId = 0x11D;
            } else {
                ModeMenuCursorSaved = 7;
                RaceType = 6;
            }
            break;
        }
    } else {
        if (s1 == D_800F4CD0 + 1) {
            if (((_sio_control(0, 0, 0) >> 7) & 3) == 2) {
                UiRows[MenuCursor].b0C = 0;
                s2 = 1;
                UiRows[(short)ModeMenuCursorSaved].b0C = 0;
                RestoreSaveData();
                ModeMenuCursorSaved = 8;
                RaceType = 4;
                PendingModeId = 0x11D;
            }
        }
    }
    if (s2 == 1) {
        UiRows[MenuCursor].b0C = s2;
        for (i = 0; i < UiRowCount; i++) {
            if (UiRows[i].b14 == 2) UiRows[i].b14 = 3;
        }
    }
    return s2;
}

#undef UiRows
