#include "types.h"
#include "game/system/modeslot.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/system/frmtask.h"
#include "game/gfx/meshquad.h"
#include "game/menu/menuwork.h"
#include "rr/menu_background.h"
#include "game/data/globals.h"
#include "game/data/selection.h"
#include "game/data/screenframe.h"

#define P8(base, off) (*(unsigned char *)((char *)(base) + (off)))
#define IDX30(base, i) (*(unsigned char *)((char *)(base) + (i) * 0x24 + 0x30))

typedef struct { int x; } __attribute__((packed)) W7C0_U;

typedef struct { char p0; signed char b1; unsigned char b2; } W7C0_S;

extern unsigned char NameCharset[];
extern int StepMenuValue();
extern s32 ModeFrame;
extern s16 ModeId;
extern s16 ModeGroup;
extern s16 D_800F4D8A;
extern unsigned char D_8009F810[];
extern unsigned char SelectionState[];
extern unsigned char D_800AD4EB;
extern unsigned char SaveProgress[];
extern int SetupRenderer();
extern int ResetCarState();
extern int InitSelectionRows();
extern int SetupMenuCarCamera();
extern int InitPreviewImageRows();
extern int ResetScreenCounters();
extern void InitUiRow(int,int,int,int,int,int,int,int,int,int,...);
extern M2C_UNK PlayerCars;
extern s32 VSyncThreshold;
extern s16 SceneId;
extern int SetMenuFrameOptions();
extern int DrawPreviewCar();
extern int func_8002E5A0();
extern int ScaleMatrixRow1();
extern int UpdateImageRow();
extern int CdAudioFade();
extern int SeqStartTrack();
extern int BuildCameraMatrices();
extern int SetupViewport();
extern unsigned char D_8009F818[];
extern short MenuFadeTimer;
extern signed char *ImageRows;
extern int LoadCursor;
extern int MenuConfirmPressed;
extern W7C0_F *BadgeRows;
extern int MenuArenas;
extern W7C0_E *UiRows;
extern void DrawSelectionIconGrid();
extern int LoadCarRangeStep();
extern void UpdateBadgeRowAnim();
extern void DrawUiRow();
extern void PlaySfx();
extern void InitMotionController();

void GetNameChar(unsigned char *a0, int a1);
void StepNameChar(unsigned char *a0);
void func_8003984C(void);
void func_80039894(void);
void func_8003989C(void);
void func_80039920(void);
void CarPreviewModeUpdate(void);
void func_80039C0C(void);

void GetNameChar(unsigned char *a0, int a1) {
    *a0 = NameCharset[a1];
}

void StepNameChar(unsigned char *a0) {
    int i;
    for (i = 0; i < 0x29; i++) {
        if (*a0 == NameCharset[i]) break;
    }
    StepMenuValue(2, 4, &i, 0x29, 0);
    *a0 = NameCharset[i];
}

void func_8003984C(void) {
    ModeFrame = 0;
    FrmTaskReset();
    UpdateScreenFade(0x1000C);
    ModeGroup = 0;
    ModeId = 6;
    D_800F4D8A = 0;
}

void func_80039894(void) {
}

void func_8003989C(void) {
    int v;
    if (SelectionState[0x1E] == 0x12) {
        int i = SelectionState[0x1C] * 2 + (SelectionState[0x1D] >> 1);
        v = D_8009F810[i];
    } else {
        v = 1;
    }
    if (v < 0) {
        v = 0;
    } else if (v >= 9) {
        v = 8;
    }
    SelectionState[0x12] = v;
    if (v == 1) {
        SelectionState[0x13] = SelectionState[0x18];
    }
}

void func_80039920(void) {
    int i = SelectionState[0x1C] * 2 + (SelectionState[0x1D] >> 1);
    int v = D_8009F810[i];
    if (v < 0) {
        v = 0;
    } else if (v >= 9) {
        v = 8;
    }
    D_800AD4EB = SaveProgress[v + 0x60];
}

void CarPreviewSetup(void) {
    SetupRenderer(5);
    ResetScreenCounters();
    SetupMenuCarCamera(1);
    ResetCarState(&PlayerCars);
    M2C_FIELD(&PlayerCars, s32 *, 0x28) = 0x134;
    M2C_FIELD(&PlayerCars, s32 *, 0x24) = 0xA;
    M2C_FIELD(&PlayerCars, s32 *, 0x54) = 0xA00;
    InitPreviewImageRows();
    InitUiRow(0, 0x18, 0x58, 0x44, 0x20, 0, 0, 1, 0, 0x53);
    InitUiRow(1, 0x74, 0x58, 0x1C, 0x20, 5, 0, 1, 0, 0x37);
    InitUiRow(2, 0x74, 0x80, 0x1C, 0x20, 5, 0, 1, 0, 0x38);
    ScreenFrame = 0;
    ModeFrame = 0;
    SceneId = 0x122;
    VSyncThreshold = 0x80;
    InitSelectionRows();
}

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(char (*)[])&PlayerCars)
#define D_800F4D8A (*(unsigned short (*))&D_800F4D8A)

