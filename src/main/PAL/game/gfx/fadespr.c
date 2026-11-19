#include "game/ui/badgeprim.h"
#include "scratchpad.h"
#include "rr/frame_storage_types.h"
#include "rr/fade_sprite.h"
#include "game/effect/ptinit.h"
#include "game/sound/cdmain.h"
#include "psyq/libsnd/ssmvol.h"
#include "game/camera/campath.h"
#include "game/gfx/fadespr.h"
#include "rr/audio_state_types.h"
#include "rr/bios_devices.h"
#include "rr/library.h"
#include "psyq/libsnd/vmcfg.h"

struct OptionsDefaultTail16 { int a[4]; };

typedef struct {
    short s00, s02;
    s32 unk04[3];
    short s10, s12, s14, s16;
    s32 unk18[4];
    char b28, b29, b2A, b2B;
    s32 unk2C;
    char b30, b31;
    short h32;
    int w34, w38;
} OptionsState3C;

extern OptionsState3C Options;
extern struct OptionsDefaultTail16 OptionsDefaultTailData;
extern unsigned char HudPauseMenuTitleY[];
#include "rr/scene_camera_state.h"

void EmitFadeSprite(s32 index, s32 x, s32 y, s32 intensity);

void InitOptionsDefaults(void) {
    int i;
    char *b;
    char *q;

    i = 0;
    b = (char *)&Options;
    q = b;
    do {
        ((OptionsState3C *)(b + i * 4))->s00 = 0;
        ((OptionsState3C *)(b + i * 4))->s02 = 0;
        ((OptionsState3C *)(q + i * 16))->s10 = 1;
        ((OptionsState3C *)(q + i * 16))->s12 = 1;
        ((OptionsState3C *)(q + i * 16))->s14 = 0;
        ((OptionsState3C *)(b + i * 4))->b28 = 1;
        ((OptionsState3C *)(b + i * 4))->b29 = 0;
        ((OptionsState3C *)(b + i * 4))->b2A = 2;
        ((OptionsState3C *)(b + i * 4))->b2B = 0;
        i++;
    } while (i < 2);
    Options.b30 = 0x10;
    Options.b31 = 0x10;
    Options.h32 = 0;
    Options.w34 = 0;
    Options.w38 = 0x10;
}

void InitDemoFadeContext(int sound) {
    struct OptionsDefaultTail16 bankOrder;
    int i;
    s16 t;
    bankOrder = OptionsDefaultTailData;
    PsyqBzero(&RaceContext.fade3E0, sizeof(RaceContext.fade3E0));
    RaceContext.fade3E0.spriteY20 = 0xCE;
    RaceContext.fade3E0.sound4C = sound;
    RaceContext.fade3E0.scale08 = 0x100;
    RaceContext.sceneStateB0 = 0;
    InitParticles();
    i = 0;
    if (SoundState.seqHandle2864 >= 0) {
        SsSeqStop(*(short *)&SoundState.seqHandle2864);
        SsSeqClose(*(short *)&SoundState.seqHandle2864);
        SoundState.seqHandle2864 = -1;
    }
    for (i = 0; i < 4; i++) {
        t = SoundState.banks24.ids[bankOrder.a[i]];
        if (t >= 0) {
            SsVabClose(t);
            SoundState.banks24.ids[bankOrder.a[i]] = -1;
        }
    }
    ResetSeqFadeState();
    SoundState.seqTrack2868 = -1;
}

void UpdateFadeTint(int intensity) {
    int redScaled, greenScaled, blueScaled;
    RaceContext.fade3E0.color10[0] = (HudPauseMenuTitleY[0] * intensity) / 255;
    RaceContext.fade3E0.color10[1] = (HudPauseMenuTitleY[1] * intensity) / 255;
    redScaled = RaceContext.fade3E0.color10[0] * RaceContext.fade3E0.scale08;
    RaceContext.fade3E0.color10[2] = (HudPauseMenuTitleY[2] * intensity) / 255;
    if (redScaled < 0) redScaled += 0xFF;
    greenScaled = RaceContext.fade3E0.color10[1] * RaceContext.fade3E0.scale08;
    RaceContext.fade3E0.color10[0] = redScaled >> 8;
    if (greenScaled < 0) greenScaled += 0xFF;
    blueScaled = RaceContext.fade3E0.color10[2] * RaceContext.fade3E0.scale08;
    RaceContext.fade3E0.color10[1] = greenScaled >> 8;
    if (blueScaled < 0) blueScaled += 0xFF;
    RaceContext.fade3E0.color10[2] = blueScaled >> 8;
}

void EmitFadeSprite(s32 index, s32 x, s32 y, s32 intensity) {
    s32 phase;
    s32 selector;
    s32 scale;
    GpuScratchPointers *scratch;
    GpuCommandPacket *p;

    selector = (index == 4);
    phase = FadeSpritePhaseAngle;
    if (phase >= 128) {
        selector += 2;
        scale = phase - 127;
    } else {
        scale = 128U - (u32)phase;
    }
    scale = (s32)((u32)scale * intensity) / 256;
    scale = (s32)((u32)scale * FadeSpriteIntensityScale) / 256;
    if (scale > 0) {
        u32 command;
        u32 low;
        u32 high;
        u32 mode;
        u32 tag;
        scratch = (GpuScratchPointers *)SCRATCHPAD;
        p = (GpuCommandPacket *)scratch->cursor;
        p = (GpuCommandPacket *)AddSpritePrim(
            &CurrentDrawBuf->buffer.ordering70.words[10],
            (SpritePacket *)p,
            x, y, &HudCourseMapScaleTable[index], 0x7F38,
            (scale * 2U) | 1U);
        command = 0xE1000200U;
        low = 0xFFFFFFU;
        mode = FadeSpriteDrawModes[selector];
        /* ASM_BARRIER: capture the mode word before publishing the packet. */
        __asm__("" : "+r"(mode) : "r"(low));
        scratch->cursor = (u8 *)p;
        p->tag.bytes.length = 1;
        tag = p->tag.word;
        /* ASM_BARRIER: capture the tag before storing the command word. */
        __asm__("" : "+r"(tag), "+r"(mode));
        high = 0xFF000000U;
        p->command = (mode & 0x9FF) | command;
        p->tag.word = (tag & high) | (CurrentDrawBuf->buffer.ordering70.words[10] & low);
        CurrentDrawBuf->buffer.ordering70.words[10] = (CurrentDrawBuf->buffer.ordering70.words[10] & high) | ((unsigned long)p & low);
        scratch->cursor = (u8 *)(p + 1);
    }
}
