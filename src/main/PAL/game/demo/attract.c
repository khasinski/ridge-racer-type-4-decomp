#include "types.h"
#include "m2c_macros.h"
#include "game/gfx/fadesel.h"
#include "game/math/shuffle.h"
#include "game/sound/sndctrl.h"
#include "game/gfx/fadein.h"
#include "game/gfx/fadespr.h"
#include "game/ui/hudmap2.h"
#include "game/demo/attract.h"
#include "game/menu/title.h"
#include "game/ui/hudmap2.h"
#include "rr/audio_loop.h"
#include "rr/bios_devices.h"
#include "rr/color_conversion.h"
#include "rr/race_setup.h"
#include "rr/driver_control.h"
#include "game/data/selection.h"

extern int SlotOrder[];
extern char AttractWork[];
extern int AttractCarSlotMap[];
extern short CourseIndex;
extern unsigned short AttractCourseButtonMask;
extern unsigned short DemoCourseButtonMap[];
extern short MenuDispMode;
extern short MenuScriptId;
extern short ModeGroup;
extern short ModeId;
extern short ModeSubState;
extern int MaskBitSetting;
extern char RaceContext[];
extern short SceneId;
extern int PsyqRand();
extern int ResetScreenCopyOverlay();
extern int SetSceneColorParams();
extern int SetSceneOverlayEnabled();
extern int ResetScreenCounters();

void StartAttractDemo(int a0);
void AttractDemoRaceEnter(void);

void StartAttractDemo(int a0) {
    int sp[4];
    int i;
    int y;
    int u;
    int *tb;
    int *q;
    int s0;
    int s2;
    int v;
    int t;
    /* ASM_BARRIER: pin rand()%2 result to $v0 like the original shuffle loop */
    register int r __asm__("$2");
    int *w;
    struct { unsigned char a, b, c, d; } *b;
    unsigned short m;
    unsigned short *e;
    int n;
    int *d;
    unsigned short h;
    char *c;
    int x6;
    int *pa, *pb;
    int vq;
    char *c2;
    int x5;

    if (SlotOrder[0] != 0) {
        ShuffleSlotOrder();
        SlotOrder[0] = 0;
    }
    PsyqBzero(AttractWork, 0x80);
    *(int *)(AttractWork + 0xC) = a0;
    i = 3;
    q = sp + 3;
    do {
        *q = i;
        i--;
        q--;
    } while (i >= 0);
    i = 0;
    do {
        s0 = PsyqRand() % 4;
        r = PsyqRand() % 2;
        v = s0 + r;
        t = v;
        if (v < 0) t = v + 3;
        t = (t >> 2) << 2;
        vq = v - t;
        /* ASM_BARRIER: schedule barrier so the sp[s0] address is computed after vq (original order) */
        __asm__ volatile("" : "=r"(vq) : "0"(vq));
        s2 = sp[s0];
        y = sp[vq];
        sp[s0] = y;
        sp[vq] = s2;
        i++;
    } while (i < 0x10);
    i = 0;
    x6 = PsyqRand();
    s2 = x6 / 18;
    s2 = x6 - s2 * 18;
    tb = AttractCarSlotMap;
    w = AttractDemoSlotKinds;
    b = (void *)SelectionRows;
    do {
        b[i].a = sp[i];
        b[i].b = tb[sp[i]];
        *w = tb[sp[i]];
        u = s2 + PsyqRand() % 4;
        v = u - 2;
        if (v < 0) v = 0;
        if (v > 0x12) v = 0x12;
        b[i].c = v;
        w++;
        i++;
    } while (i < 4);
    CourseIndex = -1;
    if (a0 == 1) {
        m = Pad0HeldButtons & AttractCourseButtonMask;
        if (m != 0) {
            i = 0;
            e = DemoCourseButtonMap;
            do {
                if (m == e[0])
                    CourseIndex = e[1];
                i++;
                e += 2;
            } while (i < 8);
        }
    }
    if (CourseIndex < 0) {
        d = SlotOrder;
        n = d[9];
        h = ((unsigned short *)d)[n * 2 + 2];
        n = n + 1;
        d[9] = n;
        CourseIndex = h;
        if (n >= 8)
            ShuffleSlotOrder();
    }
    OpeningInit(0, 0);
    RaceType = 5;
    MenuDispMode = 0;
    MenuScriptId = 0xA;
    ModeGroup = 1;
    ModeId = 0x23;
    ModeSubState = 0;
    if (a0 == 0) {
        StartSceneFadeIn(-1);
        RaceBgmListIndex = CourseBgmChoiceTable2[CourseIndex] + 1;
    } else {
        ResetSceneSoundState(0);
        StartDemoSceneFadeIn(2);
        RaceBgmListIndex = 1;
    }
    ResetScreenCopyOverlay();
    c = RaceContext;
    MaskBitSetting = 1;
    *(short *)(c + 0x4E0) = 0xD2;
    *(short *)(c + 0x4E2) = 0;
    *(short *)(c + 0x4E4) = 0x8FC;
    HsvToRgb(c + 0x4E0, c + 0x4E8);
    SetSceneColorParams(0, c[0x4E8], c[0x4E9], c[0x4EA]);
    *(int *)(c + 0x534) = 0;
    if (a0 == 0) {
        x5 = *(int *)(c + 0x3DC);
    } else {
        *(int *)(c + 0x3DC) = 0;
        x5 = 0;
    }
    SetSceneOverlayEnabled(x5);
    c2 = RaceContext;
    *(int *)(c2 + 0x4F0) = -0x20;
    *(int *)(c2 + 0x4EC) = 0;
    *(int *)(c2 + 0x4F4) = 0;
    *(int *)(c2 + 0x4FC) = 0;
    *(int *)(c2 + 0x500) = 0;
    *(int *)(c2 + 0x504) = 0;
    *(int *)(c2 + 0x514) = 0x122;
    SoundReverbOffAll();
    ClearTrackObjectStates(0);
    SceneId = 0x209;
}

void AttractDemoRaceEnter(void) {
    SetDispMask(0);
    ResetScreenCounters();
    MenuScriptId = 8;
    ModeGroup = 1;
    ModeId = 0x23;
    ModeSubState = 0;
    ResetSceneSoundState(0);
    StartDemoSceneFadeIn(1);
    DrawSubOtFlag = 0;
}
