/* RODATA 0x80010514 0x24 */
#include "rr/menu_transition.h"
#include "game/menu/menutran.h"
#include "game/data/ldscn1.h"
#include "game/data/globals.h"
#include "game/data/selection.h"

typedef struct { int a; short b; char c; char d; } S8;

typedef struct { int x; } __attribute__((packed)) U4;

extern s16 MenuResultCode;
extern int PsyqBzero();
extern int DecodeCourseIndex();
extern unsigned char GpStagePlaceLimits[];
extern short SelectedCarIndex;
extern short CourseMenuSelection;
extern short SelectedTeamIndex;
extern short CourseListIndex;
extern short CourseIndex;
extern signed char SelectionState[];
extern unsigned char SelectionGpStartProgress;
extern unsigned char SaveProgressCourseUnlock1;
extern S8 RankEntries[];
extern int SaveProgress[];

void SetNextModeByGameType(void);
void AdvanceGrandPrixStage();
int CountProgressFlags();

void SetNextModeByGameType(void){
 switch(ModeMenuCursorSaved){
 case 0:SceneId=0x105;break;
 case 1:default:SceneId=0x131;break;
 case 2:SceneId=0x137;break;
 case 3:SceneId=0x127;break;
 case 6:SceneId=0x201;break;
 case 4:SceneId=0x149;break;
 case 5:MenuResultCode=0;SceneId=0x147;break;
 case 7:SceneId=0x14E;break;
 case 8:SceneId=0x13B;break;
 }
}

void AdvanceGrandPrixStage() {
    int x, c0, i0, j;
    int a0;
    register int v1;
    int a2;
    char *s0, *p;
    a2 = (unsigned char)SelectionState[0x17];
    i0 = (signed char)a2;
    if (i0 != -1) {
        if (SelectionState[0x19] <= GpStagePlaceLimits[i0 / 2]) {
            if (SelectionState[i0] != -1) {
                SelectionState[0x17] = a2 + 1;
            }
        }
    }
    a0 = SelectionGpStage;
    if (a0 == -1) {
        PsyqBzero(RankEntries, 0x40);
        j = 0;
        do {
            RankEntries[j].c = j;
            RankEntries[j].b = j;
            j++;
        } while (j < 8);
        v1 = 0;
        PsyqBzero(SelectionRows, 0x20);
        PsyqBzero(SelectionState, 0x2C);
        a0 = 0;
        v1 = 0;
        a2 = -1;
        *(U4 *)&SelectionState[0x1C] = *(U4 *)&SelectionRows[0];
        SelectedCarIndex = 0;
        do {
            p = (char *)SelectionState + v1;
            v1++;
            p[0] = a2;
            p[8] = 3;
        } while (v1 < 8);
        SelectionGpStartProgress = SaveProgressCourseUnlock1;
    }
    x = (unsigned char)SelectionState[0x17];
    x = (x << 24) >> 25;
    SelectionState[0x15] = x;
    if ((unsigned)(x & 0xFF) >= 3) {
        SelectionState[0x15] = 2;
    }
    *(U4 *)&SelectionRows[0] = *(U4 *)&SelectionState[0x1C];
    CourseMenuSelection = a0;
    c0 = SelectionRows[0];
    SelectedTeamIndex = c0;
    CourseListIndex = SelectionRows[1];
    SelectedCarRow = SelectionRows[2];
    CourseIndex = DecodeCourseIndex(a0, c0, a2);
    SelectedCarIndex = 0;
    SaveProgress[3] |= 1 << CourseMenuSelection;
    if (SelectionRows[2] >= 5) {
        SelectedCarIndex = (SelectionRows[2] - 5) % 7;
        return;
    }
    if (SelectionRows[2] != 0) {
        SelectedCarIndex = SelectionRows[2] - 1;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SaveProgress (*(char (*)[])&SaveProgress)

int CountProgressFlags() {
    int b, off, i;
    /* ASM_BARRIER: pin reproduces original register assignment */
    register int n __asm__("$7");
    int a;
    unsigned v;
    n = 0;
    a = 0;
    do {
        char *p = SaveProgress;
        b = 0;
        off = a << 4;
      L1:
        i = 0;
        v = *(unsigned *)(off + (int)p + 0x10);
        do {
            if ((v >> i) & 1) n++;
            i++;
        } while (i < 20);
        b++;
        off += 4;
        if (b < 4) goto L1;
        a++;
    } while (a < 4);
    return n;
}

#undef SaveProgress
