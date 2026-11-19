#ifndef RR_TRACK_LANE_TYPES_H
#define RR_TRACK_LANE_TYPES_H
#include "rr/layout.h"

/* Horizontal offsets of the selected driving line, one record per sample. */
typedef struct {
    s16 x, z;
    s32 reserved04;
} TrackLaneOffset8;

RR_ASSERT_SIZE(TrackLaneOffset8, 8);
RR_ASSERT_OFFSET(TrackLaneOffset8, z, 2);
#endif
