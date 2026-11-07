/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#define ModeMenuCursorSaved (*(unsigned char (*))&ModeMenuCursorSaved)

#undef LoadIndex

#undef ModeMenuCursorSaved

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define CarClutData (*(int (*))&CarClutData)

#undef CarClutData

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadTimList ((void (*)())LoadTimList)

#define RaceLoadReset ((void (*)())RaceLoadReset)

#define SoundCloseVabsAndReset ((void (*)())SoundCloseVabsAndReset)

#undef LoadTimList

#undef RaceLoadReset

#undef SoundCloseVabsAndReset

extern char LoadIndex;

extern unsigned char LoadState;

extern int GetR4FileIndex();

extern int CdFileReadStart();

extern int CdFileBytesLoaded();

extern int LoadCursor;

extern short CourseIndex;

extern void SetTrackNodeData();

extern char *TrackNodes;

extern int TrackNodeCount;

extern char *TrackNodesEnd;

extern int TrackPreviewTick;

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
