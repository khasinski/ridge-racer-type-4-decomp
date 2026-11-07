#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define RearViewMirrorOffsetY (*(unsigned short (*))&RearViewMirrorOffsetY)

#undef RearViewMirrorOffsetY

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
#define OctantLookupTable (*(s32 (*))&OctantLookupTable)

#undef OctantLookupTable

extern s16 RearViewMirrorEnabled;

extern s32 RearViewMirrorOffsetY;

extern s32 RearViewMirrorReady;

void ResetRearViewMirror(void) {
    RearViewMirrorEnabled = 1;
    RearViewMirrorOffsetY = -0x24;
    RearViewMirrorReady = 0;
}
