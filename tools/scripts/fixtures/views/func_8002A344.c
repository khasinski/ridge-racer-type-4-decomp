#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define MenuFrameState (*(short (*))&MenuFrameState)

#undef MenuFrameState

/* legacy typing of shared globals in this part of the unit */
#define ScreenFadeState (*(M2C_UNK (*))&ScreenFadeState)

#define MenuFrameState (*(M2C_UNK (*))&MenuFrameState)

#undef ScreenFadeState

#undef MenuFrameState

/* legacy typing of shared globals in this part of the unit */
#define ScreenFadeState (*(S_ACC68 (*))&ScreenFadeState)

#define MenuFrameState (*(S_F4CB0 (*))&MenuFrameState)

#undef ScreenFadeState

#undef MenuFrameState

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf (*(s32 (*))&CurrentDrawBuf)

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf (*(s32 (*))&CurrentDrawBuf)

#define AddTilePrim ((s32 (*)())AddTilePrim)

#undef CurrentDrawBuf

#undef AddTilePrim

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(E (*)[])&PlayerControllers)

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(S_800ABEF8 (*)[])&PlayerCars)

#undef PlayerCars

typedef struct { unsigned char c[4]; } B4;

typedef struct { unsigned char a, b, c, d; } S;

extern S ScreenFadeState;

extern B4 ScreenFadeColors[];

extern char *CurrentDrawBuf;

extern short ModeMenuCursorSaved;

extern int AddTileGrid2x2();

extern int func_8005B2D4();

extern int StepCameraScript();

extern int SetDispMask();

extern struct { char pad[0xC]; unsigned char a; unsigned char b; short c; } MenuFrameState;

void UpdateScreenFade(a0) int a0; {
    B4 sp;
    int a, t, a1;
    S *st = &ScreenFadeState;
    *(CurrentDrawBuf + 0x18) = 0;
    a = (a0 >> 1) & 1;
    st->c = a;
    if (a != st->d) ScreenFadeState.a = 0;
    st->d = a;
    t = st->b;
    switch (ScreenFadeState.a) {
    case 0:
        sp = ScreenFadeColors[a ^ 1];
        st->b = 0;
        ScreenFadeState.a = 1;
        break;
    case 1:
        {
            B4 *p = &ScreenFadeColors[st->c];
            B4 *q = &ScreenFadeColors[st->c ^ 1];
            a1 = 0x1E - t;
            sp.c[0] = (p->c[0] * t + q->c[0] * a1) / 30;
            sp.c[1] = (p->c[1] * t + q->c[1] * a1) / 30;
            sp.c[2] = (p->c[2] * t + q->c[2] * a1) / 30;
        }
        if (t >= 0x1E) {
            ScreenFadeState.a = 2;
            st->b = 0;
        } else {
            if (t == 2) {
                if (MenuFrameState == 0) SetDispMask(1);
            }
            ScreenFadeState.b = ScreenFadeState.b + 1;
        }
        break;
    case 2:
        sp = ScreenFadeColors[st->c];
        break;
    }
    {
        int t = *(int *)0x1F800000;
        *(int *)0x1F800000 = AddTileGrid2x2(CurrentDrawBuf + 0xB6C, t, 0, 0, 0x140, 0x200, sp.c[0], sp.c[1], sp.c[2]);
    }
    DrawMenuFrame(a0 >> 2);
    if (ModeMenuCursorSaved != 6 && ModeMenuCursorSaved != 8) {
        func_8005B2D4(a0 & 1);
        StepCameraScript();
    }
}
