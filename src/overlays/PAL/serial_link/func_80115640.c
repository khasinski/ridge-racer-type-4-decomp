#include "types.h"

/* Versus (serial link) race frame: drives the shared race state machine,
 * exchanges pause/menu input through the serial packet bytes and renders
 * one or two viewports. */

typedef struct {
    u8 pad000[0x1D8];
    s16 speed1D8;
    u8 pad1DA[0x1E8 - 0x1DA];
    s16 active1E8;
    s16 flag1EA;
    u8 pad1EC[0x1EE - 0x1EC];
    u16 rank1EE;
    u8 pad1F0[0x27A - 0x1F0];
    s16 half27A;
    u8 pad27C[0x2AA - 0x27C];
    s16 lap2AA;
    u8 pad2AC[0x320 - 0x2AC];
} Ovl667Vehicle;

typedef struct {
    s32 words[4];
} Ovl667Vector16;

typedef struct {
    u32 pad00[6];
    u8 enable18;
    u8 pad19[0x22778 - 0x19];
} Ovl667FrameStorage;

extern s16 PlayerCount;
extern s32 RaceIntroWipeY;
extern char *CurrentDrawBuf;
extern u32 ModeFrame;
extern s32 RaceState;
extern s32 TotalRaceTime;
extern s16 MenuFadeTimer;
extern s16 NumCars;
extern s32 RaceTimeRemaining;
extern s32 ScreenFrame;
extern s16 LinkRoleSlot;
extern u16 Pad0Buttons;
extern s8 RacePaused;
extern s8 PauseCursor;
extern s16 RaceType;
extern s16 EngineRevDisplay[];
extern s16 EngineThrottleOn[];
extern u16 FinalCarRanks[];
extern Ovl667Vehicle PlayerCars[];
extern Ovl667Vehicle *CarTable[];
extern Ovl667FrameStorage FrameEnvs[2];

extern s32 serial_link_shutdown_state;
extern s32 serial_link_pause_sequence;
extern u8 serial_link_rx_cursor, serial_link_last_rx_cursor;
extern u8 serial_link_local_menu_buttons, serial_link_remote_menu_buttons, serial_link_sent_menu_buttons;
extern Ovl667Vector16 serial_link_player_camera_angles[];
extern Ovl667Vector16 serial_link_player_camera_positions[];

extern void SetModelTexturePage();
extern char *AddDrawAreaPrim();
extern char *AddTilePrim();
extern void AddScreenFadeTiles();
extern void SubmitRaceHudPackets();
extern void DrawHudFrameSprites();
extern void DrawHudRotatingQuad();
extern void DrawHudNumber3();
extern void DrawHudNumber5();
extern void DrawHudDigitMod10();
extern void UpdateHudRankDisplay();
extern void UpdateFinalLapIndicator();
extern void DrawLapCounter();
extern void DrawRaceMinimap();
extern void DrawSteeringIndicator();
extern void DrawSplitScreenDivider();
extern void DrawWrongWaySprite();
extern void ApplyRaceStartLaunch();
extern void DrawTimeRemainingForPlayer();
extern void DrawCars();
extern void UpdateRaceIntroCamera();
extern void UpdateRaceChaseCamera();
extern void DrawStartCountdown();
extern void PlayStartCountdownVoice();
extern void InitPlayerRumble();
extern void ApplyPauseMenuChoice();
extern void DrawPauseMenu();
extern void StopRaceRumble();
extern void StopRaceAudioAndRumble();
extern void FinishRace();
extern void DrawTrackSceneryForPlayer();
extern void StepRaceFrame();
extern void UpdateRaceRanking();
extern void PlaySfx();
extern void SoundPause();
extern void SoundResume();
extern void SoundSetMasterVolume();
extern void SlotsStartForRace();
extern void SlotSetLevelProgram();
extern void CdAudioBeginPlay();
extern void BuildCameraMatrices();
extern void SetupPlayerView();
extern void SetupViewport();
extern void DrawRearViewMirror();
extern void UpdateTrackObjects();
extern void DrawTrackObjects();
extern void SetDispMask();
extern void _sio_control();
extern void serial_link_close_sio(void);
extern void serial_link_update_receive(void);
extern void serial_link_update_send(void);
extern void serial_link_check_link_lost(void);
extern void serial_link_update_checkpoint();

#define SCRATCH_CURSOR (*(char **)0x1F800000)

