#include "rr/emblem_images.h"
#include "rr/save_slots.h"
#include "rr/menu_state_types.h"
#include "rr/work_arena.h"
#include "rr/load_cursor.h"
#include "game/camera/campath.h"
/* RODATA 0x800107c4 0x74 */
#include "game/data/bankrec.h"
#include "game/data/cdfile2.h"
#include "game/data/trkld1.h"
#include "game/data/trkld6.h"
#include "game/menu/mntrow.h"
#include "game/sound/sndcmd.h"
#include "game/car/carmove1.h"
#include "game/menu/menuslot.h"
#include "game/data/ldscn1.h"
#include "game/data/ldscn4.h"
#include "game/menu/mnrows.h"
#include "game/race/raceinfo.h"
#include "game/data/cdfile1.h"
#include "rr/audio_loop.h"
#include "rr/menu_text_state.h"
#include "game/course/trkintp2.h"
#include "game/gfx/clut.h"
#include "game/sound/fxplay.h"

#include "rr/scene_camera_state.h"
#include "rr/track_sample_types.h"
extern unsigned char LoadState;
#include "rr/cd_directory_buffer.h"
#include "rr/menu_arena_base.h"
extern int MenuSharedScratchPtr;
extern short CourseIndex;
extern struct { short a; short b; } PlayerCourseChoices[];
extern char *PresetEmblemTimPtr;
#include "game/data/selection.h"
extern int DialogueSceneMode;
extern int SelectedStoryBranchId;
extern unsigned char MenuSelectedTeamId;
extern char SceneOffsetLookupBuf[];
extern int TrackNodeCount;
extern char MenuSoundVabBuffer[];

int LoadDefaultRaceStep(void);
int LoadModeMenuAssetsStep(void);
int LoadMenuSoundStep(void);
int LoadSceneStep(void);
int LoadAllCarsStep(void);
int LoadCarRangeStep(int arg0, int arg1);
int LoadIndexedTimStep(void);
int LoadResultsSceneStep(void);
int LoadCarClutStep(void);
int LoadGlyphRowsStep(void);
int LoadMenuCarModelsStep(void);
int LoadTrackNodesStep(void);

int LoadDefaultRaceStep(void) {
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
                RaceContext.startOrder00[i] = i + 1;
                i++;
            } while (i < 7);
            RaceContext.startOrder00[7] = 0;
            LoadState = 3;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

