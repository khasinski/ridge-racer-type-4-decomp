#ifndef RR_DEMO_FADE_TYPES_H
#define RR_DEMO_FADE_TYPES_H
#include "rr/layout.h"

typedef struct {s32 frame, command;} DemoFadeCommand8;
RR_ASSERT_SIZE(DemoFadeCommand8, 8);

/* Intro/demo fade state, cleared as one complete record by its constructor. */
typedef struct {
    s32 alternate00, frame04, scale08, opacity0C;
    u8 color10[3], reserved13;
    s32 particlesEnabled14, spriteMode18, spriteFrame1C;
    s32 spriteY20, spriteIntensity24, opacityMode28, reserved2C;
    s16 linesEnabled30, linesProgress32;
    s16 sprite2Enabled34, sprite2Progress36;
    s16 sprite3Enabled38, sprite3Progress3A;
    s32 mode3C, closeFrame40, cycleFrame44, scaleMode48;
    s32 sound4C, soundPlayed50;
    s32 rippleMode54, ripplePreset58, rippleFrame5C, rippleIntensity60;
} DemoFadeState64;
RR_ASSERT_SIZE(DemoFadeState64, 0x64);
RR_ASSERT_OFFSET(DemoFadeState64, color10, 0x10);
RR_ASSERT_OFFSET(DemoFadeState64, linesEnabled30, 0x30);
RR_ASSERT_OFFSET(DemoFadeState64, mode3C, 0x3C);
RR_ASSERT_OFFSET(DemoFadeState64, rippleIntensity60, 0x60);
#endif
