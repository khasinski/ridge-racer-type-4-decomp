/* CC1_FLAGS: -fno-strength-reduce */

#include "rr/menu_transition.h"

/* legacy typing of shared globals in this part of the unit */
#define SetDispMask ((int (*)())SetDispMask)

#define IntroScriptStep ((int (*)())IntroScriptStep)

#define MenuArenas (*(int (*))&MenuArenas)

#define ModeGroup (*(short (*))&ModeGroup)

#undef SetDispMask

#undef IntroScriptStep

#undef MenuArenas

#undef ModeGroup

void UpdateMenuScript(void) {
    s32 next_frame;
    s32 mode;
    s32 status;
    s8 next_step;
    s8 opcode;
    s32 step;

    next_frame = (s32)((u32)ModeFrame + 1U);
    ModeFrame = next_frame;
    if (next_frame == 2) {
        SetDispMask(1);
    }
    if (RaceType == 4) {
        UpdateLinkHandshake(1);
    }
    mode=MenuDispMode;
    if (mode == 1) {
        status = OpeningScriptStep(ModeFrame);
    } else {
        if (mode == 0) {
            status = IntroScriptStep();
        } else {
            status = 1;
        }
    }
    step = MenuScriptStep;
    if (step >= 0) {
        if (MenuScriptOps[MenuScripts[MenuScriptId][step]]() == 1) {
            LoadState = 0;
            next_step = (u8) MenuScriptStep + 1;
            MenuScriptStep = next_step;
            opcode = MenuScripts[MenuScriptId][next_step];
            if (opcode < 0) {
                MenuScriptId = (u16) -opcode;
                MenuScriptStep = -1;
                if (MenuDispMode == 0) {
                    StartSceneFadeOut();
                }
            }
        }
    } else {
        if((status==1 && MenuDispMode==status) || (MenuDispMode==0 && status==2) || (MenuDispMode>=2 && status==1)) {
            MenuSceneEntries[MenuScriptId]();
            D_800F4D8A = 0;
            ModeGroup = (u16) ((u16) SceneId >> 8);
            ModeId = SceneId & 0xFF;
        }
    }
    if (ModeGroup != 1) {
        *(void **)0x1F800000 = AddTileGrid2x2((u32 *)((u8 *)CurrentDrawBuf + 0xB6C), *(void **)0x1F800000, 0, 0, 0x140, 0x200, 0, 0, 0);
    }
}
