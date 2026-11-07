#ifndef RR_VERSUS_SELECT_RACE_CONTROLLER_H
#define RR_VERSUS_SELECT_RACE_CONTROLLER_H
#include "rr/time_attack_course_select_race_controller.h"
#include "rr/race_checkpoint_types.h"
typedef struct {s32 x,y,z,w;} RrVersusSelectFrameVector16;
typedef struct {RrGpuScratchPointers pointers;RrVersusSelectFrameVector16 position08,rotation18;u8 reserved28[0x5E - 0x28];s16 mode5E;} RrVersusSelectFrameScratch;
#ifndef RR_VERSUS_SELECT_FRAME_SCRATCH
#define RR_VERSUS_SELECT_FRAME_SCRATCH ((RrVersusSelectFrameScratch *)0x1F800000U)
#endif
#ifndef RR_VERSUS_SELECT_FRAME_MODE
#define RR_VERSUS_SELECT_FRAME_MODE (*(s16 *)0x1F80005EU)
#endif
extern RrVersusSelectFrameVector16 versus_select_player_camera_angles[2],versus_select_player_camera_positions[2];
extern s32 TotalRaceTime;
extern s16 versus_select_finish_ranks[2];
void DrawHudNumber5(s32,s32);void UpdateHudRankDisplay(s32,s32);void DrawLapCounter(s32,s32);void UpdateFinalLapIndicator(s32);void DrawSplitScreenDivider(s32);void DrawTimeRemainingForPlayer(s32,s32);void SoundSetMasterVolume(s16,s16);
void versus_select_update_lap_progress(RrRaceCheckpointVehicle *);
/* The hardware view has two full CPU pointers followed by two four-word
 * vectors. Native builds grow the pointer prefix; vector and tail offsets
 * follow that prefix rather than retaining PAL pointer widths.
 * Services may replace frame roots and mutate both complete vector tables,
 * vehicle/input owners and the later-read counters. Frame/tick arithmetic
 * wraps32; rendering visits both players and publishes current scratch state. */
void versus_select_update_race(void);
RR_ASSERT_SIZE(RrVersusSelectFrameVector16,0x10);
RR_ASSERT_OFFSET(RrVersusSelectFrameScratch,pointers,0);
RR_ASSERT_OFFSET(RrVersusSelectFrameScratch,position08,sizeof(RrGpuScratchPointers));
RR_ASSERT_OFFSET(RrVersusSelectFrameScratch,rotation18,sizeof(RrGpuScratchPointers)+0x10);
RR_ASSERT_OFFSET(RrVersusSelectFrameScratch,mode5E,sizeof(RrGpuScratchPointers)+0x56);
RR_ASSERT_SIZE(RrVersusSelectFrameScratch,sizeof(RrGpuScratchPointers)+0x58);
#endif
