/* RODATA 0x8001083c 0x1c */
#include "game/data/cdfile2.h"
#include "game/data/trkld6.h"
#include "game/gfx/fadespr.h"
#include "game/data/ldscn4.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "rr/course_choices.h"
extern void LoadTimList();
extern void RaceLoadReset();
extern int PsyqRand();
extern int CdDirectoryBuffer;
extern int MenuArenaBase;
extern unsigned char LoadState;

int LoadIntroDemoAssetsStep(void) {
    int r = 0;
    int t;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0x10), LoadCursor) != 0) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded() != 0) {
            LoadTimList(LoadCursor);
            LoadState = 3;
            LoadCursor = CdDirectoryBuffer;
        }
        break;
    case 3:
        if (CdFileReadStart(GetR4FileIndex(1, 10), LoadCursor) != 0) {
            LoadState = 4;
        }
        break;
    case 4:
        t = CdFileBytesLoaded();
        if (t != 0) {
            LoadCursor += t;
            LoadState = 5;
            MenuArenaBase = LoadCursor;
            RaceLoadReset();
        }
        break;
    case 5:
        if (LoadVabAndTimsStep() == 1) {
            ResetSceneSoundState(PsyqRand() & 1);
            LoadState = 6;
    case 6:
            r = 1;
        }
        break;
    }
    return r;
}