void CarPreviewModeUpdate(void) {
    ModeFrame += 1;
    SetupViewport(6);
    if (D_800F4D8A == 0) {
        if (func_800578D8(D_8009F818[D_800AD4EF / 2]) != 0) {
            MenuFadeTimer = 0x40;
            CdAudioFade(0, 0x3C);
            D_800F4D8A = 1;
            CarPreviewSetup();
            ImageRows[1] = 0;
        }
    } else {
        BuildCameraMatrices();
        ScaleMatrixRow1(2, SCRATCH_ROT_MATRIX);
        func_8002E5A0(PlayerCars);
        DrawPreviewCar(PlayerCars, 0, 0x100, 1);
        UpdateImageRow(0);
        if (ImageRows[1] == 2) {
            ModeGroup = 1;
            ModeId = 0x22;
            D_800F4D8A = 0;
            SeqStartTrack(0);
        }
    }
    UpdateScreenFade(0x10028);
    SetMenuFrameOptions(0, 0, 0);
}

#undef PlayerCars
#undef D_800F4D8A

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(char (*)[])&PlayerCars)
#define SelectionState (*(char (*)[])&SelectionState)
#define ImageRows ((W7C0_S *)ImageRows)
#define D_800F4D8A (*(unsigned short (*))&D_800F4D8A)
#define UpdateScreenFade ((void (*)())UpdateScreenFade)
#define SetMenuFrameOptions ((void (*)())SetMenuFrameOptions)
#define StepMenuValue ((void (*)())StepMenuValue)
#define DrawPreviewCar ((void (*)())DrawPreviewCar)
#define func_8002E5A0 ((void (*)())func_8002E5A0)
#define SetupMenuCarCamera ((void (*)())SetupMenuCarCamera)
#define ResetScreenCounters ((void (*)())ResetScreenCounters)
#define UpdateImageRow ((void (*)())UpdateImageRow)
#define SetupViewport ((void (*)())SetupViewport)

void func_80039C0C(void)
{
    int s0, s2, c2, v1, a0, a1;
    char *t0;
    unsigned char *q;
    unsigned char sp18[4];
    int s1;

    ModeFrame = ModeFrame + 1;
    UpdateScreenFade(0x10022);
    s2 = LoadCarRangeStep(1, 3);
    if (s2 == 1) {
        v1 = LoadCursor;
        s0 = D_800F4D8A;
        MenuArenas = v1;
        if (s0 == s2)
            goto b1;
        if (s0 < 2) {
            if (s0 == 0)
                goto b0;
            goto end;
        }
        if (s0 == 2)
            goto b2;
        goto end;
    b0:
        s0 = P8(UiRows, 0x14);
        if (s0 == 2) {
            SetMenuFrameOptions(1, 1, 0);
            P8(UiRows, 0xC) = s0;
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                P8(UiRows, 0xC) = s2;
                P8(UiRows, 0x38) = 0;
                P8(UiRows, 0x5C) = 0;
                D_800F4D8A = s2;
            }
        }
        goto end;
    b1:
        t0 = (char *)UiRows;
        v1 = P8(t0, 0x38);
        c2 = 2;
        if (v1 == c2) {
            q = SelectionRows;
            IDX30(t0, q[3]) = 0;
            StepMenuValue(0, 1, q + 3, c2, 0);
            IDX30(UiRows, q[3]) = c2;
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                a1 = 0;
                a0 = 0;
                IDX30(UiRows, q[3]) = s2;
                for (a1 = 0; a1 < 3; a1++)
                    P8(UiRows, a1 * 0x24 + 0x14) = 3;
            }
        } else if (v1 == 5) {
            D_800F4D8A = c2;
        }
        goto end;
    b2:
        ModeFrame = 0;
        ImageRows->b1 = 3;
        BadgeRows->s10 = 4;
        ResetScreenCounters();
        {
        char *b = SelectionState;
        *(W7C0_U *)(b + 0x1C) = *(W7C0_U *)SelectionRows;
        if ((signed char)b[0x17] <= 0)
            *(short *)(b + 0x1A) = s0;
        }
        InitMotionController();
        ModeGroup = s2;
        ModeId = 0xB;
        D_800F4D8A = 0;
    }
end:
    SetupMenuCarCamera(1);
    SetupViewport(6);
    if (ImageRows->b1 > 0) {
        char *g = PlayerCars;
        sp18[0] = 0x80;
        sp18[1] = 0x80;
        sp18[2] = 0x80;
        *(int *)(g + 0x54) = (*(int *)(g + 0x54) + 8) & 0xFFF;
        if (ImageRows->b1 == 2)
            s1 = 0x100;
        else
            s1 = (ImageRows->b2 << 8) / 30;
        BadgeRows->s2 = 1;
        DrawUiRow(0);
        DrawUiRow(1);
        DrawUiRow(2);
        DrawSelectionIconGrid(0);
        func_8002E5A0(PlayerCars);
        DrawPreviewCar(PlayerCars, *(int *)sp18, s1, 1);
    }
    UpdateBadgeRowAnim(0);
    UpdateImageRow(0);
}

#undef PlayerCars
#undef SelectionState
#undef ImageRows
#undef D_800F4D8A
#undef UpdateScreenFade
#undef SetMenuFrameOptions
#undef StepMenuValue
#undef DrawPreviewCar
#undef func_8002E5A0
#undef SetupMenuCarCamera
#undef ResetScreenCounters
#undef UpdateImageRow
#undef SetupViewport
