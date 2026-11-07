/* RODATA 0x800110a0 0x1c */
/* CC1_FLAGS: -fno-strength-reduce */
#include "game/data/cdfile2.h"
#include "game/data/trkld1.h"
#include "game/data/trkld5.h"
#include "game/data/trkld5.h"
#include "game/data/trkld1.h"
#include "game/data/trkld6.h"
#include "game/course/trkres2.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "game/sound/sndcmd.h"
#include "rr/controller_hud.h"
#include "rr/course_choices.h"
#include "game/gfx/model.h"
extern struct { char pad[0x56C]; int a, b, c, d; } RaceContext;
extern struct { char pad[0x10]; int f10; char pad2[0x19CC - 0x14]; int f19cc; } SoundState;
extern short RaceType;
extern short CourseListIndex;
extern int SoundSequenceSet;
extern unsigned short SavedCourseIndex;
extern short CourseIndex;
extern int LoadTimList();
extern int LoadVab();
extern char RaceVabLoadParams[];
extern char *RaceLoadOffsetTable;
extern char VabLoadArgs[];
extern int VabTimLoadParams[];
extern int LoadVab();

void SetupRaceSoundSlots(void);
int LoadRaceSoundAndTrackStep(void);
int LoadTrackVabsStep(void);
int LoadModelBank8Step(void);
int LoadVabAndTimsStep();

void SetupRaceSoundSlots(void) {
    switch (RaceType) {
    case 6:
        SoundState.f19cc = 1;
        RaceContext.a = 5;
        RaceContext.b = GetSelectionSoundBank(0);
        RaceContext.c = -1;
        SoundState.f10 = 0;
        RaceContext.d = CourseListIndex + 1;
        break;
    case 0:
        SoundState.f19cc = 1;
        RaceContext.a = 5;
        RaceContext.b = GetSelectionSoundBank(0);
        RaceContext.c = -1;
        RaceContext.d = -1;
        SoundState.f10 = 0;
        break;
    default:
    case 1:
    case 5:
        SoundState.f19cc = 0;
        RaceContext.a = 4;
        RaceContext.b = GetSelectionSoundBank(0);
        RaceContext.c = -1;
        RaceContext.d = 0;
        SoundState.f10 = 0;
        break;
    case 2:
    case 3:
        SoundState.f19cc = 1;
        RaceContext.a = 5;
        RaceContext.b = GetSelectionSoundBank(0);
        RaceContext.c = GetSelectionSoundBank(1);
        RaceContext.d = -1;
        SoundState.f10 = 1;
        break;
    case 4:
        SoundSequenceSet = 1;
        RaceContext.a = 5;
        if (LinkRoleSlot != 0) {
            RaceContext.b = GetSelectionSoundBank(0);
            RaceContext.c = GetSelectionSoundBank(1);
        } else {
            RaceContext.b = GetSelectionSoundBank(2);
            RaceContext.c = GetSelectionSoundBank(3);
        }
        D_800ACBC8 = -1;
        LinkRaceSoundFlag = 1;
        break;
    }
}

