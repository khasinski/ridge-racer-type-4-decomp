#include "scratchpad.h"
#include "game/race/objsim.h"
#include "game/ui/hudmap.h"
#include "game/course/trktext.h"
#include "game/ui/uipart1.h"
#include "game/gfx/drawbuf.h"
struct A { char pad[0xF0]; int idx; char p2[0xE0]; short ang; };

struct B { char pad[0x12]; short x; char p2[0x28]; };

extern int AddDrawAreaPrim();
extern int FormatDecimalDigits();
extern char StrTotalTimeLabel[];
extern char LapTimeLabel[];
extern char StrPressStart[];
extern short BestLapIndex;
extern int NumLaps;
extern int TotalRaceTime;
extern int LapTimes[];
extern int AddDrawModePrim();
extern int AddSpritePrim();
extern char D_8009EE6C[];
extern int RaceState;
extern int GameQueueTileTrans();
extern int AddLineF2SemiTrans();
extern struct B *TrackNodes;

void DrawRaceTimesScreen(void);
void DrawWrongWaySprite(int a0);
void AddPartialLine(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10);
int IsCarFacingBackward(struct A *a0);

void DrawRaceTimesScreen(void) {
    char sp18[8];
    int i;
    int y;
    int *q;
    char *b;
    int col, v;
    int sel;
    *(int *)SCRATCHPAD = AddDrawAreaPrim(CurrentDrawBuf + 0x70, *(int *)SCRATCHPAD, 0, 0, 0x140, 0x100);
    i = 0;
    sel = BestLapIndex;
    if (NumLaps > 0) {
        y = 0x86;
        q = LapTimes;
    L1:
        v = *q;
        col = 0x7EF8;
        if (i == sel) {
            /* ASM_BARRIER: hide the known value of col so the then-arm reload is kept */
            __asm__ volatile("" : "=r"(col));
            col = 0x7EF8;
        }
        DrawRaceTime(0x78, y, v, col, 1);
        y += 0xA;
        q += 1;
        if (++i < NumLaps) goto L1;
    }
    DrawRaceTime(0x78, 0x5E, TotalRaceTime, 0x7EF8, 1);
    DrawFontStringCentered(0x78, 0x50, StrTotalTimeLabel, 0x80, 0);
    DrawFontStringCentered(0x78, 0x78, LapTimeLabel, 0x80, 0);
    i = 7;
    b = sp18 + 7;
    do {
        *b = 0;
        i -= 1;
        b -= 1;
    } while (i >= 0);
    FormatDecimalDigits(sp18, PlayerFinishPlace + 1, 1);
    DrawFontStringCentered(0x78, 0x3C, sp18, 0x80, 0);
    DrawFontStringCentered(0x78, 0xC8, StrPressStart, 0x80, 0);
    AddSetTexpagePrim(0);
}

void DrawWrongWaySprite(int a0) {
    int a3;
    char *s0;
    void *p;
    if ((unsigned)(RaceState - 1) < 3U) {
        p = *(void **)SCRATCHPAD;
        if (a0 > 0) {
            a3 = ((a0 - 1) << 7) + 0x38;
            if (a0 == 1) {
                s0 = CurrentDrawBuf + 0x70;
            } else {
                s0 = CurrentDrawBuf + 0xB70;
            }
        } else {
            a3 = 0x78;
            s0 = CurrentDrawBuf + 0x70;
        }
        *(int *)SCRATCHPAD = AddDrawModePrim(s0, AddSpritePrim(s0, p, 0x6A, a3, D_8009EE6C, 0x7EF8, 0), 0x3E);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf (*(int (*))&CurrentDrawBuf)

void AddPartialLine(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
    int d1, d2, x, y, r, t;
    if (a5 > 0) {
        d1 = a3 - a1;
        d2 = a4 - a2;
        if (a5 >= 0x100) a5 = 0x100;
        r = *(int *)SCRATCHPAD;
        x = a1 + ((d1 * a5) >> 8);
        y = a2 + ((d2 * a5) >> 8);
        if (a10 >= 0) {
            r = AddLineF2SemiTrans(a0, r, a1, a2, x, y, a7, a8, a9);
            if (a6 != 0) {
                r = AddLineF2SemiTrans(a0, r, a1, a2 + 1, x, y + 1, a7, a8, a9);
            }
            r = AddDrawModePrim(CurrentDrawBuf + 0x70, r, ((a10 & 3) << 5) | 5);
        } else {
            r = GameQueueTileTrans(a0, r, a1, a2, x, y, a7, a8, a9);
            if (a6 != 0) {
                r = GameQueueTileTrans(a0, r, a1, a2 + 1, x, y + 1, a7, a8, a9);
            }
        }
        *(int *)SCRATCHPAD = r;
    }
}

#undef CurrentDrawBuf

int IsCarFacingBackward(struct A *a0) {
    int t = 0x800 - TrackNodes[a0->idx].x;
    return (unsigned)(((a0->ang - t) & 0xFFF) - 0x401) < 0x7FF;
}
