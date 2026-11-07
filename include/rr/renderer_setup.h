#ifndef RR_RENDERER_SETUP_H
#define RR_RENDERER_SETUP_H
#include "rr/gpu.h"

/* RGB bytes and command byte shared with the handwritten polygon emitters. */
typedef struct {u8 r,g,b,code;} RrGpuColorCommand;
typedef struct {
    u8 pad00[0x60];
    s32 depthShift60,renderFlags64;
    u32 reserved68;
    RrGpuRectangle viewport6C;
} RrRendererScratch;
extern s32 MirrorMode;
extern RrGpuColorCommand DefaultFT4Color,PolyGT4WhiteColor;

/* Captures the raw depth shift and current render flags in scratchpad; resets
 * the two RGB/command templates and the viewport to (0,0,320,256). */
void SetupRenderer(s32 shift);
RR_ASSERT_SIZE(RrGpuColorCommand, 4);
RR_ASSERT_OFFSET(RrGpuColorCommand, code, 3);
RR_ASSERT_SIZE(RrRendererScratch, 0x74);
RR_ASSERT_OFFSET(RrRendererScratch, depthShift60, 0x60);
RR_ASSERT_OFFSET(RrRendererScratch, renderFlags64, 0x64);
RR_ASSERT_OFFSET(RrRendererScratch, reserved68, 0x68);
RR_ASSERT_OFFSET(RrRendererScratch, viewport6C, 0x6C);
#endif
