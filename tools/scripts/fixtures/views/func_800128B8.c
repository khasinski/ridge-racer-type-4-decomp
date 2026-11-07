/* Candidate module: CD read state machine, 80012578..80012A68.
 * Adjacent matching functions share one directory and location definition.
 * This grouping does not establish the original compiler unit boundary. */
#include "rr/cd.h"

#define B (*(int *)&LoadCursor)

/* legacy typing of shared globals in this part of the unit */
#define RegisterModelBank ((void (*)(void *, s32))RegisterModelBank)

#define LoadCursor ((Resource *)LoadCursor)

#define CarBoundsTable (*(void *(*)[])&CarBoundsTable)

#undef RegisterModelBank

#undef LoadCursor

#undef CarBoundsTable

/* legacy typing of shared globals in this part of the unit */
#define DrawSync ((int (*)())DrawSync)

#define LoadImage ((int (*)())LoadImage)

#undef DrawSync

#undef LoadImage

/* legacy typing of shared globals in this part of the unit */
#define TrackNodeLightBits ((short *)TrackNodeLightBits)

#undef TrackNodeLightBits

extern s32 CdFileState;

extern s32 (*CdFileStateHandlers[])();

extern s32 CdFileSyncResult;

s32 CdFileStep(void) {
    if (CdFileState > 0) {
        CdFileSyncResult = CdSync(1, 0);
        if (CdFileSyncResult != 0) {
            return CdFileStateHandlers[CdFileState]();
        }
    }
    return 0;
}
