#include "rr/overlay661_race_controller.h"
void ovl661_func_80114B00(void) {
    s32 var_a2;
    s32 var_a2_2;
    s16 temp_a0;
    s16 temp_a1_3;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_a2;
    s32 temp_s0_2;
    s32 temp_s0_4;
    register s32 temp_s1 __asm__("$17");
    s32 temp_v0_3;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_s2;
    register s32 var_v1 __asm__("$3");
    s32 var_v1_2;
    register s32 var_v1_3 __asm__("$3");
    s32 var_v1_4;
    s8 temp_v1;
    RrRaceVehicle *temp_s0;
    RrRaceVehicle *temp_s0_3;
    Rr661FrameVector16 *temp_v0;
    Rr661FrameVector16 *temp_v0_2;
    Rr661FrameVector16 *temp_v0_4;
    Rr661FrameVector16 *temp_v0_5;
    Rr661FrameVector16 *temp_v1_2;
    Rr661FrameVector16 *temp_v1_3;
    Rr661FrameVector16 *temp_v1_4;
    Rr661FrameVector16 *temp_v1_5;

    ModeFrame=(u32)ModeFrame+1;
    if (((u32) ((u32)RaceState - 1) < 2U) && (PlayerControllers[0].buttons08 & 0x800)) {
        if (MenuFadeTimer <= 0) {
            MenuFadeTimer = 0xF;
            temp_v1 = RacePaused == 0;
            RacePaused = temp_v1;
            if (temp_v1 != 0) {
                SoundPause();
                PauseCursor = 0;
            } else {
                ApplyPauseMenuChoice(RaceType);
                SoundResume();
            }
        } else {
            goto block_7;
        }
    } else if (MenuFadeTimer > 0) {
block_7:
        if (RaceState < 4) {
            MenuFadeTimer = (u16) MenuFadeTimer - 1;
        }
    }
    if (RacePaused != 0) {
        UpdatePauseMenu(RaceType);
        ModeFrame=(u32)ModeFrame-1;
        DrawPauseMenu(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, PauseCursor);
        if (RaceState < 4) {
            DrawStartCountdown(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, ModeFrame);
        }
        var_s2 = 0;
        if ((u32) ((u32)RaceState - 1) < 3U) {
            do {
                
                temp_s0 = &PlayerCars[var_s2];
                DrawHudNumber3((temp_s0->input * 0xA0) / 1168, var_s2);
                DrawHudNumber5(*(s16 *)((u8 *)&EngineRevDisplay+var_s2*2), var_s2);
                DrawHudDigitMod10(temp_s0->half27A, var_s2);
                UpdateHudRankDisplay(temp_s0->half1EE, var_s2);
                {register s32 playerArg __asm__("$5")=var_s2;s32 budget=RaceTimeRemaining;__asm__("" : : "r"(budget));{register s32 nextPlayer __asm__("$2")=playerArg+1;__asm__("" : : "r"(nextPlayer));var_s2=nextPlayer;DrawTimeRemainingForPlayer(budget,playerArg);}}
                DrawLapCounter(var_s2, temp_s0->steering288.count22);
                DrawRaceMinimap(var_s2);
                DrawSteeringIndicator(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, var_s2);
                UpdateFinalLapIndicator(var_s2);
            } while (var_s2 < 2);
            DrawSplitScreenDivider(0);
            DrawHudFrameSprites(2);
            SubmitRaceHudPackets(2);
        }
        var_s2 = 0;
        do {
            var_v1 = var_s2 * 0x10;
            __asm__("" :: "r"(var_v1));
            temp_v0 = (Rr661FrameVector16 *)((u8 *)D_801183D0+var_v1);
            temp_v1_2 = (Rr661FrameVector16 *)((u8 *)D_801183F0+var_v1);
__asm__("" : : "r"(temp_v0),"r"(temp_v1_2));
        RR_661_FRAME_SCRATCH->rotation18 = *temp_v0;
        RR_661_FRAME_SCRATCH->position08 = *temp_v1_2;
            __asm__("" : "+r"(var_s2));
        SetupViewport(var_s2 + 2);
            if (RaceState == 0) {
                var_a0 = var_s2 * 2;
                if ((u32) ModeFrame < 0x4BU) {
                    UpdateRaceIntroCamera(&PlayerCars[var_s2], ModeFrame);
                    var_v1_2 = var_s2 * 0x10;
                } else {
                    goto block_22;
                }
            } else {
                var_a0 = var_s2 * 2;
block_22:
                UpdateRaceChaseCamera((struct w3g2p *)((unsigned long)((((var_a0+var_s2)*8)+var_s2)<<5)+(unsigned long)PlayerCars));
                var_v1_2 = var_s2 * 0x10;
            }
            temp_v0_2 = (Rr661FrameVector16 *)((u8 *)D_801183D0+var_v1_2);
            temp_v1_3 = (Rr661FrameVector16 *)((u8 *)D_801183F0+var_v1_2);
__asm__("" : : "r"(temp_v0_2),"r"(temp_v1_3));
        *temp_v0_2 = RR_661_FRAME_SCRATCH->rotation18;
        *temp_v1_3 = RR_661_FRAME_SCRATCH->position08;
            temp_s0_2 = var_s2 + 1;
            BuildCameraMatrices();
            DrawCars(var_s2 + 2);
            SetupPlayerView(var_s2);
            DrawTrackObjects(0);
            DrawTrackSceneryForPlayer(temp_s0_2);
            var_s2 = temp_s0_2;
        } while (var_s2 < 2);
        return;
    }
    ScreenFrame=(u32)ScreenFrame+1;
    if ((u32) ((u32)RaceState - 2) < 2U) {
        temp_v0_3 = (s32)((u32)RaceTimeRemaining - 1);
        RaceTimeRemaining = temp_v0_3;
        if (temp_v0_3 <= 0) {
            RaceState = 5;
            MenuFadeTimer = 0x4B;
            SoundSetMasterVolume(0,0x21);
        }
    }
    if (((u32) ModeFrame >= 0x4BU) && (RaceState == 0)) {
        RaceState = 1;
        SlotsStartForRace();
        SetModelTexturePage(-1);
        ResetCarSlotBuffers();
        FrameEnvs[1].environment.rgb24 = 0;
        FrameEnvs[0].environment.rgb24 = 0;
    } else {
        if (RaceState==1) {
            if ((u32) ModeFrame >= 0xB0U) {
                ApplyRaceStartLaunch(&PlayerCars[0]);
                ApplyRaceStartLaunch(&PlayerCars[1]);
                CdAudioBeginPlay(1);
                RaceState = 2;
            }
        } else if (RaceState>=4) {
            var_a2 = 0x25;
            if (TotalRaceTime == -1) {
                var_a2 = 0x45;
            }
            if (MenuFadeTimer < 0x19) {
                AddScreenFadeTiles(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, (0x19 - MenuFadeTimer) * 0xA, var_a2);
                RR_661_FRAME_SCRATCH->pointers.cursor = AddDrawAreaPrim(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, RR_661_FRAME_SCRATCH->pointers.cursor, 0, 0, 0x140, 0x100);
            }
            if (MenuFadeTimer <= 0) {
                ovl661_D_80118448[0] = (u16) PlayerCars[0].half1EE;
                ovl661_D_80118448[1] = (u16) PlayerCars[1].half1EE;
                if (RaceState == 4) {
                    ModeGroup = 1;
                    ModeId = 0x15;
                    ModeSubState = 0;
                } else {
                    FinishRace();
                }
            } else {
                MenuFadeTimer = (u16) MenuFadeTimer - 1;
            }
        } else if (RaceState==0) {
            if ((u32)ModeFrame >= 51U) {
                AddScreenFadeTiles(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70,((u32)ModeFrame-50U)*10U,37);
            } else if ((u32)ModeFrame < 25U) {
                AddScreenFadeTiles(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70,(25U-(u32)ModeFrame)*10U,69);
            }
            RR_661_FRAME_SCRATCH->pointers.cursor = AddDrawAreaPrim(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, RR_661_FRAME_SCRATCH->pointers.cursor, 0, 0, 0x140, 0x100);
        }
    }
    ovl661_func_80114924((RrRaceCheckpointVehicle *)(void *)&PlayerCars[0]);
    ovl661_func_80114924((RrRaceCheckpointVehicle *)(void *)&PlayerCars[1]);
    UpdateRaceRanking();
    if ((s16) PlayerCars[0].half1EE < (s16) PlayerCars[1].half1EE) {
        PlayerCars[0].half1EE = 0U;
        PlayerCars[1].half1EE = 1U;
    } else {
        PlayerCars[0].half1EE = 1U;
        PlayerCars[1].half1EE = 0U;
    }
    StepRaceFrame(RaceType);
    if (RaceState < 4) {
        DrawStartCountdown(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, ModeFrame);
        PlayStartCountdownVoice(ModeFrame);
    }
    var_s2 = 0;
    if ((u32) ((u32)RaceState - 1) < 3U) {
        do {
            
                temp_s0_3 = &PlayerCars[var_s2];
            DrawHudNumber3((temp_s0_3->input * 0xA0) / 1168, var_s2);
            DrawHudNumber5(*(s16 *)((u8 *)&EngineRevDisplay+var_s2*2), var_s2);
            DrawHudDigitMod10(temp_s0_3->half27A, var_s2);
            UpdateHudRankDisplay(temp_s0_3->half1EE, var_s2);
            {register s32 playerArg __asm__("$5")=var_s2;s32 budget=RaceTimeRemaining;__asm__("" : : "r"(budget));{register s32 nextPlayer __asm__("$2")=playerArg+1;__asm__("" : : "r"(nextPlayer));var_s2=nextPlayer;DrawTimeRemainingForPlayer(budget,playerArg);}}
            DrawLapCounter(var_s2, temp_s0_3->steering288.count22);
            DrawRaceMinimap(var_s2);
            DrawSteeringIndicator(((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70, var_s2);
            UpdateFinalLapIndicator(var_s2);
        } while (var_s2 < 2);
        DrawSplitScreenDivider(0);
        DrawHudFrameSprites(2);
        SubmitRaceHudPackets(2);
    }
    var_s2 = 0;
    do {
        var_v1_3 = var_s2 * 0x10;
        __asm__("" :: "r"(var_v1_3));
        temp_v0_4 = (Rr661FrameVector16 *)((u8 *)D_801183D0+var_v1_3);
        temp_v1_4 = (Rr661FrameVector16 *)((u8 *)D_801183F0+var_v1_3);
__asm__("" : : "r"(temp_v0_4),"r"(temp_v1_4));
        RR_661_FRAME_SCRATCH->rotation18 = *temp_v0_4;
        RR_661_FRAME_SCRATCH->position08 = *temp_v1_4;
        __asm__("" : "+r"(var_s2));
        SetupViewport(var_s2 + 2);
        if (RaceState == 0) {
            var_a0_2 = var_s2 * 2;
            if ((u32) ModeFrame < 0x4BU) {
                UpdateRaceIntroCamera(&PlayerCars[var_s2], ModeFrame);
                var_v1_4 = var_s2 * 0x10;
            } else {
                goto block_66;
            }
        } else {
            var_a0_2 = var_s2 * 2;
block_66:
            UpdateRaceChaseCamera((struct w3g2p *)((unsigned long)((((var_a0_2+var_s2)*8)+var_s2)<<5)+(unsigned long)PlayerCars));
            var_v1_4 = var_s2 * 0x10;
        }
        temp_v0_5 = (Rr661FrameVector16 *)((u8 *)D_801183D0+var_v1_4);
        temp_v1_5 = (Rr661FrameVector16 *)((u8 *)D_801183F0+var_v1_4);
__asm__("" : : "r"(temp_v0_5),"r"(temp_v1_5));
        *temp_v0_5 = RR_661_FRAME_SCRATCH->rotation18;
        *temp_v1_5 = RR_661_FRAME_SCRATCH->position08;
        temp_s1 = var_s2 * 2;
        BuildCameraMatrices();
        __asm__("" : : "r"(temp_s1));
        DrawCars(var_s2 + 2);
        if (((RrRaceVehicle *)((unsigned long)((((temp_s1+var_s2)*8)+var_s2)<<5)+(unsigned long)PlayerCars))->half1EA != 0) {
            DrawWrongWaySprite(var_s2 + 1);
        }
        RR_661_FRAME_SCRATCH->mode5E = 1;
        SetupPlayerView(var_s2);
        UpdateTrackObjects(0);
        DrawTrackObjects(0);
        temp_s0_4 = var_s2 + 1;
        DrawTrackSceneryForPlayer(temp_s0_4);
        {register s32 playerArg2 __asm__("$6")=var_s2;
        temp_a0 = *(s16 *)((u8 *)&EngineRevDisplay+temp_s1);
        temp_a1_3 = *(s16 *)((u8 *)&EngineThrottleOn+temp_s1);
        var_s2 = temp_s0_4;
        SlotSetLevelProgram(temp_a0,temp_a1_3,playerArg2);}
        __asm__("" : : "r"(temp_s1));
    } while (var_s2 < 2);
    if (RaceState >= 4) {
        StopRaceRumble();
        if (RaceState == 6) {
            StopRaceAudioAndRumble();
        }
    }
}
