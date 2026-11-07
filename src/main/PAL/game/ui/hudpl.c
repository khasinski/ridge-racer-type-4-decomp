#include "scratchpad.h"
#include "game/gfx/drawbuf.h"
typedef struct { int x; } __attribute__((packed)) U4;
extern int AddSpritePrim();
extern int AddDrawModePrim();
extern U4 LapCounterSprites[];
extern int NumLaps;

void DrawLapCounter(a0, a1)
int a0; int a1;
{
    U4 buf;
    int s4, s5, a, i;
    if (a0 == 0) {
        s5 = 0xC;
        s4 = CurrentDrawBuf + 0x70;
    } else {
        s4 = CurrentDrawBuf + 0xB70;
        s5 = ((a0 - 1) << 7) + 0xC;
    }
    a = *(int *)SCRATCHPAD;
    i = 0;
    do {
        buf = LapCounterSprites[i];
        a = AddSpritePrim(s4, a, 0xEC + i * 0x38, s5, &buf, 0x7EF8, 0);
        i++;
    } while (i < 2);
    buf = LapCounterSprites[2];
    i = 0;
    if (a1 <= 0) {
        a1 = 1;
    }
    do {
        if (i == 0) {
            *(char *)&buf = a1 * 0xC;
        }
        if (i == 1) {
            *(char *)&buf = NumLaps * 0xC;
        }
        a = AddSpritePrim(s4, a, 0x118 + i * 0x14, s5, &buf, 0x7EF8, 0);
        i++;
    } while (i < 2);
    *(int *)SCRATCHPAD = AddDrawModePrim(s4, a, 0x25);
}
