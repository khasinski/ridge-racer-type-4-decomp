#include "rr/frame_loop.h"

/* legacy typing of shared globals in this part of the unit */
#define ModeId (*(s16 (*))&ModeId)

#define ModeGroup (*(s16 (*))&ModeGroup)

#undef ModeId

#undef ModeGroup

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf (*(int (*))&CurrentDrawBuf)

#define ModeGroup (*(short (*))&ModeGroup)

#define ModeId (*(short (*))&ModeId)

#undef CurrentDrawBuf

#undef ModeGroup

#undef ModeId

extern void MainLoopInitStub(void),ResetCallback(void),SetMem(s32),SystemInit(void);

extern s32 SetVideoMode(s32),CdFileStep(void);

extern void SetDispMask(s32),SoundUpdate(void),DrawSync(s32);

extern u32 *ClearOTagR(u32 *,s32);

extern s32 VSync(s32);

extern void *PutDispEnv(void *), *PutDrawEnv(void *);

extern void DrawOTag(void *),PadUpdate(void);

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
            scratch=(u8 **)0x1F800000;
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
