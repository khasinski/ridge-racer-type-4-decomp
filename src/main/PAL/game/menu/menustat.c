/* RODATA 0x80010538 0x18 */
#include "scratchpad.h"
#include "game/data/trkld1.h"
#include "game/menu/menuicon.h"
#include "game/ui/barsel0.h"
#include "game/menu/menustat.h"
#include "game/data/selection.h"
#include "game/gfx/drawbuf.h"
#include "game/data/menuvars.h"
typedef struct { char pad[0x14]; unsigned char b14; char p15, p16; signed char b17; char p18[2]; short h1A; } S4D8;

extern int CdAudioInit();
extern int CdAudioSelectTrack();
extern int AddScreenFadeTiles();
extern int AddDrawAreaPrim();
extern int InitMenuArenas();
extern int CdAudioBeginPlay();
extern int AddTileGrid2x2();
extern unsigned int ModeFrame;
extern unsigned short ModeSubState;
extern S4D8 SelectionState;
extern char PlayerCars[];
extern int LoadCursor;
extern int SavedLoadCursor;
extern unsigned char RaceInitCameraW140Table[];
extern int MenuConfirmPressed;
extern unsigned short MenuFadeTimer;
extern short ModeId;
extern int AddSpriteWithDrawMode();
extern unsigned char BadgeUvTable[];
extern signed char BadgePatternTable[];

void RaceLoadScreenModeUpdate();

void RaceLoadScreenModeUpdate()
{
    int sp[3];
    unsigned int x;
    int r;
    S4D8 *p;
    ModeFrame++;
    if (ModeFrame > 0x7FFFF) ModeFrame = 0x1000;
    switch (ModeSubState) {
    case 0:
        p = &SelectionState;
        if (p->h1A == 6) {
            if (LoadCarRangeStep(0, 1) == 1) {
                ResetCarState(PlayerCars);
                SavedLoadCursor = LoadCursor;
                r = (p->b17 - 1) / 2;
                x = RaceInitCameraW140Table[p->b14];
                if (r >= 3) { r = 2; x = x * 3; }
                InitCameraState(r, x);
                SetRaceLoadCursor(LoadCursor);
                ModeSubState = 3;
                goto tail;
            }
        } else if (ModeFrame >= 0x96) {
            if (MenuConfirmPressed == 0) {
                if (ModeFrame < 0x177) goto tail;
            }
            ModeSubState = 1;
        } else {
            goto tail;
        }
        break;
    case 1:
        MenuFadeTimer = 0x20;
        ModeSubState = 5;
        break;
    case 3:
        if (LoadSelectionTimAndModelStep(SelectionRows[0], SelectionRows[1])) ModeSubState = 4;
        break;
    case 4:
        if (MenuConfirmPressed == 0 && ModeFrame < 0x177) goto tail;
        CdAudioInit();
        CdAudioSelectTrack(0, 1);
        ModeSubState = 1;
        break;
    case 5:
        MenuFadeTimer--;
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x20 - (short)MenuFadeTimer) << 3, 0x45);
        *(int *)SCRATCHPAD = AddDrawAreaPrim(CurrentDrawBuf + 0x70, *(int *)SCRATCHPAD, 0, 0, 0x140, 0x200);
        if ((short)MenuFadeTimer <= 0) {
            if (SelectionDialogueMode == 6) {
                InitMenuArenas();
                ModeGroup = 1;
                ModeId = 0x21;
                ModeSubState = 0;
                LoadCursor = SavedLoadCursor;
                CdAudioBeginPlay(0);
            } else {
                ModeGroup = 1;
                ModeId = 0x11;
                ModeSubState = 0;
            }
            return;
        }
        break;
    default:
        break;
    }
tail:
    DrawRaceLoadScreen(ModeFrame);
    r = x = ModeFrame;
    if (x < 0x20) {
        sp[0] = (int)(x * 255 + (0x20 - x) * 8) / 32;
        sp[1] = (int)(x * 189 + (0x20 - x) * 8) / 32;
        r = 0x20 - x;
        sp[2] = r / 4;
    } else {
        sp[0] = 0xFF;
        sp[1] = 0xBD;
        sp[2] = 0;
    }
    *(int *)SCRATCHPAD = AddTileGrid2x2(CurrentDrawBuf + 0xB6C, *(int *)SCRATCHPAD, 0, 0, 0x140, 0x200, sp[0], sp[1], sp[2]);
}

int AddBadgeSpriteStrip(a, p, x, y, w, z) int a; int p; int x; int y; int w; int z; {
    int t0 = x;
    int s1 = y;
    int i = 0;
    do {
        signed char *q = &BadgePatternTable[i + a * 8];
        int v = *q;
        unsigned char *e;
        int s0, d, c0, c1, c3;
        i++;
        if (v == -1) break;
        e = BadgeUvTable + v * 4;
        d = q[4];
        c0 = e[0]; c1 = e[1]; s0 = e[2]; c3 = e[3];
        s1 += d;
        t0 = AddSpriteWithDrawMode(p, t0, s1, w, s0, c3, c0, c1, 0x4A, 0x2E28, 1, z);
        s1 += s0;
    } while (i < 4);
    return t0;
}
