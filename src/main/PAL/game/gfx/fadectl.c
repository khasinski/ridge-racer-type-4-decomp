#include "game/gfx/fadeset.h"
#include "game/sound/sndvab.h"
#include "game/gfx/fadectl.h"
extern struct {
    char pad0[0x3E4]; int f3E4; char pad1[4]; int f3EC; char pad2[4]; int f3F4; int f3F8; char pad3[4];
    int f400; int f404; int f408; char pad4[4]; short f410, f412, f414, f416, f418, f41A; int f41C; char pad5[4];
    int f424; char pad6[4]; int f42C; int f430; int f434; char pad7[8]; int f440;
} RaceContext;

void InitSceneFade(void) {
    int v, w, x;
    RaceContext.f434 = 0;
    RaceContext.f440 = 0;
    v = RaceContext.f408;
    if (v == 1 || (v == 0 && RaceContext.f3EC == 0)) {
        if (RaceContext.f3F4 == 0) {
            SetParticleBrightFade(1, 0, 0x100);
            RaceContext.f3F4 = 1;
        }
    }
    w = RaceContext.f408;
    if (w == 2 || (w == 0 && RaceContext.f3EC == 0xFF)) {
        if (RaceContext.f3F4) {
            SetParticleBrightFade(0, 0, 0);
            RaceContext.f3F4 = 0;
        }
    }
    RaceContext.f400 = 0x8C;
    RaceContext.f404 = 0x100;
    RaceContext.f3F8 = 0;
    RaceContext.f410 = 1;
    RaceContext.f412 = 0x10;
    RaceContext.f41A = 0x10;
    RaceContext.f416 = 0x10;
    RaceContext.f418 = 1;
    RaceContext.f414 = 1;
    RaceContext.f3E4 = 0xBB8;
    x = RaceContext.f430;
    RaceContext.f424 = 0;
    RaceContext.f41C = 1;
    if (x == 0) {
        PlaySfxBank2(RaceContext.f42C);
        RaceContext.f430 = 1;
    }
}