void serial_link_update_race(void) {
    s32 split;
    char *frame;
    s32 fade;
    s32 i;
    s32 n;
    s32 state;
    u16 buttons;

    split = (PlayerCount != 2) << 1;
    if (split == 0) {
        frame = CurrentDrawBuf + 0x70;
    } else {
        *(s16 *)0x1F80005E = 1;
        frame = CurrentDrawBuf + 0xB70;
    }
    if (ModeFrame < 2) {
        SetDispMask(0);
    } else {
        SetDispMask(1);
    }
    SetModelTexturePage(-1);
    if (serial_link_shutdown_state == 2) {
        serial_link_close_sio();
        serial_link_shutdown_state = 3;
    } else if (serial_link_shutdown_state == 3) {
        _sio_control(1, 1, 3);
    }
    serial_link_update_receive();

    if (ModeFrame >= 75 && RaceState == 0) {
        RaceState = 1;
        SlotsStartForRace();
    } else if (RaceState == 1) {
        if (ModeFrame >= 176) {
            for (i = 0; i < 4; i++) {
                ApplyRaceStartLaunch(CarTable[i]);
            }
            RaceState = 2;
            CdAudioBeginPlay(1);
            MenuFadeTimer = 30;
        }
    } else if (RaceState >= 4) {
        fade = 37;
        if (TotalRaceTime == -1) {
            fade = 69;
        }
        if (MenuFadeTimer < 25) {
            AddScreenFadeTiles(frame, (25 - MenuFadeTimer) * 10, fade);
            SCRATCH_CURSOR = AddDrawAreaPrim(frame, SCRATCH_CURSOR, 0, 0, 320, 256);
        }
        if (MenuFadeTimer <= 0) {
            if (serial_link_shutdown_state != 3) {
                serial_link_close_sio();
            }
            for (i = 0; i < NumCars; i++) {
                FinalCarRanks[i] = CarTable[i]->rank1EE;
            }
            FinishRace();
            _sio_control(1, 1, 3);
        } else {
            MenuFadeTimer--;
        }
    } else if (ModeFrame >= 51 && RaceState == 0) {
        AddScreenFadeTiles(frame, (ModeFrame - 50) * 10, 37);
        SCRATCH_CURSOR = AddDrawAreaPrim(frame, SCRATCH_CURSOR, 0, 0, 320, 256);
    }

    for (i = 0; i < NumCars; i++) {
        serial_link_update_checkpoint(CarTable[i]);
    }
    UpdateRaceRanking();

    if (serial_link_rx_cursor != serial_link_last_rx_cursor) {
        buttons = Pad0Buttons;
        serial_link_local_menu_buttons = (buttons & 0x800) && MenuFadeTimer == 0;
        serial_link_local_menu_buttons |= (buttons & 0x1000) ? 2 : 0;
        serial_link_local_menu_buttons |= (buttons & 0x4000) ? 4 : 0;
        state = RaceState;
        if ((u32)(state - 1) < 3) {
            if ((serial_link_sent_menu_buttons | serial_link_remote_menu_buttons) & 1) {
                RacePaused ^= 1;
                serial_link_pause_sequence = serial_link_rx_cursor;
                if (RacePaused == 0) {
                    MenuFadeTimer = 10;
                    SoundResume();
                    ApplyPauseMenuChoice(RaceType);
                    if (PauseCursor != 0) {
                        serial_link_close_sio();
                        StopRaceAudioAndRumble();
                        return;
                    }
                } else {
                    SoundPause();
                }
                PauseCursor = 0;
            } else if (MenuFadeTimer > 0 && state < 4) {
                MenuFadeTimer--;
            }
        }
        if (RacePaused == 0) {
            ModeFrame++;
            ScreenFrame++;
            if ((u32)(RaceState - 2) < 2) {
                if (--RaceTimeRemaining <= 0) {
                    RaceState = 5;
                    MenuFadeTimer = 75;
                    SoundSetMasterVolume(0, 33);
                }
            }
            StepRaceFrame(4);
        } else {
            if ((serial_link_sent_menu_buttons | serial_link_remote_menu_buttons) & 2) {
                if (PauseCursor > 0) {
                    PlaySfx(0);
                    PauseCursor--;
                }
            }
            if ((serial_link_sent_menu_buttons | serial_link_remote_menu_buttons) & 4) {
                if (PauseCursor < 2) {
                    PlaySfx(0);
                    PauseCursor++;
                }
            }
            DrawPauseMenu(frame, PauseCursor);
            InitPlayerRumble();
        }
        serial_link_last_rx_cursor = serial_link_rx_cursor;
    } else {
        serial_link_local_menu_buttons = 0;
    }

    n = 1;
    serial_link_update_send();
    if (split != 0) {
        n = 2;
    }
    if (RaceState < 4) {
        DrawStartCountdown(frame, ModeFrame);
        if (RacePaused == 0) {
            PlayStartCountdownVoice(ModeFrame);
        }
    }
    if ((u32)(RaceState - 1) < 3) {
        for (i = 0; i < n; i++) {
            if (i == 1 && PlayerCars[i].active1E8 == -1) {
                SCRATCH_CURSOR = AddTilePrim(frame, SCRATCH_CURSOR, 0, 128, 320, 128, 8, 8, 8);
            }
            if (n == 1) {
                if (PlayerCars[i].active1E8 == -1) {
                    continue;
                }
                DrawHudRotatingQuad(EngineRevDisplay[i], i);
                DrawRaceMinimap(0);
                DrawLapCounter(0, PlayerCars[i].lap2AA);
                DrawSteeringIndicator(frame, 0);
            } else {
                DrawHudNumber5(EngineRevDisplay[i], i);
                DrawRaceMinimap(i + 1);
                DrawLapCounter(i + 1, PlayerCars[i].lap2AA);
                DrawSteeringIndicator(frame, i + 1);
            }
            DrawHudNumber3((PlayerCars[i].speed1D8 * 160) / 1168, i);
            DrawHudDigitMod10(PlayerCars[i].half27A, i);
            UpdateHudRankDisplay((s16)PlayerCars[i].rank1EE, i);
            DrawTimeRemainingForPlayer(RaceTimeRemaining, i);
            UpdateFinalLapIndicator(i + 1);
            if (i == 1) {
                DrawSplitScreenDivider(0);
            }
        }
        DrawHudFrameSprites(split);
        SubmitRaceHudPackets(3);
    }

    for (i = 0; i < n; i++) {
        if (PlayerCars[i].active1E8 != -1) {
            *(Ovl667Vector16 *)0x1F800018 = serial_link_player_camera_angles[i];
            *(Ovl667Vector16 *)0x1F800008 = serial_link_player_camera_positions[i];
            SetupViewport(split + i);
            if (RaceState == 0 && ModeFrame < 75) {
                UpdateRaceIntroCamera(&PlayerCars[i], ModeFrame);
            } else {
                UpdateRaceChaseCamera(&PlayerCars[i]);
            }
            serial_link_player_camera_angles[i] = *(Ovl667Vector16 *)0x1F800018;
            serial_link_player_camera_positions[i] = *(Ovl667Vector16 *)0x1F800008;
            BuildCameraMatrices();
            DrawCars(i + ((s16)(LinkRoleSlot ^ 1) + 1) * 2);
            if (PlayerCars[i].flag1EA != 0 && RacePaused == 0) {
                DrawWrongWaySprite(n == 1 ? 0 : i + 1);
            }
            SetupPlayerView(i);
            UpdateTrackObjects(0);
            DrawTrackObjects(0);
            DrawTrackSceneryForPlayer(i + 1);
            if (n == 1) {
                DrawRearViewMirror(ScreenFrame);
            }
            if (RaceState == 0 && n == 1) {
                FrameEnvs[1].enable18 = n;
                FrameEnvs[0].enable18 = n;
                SCRATCH_CURSOR = AddDrawAreaPrim(frame + 0xAF8, SCRATCH_CURSOR, 0, RaceIntroWipeY, 320,
                                               256 - (RaceIntroWipeY << n));
                if (i == 0) {
                    if ((RaceIntroWipeY -= 16) < 0) {
                        RaceIntroWipeY = 0;
                        FrameEnvs[1].enable18 = 0;
                        FrameEnvs[0].enable18 = 0;
                    }
                }
            }
            SlotSetLevelProgram(EngineRevDisplay[i], EngineThrottleOn[i], i);
            if (i == 0) {
                serial_link_update_send();
            }
        }
    }

    if (RaceState >= 4) {
        StopRaceRumble();
        if (RaceState == 6) {
            StopRaceAudioAndRumble();
        }
    }
    if (serial_link_shutdown_state == 0) {
        serial_link_check_link_lost();
    }
}
