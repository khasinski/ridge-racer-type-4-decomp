#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/menu/menu8.h"
#include "game/menu/menutran.h"
#include "game/menu/mnsel.h"
#include "rr/driver_control.h"
#include "rr/frame_environment.h"
#include "rr/menu_background.h"
#include "rr/selection.h"
#include "rr/bios_devices.h"
#include "game/ui/barsel0.h"
#include "game/gfx/drawbuf.h"
#include "game/data/screenframe.h"

typedef struct { char p[0x17]; signed char b; char q[0x2C - 0x18]; } T;

typedef struct { char p0[0xC]; unsigned char a; char p1[7]; unsigned char b; char p2[0xF]; } E;

extern int ClearScreenOverlayState();
extern void InitUiRow(int,int,int,int,int,int,int,int,int,int,...);
extern int SetMenuFrameOptions();
extern short MenuFadeTimer;
extern int VSyncThreshold;
extern short MenuCursor;
extern signed char SelectionState[];
extern int ModeFrame;
extern T SaveSlotSelections[];
extern short ModeId;
extern short ModeGroup;
extern short ModeSubState;
extern int DrawUiRow();
extern E *UiRows;
extern short MenuResultCode;
extern struct { int p[10]; int a28; int a2c; } PlayerControllers;
extern int StepMenuValue();
extern int PlaySfx();
extern int AddScreenFadeTiles();
extern int AddTileGrid2x2();
extern int ResetScreenCounters();
extern int InitMenuArenas();
extern int InitMenuGrid();
extern int LoadCursor;
extern int *MenuGridRecords;
extern short MenuFrameState;
extern int MenuArenaBase;
extern char SharedWorkBuffer[];

void func_8002FCEC();
void func_8002FE98(void);
void DrawFirstThreeUiRows(void);
void func_8002FEFC();
void func_8003016C(void);

void func_8002FCEC() {
    int a0, a1;
    short v;
    T *p;
    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 3);
    ModeFrame = 0;
    ScreenFrame = 0;
    MenuFadeTimer = 0;
    MenuCursor = 1;
    VSyncThreshold = 0x80;
    SelectionState[0x17] = -1;
    SelectionState[0x19] = 9;
    ResetRankEntries();
    ClearScreenOverlayState();
    a1 = 0;
    a0 = 0;
    p = SaveSlotSelections;
    L:
    a0++;
    if (p->b > 0) goto T;
    p++;
    if (a0 < 3) goto L;
    J:
    if (a1 != 0) goto B;
    func_8002F688();
    ModeGroup = 0;
    ModeId = 1;
    ModeSubState = 0;
    goto E2;
    T:
    a1 = 1;
    goto J;
    B:
    InitUiRow(0, 0x6C, 0xB8, 0x44, 0x20, 0, 0, 1, 0, 0x54);
    InitUiRow(1, 0x6C, 0xE0, 0x44, 0x20, 0, 0, 1, 0, 0x67);
    InitUiRow(2, 0x6C, 0x108, 0x44, 0x20, 0, 0, 1, 0, 0x55);
    SetMenuFrameOptions(0, 1, 1);
    ModeGroup = 1;
    ModeId = 6;
    ModeSubState = 0;
    E2:
}

void func_8002FE98(void) {
    func_8002FEFC();
    DrawFirstThreeUiRows();
    UpdateScreenFade(0x10086);
}

void DrawFirstThreeUiRows(void) {
    DrawUiRow(0);
    DrawUiRow(1);
    DrawUiRow(2);
}

/* legacy typing of shared globals in this part of the unit */
#define ModeSubState (*(unsigned short (*))&ModeSubState)
#define MenuFadeTimer (*(unsigned short (*))&MenuFadeTimer)
#define ModeId (*(unsigned short (*))&ModeId)
#define ModeGroup (*(unsigned short (*))&ModeGroup)

void func_8002FEFC() {
    int s;
    int v, t, w;
    s = ModeSubState;
    if (s != 0) goto C1;
    UiRows[MenuCursor].a = 0;
    StepMenuValue(0, 2, &MenuCursor, 3, 0);
    UiRows[MenuCursor].a = 2;
    s = UiRows->b;
    if (s != 2) goto END;
    SetMenuFrameOptions(1, 1, 1);
    if (PlayerControllers.a28 == 0) goto L0C;
    PlaySfx(1);
    UiRows[MenuCursor].a = 1;
    v = 3;
    if (MenuCursor < 2) v = 7;
    UiRows->b = v;
    UiRows[1].b = v;
    UiRows[2].b = v;
    MenuFadeTimer = 0x40;
    ModeSubState = 1;
    goto END;
L0C:
    if (PlayerControllers.a2c == 0) goto END;
    PlaySfx(2);
    v = 3;
    MenuCursor = s;
    UiRows->b = 3;
    UiRows[1].b = v;
    UiRows[2].b = v;
    MenuFadeTimer = 0x40;
    ModeSubState = 1;
    goto END;
C1:
    if (s != 1) goto END;
    SetMenuFrameOptions(0, 1, 1);
    t = MenuFadeTimer - 1;
    MenuFadeTimer = t;
    AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - (short)t) * 4, 0x45);
    if ((short)MenuFadeTimer > 0) goto END;
    w = MenuCursor;
    if (w == s) goto L120;
    if (w < 2) {
        if (w == 0) goto L104;
        goto END;
    }
    if (w == 2) goto L13C;
    goto END;
L104:
    func_8002F688();
    ModeGroup = 0;
    ModeId = s;
    ModeSubState = 0;
    goto END;
L120:
    MenuResultCode = 4;
    ModeGroup = s;
    ModeId = 0xD;
    ModeSubState = 0;
    goto END;
L13C:
    ModeGroup = s;
    ModeId = 3;
    ModeSubState = 0;
END:
    return;
}

#undef ModeSubState
#undef MenuFadeTimer
#undef ModeId
#undef ModeGroup

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

void func_8003016C(void)
{
    int *sp = (int *)SCRATCHPAD;
    int *p;
    AllocMenuGridRecords(SharedWorkBuffer, 1);
    InitMenuGrid(0, 0, 3, 0xF0, 0xA0);
    PsyqBzero((SCRATCHPAD + 0x8), 0x20);
    p = MenuGridRecords;
    p[7] = 0x60;
    p[8] = -0x110;
    p[9] = -0x160;
    p[5] = 0x1A;
    sp[6] = 0x400;
    func_8002F688();
    ModeFrame = 0;
    ScreenFrame = 0;
    VSyncThreshold = 0x80;
    ResetScreenCounters();
    if (D_800AD4F2 == 6) {
        InitSelectionRows();
        ClearScreenOverlayState();
        InitMenuArenas();
        LoadCursor = MenuArenaBase;
    }
    MenuFrameState = 0;
    sp[0] = AddTileGrid2x2(CurrentDrawBuf + 0x70, sp[0], 0, 0, 0x140, 0x200, 8, 8, 8);
    ModeGroup = 1;
    ModeId = 0x10;
    ModeSubState = 0;
}

#undef CurrentDrawBuf
