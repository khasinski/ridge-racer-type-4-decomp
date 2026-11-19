/* RODATA 0x80011270 0x2c */
#include "types.h"
#include "rr/frame_storage_types.h"
#include "rr/frame_loop.h"
#include "rr/load_cursor.h"
#include "scratchpad.h"
#include "rr/line_emit.h"
#include "game/effect/effect.h"
#include "game/effect/fxspr.h"
#include "game/gfx/fadesel.h"
#include "game/gfx/fadeset.h"
#include "game/gfx/fadespr.h"
#include "game/sound/sndvab.h"
#include "rr/driver_control.h"
#include "rr/bios_devices.h"
#include "rr/course_choices.h"
#include "rr/fade_sprite.h"
#include "rr/trig.h"
#include "rr/badge_rows.h"
#include "game/data/trkld1.h"



#include "game/demo/intro.h"
#include "rr/scene_camera_state.h"
#include "rr/cd_directory_buffer.h"
extern s32 IntroSceneWorkState;
extern s16 SubmodeState;

int IntroScriptStep(void);
void IntroInit(void);

int IntroScriptStep(void) {
    short sp28[4];
    unsigned char sp30[4];
    int i;
    int v;
    int cmd;
    int ret = 0;

    cmd = -1;
    for (i = 0; IntroTimeline[i].frame >= 0; i++) {
        if (IntroTimeline[i].frame == RaceContext.fade3E0.frame04) {
            cmd = IntroTimeline[i].command;
            break;
        }
    }

    switch (cmd) {
    case 0:
        RaceContext.fade3E0.particlesEnabled14 = 1;
        break;
    case 1:
        SetParticleBrightFade(0x20, 0x100, 0);
        break;
    case 2:
        RaceContext.fade3E0.spriteMode18 = 1;
        break;
    case 3:
        RaceContext.fade3E0.spriteMode18 = 2;
        RaceContext.fade3E0.spriteFrame1C = 0;
        break;
    case 4:
        RaceContext.fade3E0.opacityMode28 = 1;
        break;
    case 5:
        RaceContext.fade3E0.opacityMode28 = 2;
        break;
    case 6:
        RaceContext.fade3E0.linesEnabled30 = 1;
        break;
    case 7:
        RaceContext.fade3E0.sprite3Enabled38 = 1;
        break;
    case 8:
        RaceContext.fade3E0.sprite2Enabled34 = 1;
        RaceContext.fade3E0.mode3C = 1;
        RaceContext.fade3E0.cycleFrame44 = 0;
        break;
    case 9:
        RaceContext.fade3E0.rippleMode54 = 1;
        RaceContext.fade3E0.rippleFrame5C = 0;
        RaceContext.fade3E0.ripplePreset58 = 0;
        break;
    case 10:
        RaceContext.fade3E0.rippleMode54 = 2;
        RaceContext.fade3E0.rippleFrame5C = 0;
        break;
    }

    if (RaceContext.fade3E0.scaleMode48 == 1) {
        RaceContext.fade3E0.scale08 -= 0x10;
        if (RaceContext.fade3E0.scale08 <= 0x80) {
            RaceContext.fade3E0.scale08 = 0x80;
            RaceContext.fade3E0.scaleMode48 = 2;
        }
    }

    switch (RaceContext.fade3E0.rippleMode54) {
    case 1:
        RaceContext.fade3E0.rippleFrame5C++;
        RaceContext.fade3E0.rippleIntensity60 = (RaceContext.fade3E0.rippleFrame5C * 0xFF) / 16;
        if (RaceContext.fade3E0.rippleFrame5C >= 0x10) {
            RaceContext.fade3E0.rippleMode54 = 0;
            RaceContext.fade3E0.rippleIntensity60 = 0xFF;
        }
        break;
    case 2:
        RaceContext.fade3E0.rippleFrame5C++;
        RaceContext.fade3E0.rippleIntensity60 = ((0x10 - RaceContext.fade3E0.rippleFrame5C) * 0xFF) / 16;
        if (RaceContext.fade3E0.rippleFrame5C >= 0x10) {
            RaceContext.fade3E0.rippleMode54 = 0;
            RaceContext.fade3E0.rippleIntensity60 = 0;
        }
        break;
    }

    if (RaceContext.fade3E0.rippleIntensity60 > 0) {
        DrawIntroRippleSprites();
    }

    if (RaceContext.fade3E0.particlesEnabled14) {
        UpdateParticles();
    }

    switch (RaceContext.fade3E0.spriteMode18) {
    case 1:
        RaceContext.fade3E0.spriteIntensity24 += 8;
        if (RaceContext.fade3E0.spriteIntensity24 >= 0x100) {
            RaceContext.fade3E0.spriteIntensity24 = 0x100;
            RaceContext.fade3E0.spriteMode18 = 0;
        }
        break;
    case 2:
        RaceContext.fade3E0.spriteFrame1C++;
        if (RaceContext.fade3E0.spriteFrame1C >= 0x10) {
            RaceContext.fade3E0.spriteMode18 = 0;
            RaceContext.fade3E0.spriteFrame1C = 0x10;
            if (RaceContext.fade3E0.soundPlayed50 == 0) {
                PlaySfxBank2(1);
                RaceContext.fade3E0.soundPlayed50 = 1;
            }
        }
        RaceContext.fade3E0.spriteY20 = 0xCE - (RaceContext.fade3E0.spriteFrame1C * 0x42) / 16;
        break;
    case 3:
        RaceContext.fade3E0.spriteIntensity24 -= 8;
        if (RaceContext.fade3E0.spriteIntensity24 <= 0) {
            RaceContext.fade3E0.spriteIntensity24 = 0;
            RaceContext.fade3E0.spriteMode18 = 0;
        }
        break;
    }

    EmitFadeSprite(0, 0x2C, RaceContext.fade3E0.spriteY20, RaceContext.fade3E0.spriteIntensity24);

    if (RaceContext.fade3E0.linesEnabled30) {
        int c;
        RaceContext.fade3E0.linesProgress32++;
        if (RaceContext.fade3E0.linesProgress32 > 0x10) {
            RaceContext.fade3E0.linesProgress32 = 0x10;
        }
        c = 0xFF - RaceContext.fade3E0.opacity0C;
        c *= RaceContext.fade3E0.spriteIntensity24;
        if (c < 0) {
            c += 0xFF;
        }
        c >>= 8;
        c *= RaceContext.fade3E0.scale08;
        if (c < 0) {
            c += 0xFF;
        }
        c >>= 8;
        sp30[0] = sp30[1] = sp30[2] = c;
        sp28[0] = 0x10;
        sp28[2] = RaceContext.fade3E0.linesProgress32 * 18 + 0x10;
        for (i = 0; i < 2; i++) {
            sp28[1] = sp28[3] = i * 40 + 0x172;
            AddLineF2Prim(sp28, sp30, 0, 10);
            sp28[1] = sp28[3] = i * 40 + 0x173;
            AddLineF2Prim(sp28, sp30, 0, 10);
        }
    }

    if (RaceContext.fade3E0.sprite3Enabled38) {
        int v2;
        RaceContext.fade3E0.sprite3Progress3A++;
        if (RaceContext.fade3E0.sprite3Progress3A > 0x10) {
            RaceContext.fade3E0.sprite3Progress3A = 0x10;
            v2 = 0x100;
        } else {
            v2 = RaceContext.fade3E0.sprite3Progress3A * 16;
        }
        v2 = (v2 * RaceContext.fade3E0.spriteIntensity24) / 256;
        EmitFadeSprite(3, 0x42, 0x17C, v2);
    }

    if (RaceContext.fade3E0.sprite2Enabled34) {
        RaceContext.fade3E0.sprite2Progress36++;
        if (RaceContext.fade3E0.sprite2Progress36 > 0x10) {
            RaceContext.fade3E0.sprite2Progress36 = 0x10;
            v = 0x100;
        } else {
            v = RaceContext.fade3E0.sprite2Progress36 * 16;
        }
        v = (v * RaceContext.fade3E0.spriteIntensity24) / 256;
        EmitFadeSprite(2, 0x68, 0x13C, v);
        if (RaceContext.fade3E0.scaleMode48 == 0) {
            v = (rsin((FrameCounter << 7) & 0xF80) * v) / 4096;
        }
        EmitFadeSprite(1, 0x68, 0x13C, v);
    }

    switch (RaceContext.fade3E0.opacityMode28) {
    case 1:
        RaceContext.fade3E0.opacity0C += 8;
        if (RaceContext.fade3E0.opacity0C >= 0xFF) {
            RaceContext.fade3E0.opacity0C = 0xFF;
            RaceContext.fade3E0.opacityMode28 = 0;
            RaceContext.fade3E0.particlesEnabled14 = 0;
        }
        break;
    case 2:
        RaceContext.fade3E0.opacity0C -= 8;
        if (RaceContext.fade3E0.opacity0C <= 0) {
            RaceContext.fade3E0.opacity0C = 0;
            RaceContext.fade3E0.opacityMode28 = 0;
        }
        break;
    }

    UpdateFadeTint(RaceContext.fade3E0.opacity0C);
    *(void **)SCRATCHPAD = AddTileGrid2x2(&CurrentDrawBuf->buffer.ordering70.words[703], *(void **)SCRATCHPAD, 0, 0, 0x140, 0x200,
                                       RaceContext.fade3E0.color10[0], RaceContext.fade3E0.color10[1], RaceContext.fade3E0.color10[2]);
    for (i = 0; i < 2; i++) {
        DrawBuffers[i].environment.draw00.r = RaceContext.fade3E0.color10[0];
        DrawBuffers[i].environment.draw00.g = RaceContext.fade3E0.color10[1];
        DrawBuffers[i].environment.draw00.b = RaceContext.fade3E0.color10[2];
    }

    RaceContext.fade3E0.frame04++;
    if (RaceContext.fade3E0.frame04 > 3000) {
        RaceContext.fade3E0.frame04 = 3000;
    }

    if (RaceContext.fade3E0.mode3C == 1) {
        RaceContext.fade3E0.cycleFrame44++;
        if (RaceContext.fade3E0.cycleFrame44 >= 1000) {
            if (RaceContext.fade3E0.alternate00 == 0) {
                RaceContext.fade3E0.opacityMode28 = 2;
                RaceContext.fade3E0.alternate00 = 1;
                SetParticleBrightFade(0x20, 0, 0x100);
                RaceContext.fade3E0.particlesEnabled14 = 1;
            } else {
                RaceContext.fade3E0.opacityMode28 = 1;
                RaceContext.fade3E0.alternate00 = 0;
                SetParticleBrightFade(0x20, 0x100, 0);
            }
            RaceContext.fade3E0.cycleFrame44 = 0;
        }
        ret = 1;
    } else if (RaceContext.fade3E0.mode3C == 3) {
        RaceContext.fade3E0.closeFrame40++;
        if (RaceContext.fade3E0.closeFrame40 > 20) {
            RaceContext.fade3E0.closeFrame40 = 20;
            ret = 2;
        } else {
            ret = 1;
        }
    }

    return ret;
}

void IntroInit(void) {
    SetDispMask(0);
    PsyqBzero(&IntroSceneWorkState, 0x78);
    InitDemoFadeContext(0);
    SelectDemoFadePreset(0);
    LoadCursor = CdDirectoryBuffer;
    RaceLoadReset();
    ModeGroup = 2;
    ModeIndex = 6;
    SubmodeState = 0;
}
