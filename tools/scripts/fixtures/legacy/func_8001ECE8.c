/* CC1_FLAGS: -fno-strength-reduce */
#include "rr/frame_loop.h"
extern void SetGeomOffset(s32,s32),SetGeomScreen(s32);
extern void *SetDefDrawEnv(void *,s32,s32,s32,s32),*SetDefDispEnv(void *,s32,s32,s32,s32);
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags) {
    /* ASM_BARRIER: keep flags in S0 until the arena replaces their dead value. */
    register u32 input asm("$16")=flags;
    /* ASM_BARRIER: retain red in the original saved register across environment helpers. */
    register s32 red asm("$23")=r;
    register s32 green;
    register u32 temp;
    /* ASM_BARRIER: retain interlace independently of height and offset. */
    register s32 interlace asm("$19");
    register u32 height;
    register u32 other_y;
    register s32 width;
    /* ASM_BARRIER: retain the wide-mode bit across environment helpers. */
    register u32 wide asm("$21");
    /* ASM_BARRIER: preserve flags until the RGB24 loop value is extracted. */
    register u32 saved_flags asm("$22");
    /* ASM_BARRIER: reuse S0 for the arena after the input flags have been consumed. */
    register RrFrameStorage *env asm("$16");
    RrGpuRectangleFields window;
    register s32 i;
    register u16 *settings;
    register s32 one;
    /* ASM_BARRIER: keep the loop wide-mode value in A3 after the helper calls. */
    register u32 loop_wide asm("$7");
    /* ASM_BARRIER: keep the loop RGB24 value in T0 after the helper calls. */
    register u32 rgb24 asm("$8");
    register s32 center_x;
    register u32 center_y;
    /* ASM_BARRIER: capture red and flags before assigning green and geometry temporaries. */
    __asm__("" : "+r"(red), "+r"(input) : : "$2", "$30");
    green=g;
    center_x=160;
    /* ASM_BARRIER: capture green and the X center before using V0 for geometry. */
    __asm__("" : "+r"(green), "+r"(center_x) : : "$2");
    temp=input>>1;
    /* ASM_BARRIER: preserve the intermediate before masking its low bit. */
    __asm__("" : "+r"(temp));
    interlace=temp&1;
    temp=interlace+1;
    height=temp<<8;
    center_y=height>>1;
    /* ASM_BARRIER: finish the Y center before reusing V0 for the opposite bank offset. */
    __asm__("" : "+r"(center_y) : : "$2");
    temp=1-interlace;
    other_y=temp<<8;
    SetGeomOffset(center_x,center_y);
    SetGeomScreen(290);
    width=320;
    wide=input&4;
    saved_flags=input;
    if(wide)width=480;
    env=FrameEnvs;
    SetDefDrawEnv(env,0,0,width,height);
    SetDefDrawEnv(&SecondaryFrameEnv,0,other_y,width,height);
    SetDefDispEnv(&env->environment.display,0,other_y,width,height);
    SetDefDispEnv(&D_800D02C4,0,0,width,height);
    i=0;
    rgb24=saved_flags&1;
    one=1;
    settings=Options.halves;
    loop_wide=wide;
    window.y=0;window.x=0;window.h=0;window.w=0;
loop:
    {
        env->environment.r=red;env->environment.g=green;env->environment.b=b;
        env->environment.rgb24=rgb24;
        env->environment.dither=one;
        env->environment.window=window;
        env->environment.screen.x=settings[0x34/2];
        env->environment.screen.y=settings[0x38/2];
        env->environment.display.h=height;
        env->environment.interlace=interlace;
        env->environment.screen.h=256;
        if(loop_wide) {
            env->environment.high_res=one;
            env->environment.display.w=(env->environment.display.w*2)/3;
        }
        env++;
    }
    i++;
    if(i<2)goto loop;
    SetupDrawDispEnvsHookStub(0);
    SetupDrawDispEnvsHookStub(1);
}
