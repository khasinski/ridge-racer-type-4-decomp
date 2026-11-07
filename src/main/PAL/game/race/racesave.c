/* RODATA 0x80010420 0x20 */
#include "scratchpad.h"
#include "game/gfx/scene1.h"
#include "game/sound/sndcmd.h"
#include "game/ui/hudmap2.h"
#include "game/gfx/gfxdraw.h"
#include "game/sound/sndvab.h"
#include "game/system/pad.h"
#include "game/system/pad3.h"
#include "game/race/racesave.h"
#include "game/gfx/drawbuf.h"
#include "game/data/screenframe.h"
typedef struct { int w[200]; } B800;

typedef struct {
    int x;
} __attribute__((packed)) U4;

typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned short h2;
    unsigned char pad4[4];
    unsigned short h8;
    unsigned char padA[6];
    int w10;
    unsigned char pad14[0xE];
    short s22;
    short s24;
    short s26;
    unsigned char pad28[0x14];
    short s3C;
    unsigned char pad3E[0x1E];
} S5C;

typedef struct {
    short h0;
    short h2;
    short h4;
    short h6;
    short h8;
    short hA;
    short hC;
    short hE;
    short h10;
    short h12;
    short h14;
    short h16;
    short h18;
    short h1A;
    short h1C;
    short h1E;
    short h20;
    short h22;
    short h24;
    short h26;
    short h28;
    short h2A;
    short h2C;
    short h2E;
} S30;

typedef struct {
    short h0;
    short s12;
    short s14;
    short pad[5];
} S10_sub;

typedef struct {
    char pad[0x10];
    S10_sub sub[2];
} S_AD528;

typedef struct {
    char pad[0xB0];
    int sB0;
} SA0;

extern int ReplayState;
extern int SavedRaceState;
extern int RaceState;
extern short NumCars;
extern B800 *CarSnapshots[];
extern short SteeringRangeTable[];
extern int ReplayFrame;
extern int LinkRemoteInputs[];
extern int D_800ACBF0[];
extern short LinkRoleSlot;
extern U4 *ReplayInputs[];
extern S_AD528 Options;
extern unsigned char CarInputs[][4];
extern S30 PadButtonMap[];
extern S5C PlayerControllers[];
extern int LinkLocalInputs[];
extern short RaceType;
extern int ReplayLength;
extern short MenuFadeTimer;
extern int TrackLength;
extern int NumLaps;
extern int MenuConfirmPressed;
extern int SoundSetMasterVolume();
extern int AddScreenFadeTiles();
extern int InitReplayBuffers();
extern int ClearTrackObjectStates();
extern int SetModelTexturePage();
extern int PlayerCars[];
extern short ModeId;
extern short ModeGroup;
extern short ModeSubState;
extern int ReplayFocusCarIndex;
extern int AddSpriteWithDrawMode();
extern int DrawCars();
extern int FinishRace();
extern int DrawTrackSceneryForPlayer();
extern int StepRaceFrame();
extern int SlotSetLevelProgram();
extern int CdAudioBeginPlay();
extern int BuildCameraMatrices();
extern void SetupPlayerView(s32);
extern int SetupViewport();
extern int UpdateTrackObjects();
extern int DrawTrackObjects();
extern int rsin();
extern int D_800F2A64;
extern short EngineThrottleOn[];
extern int ModeFrame;
extern short EngineRevDisplay[];
extern struct { char pad[0x1E4]; short f1e4; } *CarTable[];

void SaveCarSnapshots(int unused);
void ReadCarInput(char *arg0, int arg1);
void CheckRaceFinish();
void ReplayModeEnter();
void ReplayModeUpdate(void);

/* legacy typing of shared globals in this part of the unit */
#define CarTable (*(B800 *(*)[])&CarTable)

/* Callers explicitly supply this legacy argument; the body ignores it. */
void SaveCarSnapshots(int unused)
{
  int i;
  if (ReplayState == 0)
  {
    SavedRaceState = RaceState;
    for (i = 0; i < NumCars; i++)
      *CarSnapshots[i] = *CarTable[i];
    ReplayState = 1;
  }
}

#undef CarTable

