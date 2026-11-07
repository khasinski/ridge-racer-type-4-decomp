#include "rr/overlay666.h"

void extra_course_select_update_checkpoint(RrRaceCheckpointVehicle *vehicle) {
    RrRaceVehicleControl *control;
    /* Preserve the target through the mode-dependent adjustment. */
    s32 target;
    s32 id;

    control = &vehicle->control;
    if ((s32)((u32)vehicle->second + (u32)vehicle->first) >=
        (s32)((u32)(s32)control->steering94.count22 * (u32)TrackLength)) {
        if (control->steering94.count22 <= NumLaps) {
            control->steering94.count22 = (s16)((u16)control->steering94.count22 + 1U);
            LapAnnouncePending = 0;
            target = ((u16)(CourseIndex - 6) < 2) ? (s32)((u32)NumLaps - 1U) : NumLaps;
            if (control->steering94.count22 == target) {
                SaveCarSnapshots(0);
            } else if (control->steering94.count22 > NumLaps) {
                if (RaceState < 4) {
                    LapAnnouncePending = -1;
                    RaceState = 4;
                    MenuFadeTimer = 75;
                    SoundSetMasterVolume(0, 33);
                    TotalRaceTime = 0;
                    SoundQueueEnqueue(19, 1);
                }
            }
        }
    }
    if (LapAnnouncePending == 0) {
        id = (s32)(9U - ((u32)NumLaps - (u32)(s32)control->steering94.count22));
        if (id == 9) {
            id = 15;
        }
        if (id >= 6) {
            SoundQueueEnqueue(id, 0);
        }
        LapAnnouncePending = -1;
    }
}

void extra_course_select_exit_race(void) {
    StopRaceAudioAndRumble();
    SetupRaceAndScene();
    ModeGroup = 1;
    ModeId = 82;
    ModeSubState = 0;
    AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, 255, 69);
}

