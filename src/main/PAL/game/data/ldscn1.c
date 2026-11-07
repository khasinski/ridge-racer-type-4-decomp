/* RODATA 0x800107c4 0x74 */
#include "game/data/bankrec.h"
#include "game/data/cdfile2.h"
#include "game/data/trkld1.h"
#include "game/data/trkld6.h"
#include "game/menu/mntrow.h"
#include "game/sound/sndcmd.h"
#include "game/car/carmove1.h"
#include "game/data/cdfile2.h"
#include "game/data/trkld1.h"
#include "game/menu/menuslot.h"
#include "game/data/ldscn1.h"
#include "game/data/ldscn4.h"
#include "game/menu/mnrows.h"
#include "game/race/raceinfo.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "rr/audio_loop.h"
#include "game/data/ldscn1.h"
typedef struct { char p[7]; char b; } Z;

extern int RaceLoadReset();
extern short RaceContext[];
extern unsigned char LoadState;
extern int CdDirectoryBuffer;
extern int LoadCursor;
extern int MenuArenaBase;
extern int MenuArenas;
extern int InitMenuArenas();
extern int LoadCursorBase;
extern short CourseIndex;
extern int UploadCarClut();
extern struct { short a; short b; } PlayerCourseChoices[];
extern char *CarClutData;
extern char SaveDataBackup[];
extern unsigned char SelectionRows[][4];
extern int LoadTimList();
extern char SharedWorkBuffer[];
extern int InitMotionController();
extern int SetMotionControllerVisible();
extern int SeqStartTrack();
extern int LoadTimImage();
extern struct { char pad[4]; signed char f4; char pad2[0x1B]; int f20; } *Dialogue;
extern int DialogueContext;
extern struct { char pad[0x17]; signed char f17; char pad2[2]; short f1A; unsigned char f1C; } SelectionState;
extern int GameLanguage;
extern unsigned char SelectionCurrentRow;
extern char DialogueLangEntry[];
extern Z *DialogueState;
extern char *TrackNodes;
extern int TrackNodeCount;
extern char VabHeaderBuffer[];

int LoadRaceSceneStep(void);
int LoadModeMenuAssetsStep(void);
int LoadSelectedModeFileStep(void);
int ReinitMenuArenasStep(void);
int LoadSceneStep(void);
int LoadAllCarsStep(void);
int LoadCarRangeStep(int arg0, int arg1);
int LoadIndexedTimStep(void);
int LoadResultsSceneStep(void);
int LoadCarClutStep(void);
int LoadTimListStep(void);
int LoadGlyphRowsStep(void);
int LoadDialogueStep(void);
int LoadDialogueTableStep(void);
int LoadTrackNodesStep(void);
int LoadSplashAssetsStep(void);

int LoadRaceSceneStep(void) {
    int r = 0;
    int i;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
        RaceLoadReset();
    case 1:
        if (LoadRaceSoundAndTrackStep() == 1) {
            LoadState = 2;
    case 2:
            i = 0;
            do {
                RaceContext[i] = i + 1;
                i++;
            } while (i < 7);
            RaceContext[7] = 0;
            LoadState = 3;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

int LoadModeMenuAssetsStep(void) {
    int r = 0;
    int v;
    switch (LoadState) {
    case 0:
        if (RaceType == 4) {
            r = 1;
            MenuArenas = MenuArenaBase;
            LoadCursor = MenuArenaBase;
            break;
        }
        if (RaceType == 5 && ModeMenuCursorSaved == 6) {
            r = 1;
            LoadCursor = MenuArenaBase;
            break;
        }
        LoadState = 1;
        LoadCursor = CdDirectoryBuffer;
        LoadIndex = *(unsigned char *)&RaceType;
        if (*(unsigned char *)&RaceType >= 5) {
            LoadIndex = 0;
        }
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, LoadIndex + 0xB), LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        v = CdFileBytesLoaded();
        if (v) {
            LoadCursor += v;
    case 3:
            r = 1;
            MenuArenaBase = LoadCursor;
        }
        break;
    }
    return r;
}

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)
#define ModeMenuCursorSaved (*(unsigned char (*))&ModeMenuCursorSaved)