void ReadCarInput(char *arg0, int arg1) {
    SA0 *a0;
    short t3;
    short t5;
    short t1;
    short a1;
    register int t2 asm("t2"); /* ASM_BARRIER: pin t2 for idx copy */
    int idx;
    int b;
    int f;
    int mode;

    a0 = (SA0 *)(arg0 + 0x1F4);
    if (RaceState != 0) {
        if (ReplayState == 2) {
            if (ReplayFrame < ReplayLength) {
                *(U4 *)CarInputs[arg1] = ReplayInputs[arg1][ReplayFrame];
                if (RaceType == 4) {
                    if (arg1 == 3) {
                        ReplayFrame += 1;
                    }
                } else if (arg1 == 1 || RaceType < 2 || RaceType == 6) {
                    ReplayFrame += 1;
                }
            }
        } else {
            t3 = 0;
            t2 = idx = arg1 & 1;
            if (RaceState < 4) {
                mode = PlayerControllers[idx].w10;
                if (mode == 0) {
                    t1 = 0;
                    if (PlayerControllers[idx].h2 & PadButtonMap[idx].h4) {
                        t1 = 0xFF;
                    }
                    b = (PlayerControllers[idx].h2 & PadButtonMap[idx].h6) != 0;
                    {
                        register int f1 asm("v1"); /* ASM_BARRIER: pin f1 to v1 */
                        f1 = (PlayerControllers[idx].h2 & PadButtonMap[idx].h0) != 0;
                        if ((PlayerControllers[idx].h2 & PadButtonMap[idx].h2) > 0) {
                            f1 |= 2;
                        }
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].h8) > 0) {
                            f1 |= 4;
                        }
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].hA) > 0) {
                            f1 |= 8;
                        }
                        f = f1;
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].hE) > 0) {
                            f |= 0x10;
                        }
                    }
                    if (PlayerControllers[idx].b1 == 0x73) {
                        t5 = (PlayerControllers[idx].s3C * 0x7F) / 128;
                        t3 = f | 0x80;
                    } else {
                        t5 = 0;
                        t3 = f | 0x20;
                    }
                    a0->sB0 += b * 4;
                    if (b == 0) {
                        a0->sB0 = 0;
                    }
                    if (a0->sB0 >= 0x74) {
                        a0->sB0 = 0x73;
                    }
                    a1 = a0->sB0 + (b * 140);
                } else if (mode == 1) {
                    t5 = (PlayerControllers[idx].s3C * 0x7F) / SteeringRangeTable[Options.sub[idx].s12];
                    t1 = 0;
                    if (PlayerControllers[idx].h2 & PadButtonMap[idx].h14) {
                        t1 = 0xFF;
                    }
                    a1 = 0;
                    if (PlayerControllers[idx].h2 & PadButtonMap[idx].h16) {
                        a1 = 0xFF;
                    }
                    {
                        int fm1;
                        fm1 = ((PlayerControllers[idx].h8 & PadButtonMap[idx].h18) != 0) * 4;
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].h1A) > 0) {
                            fm1 |= 8;
                        }
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].h1E) > 0) {
                            fm1 |= 0x10;
                        }
                        t3 = fm1 | 0x40;
                    }
                    switch (Options.sub[idx].s14) {
                    case 0:
                    case 5: {
                        int i0;
                        t1 = (PlayerControllers[({ i0 = t2; __asm__("" : "+r"(i0)); i0; })].s22 * 0xFF) / 106; /* ASM_BARRIER: keep index copy */
                        a1 = (PlayerControllers[i0].s24 * 0xFF) / 106;
                        break;
                    }
                    case 1:
                    case 6: {
                        int i1;
                        t1 = (PlayerControllers[({ i1 = t2; __asm__("" : "+r"(i1)); i1; })].s24 * 0xFF) / 106; /* ASM_BARRIER: keep index copy */
                        a1 = (PlayerControllers[i1].s22 * 0xFF) / 106;
                        break;
                    }
                    case 2: {
                        int i2;
                        a1 = (PlayerControllers[({ i2 = t2; __asm__("" : "+r"(i2)); i2; })].s26 * 0xFF) / 106; /* ASM_BARRIER: keep index copy */
                        break;
                    }
                    case 3: {
                        int i3;
                        t1 = (PlayerControllers[({ i3 = t2; __asm__("" : "+r"(i3)); i3; })].s24 * 0xFF) / 106; /* ASM_BARRIER: keep index copy */
                        a1 = (PlayerControllers[i3].s26 * 0xFF) / 106;
                        break;
                    }
                    case 4:
                    case 7:
                        break;
                    }
                } else if (mode == 2) {
                    t5 = (PlayerControllers[idx].s3C * 0x7F) / 128;
                    t1 = 0;
                    if (PlayerControllers[idx].h2 & PadButtonMap[idx].h24) {
                        t1 = 0xFF;
                    }
                    b = (PlayerControllers[idx].h2 & PadButtonMap[idx].h26) != 0;
                    a0->sB0 += b * 4;
                    if (b == 0) {
                        a0->sB0 = 0;
                    }
                    if (a0->sB0 >= 0x74) {
                        a0->sB0 = 0x73;
                    }
                    a1 = a0->sB0 + (b * 140);
                    {
                        register unsigned short u2 asm("a0"); /* ASM_BARRIER: pin h2 load to a0 */
                        register int tand asm("v1"); /* ASM_BARRIER: pin h20 mask to v1 */
                        int fb, fc;
                        u2 = PlayerControllers[idx].h2;
                        tand = u2 & PadButtonMap[idx].h20;
                        f = tand != 0;
                        if ((u2 & PadButtonMap[idx].h22) > 0) {
                            f |= 2;
                        }
                        fb = f;
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].h28) > 0) {
                            fb |= 4;
                        }
                        fc = fb;
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].h2A) > 0) {
                            fc |= 8;
                        }
                        if ((PlayerControllers[idx].h8 & PadButtonMap[idx].h2E) > 0) {
                            fc |= 0x10;
                        }
                        t3 = fc | 0x60;
                    }
                } else {
                    a1 = 0;
                    t1 = 0;
                }
            } else {
                t1 = 0;
                a1 = 0;
            }
            CarInputs[arg1][0] = t5;
            CarInputs[arg1][1] = t1;
            CarInputs[arg1][2] = a1;
            CarInputs[arg1][3] = t3;
            if (RaceType == 4) {
                if (LinkRoleSlot != 0) {
                    if (arg1 >= 2) {
                        *(int *)CarInputs[arg1] = LinkRemoteInputs[t2];
                    } else {
                        LinkLocalInputs[t2] = *(int *)CarInputs[arg1];
                        *(int *)CarInputs[arg1] = D_800ACBF0[t2];
                    }
                } else {
                    if (arg1 < 2) {
                        *(int *)CarInputs[arg1] = LinkRemoteInputs[t2];
                    } else {
                        LinkLocalInputs[t2] = *(int *)CarInputs[arg1];
                        *(int *)CarInputs[arg1] = D_800ACBF0[t2];
                    }
                }
                __asm__ volatile("" : : "r"(t2)); /* ASM_BARRIER: keep t2 live so t2<<2 allocates v1 */
                if (ReplayState == 1) {
                    if (ReplayFrame < ReplayLength) {
                        ReplayInputs[arg1][ReplayFrame] = *(U4 *)CarInputs[arg1];
                        if (arg1 == 3) {
                            ReplayFrame += 1;
                        }
                    }
                }
            } else if (ReplayState == 1) {
                if (ReplayFrame < ReplayLength) {
                    ReplayInputs[arg1][ReplayFrame] = *(U4 *)CarInputs[arg1];
                    if (arg1 == 1 || RaceType < 2 || RaceType == 6) {
                        ReplayFrame += 1;
                    }
                }
            }
        }
    }
}

