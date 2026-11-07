#include "rr/overlay660.h"

void ovl660_func_80117084(RrRaceVehicleViews *player){
 RrRaceVehicleControl *control=&player->parts.control;
 if((u32)RaceState-2U<2){
  s32 allow=0;
  if(control->steering94.count22<=0)allow=player->vehicle.half1EA!=ReverseCourse;
  else if(player->vehicle.first<=(s32)(0U-(u32)TrackLength))allow=1;
  if(allow==1){RaceState=5;MenuFadeTimer=75;SoundSetMasterVolume(0,71);}
 }
}

void ovl660_func_8011712C(void)
{
    StopRaceAudioAndRumble();
    SetupRaceAndScene();
    ModeGroup = 1;
    ModeId = 53;
    D_800F4D8A = 0;
    AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70, 255, 69);
}

void ovl660_func_8011718C(void) {
    s32 fadeFlags;
    s8 paused;

    ModeFrame = (s32)((u32)ModeFrame + 1U);
    if ((((u32)RaceState - 1U) < 2U) && (PlayerControllers[0].buttons08 & 0x800)) {
        if (MenuFadeTimer <= 0) {
            MenuFadeTimer = 0xF;
            paused = RacePaused == 0;
            RacePaused = paused;
            if (paused != 0) {
                SoundPause();
                PauseCursor = 0;
            } else {
                ApplyPauseMenuChoice(RaceType);
                SoundResume();
            }
        } else {
            goto update_pause_timer;
        }
    } else if (MenuFadeTimer > 0) {
update_pause_timer:
        if (RaceState < 4) {
            MenuFadeTimer = (u16) MenuFadeTimer - 1;
        }
    }
    if (RacePaused != 0) {
        UpdatePauseMenu(RaceType);
        ModeFrame = (s32)((u32)ModeFrame - 1U);
        DrawPauseMenu(CurrentDrawBuf->ordering.ordering70, PauseCursor);
        func_8003BFC8(1);
        DrawLapTimeList(0);
        DrawStartCountdown(CurrentDrawBuf->ordering.ordering70, ModeFrame);
        DrawHudRotatingQuad(EngineRevDisplay.first, 0);
        func_800211A4(0);
        DrawHudNumber3((PlayerCars[0].input * 0xA0) / 1168, 0);
        DrawHudDigitMod10(PlayerCars[0].half27A, 0);
        DrawRaceMinimap(0);
        DrawSteeringIndicator(CurrentDrawBuf->ordering.ordering70, 0);
        SubmitRaceHudPackets(RaceType);
        BuildCameraMatrices();
        SetupViewport(0);
        DrawCars(0);
        RR_660_FRAME_SCRATCH->word78 = D_800F2A64;
        SetupPlayerView(0);
        DrawTrackObjects(0);
        DrawTrackSceneryForPlayer(0);
        func_80070734(ScreenFrame);
        return;
    }
    ScreenFrame = (s32)((u32)ScreenFrame + 1U);
    if ((u32)ModeFrame >= 75U && RaceState == 0) {
        SlotsStartForRace();
        RaceState=1;
        ResetCarSlotBuffers();
        SetModelTexturePage(-1);
    } else if (RaceState == 1) {
            if ((u32) ModeFrame >= 0xB0U) {
                func_80022CD4(PlayerCars);
                RaceState = 2;
                CdAudioBeginPlay(1);
            }
    } else if (RaceState >= 4) {
            if (MenuFadeTimer < 0x19) {
                fadeFlags = 0x45;
                if (RaceState == 4) {
                    fadeFlags = 0x25;
                }
                AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x19 - MenuFadeTimer) * 0xA, fadeFlags);
            }
            if (MenuFadeTimer <= 0) {
                if (RaceState == 4) {
                    ModeGroup = 1;
                    ModeId = 0x15;
                    D_800F4D8A = 0;
                } else {
                    FinishRace();
                }
            } else {
                MenuFadeTimer = (u16) MenuFadeTimer - 1;
            }
    } else if (RaceState == 0) {
            if ((u32) ModeFrame >= 0x33U) {
                AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (ModeFrame - 0x32) * 0xA, 0x25);
                FrameEnvs[1].environment.rgb24 = 0;
                FrameEnvs[0].environment.rgb24 = 0;
            } else if ((u32) ModeFrame < 0x19U) {
                AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x19 - ModeFrame) * 0xA, 0x45);
                FrameEnvs[1].environment.rgb24 = 1;
                FrameEnvs[0].environment.rgb24 = 1;
            }
            RR_660_FRAME_SCRATCH->pointers.cursor = AddDrawAreaPrim(CurrentDrawBuf->ordering.ordering70, RR_660_FRAME_SCRATCH->pointers.cursor, 0, 0, 0x140, 0x100);
    }
    UpdateLapTiming(PlayerCars);
    func_8003BFC8(0);
    ovl660_func_80117084((RrRaceVehicleViews *)(void *)PlayerCars);
    UpdateRaceRanking();
    if (RaceState < 4) {
        DrawStartCountdown(CurrentDrawBuf->ordering.ordering70, ModeFrame);
        PlayStartCountdownVoice(ModeFrame);
    }
    CheckFinalLapEvent((char *)(void *)PlayerCars);
    if (((u32)RaceState - 1U) < 3U) {
        DrawLapTimeList(0);
        DrawHudRotatingQuad(EngineRevDisplay.first, 0);
        func_800211A4(0);
        DrawHudNumber3((PlayerCars[0].input * 0xA0) / 1168, 0);
        DrawHudDigitMod10(PlayerCars[0].half27A, 0);
        DrawRaceMinimap(0);
        DrawSteeringIndicator(CurrentDrawBuf->ordering.ordering70, 0);
        SubmitRaceHudPackets(RaceType);
    }
    StepRaceFrame(RaceType);
    UpdateTrackObjects(0);
    if (RaceState == 0) {
        if ((u32) ModeFrame < 0x4BU) {
            UpdateRaceIntroCamera(PlayerCars, ModeFrame);
        } else {
            goto draw_vehicle;
        }
    } else {
draw_vehicle:
        UpdateRaceChaseCamera((struct w3g2p *)(void *)PlayerCars);
    }
    BuildCameraMatrices();
    DrawCars(0);
    if (PlayerCars[0].half1EA != ReverseCourse) {
        if (RaceState < 4) {
            func_800225DC(0);
        }
    }
    RR_660_FRAME_SCRATCH->word78 = (s32) D_800F2A64;
    SetupPlayerView(0);
    DrawTrackObjects(0);
    DrawTrackSceneryForPlayer(0);
    SlotSetLevelProgram(EngineRevDisplay.first, D_800F2AB4.first, 0);
    if (RaceState >= 4) {
        StopRaceRumble();
        if (RaceState == 6) {
            StopRaceAudioAndRumble();
        }
    }
}