int LoadSelectedModeFileStep(void) {
    int r = 0;
    int v;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadCursor = CdDirectoryBuffer;
        LoadIndex = ModeMenuCursorSaved;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, LoadIndex + 1), LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        v = CdFileBytesLoaded();
        if (v) {
            int t;
            LoadState = 3;
            t = LoadCursor + v;
            LoadCursor = t;
            MenuArenaBase = t;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

#undef LoadIndex
#undef ModeMenuCursorSaved

int ReinitMenuArenasStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadCursor = MenuArenaBase;
        RaceLoadReset();
    case 1:
        if (UnloadRaceStepStub()) {
            MenuArenaBase = LoadCursor;
            SoundCloseVabsAndReset();
            LoadState = 2;
            InitMenuArenas();
            LoadCursor = LoadCursorBase;
    case 2:
            r = 1;
        }
        break;
    }
    return r;
}

int LoadSceneStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (RequestSceneLoad(CourseIndex)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (SceneLoadStep()) r = 1;
        break;
    case 3:
        r = 1;
        break;
    }
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

int LoadAllCarsStep(void) {
    int s0;
    int s1 = 0;
    int r;
    int a1;
    int t;
    char *s2;
    switch (LoadState) {
    case 0:
        LoadState = 3;
        LoadIndex = 0;
    case 3:
        s0 = LoadIndex;
        if (s0 < 0) goto l5;
        if (RequestCarLoad(SelectionRows[s0][1], SelectionRows[s0][0], SelectionRows[s0][2] % 20, PlayerCourseChoices[s0].a, s0) == 1) {
            LoadState = 4;
        }
        break;
    case 4:
        s0 = LoadIndex;
        r = CarLoadStep();
        if (r != 1) break;
        a1 = PlayerCourseChoices[s0].b;
        if (a1 >= 2) {
            s2 = CarClutData;
            a1 -= 2;
        } else {
            if (a1 == r) {
                s2 = SaveDataBackup + (s0 & 1) * 0x840;
                a1 = 0;
            } else {
                a1 = -1;
            }
        }
        UploadCarClut(s2, a1, s0);
        LoadIndex++;
        s1 = -LoadIndex;
        if (LoadIndex != 4) {
            LoadState = 3;
        } else {
l5:
            LoadState = 5;
        }
        break;
    case 5:
        s1 = 1;
        if (RaceType == 4) MenuArenas = LoadCursor;
        break;
    default:
        break;
    }
    return s1;
}

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

int LoadCarRangeStep(int arg0, int arg1) {
    int r;
    int q4 = 0;
    int a1;
    char *q3;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
        break;
    case 1:
        arg0 += LoadIndex;
        if (RequestCarLoad(SelectionRows[arg0][1], SelectionRows[arg0][0], SelectionRows[arg0][2] % 20, PlayerCourseChoices[arg0].a, arg0) == 1) {
            LoadState = 2;
        }
        break;
    case 2:
        r = CarLoadStep();
        if (r != 1) break;
        arg0 += LoadIndex;
        a1 = PlayerCourseChoices[arg0].b;
        if (a1 >= 2) {
            q3 = CarClutData;
            a1 -= 2;
        } else {
            if (a1 == r) {
                q3 = SaveDataBackup + (arg0 & 1) * 0x840;
                a1 = 0;
            } else {
                a1 = -1;
            }
        }
        if (RaceType != 1) {
            UploadCarClut(q3, a1, arg0);
        }
        LoadIndex++;
        if (LoadIndex == arg1) {
            LoadState = 3;
        } else {
            LoadState = 1;
        }
        break;
    case 3:
        q4 = 1;
        break;
    default:
        break;
    }
    return q4;
}

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

int LoadIndexedTimStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadCursor = (int)SharedWorkBuffer;
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, LoadIndex + 0x11), LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            LoadTimList(LoadCursor);
            LoadState = 3;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

#undef LoadIndex

int LoadResultsSceneStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        SetMotionControllerVisible(0);
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0xF), LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            LoadTimList(LoadCursor);
            LoadState = 3;
            LoadCursor = LoadCursorMark;
            RaceLoadReset();
        }
        break;
    case 3:
        if (LoadTrackVabsStep()) {
            InitMotionController();
            SoundCloseVabsAndReset();
            if (RaceType == 1) {
                goto s7;
            }
            LoadState = 5;
        }
        break;
    case 5:
        if (RequestVramImageLoad()) {
            LoadState = 6;
        }
        break;
    case 6:
        if (VramImageLoadStep()) {
s7:
            LoadState = 7;
        }
        break;
    case 7:
        r = 1;
        break;
    }
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define CarClutData (*(int (*))&CarClutData)

int LoadCarClutStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        if (RaceType == 1) {
            LoadState = 3;
        } else {
            LoadState = 1;
        }
        LoadDestCursor = CdDirectoryBuffer + 0xAF000;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0x12), LoadDestCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            LoadState = 3;
            CarClutData = LoadDestCursor;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

#undef CarClutData

int LoadTimListStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadIndex = 0;
        LoadState = 1;
        LoadCursor = LoadCursorBase;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0x13), LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            LoadTimList(LoadCursor);
            LoadState = 3;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

