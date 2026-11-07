#include "types.h"
#include "m2c_macros.h"
#include "game/gfx/fadesel.h"

extern struct {
    char pad0[0x3E4]; int f3E4; char pad1[0x3F4 - 0x3E8]; int f3F4; int f3F8;
    char pad2[0x404 - 0x3FC]; int f404; char pad3[0x41C - 0x408]; int f41C;
    char pad4[0x434 - 0x420]; int f434; int f438; int f43C; int f440;
} RaceContext;
extern s32 D_800ACA78;

void StartDemoSceneFadeIn(a0)
int a0;
{
    RaceContext.f404 = 0;
    RaceContext.f3F8 = 0;
    RaceContext.f434 = 1;
    RaceContext.f43C = 0;
    RaceContext.f440 = 0;
    switch (a0) {
    case 0:
        RaceContext.f438 = 0xB;
        break;
    case 1:
        RaceContext.f438 = 8;
        break;
    case 2:
        RaceContext.f438 = 0xC;
        break;
    }
    RaceContext.f3F4 = 1;
    RaceContext.f3E4 = 0xBB8;
    RaceContext.f41C = 2;
}

void SetSceneFadeStartFlag(void) {
    D_800ACA78 = 1;
}
