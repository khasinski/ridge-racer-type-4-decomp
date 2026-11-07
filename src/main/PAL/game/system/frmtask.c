/* RODATA 0x800102cc 0x24 */
#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "rr/frame_loop.h"
#include "game/data/cdfile2.h"
#include "game/system/frmenv.h"
#include "game/system/pad.h"
#include "game/effect/spfade.h"
#include "psyq/libgpu/gpuenv.h"
#include "game/system/frmtask.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "game/sound/sndslot.h"
#include "psyq/libgpu/gpuenv.h"
#include "rr/course_choices.h"
#include "rr/menu_background.h"
#include "rr/driver_control.h"
#include "rr/movie_playback_playback_step.h"
#include "rr/movie_playback_stream.h"

extern s16 MenuFadeTimer;
extern s32 FrmTaskState;
extern short *FrmTaskRecord;
extern int MenuConfirmPressed;
extern int CdAudioInit();
extern int CdAudioSelectTrack();
extern int CdAudioBeginPlay();
extern int LoadTimList();
extern int SetMenuFrameOptions();
extern int AddScreenFadeTiles();
extern int CdAudioFade();
extern int EmitPresetSprite();
extern int EmitSpriteGrid();
extern void *PutDispEnv(void *);

void FrmTaskModeEnter(void);
void FrmTaskModeUpdate(void);

/* legacy typing of shared globals in this part of the unit */
#define ModeId (*(s16 (*))&ModeId)
#define ModeGroup (*(s16 (*))&ModeGroup)

void FrmTaskModeEnter(void) {
    FrmTaskState = 0;
    MenuFadeTimer = -1;
    ModeGroup = 0;
    ModeId = 6;
    ModeSubState = 0;
}

#undef ModeId
#undef ModeGroup

void FrmTaskReset(void) {
    FrmTaskState = 0;
    MenuFadeTimer = -1;
}

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf (*(int (*))&CurrentDrawBuf)
#define ModeGroup (*(short (*))&ModeGroup)
#define ModeId (*(short (*))&ModeId)

void FrmTaskModeUpdate(void) {
    short *p;
    short *q;
    short *r;
    volatile int *g;
    int *s_;
    int *d_;
    int v;

    switch (FrmTaskState) {
    case 0:
        v = GetR4FileIndex(0, 0x15);
        if (CdFileReadStart(v, LoadCursor)) {
            FrmTaskState = 1;
        }
        return;
    case 1:
        if (CdFileBytesLoaded()) {
            FrmTaskState = 2;
        }
        return;
    case 2:
        CdAudioInit();
        CdAudioSelectTrack(0, 0x24);
        CdAudioBeginPlay(0);
        LoadTimList(LoadCursor);
        FrmTaskState = 7;
        q = (short *)LoadCursor;
        FrmTaskRecord = q;
        LoadCursor = (int)q + 0x10;
        q[1] = 0;
        q[2] = 0;
        q[3] = 0;
        q[4] = 0;
        q[5] = 0;
        q[6] = 8;
        q[7] = 0;
        q[0] = 0;
    case 7:
        r = FrmTaskRecord;
        if (r[0] < 0x40) {
            if (r[0] == 8) {
                r[4] = 4;
            } else if (r[0] == 0x20) {
                r[5] = 4;
            }
            goto common;
        }
        FrmTaskState = 8;
    case 8:
        SetMenuFrameOptions(1, 1, 0);
        if (MenuFadeTimer > 0) {
            MenuFadeTimer--;
            AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) * 4, 0x45);
            if (MenuFadeTimer == 0) {
                ModeGroup = 1;
                ModeId = 3;
                ModeSubState = 0;
            }
        } else if (MenuConfirmPressed) {
            MenuFadeTimer = 0x40;
            CdAudioFade(0, 0x40);
        }
    case 3: case 4: case 5: case 6:
    default:
    common:
        p = FrmTaskRecord;
        RampValueToLimit(p + 1, p + 4, 0x80);
        p = FrmTaskRecord;
        RampValueToLimit(p + 2, p + 5, 0x80);
        p = FrmTaskRecord;
        RampValueToLimit(p + 3, p + 6, 0x80);
        EmitPresetSprite(FrmTaskRecord[1], 0x2F);
        EmitPresetSprite(FrmTaskRecord[2], 0x30);
        p = FrmTaskRecord;
        EmitSpriteGrid(p[3]);
        g = (int *)SCRATCHPAD;
        *(int *)(g[0] + 4) = 0xE100022A;
        s_ = (int *)g[1];
        d_ = (int *)g[0];
        *d_ = *s_;
        *(char *)(g[0] + 3) = 1;
        *(int *)g[1] = g[0];
        *(char *)(g[1] + 3) = 0;
        *(int *)SCRATCHPAD += 8;
        UpdateScreenFade(0x1000C);
        FrmTaskRecord[0]++;
    }
}

#undef CurrentDrawBuf
#undef ModeGroup
#undef ModeId

void MainLoop(void) {
    s32 threshold;
    register u8 *bank_page;
    u32 bank;
    u8 *frame;
    MainLoopInitStub();
    SetVideoMode(1);
    ResetCallback();
    bank_page=(u8 *)0x800F0000U;
    SetMem(2);
    SystemInit();
    VSyncThreshold=128;
    SetDispMask(0);
    ModeFrame=0;
    ModeGroup=1;
    ModeId=1;
    ModeSubState=0;
    FrameCounter=0;
    for (;;) {
        {
            /* ASM_BARRIER: prepare the real ordering-table length before its address. */
            register s32 length asm("$5")=0x2C0;
            /* ASM_BARRIER: materialize the frame arena in V1 before indexing it. */
            register RrFrameStorage *frames asm("$3");
            register u8 *frame_page;
            register u8 **scratch;
            register u32 *ordering;
            /* ASM_BARRIER: keep the arena address after the length preparation. */
            __asm__("" : "+r"(length) : : "$3");
            frame_page=(u8 *)0x800B0000U;
            bank=FrameCounter;
            /* ASM_BARRIER: read the counter before completing the arena address. */
            __asm__("" : "+r"(frame_page), "+r"(bank));
            frames=(RrFrameStorage *)(frame_page-0x2510);
            bank &=1;
            frame=(u8 *)((unsigned long)bank*sizeof(RrFrameStorage)+(unsigned long)frames);
            ordering=(u32 *)(frame+0x70);
            CurrentDrawBuf=frame;
            *(s32 *)(bank_page+0x4BF0)=bank;
            scratch=(u8 **)SCRATCHPAD;
            scratch[1]=(u8 *)ordering;
            scratch[0]=frame+0x1670;
            ClearOTagR(ordering,length);
        }
        ClearOTagR((u32 *)(CurrentDrawBuf+0xB70),0x2C0);
        CdFileStep();
        SoundUpdate();
        ModeFuncTable[ModeGroup][ModeId]();
        DrawSync(0);
        threshold=VSyncThreshold;
        do {} while (VSync(1)<threshold);
        VSync(0);
        PutDispEnv(CurrentDrawBuf+0x5C);
        PutDrawEnv(CurrentDrawBuf);
        DrawOTag(CurrentDrawBuf+0xB6C);
        if(DrawSubOtFlag) DrawOTag(CurrentDrawBuf+0x166C);
        PadUpdate();
        FrameCounter++;
    }
}