int LoadRaceSoundAndTrackStep(void) {
    switch (RaceLoadPhase) {
    case 0:
        SetupRaceSoundSlots();
        RaceLoadPhase += 1;
    case 1:
        if (LoadRaceSoundStep() != 0) {
            RaceLoadStepIndex = 0;
            RaceLoadState = 0;
            RaceLoadSubState = 0;
            RaceLoadCursor = LoadCursor;
            *(int *)&SavedCourseIndex = CourseIndex;
            if (CourseIndex >= 8) {
                CourseIndex = 7;
            }
            RaceLoadPhase += 1;
        }
        return 0;
    case 2:
        if (LoadTrackStep() != 0) {
            RaceLoadPhase += 1;
            CourseIndex = SavedCourseIndex;
        }
        return 0;
    case 3:
        return 1;
    default:
        return 0;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define LoadCursor ((char *)LoadCursor)
#define SoundState (*(char (*)[])&SoundState)
#define RaceLoadCursor ((char *)RaceLoadCursor)

int LoadTrackVabsStep(void)
{
    int i, i4, r, ret;
    char *s1, *s3;
    char *t;
    switch (RaceLoadStepIndex) {
    case 0:
        ret = r = CdFileReadStart(GetR4FileIndex(2, 1), RaceLoadCursor);
        if (r == 0) break;
        RaceLoadStepIndex = RaceLoadStepIndex + 1;
    case 1:
        if (CdFileBytesLoaded() == 0) { ret = 0; break; }
        t = RaceLoadCursor + *(int *)(RaceLoadCursor + 4);
        RegisterModelBank(t, 7);
        i = 0;
        RelocateOffsetTable(RaceLoadCursor + *(int *)(RaceLoadCursor + 8));
        s3 = SoundState;
        s1 = RaceVabLoadParams;
        RaceLoadOffsetTable = RaceLoadCursor + *(int *)(RaceLoadCursor + 8);
        do {
            *(char **)VabLoadArgs = RaceLoadCursor + *(int *)(RaceLoadCursor + i * 4 + 0x14);
            *(char **)(VabLoadArgs + 4) = RaceLoadCursor + *(int *)(RaceLoadCursor + i * 4 + 0x1C);
            LoadVab(VabLoadArgs, *(int *)(s1 + 4), *(int *)(s1 + 8));
            s1 += 0xC;
            *(char **)(s3 + 0x2858) = RaceLoadCursor + *(int *)(RaceLoadCursor + i * 4 + 0xC);
            s3 += 4;
            i++;
        } while (i < 2);
        LoadTimList(RaceLoadCursor + *(int *)(RaceLoadCursor + 0x24));
        t = AlignUp4(RaceLoadCursor + *(int *)(RaceLoadCursor + 0x1C));
        LoadCursor = t;
        RaceLoadCursor = t;
        RaceLoadStepIndex = 0;
        SoundCloseVabsAndReset();
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}

#undef LoadCursor
#undef SoundState
#undef RaceLoadCursor

/* legacy typing of shared globals in this part of the unit */
#define LoadTimList ((int (*)())LoadTimList)
#define RegisterModelBank ((int (*)())RegisterModelBank)
#define RaceLoadCursor ((char *)RaceLoadCursor)

int LoadModelBank8Step(void) {
    int v;
    switch (RaceLoadStepIndex) {
    case 0:
        v = CdFileReadStart(GetR4FileIndex(2, 2), RaceLoadCursor);
        if (v != 0) {
            RaceLoadStepIndex += 1;
    case 1:
            if (CdFileBytesLoaded() != 0) {
                RegisterModelBank(RaceLoadCursor + *(int *)(RaceLoadCursor + 4), 8);
                LoadTimList(RaceLoadCursor + *(int *)(RaceLoadCursor + 8));
                RaceLoadStepIndex = 0;
                return 1;
            }
            v = 0;
        }
        return v;
    default:
        return 0;
    }
}

#undef LoadTimList
#undef RegisterModelBank
#undef RaceLoadCursor

/* legacy typing of shared globals in this part of the unit */
#define LoadCursor ((int *)LoadCursor)
#define RaceLoadCursor ((int *)RaceLoadCursor)
#define VabLoadArgs (*(int (*)[])&VabLoadArgs)
#define LoadTimList ((int (*)())LoadTimList)

int LoadVabAndTimsStep()
{
    int r;
    int *q;
    switch (RaceLoadStepIndex) {
    case 0:
        r = CdFileReadStart(GetR4FileIndex(2, 0), RaceLoadCursor);
        if (r != 0) {
            RaceLoadStepIndex++;
    case 1:
            if (CdFileBytesLoaded()) {
                q = VabTimLoadParams;
                VabLoadArgs[0] = (int)RaceLoadCursor + RaceLoadCursor[1];
                VabLoadArgs[1] = (int)RaceLoadCursor + RaceLoadCursor[2];
                LoadVab(VabLoadArgs, q[1], q[2]);
                LoadTimList((int)RaceLoadCursor + RaceLoadCursor[3]);
                RaceLoadStepIndex = 0;
                RaceLoadCursor = (int *)((int)RaceLoadCursor + RaceLoadCursor[2]);
                LoadCursor = RaceLoadCursor;
                return 1;
            }
            r = 0;
        }
        break;
    default:
        r = 0;
    }
    return r;
}

#undef LoadCursor
#undef RaceLoadCursor
#undef VabLoadArgs
#undef LoadTimList
