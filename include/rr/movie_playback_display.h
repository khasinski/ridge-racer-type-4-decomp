#ifndef RR_MOVIE_PLAYBACK_DISPLAY_H
#define RR_MOVIE_PLAYBACK_DISPLAY_H
#include "rr/gpu.h"
#include "rr/cd.h"

typedef struct {
    u32 word00, alternate04;
    u32 word08;
    volatile u32 ready0C;
    RrGpuRectangleFields rectangles[3];
} RrStrDisplayConfig;
typedef struct {u8 pad00[0x10];void *buffers[5];} RrStrBufferSet;
typedef struct {
    u8 pad00[0xB0];s32 resourceB0,phaseB4;void *activeB8;
    RrCdLocation locationBC,currentC0;s32 counterC4;
    u32 thresholdC8,budgetCC;s32 stopD0;
} RrStrContextPrefix;
extern s32 DrawBufIndex;
extern RrStrContextPrefix RaceContext;

/* Preserve the other configuration fields and store low16 coordinate bits. */
void movie_playback_set_dec_env(RrStrDisplayConfig *config,s32 x,s32 y,s32 second_x,s32 second_y);
/* Base covers the complete partitioned allocation through offset0x19A00.
 * Reload activeB8 for every publication: a destination slot may overlap it. */
void movie_playback_set_buffers(u8 *base);

/* Consume readiness; on timeout toggle the buffer and select its coordinates. */
/* The playback caller supplies mode0. This implementation ignores the mode. */
void movie_playback_sync(RrStrDisplayConfig *config,s32 mode);

RR_ASSERT_SIZE(RrStrDisplayConfig,0x28);
RR_ASSERT_OFFSET(RrStrDisplayConfig,ready0C,0x0C);
RR_ASSERT_OFFSET(RrStrDisplayConfig,rectangles,0x10);
RR_ASSERT_SIZE(RrStrBufferSet,0x24);
RR_ASSERT_OFFSET(RrStrBufferSet,buffers,0x10);
RR_ASSERT_SIZE(RrStrContextPrefix,0xD4);
RR_ASSERT_OFFSET(RrStrContextPrefix,phaseB4,0xB4);
RR_ASSERT_OFFSET(RrStrContextPrefix,currentC0,0xC0);
RR_ASSERT_OFFSET(RrStrContextPrefix,activeB8,0xB8);
#endif
