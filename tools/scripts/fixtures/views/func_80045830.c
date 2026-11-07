#include "m2c_macros.h"

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(Context650 (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(int (*)[])&RaceContext)

#define D_800AC6B0 (*(char (*)[])&D_800AC6B0)

#define RaceIntroState (*(int (*))&RaceIntroState)

#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef RaceContext

#undef D_800AC6B0

#undef RaceIntroState

#undef CurrentDrawBuf

extern int PsyqBzero();

extern M2C_UNK RaceIntroState;

void ResetRaceSceneFadeState(void) {
    PsyqBzero(&RaceIntroState, 0x20);
    M2C_FIELD(&RaceIntroState, s32 *, 0x1C) = -1;
}
