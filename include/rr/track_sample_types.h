#ifndef RR_TRACK_SAMPLE_TYPES_H
#define RR_TRACK_SAMPLE_TYPES_H
#include "rr/layout.h"
#include "rr/track_lane_types.h"

/* One side's two resource bytes. The wheel-contact path copies contactFlags;
 * the meaning of the second byte is not established. */
typedef struct {
    u8 contactFlags, unknown;
} TrackSurfaceSide2;

/* Centre-line and surface data, copied intact into the wheel scratchpad.
 * This subrecord starts at byte 8 of each 0x3C-byte track sample. Unknown
 * names retain their offsets in that enclosing on-disc record. */
typedef struct {
    s32 x, z;
    s16 height, heading;
    s16 unknown14, pitch;
    s16 leftWidth, rightWidth;
    s16 segmentLength, angle;
    s16 leftAngle, rightAngle;
    s32 distance;
    TrackSurfaceSide2 sides[2];
    s16 rightExtension, leftExtension;
    s16 unknown30[2];
    s16 unknown34[2];
    s16 unknown38[2];
} TrackRoadSample34;

/* Lateral offsets precede the common centre-line/surface subrecord. */
typedef struct {
    s16 rightOffsetX00, rightOffsetZ02;
    s16 leftOffsetX04, leftOffsetZ06;
    TrackRoadSample34 road;
} TrackSample3C;


extern TrackSample3C *TrackNodes;
extern s32 TrackNodeCount;

RR_ASSERT_SIZE(TrackSurfaceSide2,2);
RR_ASSERT_SIZE(TrackRoadSample34,0x34);
RR_ASSERT_OFFSET(TrackRoadSample34,x,0x00);
RR_ASSERT_OFFSET(TrackRoadSample34,z,0x04);
RR_ASSERT_OFFSET(TrackRoadSample34,height,0x08);
RR_ASSERT_OFFSET(TrackRoadSample34,heading,0x0A);
RR_ASSERT_OFFSET(TrackRoadSample34,pitch,0x0E);
RR_ASSERT_OFFSET(TrackRoadSample34,leftWidth,0x10);
RR_ASSERT_OFFSET(TrackRoadSample34,rightWidth,0x12);
RR_ASSERT_OFFSET(TrackRoadSample34,segmentLength,0x14);
RR_ASSERT_OFFSET(TrackRoadSample34,angle,0x16);
RR_ASSERT_OFFSET(TrackRoadSample34,leftAngle,0x18);
RR_ASSERT_OFFSET(TrackRoadSample34,rightAngle,0x1A);
RR_ASSERT_OFFSET(TrackRoadSample34,distance,0x1C);
RR_ASSERT_OFFSET(TrackRoadSample34,sides,0x20);
RR_ASSERT_OFFSET(TrackRoadSample34,rightExtension,0x24);
RR_ASSERT_OFFSET(TrackRoadSample34,leftExtension,0x26);
RR_ASSERT_OFFSET(TrackRoadSample34,unknown30,0x28);
RR_ASSERT_OFFSET(TrackRoadSample34,unknown34,0x2C);
RR_ASSERT_OFFSET(TrackRoadSample34,unknown38,0x30);
RR_ASSERT_SIZE(TrackSample3C,0x3C);
RR_ASSERT_OFFSET(TrackSample3C,road,0x08);
RR_ASSERT_OFFSET(TrackSample3C,leftOffsetX04,0x04);
#endif
