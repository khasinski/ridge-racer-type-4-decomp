#include "rr/overlay659.h"

void ovl659_exit_race(void) {
    SetupRaceAndScene();
    ModeGroup = 1;
    ModeId = 0x2F;
    D_800F4D8A = 0;
}

void ovl659_update_race(void) {
    ModeFrame = (u32)ModeFrame + 1;
    SetupViewport(0);
    if (((u32)RaceState - 1) < 2U && (PlayerControllers[0].buttons08 & 0x800) && MenuFadeTimer <= 0) {
        MenuFadeTimer = 15;
        RacePaused = (RacePaused == 0);
        if (RacePaused != 0) {
            SoundPause();
            PauseCursor = 0;
        } else {
            ApplyPauseMenuChoice(RaceType);
            SoundResume();
        }
    } else if (MenuFadeTimer > 0) {
        if (RaceState < 4) {
            MenuFadeTimer--;
        }
    }

    if (RacePaused != 0) {
        UpdatePauseMenu(RaceType);
        ModeFrame = (u32)ModeFrame - 1;
        func_8003BFC8();
        DrawPauseMenu(&CurrentDrawBuf->ordering.ordering70[0], PauseCursor);
        DrawLapTimeList(0);
        DrawTimeRemaining(RaceTimeRemaining);
        DrawStartCountdown(&CurrentDrawBuf->ordering.ordering70[0], ModeFrame);
        DrawHudRotatingQuad(EngineRevDisplay.first, 0);
        func_800211A4(0);
        DrawHudNumber3((PlayerCars[0].input * 160) / 1168, 0);
        DrawHudDigitMod10(PlayerCars[0].half27A, 0);
        UpdateHudRankDisplay(PlayerCars[0].half1EE, 0);
        DrawRaceMinimap(0);
        DrawSteeringIndicator(&CurrentDrawBuf->ordering.ordering70[0], 0);
        func_80021C00(0);
        SubmitRaceHudPackets(RaceType);
        BuildCameraMatrices();
        DrawCars(0);
        RR_659_SCRATCH->word78 = D_800F2A64;
        SetupPlayerView(0);
        DrawTrackObjects(0);
        DrawTrackSceneryForPlayer(0);
        func_80070734(ScreenFrame);
        return;
    }

    ScreenFrame = (u32)ScreenFrame + 1;
    if (((u32)RaceState - 2) < 2U) {
        if ((RaceTimeRemaining = (u32)RaceTimeRemaining - 1) <= 0) {
            RaceState = 5;
            PlayerCars[0].half1EE = 9;
            MenuFadeTimer = 37;
            RankEntries = (s32)(0U - (u32)TrackLength);
            SoundSetMasterVolume(0, 33);
        }
    }

    if (D_800AC6EC != 0 && RaceState == 0) {
        ScreenFrame = 75;
        ModeFrame = 75;
        SlotsStartForRace();
        RaceState = 1;
        PlayerCars[0].flags1E4 |= 0x100;
        ResetCarSlotBuffers();
        SetModelTexturePage(-1);
    } else if (RaceState == 1) {
        if ((u32)ModeFrame >= 176U) {
            func_80022CD4(PlayerCars);
            RaceState = 2;
            CdAudioBeginPlay(1);
        }
    } else if (RaceState >= 4) {
        s32 a2;
        if (MenuFadeTimer < 25) {
            if (CarTable[0]->half1EE <= ovl659_finish_rank_limits[SelectionState.phase17 >> 1]) {
                a2 = 37;
            } else {
                a2 = 69;
            }
            AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (25 - MenuFadeTimer) * 10, a2);
        }
        if (MenuFadeTimer <= 0) {
            SelectionState.pad18[1] = PlayerCars[0].half1EE;
            if (PlayerCars[0].half1EE <= ovl659_finish_rank_limits[SelectionState.phase17 / 2] &&
                RaceState == 4) {
                ModeGroup = 1;
                ModeId = 21;
                D_800F4D8A = 0;
            } else {
                FinishRace();
            }
        } else {
            MenuFadeTimer--;
        }
    }

    UpdateLapTiming(PlayerCars);
    if (RaceState < 4) {
        DrawStartCountdown(&CurrentDrawBuf->ordering.ordering70[0], ModeFrame);
        PlayStartCountdownVoice(ModeFrame);
    }
    UpdateRaceRanking();
    CheckFinalLapEvent((char *)PlayerCars);
    if (((u32)RaceState - 1) < 3U) {
        DrawLapTimeList(0);
        DrawTimeRemaining(RaceTimeRemaining);
        DrawHudRotatingQuad(EngineRevDisplay.first, 0);
        func_800211A4(0);
        DrawHudNumber3((PlayerCars[0].input * 160) / 1168, 0);
        DrawHudDigitMod10(PlayerCars[0].half27A, 0);
        UpdateHudRankDisplay(PlayerCars[0].half1EE, 0);
        DrawRaceMinimap(0);
        DrawSteeringIndicator(&CurrentDrawBuf->ordering.ordering70[0], 0);
        func_80021C00(0);
        SubmitRaceHudPackets(RaceType);
    }
    StepRaceFrame(RaceType);
    if (RaceState == 0) {
        func_80045868(PlayerCars);
    } else if (RaceState == 7) {
        func_80033808(&PlayerCars[1]);
    } else {
        UpdateRaceChaseCamera((struct w3g2p *)(void *)PlayerCars);
    }
    UpdateTrackObjects(0);
    BuildCameraMatrices();
    DrawCars(0);
    if (PlayerCars[0].half1EA != ReverseCourse) {
        func_800225DC(0);
    }
    RR_659_SCRATCH->word78 = D_800F2A64;
    SetupPlayerView(0);
    DrawTrackObjects(0);
    DrawTrackSceneryForPlayer(0);
    func_80070734(ScreenFrame);
    if (RaceState == 0) {
        RR_659_SCRATCH->pointers.cursor = AddDrawAreaPrim(
            CurrentDrawBuf->ordering.ordering70 + 2, RR_659_SCRATCH->pointers.cursor, 0, 0, 320, 256);
        FrameEnvs[1].environment.rgb24 = 1;
        FrameEnvs[0].environment.rgb24 = 1;
        RR_659_SCRATCH->pointers.cursor =
            AddDrawAreaPrim(CurrentDrawBuf->ordering.ordering70 + 702, RR_659_SCRATCH->pointers.cursor, 0,
                          RaceIntroWipeY, 320, (s32)(256U - (u32)RaceIntroWipeY * 2U));
        if ((RaceIntroWipeY = (s32)((u32)RaceIntroWipeY - 16U)) < 0) {
            RaceIntroWipeY = 0;
            FrameEnvs[1].environment.rgb24 = 0;
            FrameEnvs[0].environment.rgb24 = 0;
        }
    }
    SlotSetLevelProgram(EngineRevDisplay.first, D_800F2AB4.first, 0);
    if (RaceState >= 4) {
        StopRaceRumble();
        if (RaceState == 6) {
            StopRaceAudioAndRumble();
        }
    }
}