int LoadGlyphRowsStep(void) {
    int r = 0;
    int v;
    switch (LoadState) {
    case 0:
        LoadIndex = 0;
        LoadState = 1;
        GlyphRowCount = 4;
    case 1:
        if (CdFileReadRange(GetR4FileIndex(1, 0x40), 0, 0xC0, LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        v = CdFileBytesLoaded();
        if (v) {
            SetGlyphRowImageBase(LoadCursor, 0);
            LoadState = 3;
            LoadCursor += v;
        }
        break;
    case 3:
        r = 1;
        break;
    }
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

int LoadDialogueStep(void) {
    int s3 = 0;
    int s0, s1, s2, a0, v0, k;

    switch (LoadState) {
    case 0:
        k = 0xAF000;
        *(unsigned char *)&LoadIndex = 0;
        LoadDestCursor = CdDirectoryBuffer + k;
        v0 = LoadDestCursor;
        LoadDestCursor = v0;
        Dialogue = (void *)v0;
        v0 += 0x348;
        LoadDestCursor = v0;
        if (DialogueContext == 1 && SelectionState.f17 == 7 && SelectionState.f1A == 4) {
            SeqStartTrack(1);
        }
        InitDialogueState(Dialogue);
        LoadState = 1;
        break;
    case 1:
        if (DialogueContext == 1) {
            s2 = 0x41;
            s1 = 0x14;
            s2 = SelectionState.f1C * 9 + (SelectionState.f17 + s1);
        } else {
            s2 = 0x41;
            s1 = 4;
        }
        if (CdFileReadRange(GetR4FileIndex(1, s2), s1 * GameLanguage, s1, LoadDestCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        s2 = CdFileBytesLoaded();
        if (s2) {
            SetupDialogueTable(LoadDestCursor);
            LoadState = 3;
            LoadDestCursor += s2;
            Dialogue->f20 = LoadDestCursor;
        }
        break;
    case 3:
        if (CdFileReadStart(GetR4FileIndex(1, 0x42), LoadDestCursor)) {
            LoadState = 4;
        }
        break;
    case 4:
        if (CdFileBytesLoaded()) {
            LoadTimImage(LoadDestCursor);
            LoadState = 5;
        }
        break;
    case 5:
        s2 = SelectionCurrentRow * 9 + 0x1C;
        a0 = LoadIndex;
        s0 = DialogueLoadIndexTable[Dialogue->f4 * 8 + a0];
        if (s0 != -1) {
            if (DialogueContext == 0) {
                s1 = 8;
                s0 <<= 3;
            } else if (DialogueContext == 1) {
                s1 = 8;
                if (a0 == 0) {
                    s1 = 0x2E;
                    s0 = s0 * s1;
                } else {
                    s0 <<= 3;
                    s0 += 0xE6;
                }
            } else {
                s1 = 8;
                s0 <<= 3;
            }
            if (CdFileReadRange(GetR4FileIndex(1, s2), s0, s1, LoadDestCursor)) {
                LoadState = 6;
            }
        } else {
            LoadState = 7;
        }
        break;
    case 6:
        s2 = CdFileBytesLoaded();
        if (s2) {
            if (LoadIndex == 0) {
                LoadTimList(LoadDestCursor);
            } else {
                LoadDestCursor = LoadDestCursor + s2;
            }
            LoadIndex = LoadIndex + 1;
            s0 = DialogueLoadIndexTable[Dialogue->f4 * 8 + LoadIndex];
            if (LoadIndex == 8 || s0 == -1) {
                LoadState = 7;
            } else {
                LoadState = 5;
            }
        }
        break;
    case 7:
        s3 = 1;
        break;
    }
    return s3;
}

#undef LoadIndex

int LoadDialogueTableStep(void) {
    int r = 0;
    char *s;
    int a1;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0x42), SharedWorkBuffer)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            s = SharedWorkBuffer;
            LoadTimImage(s);
            s += 0x4E20;
            LoadState = 3;
            DialogueState = (Z *)s;
            InitDialogueState(s);
    case 3:
            if (DialogueContext == 2) {
                a1 = 0x44;
            } else if (DialogueContext == 4) {
                a1 = 0x46;
                DialogueState->b = 2;
            } else {
                a1 = 0x45;
            }
            if (CdFileReadRange(GetR4FileIndex(1, a1), GameLanguage << 2, 4, DialogueLangEntry)) {
                LoadState = 4;
            }
        }
        break;
    case 4:
        if (CdFileBytesLoaded()) {
            SetupDialogueTable(DialogueLangEntry);
            LoadState = 5;
    case 5:
            r = 1;
        }
        break;
    }
    return r;
}

int LoadTrackNodesStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadIndex = 0;
        LoadState = 1;
        TrackPreviewTick = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, CourseIndex + 0x38), LoadCursor) != 0) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded() != 0) {
            SetTrackNodeData(LoadCursor);
            LoadState = 3;
            TrackNodesEnd = TrackNodes + TrackNodeCount * 60;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define LoadTimList ((void (*)())LoadTimList)
#define RaceLoadReset ((void (*)())RaceLoadReset)
#define SoundCloseVabsAndReset ((void (*)())SoundCloseVabsAndReset)

int LoadSplashAssetsStep(void) {
    int r = 0;
    int t;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0), LoadCursor) != 0) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded() != 0) {
            LoadTimList(LoadCursor);
            RaceLoadReset();
            LoadState = 3;
        }
        break;
    case 3:
        if (LoadVabPairStep(VabHeaderBuffer) != 0) {
            SoundCloseVabsAndReset();
            LoadState = 4;
            LoadCursor = CdDirectoryBuffer;
        }
        break;
    case 4:
        if (CdFileReadStart(GetR4FileIndex(1, 10), LoadCursor) != 0) {
            LoadState = 5;
        }
        break;
    case 5:
        t = CdFileBytesLoaded();
        if (t != 0) {
            LoadCursor += t;
            LoadState = 6;
            MenuArenaBase = LoadCursor;
            case 6:
            r = 1;
        }
        break;
    }
    return r;
}

#undef LoadTimList
#undef RaceLoadReset
#undef SoundCloseVabsAndReset
