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

extern s16 D_80107178;
extern s32 D_80107190;
extern char *CurrentDrawBuf;
extern u32 ModeFrame;
extern s32 RaceState;
extern s32 TotalRaceTime;
extern s16 MenuFadeTimer;
extern s16 NumCars;
extern s32 RaceTimeRemaining;
extern s32 ScreenFrame;
extern s16 D_800AD508;
extern u16 Pad0Buttons;
extern s8 RacePaused;
extern s8 PauseCursor;
extern s16 RaceType;
extern s16 D_800F4C70[];
extern s16 D_800F2AB4[];
extern u16 D_8010C0C8[];
extern Ovl667Vehicle PlayerCars[];
extern Ovl667Vehicle *CarTable[];
extern Ovl667FrameStorage FrameEnvs[2];

extern s32 ovl667_D_80118ED4;
extern s32 ovl667_D_80118ECC;
extern u8 ovl667_D_80118F8D, ovl667_D_80118F8C;
extern u8 ovl667_D_80118EEC, ovl667_D_80118EED, ovl667_D_80118EEE;
extern Ovl667Vector16 ovl667_D_80118E88[];
extern Ovl667Vector16 ovl667_D_80118EA8[];

extern void SetModelTexturePage();
extern char *AddDrawAreaPrim();
extern char *AddTilePrim();
extern void AddScreenFadeTiles();
extern void SubmitRaceHudPackets();
extern void func_800211A4();
extern void DrawHudRotatingQuad();
extern void DrawHudNumber3();
extern void DrawHudNumber5();
extern void DrawHudDigitMod10();
extern void UpdateHudRankDisplay();
extern void func_80021C00();
extern void DrawLapCounter();
extern void DrawRaceMinimap();
extern void DrawSteeringIndicator();
extern void DrawSplitScreenDivider();
extern void func_800225DC();
extern void func_80022CD4();
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
extern void func_80070734();
extern void UpdateTrackObjects();
extern void DrawTrackObjects();
extern void SetDispMask();
extern void _sio_control();
extern void ovl667_func_80114A40(void);
extern void ovl667_func_80115068(void);
extern void ovl667_func_801151E0(void);
extern void ovl667_func_801153F0(void);
extern void ovl667_func_80115508();

#define SCRATCH_CURSOR (*(char **)0x1F800000)