void CheckRaceFinish(a0) char *a0; {
    char *a2 = a0 + 0x1F4;
    int a1 = *(short *)(a2 + 0xB6);
    if (*(int *)(a0 + 0x188) + *(int *)(a0 + 0x184) >= a1 * TrackLength) {
        unsigned short u = *(unsigned short *)(a2 + 0xB6);
        if (NumLaps >= a1) {
            *(short *)(a2 + 0xB6) = u + 1;
            if (NumLaps < (short)(u + 1)) {
                if (RaceState < 4) {
                    if (MenuFadeTimer == -1) {
                        RaceState = 4;
                        MenuFadeTimer = 0x4B;
                        SoundSetMasterVolume(0, 0x47);
                    }
                }
            }
        }
    }
    if (ReplayFrame >= ReplayLength - 0x4B && MenuFadeTimer == -1) {
        MenuFadeTimer = 0x25;
        SoundSetMasterVolume(0, 0x21);
    } else if (MenuConfirmPressed != 0) {
        if (MenuFadeTimer == -1) {
            MenuFadeTimer = 0x25;
            SoundSetMasterVolume(0, 0x21);
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define RaceType (*(unsigned short (*))&RaceType)

void ReplayModeEnter() {
    SoundInitForRace();
    SelectReplayBgmTrack();
    AddScreenFadeTiles(CurrentDrawBuf + 0x70, 0xFF, 0x25);
    InitReplayBuffers(0, 2);
    RaceState = SavedRaceState;
    ClearTrackObjectStates(1);
    SetModelTexturePage(10);
    ReplayFocusCarIndex = 0;
    if ((unsigned)(RaceType - 2) < 2U) {
        if (PlayerCars[0x184 / 4] < PlayerCars[0x4A4 / 4]) ReplayFocusCarIndex = 1;
    }
    MenuFadeTimer = -1;
    ModeGroup = 1;
    ModeId = 0x16;
    ModeSubState = 0;
}

#undef RaceType

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(struct { char pad[0x184]; int f184; char pad2[0x320 - 0x188]; } (*)[])&PlayerCars)

void ReplayModeUpdate(void) {
    int i;
    short t;
    struct { char pad[0x1E4]; short f1e4; } **pp;
    ModeFrame += 1;
    if (ReplayFrame < 0x1A) {
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x19 - ReplayFrame) * 10, 0x25);
        if (ReplayFrame == 10) {
            CdAudioBeginPlay(0);
        }
    } else {
        func_800556B4();
    }
    ScreenFrame += 1;
    if (MenuFadeTimer >= 0) {
        if (MenuFadeTimer == 0) {
            SetReverbDepthBoth(0);
            FinishRace();
        }
        t = (unsigned short)MenuFadeTimer - 1;
        MenuFadeTimer = t;
        if (t < 0x19) {
            AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x19 - t) * 10, 0x45);
        }
    }
    if (rsin((ReplayFrame << 7) & 0xF80) > 0) {
        { int tt = *(int *)SCRATCHPAD; *(int *)SCRATCHPAD = AddSpriteWithDrawMode(CurrentDrawBuf + 0x70, tt, 0x10, 0x10, 0x4C, 0xC, 0x64, 0xC0, 5, 0x7FF8, 0, 0x80); }
    }
    for (i = 0; i < NumCars; i++) {
        if (CarTable[i]->f1e4 >= 0x10) {
            CheckRaceFinish(CarTable[i]);
        }
    }
    StepRaceFrame(RaceType);
    if ((unsigned)((unsigned short)RaceType - 2) < 2U) {
        if ((unsigned char)ReplayFrame == 0) {
            if (PlayerCars[0].f184 < PlayerCars[1].f184) {
                ReplayFocusCarIndex = 1;
            } else {
                ReplayFocusCarIndex = 0;
            }
        }
    }
    SetupViewport(0);
    UpdateRaceCamera(2, &PlayerCars[ReplayFocusCarIndex]);
    UpdateTrackObjects(0);
    BuildCameraMatrices();
    DrawCars(6);
    *(int *)(SCRATCHPAD + 0x78) = D_800F2A64;
    SetupPlayerView(0);
    DrawTrackObjects(0);
    DrawTrackSceneryForPlayer(0);
    if (ReplayFrame >= 0x19) {
        QueueDrawStp();
    }
    SlotSetLevelProgram(EngineRevDisplay[0], EngineThrottleOn[0], 0);
    SlotSetLevelProgram(EngineRevDisplay[1], EngineThrottleOn[1], 1);
    PadClearPulse(0);
    PadClearPulse(1);
    PadRumbleStop(0);
    PadRumbleStop(1);
}

#undef PlayerCars