int LoadModeMenuAssetsStep(void) {
    int r = 0;
    int v;
    switch (LoadState) {
    case 0:
        if (GameModeType == 4) {
            r = 1;
            MenuArenas.first = MenuArenaBase;
            LoadCursor = MenuArenaBase;
            break;
        }
        if (GameModeType == 5 && MainMenuChoice == 6) {
            r = 1;
            LoadCursor = MenuArenaBase;
            break;
        }
        LoadState = 1;
        LoadCursor = CdDirectoryBuffer;
        LoadIndex = *(unsigned char *)&GameModeType;
        if (*(unsigned char *)&GameModeType >= 5) {
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

int LoadOpeningMovieTimStep(void) {
    int r = 0;
    int v;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadCursor = CdDirectoryBuffer;
        LoadIndex = MainMenuChoice;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, LoadIndex + 1), LoadCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        v = CdFileBytesLoaded();
        if (v) {
            u8 *t;
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

int LoadMenuSoundStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadCursor = MenuArenaBase;
        RaceLoadReset();
    case 1:
        if (LoadMenuSoundVabStep()) {
            MenuArenaBase = LoadCursor;
            SoundCloseVabsAndReset();
            LoadState = 2;
            InitMenuArenas();
            LoadCursor = MenuSharedScratchPtr;
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
        if (RequestCarLoad(SelectionRows.entries[s0].variant, SelectionRows.entries[s0].selection, SelectionRows.entries[s0].parameter % 20, PlayerCourseChoices[s0].a, s0) == 1) {
            LoadState = 4;
        }
        break;
    case 4:
        s0 = LoadIndex;
        r = CarLoadStep();
        if (r != 1) break;
        a1 = PlayerCourseChoices[s0].b;
        if (a1 >= 2) {
            s2 = PresetEmblemTimPtr;
            a1 -= 2;
        } else {
            if (a1 == r) {
                s2 = CustomEmblemBuffers.bytes + (s0 & 1) * sizeof(EmblemTim840);
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
        if (GameModeType == 4) MenuArenas.first = LoadCursor;
        break;
    default:
        break;
    }
    return s1;
}

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
        if (RequestCarLoad(SelectionRows.entries[arg0].variant, SelectionRows.entries[arg0].selection, SelectionRows.entries[arg0].parameter % 20, PlayerCourseChoices[arg0].a, arg0) == 1) {
            LoadState = 2;
        }
        break;
    case 2:
        r = CarLoadStep();
        if (r != 1) break;
        arg0 += LoadIndex;
        a1 = PlayerCourseChoices[arg0].b;
        if (a1 >= 2) {
            q3 = PresetEmblemTimPtr;
            a1 -= 2;
        } else {
            if (a1 == r) {
                q3 = CustomEmblemBuffers.bytes + (arg0 & 1) * sizeof(EmblemTim840);
                a1 = 0;
            } else {
                a1 = -1;
            }
        }
        if (GameModeType != 1) {
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

int LoadIndexedTimStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadCursor = SharedWorkArena;
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

int LoadResultsSceneStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        SetPresentationDrawEnabled(0);
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
            LoadCursor = MenuBaseLoadCursor;
            RaceLoadReset();
        }
        break;
    case 3:
        if (LoadTrackVabsStep()) {
            InitMotionController();
            SoundCloseVabsAndReset();
            if (GameModeType == 1) {
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

int LoadCarClutStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        if (GameModeType == 1) {
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
            PresetEmblemTimPtr = LoadDestCursor;
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

int LoadTimListStep(void) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadIndex = 0;
        LoadState = 1;
        LoadCursor = MenuSharedScratchPtr;
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

int LoadDialogueStep(void) {
    int s3 = 0;
    int s0, s1, s2, a0, k;
    u8 *v0;

    switch (LoadState) {
    case 0:
        k = 0xAF000;
        *(unsigned char *)&LoadIndex = 0;
        LoadDestCursor = CdDirectoryBuffer + k;
        v0 = LoadDestCursor;
        LoadDestCursor = v0;
        Dialogue = (void *)v0;
        v0 += sizeof(MenuTextState);
        LoadDestCursor = v0;
        if (DialogueSceneMode == 1 && SelectionState.fields.phase17 == 7 && SelectionState.fields.mode1A == 4) {
            SeqStartTrack(1);
        }
        InitDialogueState(Dialogue);
        LoadState = 1;
        break;
    case 1:
        if (DialogueSceneMode == 1) {
            s2 = 0x41;
            s1 = 0x14;
            s2 = SelectionState.fields.copies[0].bytes[0] * 9 + (SelectionState.fields.phase17 + s1);
        } else {
            s2 = 0x41;
            s1 = 4;
        }
        if (CdFileReadRange(GetR4FileIndex(1, s2), s1 * SelectedStoryBranchId, s1, LoadDestCursor)) {
            LoadState = 2;
        }
        break;
    case 2:
        s2 = CdFileBytesLoaded();
        if (s2) {
            SetupDialogueTable(LoadDestCursor);
            LoadState = 3;
            LoadDestCursor += s2;
            Dialogue->images20 = LoadDestCursor;
        }
        break;
    case 3:
        if (CdFileReadStart(GetR4FileIndex(1, 0x42), LoadDestCursor)) {
            LoadState = 4;
        }
        break;
    case 4:
        if (CdFileBytesLoaded()) {
            LoadTimImage((int *)LoadDestCursor);
            LoadState = 5;
        }
        break;
    case 5:
        s2 = MenuSelectedTeamId * 9 + 0x1C;
        a0 = LoadIndex;
        s0 = DialogueAssetTable[Dialogue->b4 * 8 + a0];
        if (s0 != -1) {
            if (DialogueSceneMode == 0) {
                s1 = 8;
                s0 <<= 3;
            } else if (DialogueSceneMode == 1) {
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
                LoadTimList((int *)LoadDestCursor);
            } else {
                LoadDestCursor = LoadDestCursor + s2;
            }
            LoadIndex = LoadIndex + 1;
            s0 = DialogueAssetTable[Dialogue->b4 * 8 + LoadIndex];
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

int LoadMenuCarModelsStep(void) {
    int r = 0;
    char *s;
    int a1;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0x42), SharedWorkArena)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            s = SharedWorkArena;
            LoadTimImage(s);
            s += 0x4E20;
            LoadState = 3;
            CardDialogueStatePtr = (MenuTextState *)s;
            InitDialogueState((MenuTextState *)s);
    case 3:
            if (DialogueSceneMode == 2) {
                a1 = 0x44;
            } else if (DialogueSceneMode == 4) {
                a1 = 0x46;
                CardDialogueStatePtr->b7 = 2;
            } else {
                a1 = 0x45;
            }
            if (CdFileReadRange(GetR4FileIndex(1, a1), SelectedStoryBranchId << 2, 4, SceneOffsetLookupBuf)) {
                LoadState = 4;
            }
        }
        break;
    case 4:
        if (CdFileBytesLoaded()) {
            SetupDialogueTable(SceneOffsetLookupBuf);
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
            TrackNodesEnd = (char *)(TrackNodes + TrackNodeCount);
    case 3:
            r = 1;
        }
        break;
    }
    return r;
}

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
        if (LoadVabPairStep(MenuSoundVabBuffer) != 0) {
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
