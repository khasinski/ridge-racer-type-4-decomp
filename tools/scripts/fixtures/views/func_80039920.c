#include "types.h"

#define P8(base, off) (*(unsigned char *)((char *)(base) + (off)))

#define IDX30(base, i) (*(unsigned char *)((char *)(base) + (i) * 0x24 + 0x30))

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(char (*)[])&PlayerCars)

#define ModeSubState (*(unsigned short (*))&ModeSubState)

#undef PlayerCars

#undef ModeSubState

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(char (*)[])&PlayerCars)

#define SelectionState (*(char (*)[])&SelectionState)

#define ImageRows ((W7C0_S *)ImageRows)

#define ModeSubState (*(unsigned short (*))&ModeSubState)

#define UpdateScreenFade ((void (*)())UpdateScreenFade)

#define SetMenuFrameOptions ((void (*)())SetMenuFrameOptions)

#define StepMenuValue ((void (*)())StepMenuValue)

#define DrawPreviewCar ((void (*)())DrawPreviewCar)

#define PrepareCarDrawPose ((void (*)())PrepareCarDrawPose)

#define SetupMenuCarCamera ((void (*)())SetupMenuCarCamera)

#define ResetScreenCounters ((void (*)())ResetScreenCounters)

#define UpdateImageRow ((void (*)())UpdateImageRow)

#define SetupViewport ((void (*)())SetupViewport)

#undef PlayerCars

#undef SelectionState

#undef ImageRows

#undef ModeSubState

#undef UpdateScreenFade

#undef SetMenuFrameOptions

#undef StepMenuValue

#undef DrawPreviewCar

#undef PrepareCarDrawPose

#undef SetupMenuCarCamera

#undef ResetScreenCounters

#undef UpdateImageRow

#undef SetupViewport

extern unsigned char D_8009F810[];

extern unsigned char SelectionState[];

extern unsigned char D_800AD4EB;

extern unsigned char SaveProgress[];

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
