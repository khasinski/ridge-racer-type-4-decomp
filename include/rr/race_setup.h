#ifndef RR_RACE_SETUP_H
#define RR_RACE_SETUP_H
#include "rr/race_types.h"
#include "rr/standings.h"
void SetupRenderer(s32);
void SetModelTexturePage(s32);
void InitRaceHudPackets(s32);
void InitHudNumberTargets(s32);
void func_80045830(void);
void ApplyOptionsPadButtonMaps(void);
void PadRumbleStart(s32);
void StopAllSoundAndReset(void);
void SelectCourseBgmTrack(void);
void BuildCameraMatrices(void);
void ResetRearViewMirror(void);
void ClearTrackObjectStates(s32);
void DrawSync(s32);
s32 func_80114988(void);
void func_80114AF8(unsigned long);
extern s32 ScreenFrame;
extern s32 RaceVoiceFlags;
extern s16 D_800ABEF0;
extern s32 D_800AC538;
extern s32 RaceTimeRemaining;
extern s16 MenuFadeTimer;
extern s32 VSyncThreshold;
extern s32 DrawSubOtFlag;
extern s16 D_800AD50C;
extern s16 ReverseCourse;
extern s32 SavedRaceState;
extern s32 ModeFrame;
extern s32 TrackLength;
extern s8 PauseCursor;
extern s16 CourseIndex;
extern s32 NumLaps;
extern s8 RacePaused;
extern s32 BestLapTime;
extern s16 RaceType;
extern s32 RaceState;
extern s32 TrackEventSfxEnabled;
extern s16 PlayerCount;
extern s32 D_80107190;
extern s32 NextSplitIndex;
extern s16 LapAnnouncePending;
extern s32 D_8010C13C;
#include "rr/race_setup_types.h"
#include "game/camera/viewini.h"
#include "game/race/raceinit.h"
extern RrRaceSetupPair4 D_800F4C70,D_800F2AB4;
extern RrRaceVehicle PlayerCars[2];
extern s32 BestSplitTimes[3],SplitTimes[3],SplitDistances[3];
extern s32 CourseBestLapTimes[48],CourseBestLap[16],CourseBestLapRecords[8];
extern u8 CourseLapCounts[16];
extern s32 LapFrameCounts[10];

/* The vector owner is flat48 words, logically [2][8][3]. The scalar and
 * participant tables are complete16-element owners, logically [2][8].
 * Course/group must remain0..7/0..1 whenever tables are selected. All signed16
 * mode values are supported. Helpers may change later-read globals, tables,
 * current/future vehicle flags and output state; both whole vehicles stay live.
 * The 8011 hooks are the overlay entry points used when entering mode4. */
void SetupRace();
RR_ASSERT_SIZE(RrRaceSetupPair4,4);
RR_ASSERT_OFFSET(RrRaceSetupPair4,second,2);
#endif