void extra_course_select_update_race(void) {
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
        DrawPauseMenu(&CurrentDrawBuf->ordering.ordering70[0], PauseCursor);
        if (RaceState < 4) {
            DrawStartCountdown(&CurrentDrawBuf->ordering.ordering70[0], ModeFrame);
        }
        if (((u32)RaceState - 1) < 3U) {
            DrawHudRotatingQuad(EngineRevDisplay.first, 0);
            DrawHudNumber3((PlayerCars[0].input * 160) / 1168, 0);
            DrawHudDigitMod10(PlayerCars[0].half27A, 0);
            UpdateHudRankDisplay(PlayerCars[0].half1EE, 0);
            DrawTimeRemainingForPlayer(RaceTimeRemaining, 0);
            DrawLapCounter(0, PlayerCars[0].steering288.count22);
            DrawRaceMinimap(0);
            DrawSteeringIndicator(&CurrentDrawBuf->ordering.ordering70[0], 0);
            UpdateFinalLapIndicator(1);
            DrawHudFrameSprites(0);
            SubmitRaceHudPackets(4);
        }
        if (RaceState == 0 && ModeFrame < 75U) {
            UpdateRaceIntroCamera(PlayerCars, ModeFrame);
        } else {
            SetModelTexturePage(-1);
            UpdateRaceChaseCamera((struct w3g2p *)(void *)PlayerCars);
        }
        BuildCameraMatrices();
        DrawCars(0);
        SetupPlayerView(0);
        DrawTrackObjects(0);
        DrawTrackSceneryForPlayer(0);
        DrawRearViewMirror(ScreenFrame);
        return;
    }

    ScreenFrame = (u32)ScreenFrame + 1;
    if (((u32)RaceState - 2) < 2U) {
        if ((RaceTimeRemaining = (u32)RaceTimeRemaining - 1) <= 0) {
            RaceState = 5;
            MenuFadeTimer = 75;
            SoundSetMasterVolume(0, 33);
        }
    }

    if (ModeFrame >= 75U && RaceState == 0) {
        RaceState = 1;
        SlotsStartForRace();
        ResetCarSlotBuffers();
        SetModelTexturePage(-1);
    } else if (RaceState == 1) {
        if (ModeFrame >= 176U) {
            ApplyRaceStartLaunch(PlayerCars);
            RaceState = 2;
            CdAudioBeginPlay(1);
        }
    } else if (RaceState >= 4) {
        s32 a2;
        if (TotalRaceTime == -1 || PlayerCars[0].half1EE != 0) {
            a2 = 69;
        } else {
            a2 = 37;
        }
        if (MenuFadeTimer < 25) {
            AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (25 - MenuFadeTimer) * 10, a2);
            RR_666_SCRATCH->cursor = AddDrawAreaPrim(&CurrentDrawBuf->ordering.ordering70[0],
                                                   RR_666_SCRATCH->cursor, 0, 0, 320, 256);
        }
        if (MenuFadeTimer <= 0) {
            if (RaceState == 4 && (extra_course_select_result_kind = PlayerCars[0].half1EE) == 0) {
                ModeGroup = 1;
                ModeId = 21;
                ModeSubState = 0;
            } else {
                FinishRace();
            }
        } else {
            MenuFadeTimer--;
        }
    } else if (RaceState == 0) {
        if (ModeFrame >= 51U) {
            AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (ModeFrame - 50) * 10, 37);
            FrameEnvs[1].environment.rgb24 = 0;
            FrameEnvs[0].environment.rgb24 = 0;
        } else if (ModeFrame < 25U) {
            AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (25 - ModeFrame) * 10, 69);
            FrameEnvs[1].environment.rgb24 = 1;
            FrameEnvs[0].environment.rgb24 = 1;
        }
        RR_666_SCRATCH->cursor = AddDrawAreaPrim(&CurrentDrawBuf->ordering.ordering70[0],
                                               RR_666_SCRATCH->cursor, 0, 0, 320, 256);
    }

    extra_course_select_update_checkpoint((RrRaceCheckpointVehicle *)(void *)PlayerCars);
    UpdateRaceRanking();
    if (RaceState < 4) {
        DrawStartCountdown(&CurrentDrawBuf->ordering.ordering70[0], ModeFrame);
        PlayStartCountdownVoice(ModeFrame);
    }
    CheckFinalLapEvent((char *)PlayerCars);
    StepRaceFrame(RaceType);
    if (((u32)RaceState - 1) < 3U) {
        DrawHudRotatingQuad(EngineRevDisplay.first, 0);
        DrawHudNumber3((PlayerCars[0].input * 160) / 1168, 0);
        DrawHudDigitMod10(PlayerCars[0].half27A, 0);
        UpdateHudRankDisplay(PlayerCars[0].half1EE, 0);
        DrawTimeRemainingForPlayer(RaceTimeRemaining, 0);
        DrawLapCounter(0, PlayerCars[0].steering288.count22);
        DrawRaceMinimap(0);
        DrawSteeringIndicator(&CurrentDrawBuf->ordering.ordering70[0], 0);
        UpdateFinalLapIndicator(1);
        DrawHudFrameSprites(0);
        SubmitRaceHudPackets(4);
    }
    if (RaceState == 0 && ModeFrame < 75U) {
        UpdateRaceIntroCamera(PlayerCars, ModeFrame);
    } else {
        UpdateRaceChaseCamera((struct w3g2p *)(void *)PlayerCars);
    }
    BuildCameraMatrices();
    DrawCars(0);
    if (PlayerCars[0].half1EA != 0) {
        DrawWrongWaySprite(0);
    }
    SetupPlayerView(0);
    UpdateTrackObjects(0);
    DrawTrackObjects(0);
    DrawTrackSceneryForPlayer(0);
    DrawRearViewMirror(ScreenFrame);
    SlotSetLevelProgram(EngineRevDisplay.first, EngineThrottleOn.first, 0);
    if (RaceState >= 4) {
        StopRaceRumble();
        if (RaceState == 6) {
            StopRaceAudioAndRumble();
        }
    }
}
