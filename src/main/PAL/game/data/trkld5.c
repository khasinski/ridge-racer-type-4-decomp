/* RODATA 0x80011088 0x14 */
#include "rr/track_resources.h"
#include "game/data/cdfile2.h"
#include "game/data/trkld1.h"
#include "game/data/trkld1.h"
#include "game/data/trkld5.h"
#include "game/course/trkres2.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "game/data/selection.h"

typedef struct { unsigned char a, b, c, d; } T;

extern short SelectedCarIndex;
extern void *LoadCursor;
extern short SelectedTeamIndex;
extern short CourseIndex;

int LoadTrackStep()
{
    int r;
    int r2;
    int r3;
    switch (RaceLoadStepIndex) {
    case 0:
        r = CdFileReadStart(GetR4FileIndex(2, CourseIndex + 0x16), RaceLoadCursor);
        if (r == 0) goto out;
        goto inc;
    case 1:
        r2 = CdFileBytesLoaded();
        if (r2 != 0) {
            LoadTrackResource((RrTrackResourceFile *)RaceLoadCursor);
            RaceLoadCursor += AlignUp4(r2);
            RaceLoadStepIndex += 1;
        }
        break;
    case 2:
        r3 = LoadCarSpecStep(CourseIndex, SelectedTeamIndex, SelectedCarIndex);
        goto chk;
    case 3:
        r3 = LoadTeamModelsStep(CourseIndex, SelectedTeamIndex);
    chk:
        if (r3 != 0) {
    inc:
            RaceLoadStepIndex += 1;
        }
        break;
    case 4:
        RaceLoadStepIndex = 0;
        LoadCursor = RaceLoadCursor;
        return 1;
    }
    r = 0;
out:
    return r;
}

int GetSelectionSoundBank(int i) {
    T *base = SelectionRows;
    T *t = base + i;
    int r = 0;
    if (t->c != 0x27) {
        r = t->b;
        if (t->c == 0x13) {
            r += 4;
        }
    } else if (t->b == 0) {
        r = (t->a == 2) * 8;
    }
    return r;
}
