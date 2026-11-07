#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((int *)RaceLoadCursor)

#define RaceContext (*(struct { char pad[0x24C]; char *p; char pad2[0x560 - 0x250]; char *r560; } (*))&RaceContext)

#undef RaceLoadCursor

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

#undef RaceLoadCursor

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

#undef RaceLoadCursor

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

#undef RaceLoadCursor

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

#define VabLoadArgs (*(int (*)[2])&VabLoadArgs)

#undef RaceLoadCursor

#undef VabLoadArgs

extern int RaceLoadState;

extern int RaceLoadCursor;

extern int RaceLoadSubState;

extern s32 RaceLoadPhase;

extern s32 RaceLoadStepIndex;

extern s32 LoadCursor;

void RaceLoadReset(void) {
    RaceLoadStepIndex = 0;
    RaceLoadState = 0;
    RaceLoadSubState = 0;
    RaceLoadPhase = 0;
    RaceLoadCursor = LoadCursor;
}
