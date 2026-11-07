#include "types.h"
#include "scratchpad.h"
#include "rr_asm.h"
#include "m2c_macros.h"
#include "game/gfx/sky.h"
#include "psyq/libgte/gtemat2.h"
#include "game/gfx/skyrot1.h"
#include "rr/driver_control.h"

struct V8 { unsigned char c[8]; };

struct M { int a[8]; };

extern int SelectModelBank();
extern int RotMatrix();
extern int SetGteObjectTransform();
extern int SubmitModel();
extern char D_8001013C[];
extern int ModelBankFormat;
extern struct V8 D_800101E4;
extern struct { char pad[0xA8]; unsigned short v; } *IntroSceneState;
extern int RotMatrixX();
extern int RotMatrixY();
extern int RotMatrixZ();
extern char D_8009E88C[];
extern s16 MenuFadeTimer;
extern s32 VSyncThreshold;
extern s16 ModeId;
extern s16 ModeGroup;
extern s16 ModeSubState;

int func_8001BD40(int a0, int a1);
void DrawRotatingSky(void);
void func_8001BFA8(int a0, int a1, int a2);
void IntroSceneModeEnter(void);

/* legacy typing of shared globals in this part of the unit */
#define IntroSceneState ((char *)IntroSceneState)

int func_8001BD40(int a0, int a1)
{
    int t[8];
    short *s3 = (short *)(SCRATCHPAD + 0x7C);
    int s2;
    SelectModelBank(4);
    s2 = *(short *)(IntroSceneState + (a0 << 1) + 0xAA);
    if (a1 || (s2 && (s2 & 0x3FF) > 0x20)) s2 += 0x20;
    else if (s2) s2 = 0;
    s2 &= 0xFFF;
    *(short *)(IntroSceneState + (a0 << 1) + 0xAA) = s2;
    s3[0] = 0;
    s3[2] = 0;
    s3[1] = -s2;
    RotMatrix(s3, (char *)s3 + 8);
    RR_GTE_SETROT(IntroSceneState);
    MulRotMatrix0((char *)s3 + 8, (char *)s3 + 0x28);
    RR_GTE_SETLLM((char *)s3 + 0x28);
    RR_GTE_SETROT(SCRATCH_ROT_MATRIX);
    MulRotMatrix0((char *)s3 + 8, t);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), D_8001013C + a0 * 16, t);
    *(int *)(SCRATCHPAD + 0x78) = 0;
    SubmitModel(SCRATCHPAD, ModelBankFormat < 1, 0, 0);
    return s2;
}

#undef IntroSceneState

void DrawRotatingSky(void)
{
    struct V8 sp10;
    struct M sp18;
    int t;
    sp10 = D_800101E4;
    IntroSceneState->v += 0x20;
    sp18.a[4] = 0x1000;
    sp18.a[2] = 0x1000;
    sp18.a[3] = 0;
    sp18.a[1] = 0;
    t = (short)IntroSceneState->v;
    sp18.a[0] = 0x1000;
    RotMatrixY(t, &sp18);
    RotMatrixX(0x100, &sp18);
    RotMatrixZ(0xAA, &sp18);
    RenderSkyToRect(D_8009E88C, &sp18, &sp10, 0xC8);
}

/* legacy typing of shared globals in this part of the unit */
#define IntroSceneState ((char *)IntroSceneState)

void func_8001BFA8(int a0, int a1, int a2)
{
    int t[8];
    SelectModelBank(4);
    RotMatrix(a1, (SCRATCHPAD + 0x84));
    RR_GTE_SETROT(IntroSceneState + 0x20);
    MulRotMatrix0((SCRATCHPAD + 0x84), (SCRATCHPAD + 0xA4));
    RR_GTE_SETLLM((SCRATCHPAD + 0xA4));
    RR_GTE_SETROT(SCRATCH_ROT_MATRIX);
    MulRotMatrix0((SCRATCHPAD + 0x84), t);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), a2, t);
    *(int *)(SCRATCHPAD + 0x78) = 0;
    SubmitModel(SCRATCHPAD, a0 < ModelBankFormat ? a0 : 1, 0, 0);
}

#undef IntroSceneState

void IntroSceneModeEnter(void) {
    VSyncThreshold = 0x80;
    MenuFadeTimer = 0;
    TitleSceneStage = 0;
    SetDispMask(1);
    ModeGroup = 0;
    ModeId = 2;
    ModeSubState = 0;
}
