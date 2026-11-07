#ifndef RR_FRAME_ENVIRONMENT_H
#define RR_FRAME_ENVIRONMENT_H
#include "rr/gpu.h"
#include "rr/runtime_settings.h"
typedef struct {
    RrGpuRectangleFields clip;
    s16 offset_x,offset_y;
    RrGpuRectangleFields window;
    u16 page;
    u8 dither,draw_display,rgb24,r,g,b;
    u32 draw_packet1C[16];
    RrGpuRectangleFields display,screen;
    u8 interlace,high_res,pad6E[2];
} RrFrameEnvironment;
/* Prefix shared by screen transitions and frame storage. The first rectangle
 * is also exposed by the screen-tile interface; ordering storage follows70. */
typedef struct {
    RrGpuRectangle rectangle;
    u8 pad08[0x68];
    u32 ordering70[704];
} RrFrameOrderingPrefix;
RR_ASSERT_SIZE(RrFrameOrderingPrefix,0xB70);
RR_ASSERT_OFFSET(RrFrameOrderingPrefix,ordering70,0x70);

/* Display environment is the 20-byte suffix at offset 0x5C. */
typedef struct {
    RrGpuRectangleFields display,screen;
    u8 interlace,high_res,pad[2];
} RrDisplayEnvironment;
extern RrFrameEnvironment SecondaryFrameEnv;
extern RrDisplayEnvironment SecondaryFrameDispEnv;
RR_ASSERT_SIZE(RrFrameEnvironment,0x70);
RR_ASSERT_OFFSET(RrFrameEnvironment,r,0x19);
RR_ASSERT_OFFSET(RrFrameEnvironment,draw_packet1C,0x1C);
RR_ASSERT_OFFSET(RrFrameEnvironment,display,0x5C);
RR_ASSERT_OFFSET(RrFrameEnvironment,screen,0x64);
RR_ASSERT_OFFSET(RrFrameEnvironment,high_res,0x6D);
RR_ASSERT_SIZE(RrDisplayEnvironment,0x14);
/* Both bank selectors are passed to this currently empty hook. */
void SetupDrawDispEnvsHookStub(s32 bank);
/* RGB uses the low eight bits; the low three flag bits select RGB24,
 * interlacing and a wide 480-pixel drawing area (320-pixel display width).
 * Settings halfwords 0x34/0x38 provide screen placement. */
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags);
#endif
