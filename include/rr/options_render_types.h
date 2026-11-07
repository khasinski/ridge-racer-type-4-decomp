#ifndef RR_OPTIONS_RENDER_TYPES_H
#define RR_OPTIONS_RENDER_TYPES_H
#include "rr/matrix_workspace.h"
#include "rr/projection_state.h"
#include "rr/renderer_setup.h"

/* Geometry and renderer state occupy the same scratch tail at different
 * stages. These are actual scratchpad records, including the full pointer
 * slots used by the mesh dispatcher. */
typedef struct {
    s16 relative00[3],pad06;
    s32 transformed08[3];
    u8 pad14[4];s32 light18[3];u8 pad24[8];s32 saved2C[3];
} RrPreviewProjectionFields38;
typedef union {
    RrPreviewProjectionFields38 fields;
    s16 halves[28];
    s32 words[14];
} RrPreviewProjection38;
typedef union {
    RrPreviewProjection38 projection;
    struct {u32 blend;RrGpuColorCommand first,second;u8 tail0C[44];} render;
} RrPreviewScratchTail;
typedef struct {
    RrProjectionScratchPrefix projection;
    u32 reserved24;
    RrPreviewMatrixWorkspace matrix28;
    u32 reserved48,reserved4C;
    const u32 *const *commands50;
    s32 count54,format58,reserved5C,depth60,flags64;
    u32 reserved68;RrGpuRectangle viewport6C;
    const u32 *primitive74;
    s32 counter78;
    RrPreviewScratchTail tail7C;
} RrPreviewRenderScratch;
RR_ASSERT_SIZE(RrPreviewProjection38,0x38);
RR_ASSERT_OFFSET(RrPreviewProjectionFields38,saved2C,0x2C);
RR_ASSERT_SIZE(RrPreviewRenderScratch,0xB4);
RR_ASSERT_OFFSET(RrPreviewRenderScratch,matrix28,0x28);
RR_ASSERT_OFFSET(RrPreviewRenderScratch,commands50,0x50);
RR_ASSERT_OFFSET(RrPreviewRenderScratch,depth60,0x60);
RR_ASSERT_OFFSET(RrPreviewRenderScratch,primitive74,0x74);
RR_ASSERT_OFFSET(RrPreviewRenderScratch,tail7C,0x7C);
#endif
