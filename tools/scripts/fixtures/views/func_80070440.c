#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define D_800F4CE0 (*(unsigned short (*))&D_800F4CE0)

#undef D_800F4CE0

/* legacy typing of shared globals in this part of the unit */
#define D_8010E78C ((char *)D_8010E78C)

#undef D_8010E78C

/* legacy typing of shared globals in this part of the unit */
#define D_8010E78C ((O *)D_8010E78C)

#undef D_8010E78C

/* legacy typing of shared globals in this part of the unit */
#define D_8010E790 (*(s32 (*))&D_8010E790)

#undef D_8010E790

/* legacy typing of shared globals in this part of the unit */
#define D_8010E798 (*(s32 (*))&D_8010E798)

#undef D_8010E798

extern s16 RearViewMirrorEnabled;

extern s32 D_800F4CE0;

extern s32 D_8010E730;

void ResetRearViewMirror(void) {
    RearViewMirrorEnabled = 1;
    D_800F4CE0 = -0x24;
    D_8010E730 = 0;
}