void ovl667_func_80115640(void) {
    s32 split;
    char *frame;
    s32 fade;
    s32 i;
    s32 n;
    s32 state;
    u16 buttons;

    split = (D_80107178 != 2) << 1;
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
    if (ovl667_D_80118ED4 == 2) {
        ovl667_func_80114A40();
        ovl667_D_80118ED4 = 3;
    } else if (ovl667_D_80118ED4 == 3) {
        _sio_control(1, 1, 3);
    }
    ovl667_func_80115068();

    if (ModeFrame >= 75 && RaceState == 0) {
        RaceState = 1;
        SlotsStartForRace();
    } else if (RaceState == 1) {
        if (ModeFrame >= 176) {
            for (i = 0; i < 4; i++) {
                func_80022CD4(CarTable[i]);
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
            if (ovl667_D_80118ED4 != 3) {
                ovl667_func_80114A40();
            }
            for (i = 0; i < NumCars; i++) {
                D_8010C0C8[i] = CarTable[i]->rank1EE;
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
        ovl667_func_80115508(CarTable[i]);
    }
    UpdateRaceRanking();

    if (ovl667_D_80118F8D != ovl667_D_80118F8C) {
        buttons = Pad0Buttons;
        ovl667_D_80118EEC = (buttons & 0x800) && MenuFadeTimer == 0;
        ovl667_D_80118EEC |= (buttons & 0x1000) ? 2 : 0;
        ovl667_D_80118EEC |= (buttons & 0x4000) ? 4 : 0;
        state = RaceState;
        if ((u32)(state - 1) < 3) {
            if ((ovl667_D_80118EEE | ovl667_D_80118EED) & 1) {
                RacePaused ^= 1;
                ovl667_D_80118ECC = ovl667_D_80118F8D;
                if (RacePaused == 0) {
                    MenuFadeTimer = 10;
                    SoundResume();
                    ApplyPauseMenuChoice(RaceType);
                    if (PauseCursor != 0) {
                        ovl667_func_80114A40();
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
            if ((ovl667_D_80118EEE | ovl667_D_80118EED) & 2) {
                if (PauseCursor > 0) {
                    PlaySfx(0);
                    PauseCursor--;
                }
            }
            if ((ovl667_D_80118EEE | ovl667_D_80118EED) & 4) {
                if (PauseCursor < 2) {
                    PlaySfx(0);
                    PauseCursor++;
                }
            }
            DrawPauseMenu(frame, PauseCursor);
            InitPlayerRumble();
        }
        ovl667_D_80118F8C = ovl667_D_80118F8D;
    } else {
        ovl667_D_80118EEC = 0;
    }

    n = 1;
    ovl667_func_801151E0();
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
                DrawHudRotatingQuad(D_800F4C70[i], i);
                DrawRaceMinimap(0);
                DrawLapCounter(0, PlayerCars[i].lap2AA);
                DrawSteeringIndicator(frame, 0);
            } else {
                DrawHudNumber5(D_800F4C70[i], i);
                DrawRaceMinimap(i + 1);
                DrawLapCounter(i + 1, PlayerCars[i].lap2AA);
                DrawSteeringIndicator(frame, i + 1);
            }
            DrawHudNumber3((PlayerCars[i].speed1D8 * 160) / 1168, i);
            DrawHudDigitMod10(PlayerCars[i].half27A, i);
            UpdateHudRankDisplay((s16)PlayerCars[i].rank1EE, i);
            DrawTimeRemainingForPlayer(RaceTimeRemaining, i);
            func_80021C00(i + 1);
            if (i == 1) {
                DrawSplitScreenDivider(0);
            }
        }
        func_800211A4(split);
        SubmitRaceHudPackets(3);
    }

    for (i = 0; i < n; i++) {
        if (PlayerCars[i].active1E8 != -1) {
            *(Ovl667Vector16 *)0x1F800018 = ovl667_D_80118E88[i];
            *(Ovl667Vector16 *)0x1F800008 = ovl667_D_80118EA8[i];
            SetupViewport(split + i);
            if (RaceState == 0 && ModeFrame < 75) {
                UpdateRaceIntroCamera(&PlayerCars[i], ModeFrame);
            } else {
                UpdateRaceChaseCamera(&PlayerCars[i]);
            }
            ovl667_D_80118E88[i] = *(Ovl667Vector16 *)0x1F800018;
            ovl667_D_80118EA8[i] = *(Ovl667Vector16 *)0x1F800008;
            BuildCameraMatrices();
            DrawCars(i + ((s16)(D_800AD508 ^ 1) + 1) * 2);
            if (PlayerCars[i].flag1EA != 0 && RacePaused == 0) {
                func_800225DC(n == 1 ? 0 : i + 1);
            }
            SetupPlayerView(i);
            UpdateTrackObjects(0);
            DrawTrackObjects(0);
            DrawTrackSceneryForPlayer(i + 1);
            if (n == 1) {
                func_80070734(ScreenFrame);
            }
            if (RaceState == 0 && n == 1) {
                FrameEnvs[1].enable18 = n;
                FrameEnvs[0].enable18 = n;
                SCRATCH_CURSOR = AddDrawAreaPrim(frame + 0xAF8, SCRATCH_CURSOR, 0, D_80107190, 320,
                                               256 - (D_80107190 << n));
                if (i == 0) {
                    if ((D_80107190 -= 16) < 0) {
                        D_80107190 = 0;
                        FrameEnvs[1].enable18 = 0;
                        FrameEnvs[0].enable18 = 0;
                    }
                }
            }
            SlotSetLevelProgram(D_800F4C70[i], D_800F2AB4[i], i);
            if (i == 0) {
                ovl667_func_801151E0();
            }
        }
    }

    if (RaceState >= 4) {
        StopRaceRumble();
        if (RaceState == 6) {
            StopRaceAudioAndRumble();
        }
    }
    if (ovl667_D_80118ED4 == 0) {
        ovl667_func_801153F0();
    }
}
