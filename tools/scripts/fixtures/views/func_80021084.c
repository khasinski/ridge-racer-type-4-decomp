/* CC1_FLAGS: -fno-strength-reduce */

#include "rr/hud_number.h"

/* legacy typing of shared globals in this part of the unit */
#define HudNumberTargets (*(struct W3G0A (*))&HudNumberTargets)

#undef HudNumberTargets

/* legacy typing of shared globals in this part of the unit */
#define HudNumberTargets (*(char (*)[])&HudNumberTargets)

#undef HudNumberTargets

extern char AltOrderingTableA[];

extern char AltOrderingTableB[];

extern char RaceSubOtsA[];

extern char RaceOrderingTableB[];

extern short PlayerCount;

void InitHudNumberTargets(int a0) {
    switch (a0) {
    case 0: case 1: case 5: break;
    case 2: case 3: case 4: case 6:
        if (a0 == 4 && PlayerCount == 2) {
        } else if (a0 != 6) {
            goto w3g0b;
        }
    }
    HudNumberTargets.p0 = AltOrderingTableA;
    HudNumberTargets.p4 = AltOrderingTableB;
    HudNumberTargets.s8 = 0x106;
    HudNumberTargets.sA = 0xE6;
    HudNumberTargets.sC = 0x122;
    HudNumberTargets.sE = 0xDC;
    HudNumberTargets.s10 = 0x112;
    HudNumberTargets.s12 = 0xCE;
    return;
w3g0b:
    HudNumberTargets.p0 = RaceSubOtsA;
    HudNumberTargets.p4 = RaceOrderingTableB;
    HudNumberTargets.s8 = 0xFC;
    HudNumberTargets.sA = 0x58;
    HudNumberTargets.sC = 0x128;
    HudNumberTargets.sE = 0x4A;
    HudNumberTargets.s10 = 0xFC;
    HudNumberTargets.s12 = 0x68;
    HudNumberTargets.p14 = RaceSubOtsA;
    HudNumberTargets.p18 = RaceOrderingTableB;
    HudNumberTargets.s1C = 0xFC;
    HudNumberTargets.s1E = 0xD8;
    HudNumberTargets.s20 = 0x122;
    HudNumberTargets.s22 = 0xCA;
    HudNumberTargets.s24 = 0xFC;
    HudNumberTargets.s26 = 0xE8;
}
