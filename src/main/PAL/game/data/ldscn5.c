/* RODATA 0x8001085c 0x18 */
#include "game/data/cdfile2.h"
#include "game/data/ldscn4.h"
#include "game/data/ldscn5.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "rr/course_choices.h"
extern void LoadTimList();
extern unsigned char LoadState;

int LoadCoursePreviewStep(int a) {
    int r = 0;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(0, a + 1), LoadCursor) != 0) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded() != 0) {
            LoadTimList(LoadCursor);
    case 3:
            if (CdFileReadStart(GetR4FileIndex(0, a + 10), LoadCursor) != 0) {
                LoadState = 4;
            }
        }
        break;
    case 4:
        if (CdFileBytesLoaded() != 0) {
            SkyCommandList = LoadCursor;
            RelocateOffsetTable(LoadCursor);
            LoadState = 5;
    case 5:
            r = 1;
        }
        break;
    }
    return r;
}
