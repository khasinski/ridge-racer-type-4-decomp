#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define RearViewMirrorOffsetY (*(unsigned short (*))&RearViewMirrorOffsetY)

#undef RearViewMirrorOffsetY

/* legacy typing of shared globals in this part of the unit */
#define TrackObjectParticleStates ((char *)TrackObjectParticleStates)

#undef TrackObjectParticleStates

/* legacy typing of shared globals in this part of the unit */
#define TrackObjectParticleStates ((O *)TrackObjectParticleStates)

#undef TrackObjectParticleStates

/* legacy typing of shared globals in this part of the unit */
#define SegmentOctantTable (*(s32 (*))&SegmentOctantTable)

#undef SegmentOctantTable

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
