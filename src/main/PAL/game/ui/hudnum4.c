#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/ui/window2.h"
#include "game/race/objsim.h"
#include "psyq/libgpu/primattr.h"
#include "rr/hud_number.h"
#include "rr/fade_sprite.h"
#include "rr/audio_loop.h"
#include "game/data/globals.h"
#include "game/data/screenframe.h"

struct S { char pad[0x2AA]; short f; char pad2[0x320 - 0x2AC]; };

extern s32 AddDrawModePrim();
extern short PlayerCount;
extern short NumCars;
extern unsigned char D_8009EE40[];
extern unsigned char D_8009EE44[];
extern int rsin();
extern struct S PlayerCars[];
extern int NumLaps;

void DrawHudDigitMod10(s32 arg0, s32 arg1);
void UpdateHudRankDisplay();
void func_80021C00(int a0);

void DrawHudDigitMod10(s32 arg0, s32 arg1) {
    s32 temp_s0;
    s32 temp_v1;
    char *temp_a3;
    char *b;
    s32 cur, m, r;
    s32 a = arg0;
    b = HudNumberTargets;
    temp_v1 = arg1 * 0x14;
    temp_a3 = b + temp_v1;
    cur = *(s32 *)SCRATCHPAD;
    temp_s0 = *(s32 *)(DrawBufIndex * 4 + temp_v1 + b);
    m = a % 10;
    r = EmitHudGlyph12Alt(temp_s0, cur, M2C_FIELD(temp_a3, s16 *, 0xC), M2C_FIELD(temp_a3, s16 *, 0xE), m, 0x7EFB);
    *(s32 *)SCRATCHPAD = AddDrawModePrim(temp_s0, r, 5);
}

void UpdateHudRankDisplay(a0, a1) int a0; int a1; {
    int base, a, s0, s1, v;
    char *s2, *p2;
    base = CurrentDrawBuf;
    s2 = (char *)(base + 0x226B0);
    *(char *)(a1 * 0x78 + (int)s2 + 0xC) = (a0 + 1) * 0x18;
    p2 = (char *)(base + 0x226D8);
    if (RaceType == 4) {
        a = PlayerCount;
    } else {
        if (RaceType == 3) {
            a = 2;
        } else {
            a = NumCars;
        }
    }
    *(char *)(a1 * 0x78 + (int)p2 + 0xC) = D_8009EE44[a - 1];
    if (RaceType == 1) {
        s0 = 0x40;
        s1 = 0;
        if (D_800AC0E6 <= D_8009EE40[D_800AD4EF / 2]) {
            s1 = 1;
            v = rsin((ScreenFrame & 0x3F) << 6);
            if (v < 0) v += 0x3F;
            s0 = v >> 6;
            if (s0 < 0) {
                s1 = 0;
                s0 = -s0;
            }
        }
        s0 += 0x40;
        *(short *)(s2 + 0xE) = s1 + 0x7EF8;
        SetShadeTex(s2, 0);
        s2[4] = s0;
        s2[5] = s0;
        s2[6] = s0;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

void func_80021C00(int a0) {
    int a1, a2;
    char *p;
    a2 = a0 - 1;
    if (a0 == 0) {
        a1 = 7;
        a2 = 0;
    } else {
        a1 = a2 * 6 + 1;
    }
    p = CurrentDrawBuf + (a1 * 0x14 + 0x22688);
    if (PlayerCars[a2].f < NumLaps) {
        *(short *)(p + 0x10) = 0;
        return;
    }
    *(short *)(p + 0x10) = 0x14;
}

#undef CurrentDrawBuf
